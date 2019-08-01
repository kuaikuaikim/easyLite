//
// Created by kkkim on 11/24/18.
//

#ifndef EASYLITE_FACEDB_H
#define EASYLITE_FACEDB_H

#include <string>
#include <vector>
#include "easylite.h"
#include "unqlite.h"
#include "ArduinoJson-v5.13.3.h"

using namespace std;

class EasyLiteEngine : public EasyLite{

public:
    explicit EasyLiteEngine();

    virtual ~EasyLiteEngine();

    virtual bool openDB(const std::string &db_file);

    virtual bool closeDB();

    virtual bool update(const std::string &collection_name, int id, const std::string &json_str);

    virtual int totalCollectionRecord(const std::string &collection_name);

    virtual int insert(const std::string &collection_name, const std::string &json_str);

    virtual int fetchById(const std::string &collection_name, const int id, std::string &out_json_str);

    virtual bool deleteById(const std::string &collection_name, const int id);

    virtual int fetchAll(const std::string &collection_name, std::vector<std::string> &out_json, std::string condition_str = "");

    virtual bool dbExsitCollection(const string &collection_name);

    virtual bool dbCreateCollection(const string &collection_name);

    virtual bool kv_store(const std::string &key, std::string value);

    virtual bool kv_fetch(const std::string &key, std::string &out_value);

    virtual bool kv_delete(const std::string &key);


private:
//    int rc;
    unqlite *pDb;
//    unqlite_vm *pVm;


};



#endif //EASYLITE_FACEDB_H
