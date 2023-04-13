#ifndef LISTITEM_H
#define LISTITEM_H

#include "db.h"

#include <string>
#include <vector>

namespace MongoCore {



template <typename T>
class ListItem : public DB
{
public:
    ListItem(DB* db) : DB(db),__limit(20),__skip(0){}

    inline const T &itemAt(const std::string &byOid)
    {
         T *item = new T();
        for( auto _item : __mlist )
        {
            auto _oid = _item.oid();
            if( _oid )
            {
                if( byOid == _oid.value().to_string() )
                {
                    item->setDocumentView(_item.view());// _item;
                    break;
                }
            }else{
                errorOccured("Hata Oid");
            }
        }
        return std::move(*item);
    }

    inline std::vector<T>& UpdateList(const T& filter , const int &limit , const int &skip  ){
        __mlist.clear ();
        __limit = limit;
        __skip = skip;
        __count = this->countItem (filter);
        auto cursor = this->find ( filter , __limit , __skip );
        if( cursor )
        {
            for( auto item : cursor.value() )
            {
                T _item;
                _item.setDocumentView(item);
                __mlist.push_back (_item);
            }
        }
        this->__onlist (__mlist);
        return __mlist;
    }

    inline std::vector<T>& UpdateList(const T& filter = T() ){
        __mlist.clear ();
        __count = this->countItem (filter);
        auto cursor = this->find ( filter , __limit , __skip );
        if( cursor )
        {
            for( auto item : cursor.value() )
            {
                T _item;
                _item.setDocumentView(item);
                __mlist.push_back (_item);
            }
        }
        this->__onlist (__mlist);
        return __mlist;
    }

    inline std::vector<T>& UpdateList( const T& filter , const MongoCore::FindOptions& options )
    {

        __mlist.clear ();
        __count = this->countItem (filter);

        __limit = options.limit ();
        __skip = options.skip ();

        auto cursor = this->find ( filter , options  );
        if( cursor )
        {
            for( auto item : cursor.value() )
            {
                T _item;
                _item.setDocumentView(item);
                __mlist.push_back (_item);
            }
        }
        this->__onlist (__mlist);
        return __mlist;

    }

    inline bool UpdateItem( const T& item ){
        auto result = DB::updateItem (item);
        if( result )
        {
            if( result.value ().modified_count () )
            {
                replace (item);
                return true;
            }else{
                return false;
            }
        }
        return false;
    }

    inline T FindOneItem( const T& filter ){
        mongocxx::stdx::optional<bsoncxx::document::value> value = this->findOneItem ( filter );
        T _item;
        if( value ){
            _item.setDocumentView(value.value ().view ());
        }
        return _item;
    }

    inline std::string InsertItem( const T& item ){
        auto result = DB::insertItem (item);
        if( result )
        {
            if( result.value ().result ().inserted_count () )
            {
                T _item;
                _item.setDocumentView(item.view());
                _item.setOid( result.value ().inserted_id ().get_oid ().value.to_string () );
                __mlist.push_back (_item);
                return result.value ().inserted_id ().get_oid ().value.to_string ();
            }else{
                return "";
            }
        }else{
            return "";
        }
    }

    inline bool DeleteItem( const T& item ){
        auto result = DB::deleteItem (item);
        if( result )
        {
            if( result.value ().deleted_count () )
            {
                remove (item);
                return true;
            }else{
                return false;
            }
        }
        return false;
    }

    inline std::vector<T>& next(const T& filter ){
        __count = this->countItem (filter);
        if( __skip + __limit < __count )
        {
            __skip += __limit;
            return this->UpdateList (filter);
        }else{
            this->UpdateList (filter);
            this->__onlist (__mlist);
            return __mlist;
        }
    }

    inline std::vector<T>& back( const T& filter ){
        __count = this->countItem (filter);
        if( __skip > __limit )
        {
            __skip -= __limit;
            return this->UpdateList (filter);
        }else{
            if( __skip > 0 )
            {
                __skip = 0;
            }
            this->UpdateList (filter);
            this->__onlist (__mlist);
            return __mlist;
        }
    }

    inline void setLimit( const int& limit ){ __limit = limit; }

    inline int limit() const { return __limit; }

    inline void setSkip( const int& skip ) { __skip = skip; }

    inline int skip() const { return __skip; }

    inline int totalCount () const { return __count; }

    inline int totalDocument(const T& value = T()) {
        __totalDocument = this->countItem (value);
        return this->__totalDocument;
    }

    inline int totalPage(const T& value = T()) {
        __totalDocument = this->countItem (value);
        return __totalDocument/__limit;
    }

    inline int currentPage(const T& value = T()) {
        __totalDocument = this->countItem (value);
        return __skip/__limit;
    }

    virtual void onList( const std::vector<T>  &mlist ) = 0;

    const std::vector<T> &List() const
    {
        return __mlist;
    }

    inline std::vector<T>& List( const T& filter , const MongoCore::FindOptions& options = MongoCore::FindOptions() )
    {

        __mlist.clear ();
        __count = this->countItem (filter);

        __limit = options.limit ();
        __skip = options.skip ();

        auto cursor = this->find ( filter , options  );
        if( cursor )
        {
            for( auto item : cursor.value() )
            {
                T _item;
                _item.setDocumentView(item);
                __mlist.push_back (_item);
            }
        }
        return __mlist;
    }

    virtual void errorOccured(const std::string &errorText) override
    {

    }

private:
    std::vector<T> __mlist;

    void __onlist( std::vector<T> &mlist ){
        this->onList (mlist);
    }

    void replace( const T& item ){
        int index = 0;
        for( auto _item : __mlist )
        {
            if( _item.oid().value().to_string() == item.oid().value().to_string() )
            {
                __mlist[index] = item;
                break;
            }
            index++;
        }
    }

    void remove( const T& item ){
        int index = 0;
        for( auto _item : __mlist )
        {
            if( _item.oid().value().to_string() == item.oid().value().to_string() )
            {
                __mlist.erase (__mlist.begin()+index);
                break;
            }
            index++;
        }
    }


    int __limit = 20;
    int __skip = 0;
    int __count = 0;

    int __totalDocument = 0;


};



}





#endif // LISTITEM_H
