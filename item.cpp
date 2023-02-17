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










