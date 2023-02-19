#include "db.h"

#include <fstream>

#include <mongocxx/gridfs/bucket.hpp>
#include <mongocxx/gridfs/downloader.hpp>
#include <mongocxx/gridfs/uploader.hpp>
#include <mongocxx/exception/gridfs_exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>

static int DBConnectionCount = 0;

MongoCore::DB::DB()
{



}

//MongoCore::DB::DB(const DB &db)
//    :mDB(db.mDB)
//{
//    //    std::cout << "DB::DB(const DB &db): " << DBConnectionCount << std::endl;
//    mConstructWithNewClient = false;
//}

//MongoCore::DB::DB(MongoCore::DB &&other)
//    :mDB(other.mDB)
//{
//    //    std::cout << "DB::DB(DB &&db): " << DBConnectionCount << std::endl;
//    mConstructWithNewClient = false;
//}

MongoCore::DB::DB(mongocxx::database *_db)
    :mDB( _db )
{
    //    std::cout << "DB::DB(mongocxx::database *_db): " << DBConnectionCount << std::endl;
    mConstructWithNewClient = false;
}

void MongoCore::DB::instance(const std::string &mUri)
{
    mongocxx::instance{};
}

MongoCore::DB::DB(DB *_db) : mDB(_db->db ())
{
    //    std::cout << "DB::DB(DB *_db): " << DBConnectionCount << std::endl;
    mConstructWithNewClient = false;
}

//MongoCore::DB::DB(const DB *_db) : mDB( _db->getDB ()->mDB )
//{

//}



//MongoCore::DB::~DB()
//{
//    std::cout << "-DB Destructor- Delete Client: " << mConstructWithNewClient << std::endl;
//    if( mConstructWithNewClient )
//    {
//        std::cout << "Delete DB Connection, CurrentConnection Count: " <<  --DBConnectionCount << std::endl;
//    }
//    std::cout << "DB Destructor End" << std::endl;
//}

//MongoCore::DB &MongoCore::DB::operator=(const DB &otherDB)
//{
//    std::cout << "DB &DB::operator=(const DB &otherDB): " << DBConnectionCount << std::endl;
//    mDB = otherDB.mDB;
//    return *this;
//}

//MongoCore::DB &MongoCore::DB::operator=(MongoCore::DB &&otherDB)
//{
//    std::cout << "DB &DB::operator=(const DB &otherDB): " << DBConnectionCount << std::endl;
//    mDB = otherDB.mDB;
//    return *this;
//}

//MongoCore::DB &MongoCore::DB::operator=(mongocxx::database *_db)
//{
//    std::cout << "DB &DB::operator=(const DB &otherDB): " << DBConnectionCount << std::endl;
//    mDB = _db;
//    return *this;
//}

void MongoCore::DB::errorOccured(const std::string &errorText)
{

}







mongocxx::database *MongoCore::DB::db()
{
    return mDB;
}

std::string MongoCore::DB::downloadFile(const std::string &fileOid, bool forceFilename)
{


    std::string file__Name{""};
    bool fileDownloadedBefore=true;

    auto filter = bsoncxx::builder::basic::document{};

    try {
        filter.append(bsoncxx::builder::basic::kvp("_id",bsoncxx::oid{fileOid}));
    } catch (bsoncxx::exception &e) {
        std::cout << "\nLOG: " << e.what() << "\n";
    }

    auto val = this->mDB->collection("fs.files").find_one(filter.view());

    if( val ){
        std::filesystem::path file_info;
        try {
            file_info = std::filesystem::path(val.value().view()["filename"].get_string().value.data());
        } catch (bsoncxx::exception &e) {
            fileDownloadedBefore = false;
        }
        if( fileDownloadedBefore ){
            if( forceFilename ){
                file__Name = std::string("tempfile/")+file_info.stem().string()+file_info.extension().string();
            }else{
                file__Name = "tempfile/"+fileOid+file_info.extension().string();
            }

            if( std::filesystem::exists(file__Name) ){
                std::cout << "\nFile Exist: " << file__Name << "\n";
            }else{
                std::cout << "\nFile NOT Exist: " << file__Name << "\n";
                fileDownloadedBefore = false;
            }
        }
    }else{
        fileDownloadedBefore = false;
    }

    if( fileDownloadedBefore ){
        return file__Name;
    }


    auto bucket = this->db ()->gridfs_bucket ();

    auto doc = bsoncxx::builder::basic::document{};

    try {
        doc.append(bsoncxx::builder::basic::kvp("key",bsoncxx::oid{fileOid}));
    } catch (bsoncxx::exception& e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() ;
        this->setLastError (str.c_str ());
        return "NULL";
    }



    mongocxx::gridfs::downloader downloader;
    try {
        auto roid = bsoncxx::types::bson_value::value(doc.view()["key"].get_oid());
        downloader = bucket.open_download_stream(roid);

    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() ;
        this->setLastError (str.c_str ());
        return "";
    }


    auto file_length = downloader.file_length();
    std::int32_t bytes_counter = 0;


    std::filesystem::path info( downloader.files_document()["filename"].get_string().value.data ());



    std::string fullFilename;

    if( !std::filesystem::exists("tempfile") ){
        if( !std::filesystem::create_directory("tempfile") ){
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " tempfile Can not Created";
            this->setLastError (str);
            return "";
        }
    }

    if( forceFilename )
    {
        fullFilename = std::string("tempfile/")+downloader.files_document()["filename"].get_string().value.data ();
    }else{
        fullFilename = std::string("tempfile/")  +downloader.files_document()["_id"].get_oid().value.to_string() + info.extension().string();
    }


    if( std::filesystem::exists(fullFilename) )
    {
        return fullFilename;
    }

    auto buffer_size = std::min(file_length, static_cast<std::int64_t>(downloader.chunk_size()));
    auto buffer = bsoncxx::stdx::make_unique<std::uint8_t[]>(static_cast<std::size_t>(buffer_size));

    std::ofstream out;

    out.open(fullFilename,std::ios::out | std::ios::app | std::ios::binary);


    if( out.is_open() )
    {
        while ( auto length_read = downloader.read(buffer.get(), static_cast<std::size_t>(buffer_size)) ) {
            auto bufferPtr = buffer.get();
            out.write(reinterpret_cast<char*>(bufferPtr),length_read);
            bytes_counter += static_cast<std::int32_t>( length_read );
        }
        out.close();
    }else{
        std::string str = "Error Can Not Open File: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + fullFilename ;
        this->setLastError (str);
    }

    return fullFilename;
}

std::string MongoCore::DB::downloadFileWeb(const std::string &fileOid, bool forceFilename)
{

    std::string file__Name{""};
    bool fileDownloadedBefore=true;

    auto filter = bsoncxx::builder::basic::document{};

    try {
        filter.append(bsoncxx::builder::basic::kvp("_id",bsoncxx::oid{fileOid}));
    } catch (bsoncxx::exception &e) {
        std::cout << "\nLOG: " << e.what() << "\n";
    }

    auto val = this->mDB->collection("fs.files").find_one(filter.view());

    if( val ){
        std::filesystem::path file_info;
        try {
            file_info = std::filesystem::path(val.value().view()["filename"].get_string().value.data());
        } catch (bsoncxx::exception &e) {
            fileDownloadedBefore = false;
        }
        if( fileDownloadedBefore ){
            if( forceFilename ){
                file__Name = std::string("tempfile/")+file_info.stem().string()+file_info.extension().string();
            }else{
                file__Name = "tempfile/"+fileOid+file_info.extension().string();
            }

            if( std::filesystem::exists(std::string("docroot/")+file__Name) ){
                std::cout << "\nFile Exist: " << file__Name << "\n";
            }else{
                std::cout << "\nFile NOT Exist: " << file__Name << "\n";
                fileDownloadedBefore = false;
            }
        }
    }else{
        fileDownloadedBefore = false;
    }

    if( fileDownloadedBefore ){
        return file__Name;
    }

    auto bucket = this->mDB->gridfs_bucket ();

    auto doc = bsoncxx::builder::basic::document{};

    try {
        doc.append(bsoncxx::builder::basic::kvp("key",bsoncxx::oid{fileOid}));
    } catch (bsoncxx::exception& e) {
        std::string str = "Error: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() ;
        this->setLastError (str.c_str ());
        return "";
    }



    mongocxx::gridfs::downloader downloader;
    try {
        auto roid = bsoncxx::types::bson_value::value(doc.view()["key"].get_oid());
        downloader = bucket.open_download_stream(roid);
    } catch (bsoncxx::exception &e) {
        std::string str = "Error: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() ;
        this->setLastError (str.c_str ());
        return "img/404-header.png";
    }


    auto file_length = downloader.file_length();


    std::filesystem::path info( downloader.files_document()["filename"].get_string().value.data ());


    std::string fullFilename;


    if( !std::filesystem::exists("docroot/tempfile") ){
        if( !std::filesystem::create_directory("docroot/tempfile") ){
            std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " tempfile Can not Created";
            this->setLastError (str);
            return "";
        }
    }


    if( forceFilename )
    {
        fullFilename = std::string("tempfile/")+downloader.files_document()["filename"].get_string().value.data ();
    }else{
        fullFilename = std::string("tempfile/")  +downloader.files_document()["_id"].get_oid().value.to_string() + info.extension().string();
    }


    if( std::filesystem::exists("docroot/"+fullFilename) )
    {
        return fullFilename;
    }

    auto buffer_size = std::min(file_length, static_cast<std::int64_t>(downloader.chunk_size()));
    auto buffer = bsoncxx::stdx::make_unique<std::uint8_t[]>(static_cast<std::size_t>(buffer_size));

    std::ofstream out;

    out.open("docroot/"+fullFilename,std::ios::out | std::ios::app | std::ios::binary);

    if( out.is_open() )
    {
        while ( auto length_read = downloader.read(buffer.get(), static_cast<std::size_t>(buffer_size)) ) {
            auto bufferPtr = buffer.get();
            out.write(reinterpret_cast<char*>(bufferPtr),length_read);
        }
        out.close();
    }else{
        std::string str = "Error Can Not Open File: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + fullFilename ;
        this->setLastError (str);
    }

    return fullFilename;
}

bsoncxx::types::bson_value::value MongoCore::DB::uploadfile(const std::string &filepath)
{
    auto bucket = this->db ()->gridfs_bucket ();
    std::ifstream file;
    file.open(filepath,std::ios::out | std::ios::app | std::ios:: binary );
    if( file.is_open() ){

        file.seekg (0, file.end);
        int length = file.tellg();
        file.seekg (0, file.beg);
        char * buffer = new char [length];

        file.read (buffer,length);

        if( file ){
            std::filesystem::path info( filepath );
            auto uploader = bucket.open_upload_stream(info.filename().string());
            uploader.write((std::uint8_t*)buffer,length);
            auto res = uploader.close();
            return bsoncxx::types::bson_value::value(res.id());
        }else{
            this->setLastError ("all bytes read failed");
        }

        file.close();

        delete[] buffer;
    }else{
        std::string str = "Error Can Not Open File: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + filepath ;
        this->setLastError (str);
        return bsoncxx::types::bson_value::value(nullptr);
    }
}

//bsoncxx::types::bson_value::value MongoCore::DB::uploadfile(const std::string filepath) const
//{
//    auto bucket = this->db ()->gridfs_bucket ();
//    std::ifstream file;
//    file.open(filepath,std::ios::out | std::ios::app | std::ios:: binary );
//    if( file.is_open() ){

//        file.seekg (0, file.end);
//        int length = file.tellg();
//        file.seekg (0, file.beg);
//        char * buffer = new char [length];

//        file.read (buffer,length);

//        if( file ){
//            std::filesystem::path info( filepath );
//            auto uploader = bucket.open_upload_stream(info.filename().string());

//        }else{
//            this->setLastError ("all bytes read failed");

//        }

//        file.close();

//        delete[] buffer;
//    }else{
//        std::string str = "Error Can Not Open File: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + filepath ;
//        this->setLastError (str.c_str ());
//        return bsoncxx::types::bson_value::value(nullptr);
//    }
//}

bool MongoCore::DB::deleteGridFS(const std::string &fileOid)
{
    bsoncxx::types::bson_value::value id(bsoncxx::types::b_oid{bsoncxx::oid{fileOid}});
    try {
        mDB->gridfs_bucket ().delete_file (id);
        return true;
    } catch (mongocxx::gridfs_exception &e) {
        std::string str = "ERROR GridFS Exception: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str);
        return false;
    } catch ( mongocxx::bulk_write_exception &e) {
        std::string str = "ERROR bulk_write_exception: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str);
        return false;
    }
}

mongocxx::stdx::optional<mongocxx::result::insert_one> MongoCore::DB::insertItem(const Item &item)
{

    try {
        auto ins = this->db ()->collection (item.getCollection ()).insert_one (item.view ());
        return ins;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }
}

mongocxx::stdx::optional<mongocxx::result::update> MongoCore::DB::updateItem(const Item &item)
{

    auto filter = bsoncxx::builder::basic::document{};

    try {
        filter.append (bsoncxx::builder::basic::kvp("_id",item.view ()["_id"].get_oid ().value));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return mongocxx::stdx::nullopt;
    }

    auto doc = bsoncxx::builder::basic::document{};

    bool _errorOccured = false;
    for( auto element : item.view () )
    {
            if( element.key () != "_id" )
            {
                try {
                    doc.append (bsoncxx::builder::basic::kvp(element.key (),element.get_value ()));
                } catch (bsoncxx::exception &e) {
                    std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                    this->setLastError (str.c_str ());
                    _errorOccured = true;
                    break;
                }
            }
    }

    if( _errorOccured ) return mongocxx::stdx::nullopt;

    auto setDoc = bsoncxx::builder::basic::document{};

    try {
        setDoc.append (kvp("$set",doc));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }

    try {
        auto upt = this->db ()->collection (item.getCollection ()).update_one (filter.view (),setDoc.view ());
        return upt;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }

}

mongocxx::stdx::optional<bsoncxx::document::value> MongoCore::DB::findOneItem(const Item &item )
{
    try {
        auto value = this->db ()->collection (item.getCollection ()).find_one (item.view ());
        return value;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() + " Collection: " + item.getCollection ();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }
}

mongocxx::stdx::optional<bsoncxx::document::value> MongoCore::DB::findOneItem(const MongoCore::Item &item, const MongoCore::Item &findOptions)
{
    mongocxx::options::find findOptions_;

    findOptions_.sort (findOptions.view ());

    try {
        auto value = this->db ()->collection (item.getCollection ()).find_one (item.view (),findOptions_);
        return value;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() + " Collection: " + item.getCollection ();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }
}

mongocxx::stdx::optional<mongocxx::cursor> MongoCore::DB::find(const Item &item, const mongocxx::options::find findOptions)
{
    try {
        auto cursor = this->db ()->collection (item.getCollection ()).find (item.view (),findOptions);
        return std::move(cursor);
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() +" Collection: " + item.getCollection ();
        this->setLastError (str);
        return mongocxx::stdx::nullopt;
    }
}

mongocxx::stdx::optional<mongocxx::cursor> MongoCore::DB::find(const MongoCore::Item &item, const int &limit, const int &skip)
{
    mongocxx::options::find findOptions;

    findOptions.limit (limit);
    findOptions.skip (skip);

    Item sortItem(item.getCollection ());
    sortItem.append("_id",-1);

    findOptions.sort (sortItem.view ());


    try {
        auto cursor = this->db ()->collection (item.getCollection ()).find (item.view (),findOptions);
        return std::move(cursor);
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() +" Collection: " + item.getCollection ();
        this->setLastError (str.c_str ());
        return mongocxx::stdx::nullopt;
    }
}

mongocxx::stdx::optional<mongocxx::cursor> MongoCore::DB::find(const MongoCore::Item &item, const MongoCore::FindOptions &options)
{
    mongocxx::options::find findOptions;
    auto doc = bsoncxx::builder::basic::document{};

    if( options.limit () != 0 )
    {
        findOptions.limit (options.limit ());
    }

    findOptions.skip (options.skip ());

    doc.clear ();
    if( !options.projection ().view ().empty () )
    {
        auto __view = options.element ("projection");
        if( __view )
        {
            for( auto __item : __view->view ().get_document ().value)
            {
                try {
                    doc.append (bsoncxx::builder::basic::kvp(__item.key (),__item.get_value ()));
                } catch (bsoncxx::exception &e) {
                    std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                    this->setLastError (str.c_str ());
                }
            }
            findOptions.projection (doc.view ());
        }
    }

    doc.clear ();
    if( !options.sort ().view ().empty () )
    {
        auto __view = options.element ("sort");
        if( __view )
        {
            for( auto __item : __view->view ().get_document ().value)
            {
                try {
                    doc.append (bsoncxx::builder::basic::kvp(__item.key (),__item.get_value ()));
                } catch (bsoncxx::exception &e) {
                    std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
                    this->setLastError (str.c_str ());
                }
            }
            findOptions.sort (doc.view ());
        }
    }


    try {
        auto cursor = this->db ()->collection (item.getCollection ()).find (item.view (),findOptions);
        return std::move(cursor);
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what() +" Collection: " + item.getCollection ();
        this->setLastError (str.c_str ());
        return mongocxx::stdx::nullopt;
    }
}

bool MongoCore::DB::removeField(const MongoCore::Item &filter, const std::string &field)
{
    auto _oid = filter.oid ();

    if( !_oid )
    {
        this->setLastError(std::to_string(__LINE__) + std::string(" ") + __FUNCTION__ +  std::string("Required Object ID"));
        return false;
    }

    auto newfilter = bsoncxx::builder::basic::document{};

    try {
        newfilter.append(bsoncxx::builder::basic::kvp("_id",bsoncxx::oid{_oid.value ().to_string ()}));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        std::cout << str << std::endl;
    }

    auto unsetDoc = bsoncxx::builder::basic::document{};

    try {
        unsetDoc.append (bsoncxx::builder::basic::kvp("$unset",bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(field,""))));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        std::cout << str << std::endl;
    }


    try {
        auto result = this->db ()->collection (filter.getCollection ()).update_one (newfilter.view (),unsetDoc.view ());
        if( result )
        {
            if( result.value().modified_count() )
            {
                return true;
            }else{
                this->setLastError(std::to_string(__LINE__) + std::to_string(result.value ().modified_count ()) + " Modified Document");
                return false;
            }
        }else{
            setLastError (std::string("setField No Result"));
            return false;
        }
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        setLastError (str.c_str ());
        return false;
    }
}

bool MongoCore::DB::incValue(const MongoCore::Item &filter, const std::string &field, const int64_t &value)
{
    auto _oid = filter.oid ();
    if( !_oid )
    {
        this->setLastError(std::to_string(__LINE__) + std::string(" ") + __FUNCTION__ +  std::string("Required Object ID"));

        return false;
    }
    auto _filter = bsoncxx::builder::basic::document{};
    try {
        _filter.append (bsoncxx::builder::basic::kvp("_id",_oid.value ()));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        setLastError (str.c_str ());
        return false;
    }
    auto incDoc = bsoncxx::builder::basic::document{};
    try {
        incDoc.append (bsoncxx::builder::basic::kvp(field,bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$inc",bsoncxx::types::b_int64{value}))));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return false;
    }
    try {
        auto res = this->db ()->collection (filter.getCollection ()).update_one (_filter.view (),incDoc.view ());
        if( res )
        {
            if( res.value ().modified_count () )
            {
                return true;
            }else{
                this->setLastError ("No Field incremented");
                return false;
            }
        }else{
            this->setLastError ("increment value db not operated");
            return false;
        }
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return false;
    }
}

bool MongoCore::DB::incValue(const MongoCore::Item &filter, const std::string &field, const int32_t &value)
{
    auto _oid = filter.oid ();
    if( !_oid )
    {
        this->setLastError(std::to_string(__LINE__) + std::string(" ") + __FUNCTION__ +  std::string("Required Object ID"));

        return false;
    }
    auto _filter = bsoncxx::builder::basic::document{};
    try {
        _filter.append (bsoncxx::builder::basic::kvp("_id",_oid.value ()));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        setLastError (str.c_str ());
        return false;
    }
    auto incDoc = bsoncxx::builder::basic::document{};
    try {
        incDoc.append (bsoncxx::builder::basic::kvp(field,bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$inc",bsoncxx::types::b_int32{value}))));
    } catch (bsoncxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        setLastError (str.c_str ());
        return false;
    }
    try {
        auto res = this->db ()->collection (filter.getCollection ()).update_one (_filter.view (),incDoc.view ());
        if( res )
        {
            if( res.value ().modified_count () )
            {
                return true;
            }else{
                this->setLastError ("No Field incremented");
                return false;
            }
        }else{
            this->setLastError ("increment value db not operated");
            return false;
        }
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return false;
    }
}



mongocxx::stdx::optional<mongocxx::result::delete_result> MongoCore::DB::deleteItem(const Item &item)
{
    try {
        auto del = this->db ()->collection (item.getCollection ()).delete_one (item.view ());
        return del;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return mongocxx::stdx::nullopt;
    }
}



std::int64_t MongoCore::DB::countItem(const Item &item)
{
    try {
        auto count = this->db ()->collection (item.getCollection ()).count_documents (item.view ());
        return count;
    } catch (mongocxx::exception &e) {
        std::string str = "ERROR: " + std::to_string(__LINE__) + " " + __FUNCTION__ + " " + e.what();
        this->setLastError (str.c_str ());
        return -1;
    }
}

std::string MongoCore::DB::getLastError()
{
    auto str = mLastError;
    mLastError = "";
    return str;
}

void MongoCore::DB::setLastError(const std::string &lastError)
{
    mLastError = lastError;
    errorOccured (lastError);
}


