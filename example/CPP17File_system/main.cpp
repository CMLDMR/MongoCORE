
#include <iostream>
#include <fstream>
#include <string>

#include "db.h"
#include "item.h"
#include "listitem.h"

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>


#define LOG         std::cout << __LINE__ << " "  << __FUNCTION__ <<"\n";


using namespace std;


class altBirimler : public MongoCore::Item
{
public:
    altBirimler():MongoCore::Item("AltBirimler"){}


};


class altBirimManager : public MongoCore::ListItem<altBirimler>
{
public:
    altBirimManager( MongoCore::DB* mDB ): MongoCore::ListItem<altBirimler>(mDB){


        LOG
            this->UpdateList();

    }



    //    virtual void onList( const std::vector<T>  *mlist ) = 0;


    // DB interface
public:
    virtual void errorOccured(const string &errorText) override{}

    // ListItem interface
public:
    virtual void onList(const std::vector<altBirimler> &mlist) override{
        LOG
            for( const auto &item : mlist ){
            std::cout << bsoncxx::to_json(item.view()) << "\n";
        }

    }
};


int main()
{

    LOG
        mongocxx::instance ins{};

    LOG
        mongocxx::client* mClient;
    LOG
    try {
        mClient = new mongocxx::client(mongocxx::uri{""});

    } catch (mongocxx::exception &e) {
        std::cout << e.what() << "\n";
    }
    LOG


        auto db = mClient->database("SERIKBELTR");

    LOG
        MongoCore::DB* mDB = new MongoCore::DB(&db);
    LOG
        MongoCore::Item filter("AltBirimler");

    altBirimler filterr;
    filterr.append("tetfd","asdf");
    std::cout << bsoncxx::to_json(filterr.view()) << "\n";

    LOG

            altBirimManager* mManager = new altBirimManager(mDB);

        //LOG




        return 0;
}
