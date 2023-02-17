#ifndef DB_H
#define DB_H


#include "MongoCore_global.h"

#include <mongocxx/database.hpp>
#include <mongocxx/exception/exception.hpp>

#include <optional>
#include <vector>
#include <list>
#include <iostream>
#include <string>
#include <filesystem>
#include <mutex>
#include <memory>

#include "item.h"


namespace MongoCore {

class Item;
class FindOptions;

class MONGOCORE_EXPORT DB
{
public:
    explicit DB();
//    DB( const DB &db);
//    DB( DB&& other );
    explicit DB( mongocxx::database* _db );
    explicit DB( DB* _db );
//    explicit DB( const DB* _db );
//    virtual ~DB();

//    DB& operator=(const DB& otherDB);
//    DB& operator=( DB&& otherDB );
//    DB& operator=( mongocxx::database* _db );

    static void instance( const std::string &mUri );





    const DB* getDB() const {
        return this;
    }

    DB* getDB(){
        return this;
    }


    virtual void errorOccured( const std::string& errorText );



    mongocxx::database* db();

    std::string downloadFile(const std::string &fileOid , bool forceFilename = false);
    std::string downloadFileWeb( const std::string &fileOid , bool forceFilename = false );
    bsoncxx::types::bson_value::value uploadfile(const std::string &filepath );
//    bsoncxx::types::bson_value::value uploadfile(std::string filepath ) const;
    bool deleteGridFS( const std::string &fileOid );

    mongocxx::stdx::optional<mongocxx::result::insert_one> insertItem(const Item &item);
    mongocxx::stdx::optional<mongocxx::result::update> updateItem( const Item &item);
    mongocxx::stdx::optional<bsoncxx::document::value> findOneItem(const Item &item );
    mongocxx::stdx::optional<bsoncxx::document::value> findOneItem(const Item &item , const Item &findOptions );
    mongocxx::stdx::optional<mongocxx::cursor> find( const Item &item , const mongocxx::options::find findOptions );
    mongocxx::stdx::optional<mongocxx::cursor> find( const Item &item , const int &limit = 20 , const int &skip = 0  );
    mongocxx::stdx::optional<mongocxx::cursor> find( const Item &item , const FindOptions& options  );


    template< typename V>
    mongocxx::stdx::optional<mongocxx::result::update> pushValue( const Item& filter , const std::string& key , const V& value )
    {
        Item item("none");
        item.append("$push",Item("none").append(key,value));
        try {
            auto upt = this->db ()->collection (filter.getCollection ()).update_one (filter.view (),item.view ());
            return upt;
        } catch (mongocxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            std::cout << str << std::endl;
            return mongocxx::stdx::nullopt;
        }
    }


    template< typename V>
    mongocxx::stdx::optional<mongocxx::result::update> pullValue( const Item& filter , const std::string& key , const V& value )
    {
        Item item("none");
        item.append("$pull",Item("none").append(key,value));
        try {
            auto upt = this->db ()->collection (filter.getCollection ()).update_one (filter.view (),item.view ());
            return upt;
        } catch (mongocxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            this->setLastError (str);
            return mongocxx::stdx::nullopt;
        }
    }

    template< typename V >
    inline bool setField( const Item& filter , const std::string &field , const V& value ){

        auto _oid = filter.oid ();

        if( !_oid )
        {
            this->setLastError ( std::to_string(__LINE__) + __FUNCTION__ + std::string("Required Object ID"));
            return false;
        }

        Item setDoc( filter.getCollection() );
        setDoc.append( "_id" , filter.oid().value());
        setDoc.append( field , value );
        auto result = DB::updateItem (setDoc);
        if( result )
        {
            if( result.value().modified_count() )
            {
                return true;
            }else{
                //TODO: QString to std::string
//                setLastError (QString("%1 Modified Document").arg (result.value ().modified_count ()));
                return false;
            }
        }else{
            setLastError (std::string("setField No Result"));
            return false;
        }
    }

    inline bool removeField( const Item& filter , const std::string &field );


    bool incValue( const Item& filter , const std::string& field , const std::int64_t& value );

    bool incValue( const Item& filter , const std::string& field , const std::int32_t& value );

    mongocxx::stdx::optional<mongocxx::result::delete_result> deleteItem( const Item &item );

    int64_t countItem( const Item &item );

    std::string getLastError();
    void setLastError(const std::string &lastError);

private:
    //    mongocxx::database _mDB;
    //    mongocxx::client* mClient;

    std::string mLastError;
    bool mConstructWithNewClient;
    mongocxx::database* mDB;
};





}


#endif // DB_H
