#include "item.h"

#include <iostream>

using bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::array;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::impl::takes_array;
using bsoncxx::builder::basic::make_array;


MongoCore::Item::Item(const std::string &collection)
    :mDoc(document{}),mCollection(collection)
{
    mDoc.clear ();
}

MongoCore::Item::Item(const bsoncxx::document::view mView, const std::string &_Collection)
    :mCollection(_Collection)
{
    this->setDocumentView (mView);
}

MongoCore::Item& MongoCore::Item::setDocumentView(const bsoncxx::document::view &view)
{
    mDoc.clear ();
    for( auto item : view )
    {
        this->append(item.key (),item.get_value());
    }
    return *this;
}






MongoCore::Item::~Item()
{
    mDoc.clear ();
}

MongoCore::Item& MongoCore::Item::operator=(const bsoncxx::builder::basic::document &value)
{
    mDoc.clear ();

    for( auto item : value.view () )
    {
        this->append(item.key (),item.get_value ());
    }
    return *this;
}

MongoCore::Item& MongoCore::Item::operator=(const bsoncxx::document::view &view)
{
    mDoc.clear ();
    for( auto item : view )
    {
        this->append(item.key (),item.get_value ());
    }
    return *this;
}




std::optional<bsoncxx::types::bson_value::value> MongoCore::Item::element(const std::string &key) const
{
    try {
        return bsoncxx::types::bson_value::value(mDoc.view ()[key].get_value ());
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() + " Key: " + key;
        const_cast<MongoCore::Item*>(this)->errorOccured (str);
        return std::nullopt;;
    }
}


std::optional<bsoncxx::oid> MongoCore::Item::oid() const
{
    try {
        return this->view ()["_id"].get_oid ().value;
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        const_cast<MongoCore::Item*>(this)->errorOccured (str);
        return std::nullopt;
    }
}



std::optional<bsoncxx::types::bson_value::value> MongoCore::Item::element(std::string key)
{
    try {
        return std::make_optional(bsoncxx::types::bson_value::value(mDoc.view ()[key].get_value ()));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() + " Key: " + key;
        errorOccured (str);
        return std::nullopt;
    }
}

std::optional<bsoncxx::oid> MongoCore::Item::oid()
{
    try {
        return this->view ()["_id"].get_oid ().value;
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        errorOccured (str);
        return std::nullopt;
    }
}


std::optional<bsoncxx::builder::basic::document> MongoCore::Item::ItemFilter()
{

    auto oid = this->oid ();

    if( oid )
    {
        auto doc = document{};

        try {
            doc.append (bsoncxx::builder::basic::kvp("_id",oid.value ()));
        } catch (bsoncxx::exception &e) {
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
            errorOccured (str);
            return std::nullopt;
        }
        return std::move(doc);
    }else{
        return std::nullopt;
    }

}

std::optional<std::time_t> MongoCore::Item::getTime() const
{
    auto _oid = this->oid ();

    if( _oid )
    {
        return std::nullopt;
    }

    return _oid.value ().get_time_t ();

}




bsoncxx::document::view MongoCore::Item::view() const
{
    return mDoc.view ();
}



MongoCore::Item::Item(const Item &other) : mCollection(other.getCollection ())
{
    this->setDocumentView (other.view ());
}

MongoCore::Item::Item(Item &&other)
{
    this->setDocumentView (other.view ());
}

MongoCore::Item& MongoCore::Item::operator=(const Item &value)
{
    mDoc.clear ();

    for( auto item : value.view () )
    {
        this->append(item.key ().data(),item.get_value ());
    }
    return *this;
}

MongoCore::Item &MongoCore::Item::operator=(Item &&other)
{
    mDoc.clear ();
    for( auto item : other.view () )
    {
        this->append(item.key ().data(),item.get_value ());
    }
    return *this;
}

void MongoCore::Item::errorOccured(const std::string &errorText)
{

}

void MongoCore::Item::printView() const
{
    std::cout << __LINE__ << " " << __FUNCTION__ << " Coll: " <<this->getCollection () << " : " <<bsoncxx::to_json (this->view ()) << std::endl;
}

void MongoCore::Item::clear()
{
    this->mDoc.clear ();
}

MongoCore::Item &MongoCore::Item::setOid(const std::string &oid)
{
    this->append("_id",bsoncxx::oid{oid});
    return *this;
}

MongoCore::Item &MongoCore::Item::setOid(const bsoncxx::oid &oid)
{
    this->append("_id",oid);
    return *this;
}


std::string MongoCore::Item::getCollection() const
{
    return mCollection;
}

void MongoCore::Item::setCollection(const std::string &collection)
{
    this->mCollection = collection;
}


void MongoCore::Item::removeElement(const std::string &key)
{
    auto tempDoc = document{};
    for( auto item : mDoc.view () )
    {
        if( key != item.key () )
        {
            try {
                tempDoc.append( kvp( item.key () , item.get_value () ) );
            } catch (bsoncxx::exception &e) {
                std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                errorOccured (str);
            }
        }
    }
    mDoc.clear ();

    for( auto item : tempDoc.view () )
    {
        mDoc.append(kvp(item.key (),item.get_value ()));
    }
}

void MongoCore::Item::removeElement(const std::string_view &key)
{
    auto tempDoc = document{};
    for( auto item : mDoc.view () )
    {
        if( key != item.key () )
        {
            try {
                tempDoc.append( kvp( item.key () , item.get_value () ) );
            } catch (bsoncxx::exception &e) {
                std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                errorOccured (str);
            }
        }
    }
    mDoc.clear ();

    for( auto item : tempDoc.view () )
    {
        mDoc.append(kvp(item.key (),item.get_value ()));
    }
}

void MongoCore::Item::pushArray(std::string key, const Item &value)
{
    auto arr = bsoncxx::builder::basic::array{};
    auto existArray = this->element (key);

    if( existArray )
    {
        this->removeElement ( key );

        for( auto item : existArray->view().get_array ().value )
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
        arr.append (value.view ());
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

void MongoCore::Item::pullArray(const std::string &key, const bsoncxx::types::bson_value::value &value)
{
    auto arr = bsoncxx::builder::basic::array{};
    auto existArray = this->element (key);

    if( existArray )
    {
        this->removeElement ( key );
        for( auto item : existArray->view().get_array ().value )
        {
            if( value != item.get_value ())
            {
                try {
                    arr.append (item.get_value ());
                } catch (bsoncxx::exception &e) {
                    std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                    errorOccured (str);
                }
            }
        }
    }

    try {
        mDoc.append (kvp(key,arr));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        errorOccured (str);
    }
}








MongoCore::FindOptions::FindOptions()
    :Item("none")
{

}



MongoCore::FindOptions &MongoCore::FindOptions::setLimit(const int &limit)
{
    this->append("limit",bsoncxx::types::b_int64{limit});
    return *this;
}






MongoCore::FindOptions &MongoCore::FindOptions::setSkip(const int &skip)
{
    this->append("skip",bsoncxx::types::b_int64{skip});
    return *this;
}






MongoCore::FindOptions &MongoCore::FindOptions::setProjection(const MongoCore::Item &projItem)
{
    this->append("projection",projItem);
    return *this;
}






MongoCore::FindOptions &MongoCore::FindOptions::setSort(const MongoCore::Item &sortItem)
{
    this->append("sort",sortItem);
    return *this;
}






int MongoCore::FindOptions::limit() const
{
    auto val = this->element ("limit");

    if( val )
    {
        return static_cast<int>(val->view().get_int64().value);
    }else{
        return 20;
    }
}





int MongoCore::FindOptions::skip() const
{
    auto val = this->element ("skip");

    if( val )
    {
        return static_cast<int>(val->view().get_int64().value);
    }else{
        return 0;
    }
}






MongoCore::Item MongoCore::FindOptions::sort() const
{
    auto val = this->element ("sort");
    if( val )
    {
        return MongoCore::Item(val->view ().get_document (),"none");
    }
    return MongoCore::Item("none");
}






MongoCore::Item MongoCore::FindOptions::projection() const
{    auto val = this->element ("projection");
    if( val )
    {
        MongoCore::Item item(val->view().get_document ().value,"none");
        return item;
    }
    return MongoCore::Item("none");
}










template<>
MongoCore::Item &MongoCore::Item::append(const std::string_view &key, const Item &value)
{
    this->removeElement (key);
    try {
        mDoc.append (bsoncxx::builder::basic::kvp(key,value.view ()));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        errorOccured (str);
    }
    return *this;
}





//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::bson_value::value>(const std::string_view& ,const bsoncxx::types::bson_value::value&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_eod>(const std::string_view& ,const bsoncxx::types::b_eod&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_double>(const std::string_view& ,const bsoncxx::types::b_double&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_string>(const std::string_view& ,const bsoncxx::types::b_string&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_document>(const std::string_view& ,const bsoncxx::types::b_document&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_array>(const std::string_view& ,const bsoncxx::types::b_array&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_binary>(const std::string_view& ,const bsoncxx::types::b_binary&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_undefined>(const std::string_view& ,const bsoncxx::types::b_undefined&);

//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_oid>(const std::string_view& ,const bsoncxx::types::b_oid&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_bool>(const std::string_view& ,const bsoncxx::types::b_bool&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_date>(const std::string_view& ,const bsoncxx::types::b_date&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_null>(const std::string_view& ,const bsoncxx::types::b_null&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_regex>(const std::string_view& ,const bsoncxx::types::b_regex&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_dbpointer>(const std::string_view& ,const bsoncxx::types::b_dbpointer&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_code>(const std::string_view& ,const bsoncxx::types::b_code&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_symbol>(const std::string_view& ,const bsoncxx::types::b_symbol&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_codewscope>(const std::string_view& ,const bsoncxx::types::b_codewscope&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_int32>(const std::string_view& ,const bsoncxx::types::b_int32&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_timestamp>(const std::string_view& ,const bsoncxx::types::b_timestamp&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_int64>(const std::string_view& ,const bsoncxx::types::b_int64&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_decimal128>(const std::string_view& ,const bsoncxx::types::b_decimal128&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_minkey>(const std::string_view& ,const bsoncxx::types::b_minkey&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::b_maxkey>(const std::string_view& ,const bsoncxx::types::b_maxkey&);
//template MongoCore::Item &MongoCore::Item::append<bsoncxx::types::bson_value::view>(const std::string_view& ,const bsoncxx::types::bson_value::view&);

//template MongoCore::Item &MongoCore::Item::append<int>(const std::string_view& ,const int&);
//template MongoCore::Item &MongoCore::Item::append<double>(const std::string_view& ,const double&);
//template MongoCore::Item &MongoCore::Item::append<std::string>(const std::string_view& ,const std::string&);


