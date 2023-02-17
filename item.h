#ifndef ITEM_H
#define ITEM_H

#include "MongoCore_global.h"

#include <iterator>
#include <iostream>
#include <optional>
#include <string>
#include <ctime>

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/view_or_value.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <bsoncxx/string/to_string.hpp>
#include <bsoncxx/json.hpp>


namespace MongoCore {

class MONGOCORE_EXPORT Item
{
public:
    explicit Item(const std::string &collection);   // Alındı
    Item(const Item& other);                        //
    Item( Item &&other );
    virtual ~Item();



    Item& operator=(const Item &value);
    Item& operator=( Item &&other );

    virtual void errorOccured(const std::string& errorText ) ;



    Item(const bsoncxx::document::view mView , const std::string &_Collection);
    Item& operator=(const bsoncxx::builder::basic::document &value);
    Item& operator=(const bsoncxx::document::view &view);
    MongoCore::Item& setDocumentView( const bsoncxx::document::view &view);
    bsoncxx::document::view view() const;


    std::optional<bsoncxx::types::bson_value::value> element(const std::string &key) const;
    std::optional<bsoncxx::oid> oid() const;
    std::optional<bsoncxx::types::bson_value::value> element(std::string key);
    std::optional<bsoncxx::oid> oid();
    std::optional<bsoncxx::builder::basic::document> ItemFilter();
    std::optional<std::time_t> getTime() const;




    void printView() const;

    void clear();

    Item& setOid( const std::string &oid );

    Item& setOid( const bsoncxx::oid& oid );

    std::string getCollection() const;

    void setCollection( const std::string& collection );



    void removeElement( const std::string &key );
    void removeElement( const std::string_view &key );



    template<typename T>
    void pushArray(std::string key ,const T &value){
        auto arr = bsoncxx::builder::basic::array{};
        auto existArray = this->element (key);

        if( existArray )
        {
            this->removeElement ( key );

            for( auto item : existArray->view ().get_array ().value )
            {
                try {
                    arr.append (item.get_value ());
                } catch (bsoncxx::exception &e) {
                    std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                    errorOccured (str);
                }
            }
        }

        try {
            arr.append (value);
        } catch (bsoncxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            errorOccured (str);
        }

        try {
            mDoc.append (kvp(key,arr));
        } catch (bsoncxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            errorOccured (str);
        }
    }


    void pushArray(std::string key ,const Item &value);




    void pullArray( const std::string& key , const bsoncxx::types::bson_value::value &value );

    template<typename T>
    Item& append( const std::string_view &key ,const T &value ){

        this->removeElement (key);
        try {
            mDoc.append (bsoncxx::builder::basic::kvp(key,value));
        } catch (bsoncxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            errorOccured (str);
        }
        return *this;
    }

    template<>
    Item& append( const std::string_view &key ,const Item &value );

private:
    bsoncxx::builder::basic::document mDoc;
    std::string mCollection;
};









class MONGOCORE_EXPORT FindOptions : public Item
{
public:
    explicit FindOptions();


    FindOptions& setLimit( const int &limit );
    FindOptions& setSkip( const int &skip );

    FindOptions& setSort( const Item& sortItem);
    FindOptions& setProjection( const Item& projItem);

    int limit() const;
    int skip() const;

    Item sort() const;
    Item projection() const;


};


}


#endif // ITEM_H
