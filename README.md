## EasyLite - Easy Embedded Nosql Database
[![Build Status](https://travis-ci.org/symisc/unqlite.svg?branch=master)](https://github.com/kuaikuaikim) [![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/kuaikuaikim) [![GitHub license](https://img.shields.io/pypi/l/Django.svg)](https://github.com/kuaikuaikim) 


**EasyLite** is an open source embedded nosql database based on unqlite. We wrapper couples of easy CURD API for reading/writting a single file only nosql databse. EasyLite dont need any dependent library, it can run any devices easyly.

## Installation
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

## Usage

c++ wrapper api:  

```c++
    //set environment variables
    export LD_LIBRARY_PATH=/libeasylite.so path/


    #include "easylite_api.h"
    EasyLite* easydb = load_easylite();
    easydb->openDB("easy.db");

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["test"] = 100.52;
    std::string jstr;
    root.printTo(jstr);

    std::string collection_name = "user";
    int last_id_2 = fdb->insert(collection_name, jstr);

    std::string out_str;
    int count = fdb->fetchById(collection_name, last_id_2, out_str);
```

java wrapper api:  

```java

    System.loadLibrary("easylitejni");

    EasyLite easyDB = new EasyLite();
    easyDB.initLoad();
    easyDB.openDB("easy.db");
    
    //create user object
    User user=new User();
    user.setUsername("kylin");
    user.setPassword("123456");

    //object to json object
    JSONObject jsonObject=JSONObject.fromObject(user);
    //json object to json string
    String jsonStr=jsonObject.toString();
    int lastid = easyDB.insert("user", jsonStr);
    
    //select user json string by user id
    String userJsonStr = easyDB.fetchById(lastid);
    
```