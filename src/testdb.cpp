//
// Created by kkkim on 11/24/18.
//

#include "easylite_engine.h"
#include "ArduinoJson-v5.13.3.h"

int main(int argc, char** argv) {

    EasyLiteEngine* fdb = new EasyLiteEngine();
    fdb->openDB("yu.db");

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["test"] = 100.52;
    std::string jstr;
    root.printTo(jstr);

    std::string collection_name = "user";

    int last_id_2 = fdb->insert(collection_name, jstr);

    std::string out_features_str;
    int cou = fdb->fetchById(collection_name, last_id_2, out_features_str);

    int total = fdb->totalCollectionRecord(collection_name);

    std::vector<int> out_ids_;
    std::vector<std::string> out_features_;
    std::string cond_str_="";
    int ones_ = fdb->fetchAll(collection_name, out_features_, cond_str_);

    bool  su = fdb->closeDB();

    return 0;
}