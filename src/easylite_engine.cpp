//
// Created by kkkim on 11/24/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "easylite_engine.h"
#include "unqlite.h"
#include "jx9.h"
#include "common.h"
#include "exception/easylite_exceptio.h"
#include "base64.h"


union ulf
{
    unsigned long ul;
    float f;
};

union ucf{
    char c[4];
    float f;
};


#define JSON_BUFF_SIZE 20480

#define SCRIPT_DBSTORE_COMMON \
    "if( !db_exists($collection_name) ){"\
    "    $rc = db_create($collection_name);"\
    "    if ( !$rc ){"\
    "        print db_errlog();"\
    "       return;"\
    "    }"\
    "}"\


#define SCRIPT_FETCHBYID \
     "$data = db_fetch_by_id($collection_name, $record_id);" \


static std::vector<std::vector<float> > fetch_face_tmp;
static std::vector<int> fetch_face_idx_tmp;
static std::vector<std::string> fetch_all_tmp;

static int JsonObjectWalker4FetchFace(unqlite_value *pKey,unqlite_value *pData,void *pUserData)
{
    const char *zKey,*zData;
    /* Extract the key and the data field */
    zKey = unqlite_value_to_string(pKey,0);
    zData = unqlite_value_to_string(pData,0);

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(zData);
    if(&root ==NULL){
        return UNQLITE_OK;
    }

    JsonArray& data = root["__feature"];
    if(&data == NULL){
        return UNQLITE_OK;
    }

    int size_feature = data.size();
    if(size_feature != 128 && size_feature != 512){
        return UNQLITE_OK;
    }

    long unq_id = root["__id"];

    std::vector<float> feature_v;
    for(int i=0; i<size_feature; ++i){
        float fv = root["__feature"][i];
        feature_v.push_back(fv);
    }
    fetch_face_tmp.push_back(feature_v);
    fetch_face_idx_tmp.push_back((int)unq_id);

    return UNQLITE_OK;

//    const char* base64_fstr = root["__feature"];
//    if(base64_fstr== NULL || strlen(base64_fstr) == 0){
//        return UNQLITE_OK;
//    }
//
//    std::string feature_str;
//    Base64::Decode(base64_fstr, &feature_str);
//    const char* feature_cstr = feature_str.c_str();
//    int size_feature = (int)(feature_str.size()/4);
//
//    std::vector<float> feature_v;
//    for(int i=0; i<size_feature; i+=1){
//        ucf u;
//        u.c[0] = feature_cstr[4*i];
//        u.c[1] = feature_cstr[4*i+1];
//        u.c[2] = feature_cstr[4*i+2];
//        u.c[3] = feature_cstr[4*i+3];
//        feature_v.push_back(u.f);
//    }

//    zData = unqlite_value_to_string(pData,0);
//
//    StaticJsonBuffer<JSON_BUFF_SIZE> jsonBuffer;
//
//    JsonObject& root = jsonBuffer.parseObject(zData);
//
//    int age = root["age"];
//
//    /* Dump */
//    printf(
//            "%s ===> %s\n",
//            zKey,
//            zData
//    );
//    return UNQLITE_OK;
}


static int JsonObjectWalker(unqlite_value *pKey,unqlite_value *pData,void *pUserData)
{
    const char *zKey,*zData;
    /* Extract the key and the data field */
    zKey = unqlite_value_to_string(pKey,0);
    zData = unqlite_value_to_string(pData,0);
    std::string jstr(zData);
    fetch_all_tmp.push_back(jstr);
    return UNQLITE_OK;
}



static int JsonObjectWalkerArray(unqlite_value *pKey,unqlite_value *pData,void *pUserData /* Unused */)
{
    const char *zKey,*zData;
    /* Extract the key and the data field */
    zKey = unqlite_value_to_string(pKey,0);
    zData = unqlite_value_to_string(pData,0);
    /* Dump */
    printf(
            "%s ===> %s\n",
            zKey,
            zData
    );
    return UNQLITE_OK;
}


EasyLiteEngine::EasyLiteEngine() {

}

EasyLiteEngine::~EasyLiteEngine() {

}

bool EasyLiteEngine::openDB(const std::string &db_file) {
	//rc = unqlite_open(&pDb,"dface.db",UNQLITE_OPEN_CREATE);
	int rc;
	rc = unqlite_open(&pDb, db_file.c_str(), UNQLITE_OPEN_CREATE);
	if (rc != UNQLITE_OK) {
		return false;
		//        throw EasyLiteException("Open Dface Database Error", EASYLITE_ERR_OPEN_DB);
	}
	unqlite_config(pDb, UNQLITE_CONFIG_DISABLE_AUTO_COMMIT, 0);
	return true;
}


bool EasyLiteEngine::closeDB() {
    int rc;
    rc = unqlite_close(pDb);
    if( rc != UNQLITE_OK ){
        return false;
//        throw EasyLiteException("Open Dface Database Error", EASYLITE_ERR_OPEN_DB);
    }
    return true;
}



bool EasyLiteEngine::dbExsitCollection(const string &collection_name) {

#define SCRIPT_EXSIT_COLLECTION \
" $ret = 0; "\
"if( db_exists($collection_name) ){"\
"    $ret = 1; "\
"}"\

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pObject;;

    rc = unqlite_compile(pDb,SCRIPT_EXSIT_COLLECTION,sizeof(SCRIPT_EXSIT_COLLECTION)-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Sql Compile Error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    unqlite_vm_exec(pVm);
    pObject = unqlite_vm_extract_variable(pVm,"ret");
    int local_ret = 0;
    if( pObject && unqlite_value_is_int(pObject) ){
        local_ret = unqlite_value_to_int(pObject);
        unqlite_vm_release_value(pVm,pObject);
    }
    unqlite_vm_release_value(pVm,pScalar);
    unqlite_vm_release(pVm);
    return local_ret == 0 ? false : true;

}


bool EasyLiteEngine::dbCreateCollection(const string &collection_name) {
#define SCRIPT_CREATE_COLLECTION \
" $ret = 0; "\
"if( db_exists($collection_name) ){"\
"  $ret = 1; "\
"}else{"\
"$rc = db_create($collection_name);"\
"if( $rc ){"\
"$ret = 1;"\
"}"\
"}"\

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pObject;
    rc = unqlite_compile(pDb,SCRIPT_CREATE_COLLECTION,sizeof(SCRIPT_CREATE_COLLECTION)-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Sql Compile Error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

//    pScalar = unqlite_vm_new_scalar(pVm);
//    if( pScalar == 0 ){
//        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//    }
//
//    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//    rc = unqlite_vm_config(
//            pVm,
//            UNQLITE_VM_CONFIG_CREATE_VAR,
//            "collection_name",
//            pScalar
//    );
//    if( rc != UNQLITE_OK ){
//        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//    }

    unqlite_commit(pDb);
    pObject = unqlite_vm_extract_variable(pVm,"ret");
    int local_ret = 0;
    if( pObject && unqlite_value_is_int(pObject) ){
        local_ret = unqlite_value_to_int(pObject);
        unqlite_vm_release_value(pVm, pObject);

    }

    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release(pVm);
    return local_ret == 0 ? false : true;
}




// int EasyLiteEngine::insertFace(const std::string &collection_name, const std::vector<float> &feature, const std::string &additional_json_str) {
// #define SCRIPT_INSERT_PREFIX "db_store($collection_name, "

// #define SCRIPT_INSERT_MIDDLE ");"

// #define SCRIPT_INSERT_SUFFIX "$last_id = db_last_record_id($collection_name);"

//     std::string ad_json_str = additional_json_str;
//     if(ad_json_str.empty()){
//         ad_json_str = "{}";
//     }

//     int size_feature = feature.size();
// //    char* feature_buff = (char*)malloc(size_feature*4+1);
// //    memset(feature_buff, 0, size_feature*8+10);
// //    for(int i=0; i<size_feature; ++i){
// //        float tmp = feature[i];
// //        ucf u;
// //        u.f = tmp;
// //        for(int j=0; j<4; ++j){
// //            feature_buff[4*i+j] = u.c[j];
// //        }
// //    }

// //    StaticJsonBuffer<JSON_BUFF_SIZE>* jsonBuffer = new StaticJsonBuffer<JSON_BUFF_SIZE>;
//     DynamicJsonBuffer jsonBuffer;

//     JsonObject& root = jsonBuffer.parseObject(ad_json_str.c_str());
//     if(&root == NULL){
//         return -1;
//     }

//     root["__feature_size"] = size_feature;
// //    JsonObject& face_feature = root.createNestedObject("feature");
//     JsonArray& data= root.createNestedArray("__feature");
//     for(int i=0; i<size_feature; ++i){
//         data.add(feature[i]);
//     }

// //    std::string base64_str;
// //    std::string feature_str(feature_buff);
// //
// //    Base64::Encode(feature_str, &base64_str);
// //    root["__feature"] = base64_str;

//     std::string jstr;
//     root.printTo(jstr);

//     ostringstream script_stream;
//     script_stream << SCRIPT_DBSTORE_COMMON;
//     script_stream << SCRIPT_INSERT_PREFIX;
//     script_stream << jstr;
//     script_stream << SCRIPT_INSERT_MIDDLE;
//     script_stream << SCRIPT_INSERT_SUFFIX;

// 	//    std::cout << script_stream.str() << std::endl;

//     int rc;
//     unqlite_vm *pVm;
//     unqlite_value *pScalar, *pObject;

//     rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
//     if( rc != UNQLITE_OK ){
//         /* Compile error, extract the compiler error log */
//         const char *zBuf;
//         int iLen;
//         /* Extract error log */
//         unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
//         if( iLen > 0 ){
//             puts(zBuf);
//         }
//         throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
//     }

//     pScalar = unqlite_vm_new_scalar(pVm);
//     if( pScalar == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "collection_name",
//             pScalar
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }

// 	rc = unqlite_vm_exec(pVm);
// 	if (rc != UNQLITE_OK) {
// 		throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
// 	}

//     unqlite_commit(pDb);

//     unqlite_vm_release_value(pVm, pScalar);

//     pObject = unqlite_vm_extract_variable(pVm,"last_id");

//     long ret = -1;

//     if( pObject && unqlite_value_is_numeric(pObject) ) {
//         ret = unqlite_value_to_int64(pObject);
//         unqlite_vm_release_value(pVm, pObject);
// //         unqlite_array_walk(pObject, JsonObjectWalker, 0);
//     }
//     unqlite_vm_release(pVm);
//     return (int)ret;
// //    unqlite_close(pDb);

// }


// bool EasyLiteEngine::updateFace(const std::string &collection_name, int id, const std::vector<float> &feature, const std::string &additional_json_str){
// #define SCRIPT_UPDATE_PREFIX "$data = db_update_record($collection_name, $record_id, "
// #define SCRIPT_UPDATE_MIDDLE ");"

//     std::string ad_json_str = additional_json_str;
//     if(ad_json_str.empty()){
//         ad_json_str = "{}";
//     }

//     int size_feature = feature.size();
// //    char* feature_buff = (char*)malloc(size_feature*4+1);
// //    memset(feature_buff, 0, size_feature*8+10);
// //    for(int i=0; i<size_feature; ++i){
// //        float tmp = feature[i];
// //        ucf u;
// //        u.f = tmp;
// //        for(int j=0; j<4; ++j){
// //            feature_buff[4*i+j] = u.c[j];
// //        }
// //    }

// //    StaticJsonBuffer<JSON_BUFF_SIZE>* jsonBuffer = new StaticJsonBuffer<JSON_BUFF_SIZE>;
//     DynamicJsonBuffer jsonBuffer;

//     JsonObject& root = jsonBuffer.parseObject(ad_json_str.c_str());
//     if(&root == NULL){
//         return false;
//     }

//     root["__feature_size"] = size_feature;
// //    JsonObject& face_feature = root.createNestedObject("feature");
//     JsonArray& data= root.createNestedArray("__feature");
//     for(int i=0; i<size_feature; ++i){
//         data.add(feature[i]);
//     }
//     std::string jstr;
//     root.printTo(jstr);

//     ostringstream script_stream;
//     script_stream << SCRIPT_DBSTORE_COMMON;
//     script_stream << SCRIPT_UPDATE_PREFIX;
//     script_stream << jstr;
//     script_stream << SCRIPT_UPDATE_MIDDLE;

//     std::cout << script_stream.str() << std::endl;

//     int rc;
//     unqlite_vm *pVm;
//     unqlite_value *pScalar, *pScalar2, *pObject;

//     rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
//     if( rc != UNQLITE_OK ){
//         /* Compile error, extract the compiler error log */
//         const char *zBuf;
//         int iLen;
//         /* Extract error log */
//         unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
//         if( iLen > 0 ){
//             puts(zBuf);
//         }
//         throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
//     }

//     pScalar = unqlite_vm_new_scalar(pVm);
//     if( pScalar == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }



//     unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "collection_name",
//             pScalar
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }


//     pScalar2 = unqlite_vm_new_scalar(pVm);
//     if( pScalar2 == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $record_id", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_int64(pScalar2, (long)id);
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "record_id",
//             pScalar2
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }


//     rc = unqlite_vm_exec(pVm);
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
//     }

//     unqlite_commit(pDb);
//     pObject = unqlite_vm_extract_variable(pVm,"data");

//     bool ret = false;

//     if( pObject && unqlite_value_is_bool(pObject) ) {
//         ret = unqlite_value_to_bool(pObject);
//         unqlite_vm_release_value(pVm, pObject);
//     }

//     unqlite_vm_release_value(pVm, pScalar);
//     unqlite_vm_release_value(pVm, pScalar2);
//     unqlite_vm_release(pVm);
//     return ret;

// }


// int EasyLiteEngine::fetchFaceById(const std::string &collection_name, const int id, std::vector<float> &out_feature, std::string &out_json_str) {
// #define SCRIPT_FETCHBYID \
//     "$data = db_fetch_by_id($collection_name, $record_id);" \

//     out_feature.clear();
//     out_json_str.clear();

//     ostringstream script_stream;
//     script_stream << SCRIPT_FETCHBYID;

//     int rc;
//     unqlite_vm *pVm;
//     unqlite_value *pScalar, *pScalar2, *pObject;

// 	rc = unqlite_compile(pDb, script_stream.str().c_str(), script_stream.str().size() - 1, &pVm);
// 	if (rc != UNQLITE_OK) {
// 		/* Compile error, extract the compiler error log */
// 		const char *zBuf;
// 		int iLen;
// 		/* Extract error log */
// 		unqlite_config(pDb, UNQLITE_CONFIG_JX9_ERR_LOG, &zBuf, &iLen);
// 		if (iLen > 0) {
// 			puts(zBuf);
// 		}
// 		throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
// 	}

//     pScalar = unqlite_vm_new_scalar(pVm);
//     if( pScalar == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "collection_name",
//             pScalar
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }


//     pScalar2 = unqlite_vm_new_scalar(pVm);
//     if( pScalar2 == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_int64(pScalar2, (long)id);
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "record_id",
//             pScalar2
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }

//     rc = unqlite_vm_exec(pVm);
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
//     }

//     pObject = unqlite_vm_extract_variable(pVm,"data");
//     int ret = 0;

//     unqlite_vm_release_value(pVm, pScalar);
//     unqlite_vm_release_value(pVm, pScalar2);

//     if( pObject && unqlite_value_is_json_object(pObject) ) {
//         std::string ret_jstr = unqlite_value_to_string(pObject, 0);
//         unqlite_vm_release_value(pVm, pObject);

//         out_json_str =  ret_jstr;
// //        StaticJsonBuffer<JSON_BUFF_SIZE>* jsonBuffer = new StaticJsonBuffer<JSON_BUFF_SIZE>;
//         DynamicJsonBuffer jsonBuffer;
//         JsonObject& root = jsonBuffer.parseObject(ret_jstr);
//         if(&root == NULL){
//             return  ret;
//         }

//         JsonArray& data = root["__feature"];

// //        int feature_size = root["__feature_size"];
// //        if(feature_size == NULL || feature_size == 0){
// //            return ret;
// //        }
//         for(int i=0; i<data.size(); ++i){
//             float fva = data[i];
//             out_feature.push_back(fva);
//         }
// //        std::string feature_str;
// //        Base64::Decode(base64_fstr, &feature_str);
// //        const char* feature_cstr = feature_str.c_str();
// //        int size_feature = (int)(feature_str.size()/4);
// //        std::vector<float> feature_v;
// //        for(int i=0; i<size_feature; i+=1){
// //            ucf u;
// //            u.c[0] = feature_cstr[4*i];
// //            u.c[1] = feature_cstr[4*i+1];
// //            u.c[2] = feature_cstr[4*i+2];
// //            u.c[3] = feature_cstr[4*i+3];
// //            out_feature.push_back(u.f);
// //        }
//          ret = 1;
//      }
//     unqlite_vm_release(pVm);
//     return ret;
// }


// bool EasyLiteEngine::deleteFaceById(const std::string &collection_name, const int id) {
// #define SCRIPT_DROPBYID \
//     "$data = db_drop_record($collection_name, $record_id);" \

//     ostringstream script_stream;
//     script_stream << SCRIPT_DROPBYID;

//     int rc;
//     unqlite_vm *pVm;
//     unqlite_value *pScalar, *pScalar2, *pObject;

//     rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
//     if( rc != UNQLITE_OK ){
//         /* Compile error, extract the compiler error log */
//         const char *zBuf;
//         int iLen;
//         /* Extract error log */
//         unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
//         if( iLen > 0 ){
//             puts(zBuf);
//         }
//         throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
//     }

//     pScalar = unqlite_vm_new_scalar(pVm);
//     if( pScalar == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "collection_name",
//             pScalar
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }


//     pScalar2 = unqlite_vm_new_scalar(pVm);
//     if( pScalar2 == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_int64(pScalar2, (long)id);
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "record_id",
//             pScalar2
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }

//     rc = unqlite_vm_exec(pVm);
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
//     }
//     unqlite_commit(pDb);
//     pObject = unqlite_vm_extract_variable(pVm,"data");

//     bool ret = false;

//     if( pObject && unqlite_value_is_bool(pObject) ) {
//         ret = unqlite_value_to_bool(pObject);
//         unqlite_vm_release_value(pVm, pObject);
//     }

//     unqlite_vm_release_value(pVm, pScalar);
//     unqlite_vm_release_value(pVm, pScalar2);
//     unqlite_vm_release(pVm);
//     return ret;
// }


// int EasyLiteEngine::fetchAllFace2Memory(const std::string &collection_name, std::vector<int> &out_idx,
//                                 std::vector<std::vector<float>> &out_feature, std::string condition_str) {


// #define COND_FUNC \
// "$zCallback = " \

// #define SCRIPT_FETCH_ALL_FACE \
//  "$data = db_fetch_all($collection_name,$zCallback);" \


//     std::string condf;
//     if(condition_str.empty()){
//         condf = "function($rec){return TRUE;}";
//     }else{
//         condf = condition_str;
//     }

// 	stringstream script_stream;
// 	script_stream << COND_FUNC;
// 	script_stream << condf;
// 	script_stream << ";";
// 	script_stream << SCRIPT_FETCH_ALL_FACE;

//     unqlite_lib_init();
//     out_feature.clear();
//     out_idx.clear();

//     int rc;
//     unqlite_vm *pVm;
//     unqlite_value *pScalar, *pScalar2, *pObject;

//     rc = unqlite_compile(pDb,script_stream.str().c_str(), script_stream.str().size()-1,&pVm);
//     if( rc != UNQLITE_OK ){
//         /* Compile error, extract the compiler error log */
//         const char *zBuf;
//         int iLen;
//         /* Extract error log */
//         unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
//         string lest(zBuf);
//         if( iLen > 0 ){
//             puts(zBuf);
//         }
//         throw EasyLiteException(zBuf, EASYLITE_ERR_NOSQL_COMPILE);
//     }

//     pScalar = unqlite_vm_new_scalar(pVm);
//     if( pScalar == 0 ){
//         throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
//     }

//     unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
//     rc = unqlite_vm_config(
//             pVm,
//             UNQLITE_VM_CONFIG_CREATE_VAR,
//             "collection_name",
//             pScalar
//     );
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
//     }


// //    rc = unqlite_vm_config(pVm,UNQLITE_VM_CONFIG_OUTPUT,VmOutputConsumer,0);
// //    if( rc != UNQLITE_OK ){
// //        throw EasyLiteException("Error while installing VM console", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
// //    }


//     rc = unqlite_vm_exec(pVm);
//     if( rc != UNQLITE_OK ){
//         throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
//     }

//     pObject = unqlite_vm_extract_variable(pVm,"data");

//     fetch_face_tmp.clear();
//     fetch_face_idx_tmp.clear();


//     if( pObject) {
//         unqlite_array_walk(pObject, JsonObjectWalker4FetchFace, 0);
//         unqlite_vm_release_value(pVm, pObject);
//     }


//     unqlite_vm_release_value(pVm, pScalar);
//     unqlite_vm_release(pVm);


//     out_idx.reserve(fetch_face_idx_tmp.size());
//     out_idx.assign(fetch_face_idx_tmp.begin(), fetch_face_idx_tmp.end());

//     out_feature.reserve(fetch_face_tmp.size());
//     out_feature.assign(fetch_face_tmp.begin(), fetch_face_tmp.end());

//     long ret = out_idx.size();

//     if(true){
//         fetch_face_idx_tmp.clear();
//         fetch_face_tmp.clear();
//         std::vector<int> free_fetch_idx_tmp;
//         std::vector<std::vector<float>> free_fetch_face_tmp;
//         fetch_face_idx_tmp.swap(free_fetch_idx_tmp);
//         fetch_face_tmp.swap(free_fetch_face_tmp);
//     }

//     return (int)ret;
// }


int EasyLiteEngine::totalCollectionRecord(const std::string &collection_name) {
#define SCRIPT_TOTAL_COLLECTION \
 "$data = db_total_records($collection_name);" \

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pScalar2, *pObject;

    rc = unqlite_compile(pDb,SCRIPT_TOTAL_COLLECTION, sizeof(SCRIPT_TOTAL_COLLECTION)-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }
    pObject = unqlite_vm_extract_variable(pVm,"data");


    long ret;
    if( pObject && unqlite_value_is_numeric(pObject) ) {
        ret = unqlite_value_to_int64(pObject);
        unqlite_vm_release_value(pVm, pObject);
    }

    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release(pVm);

    return (int)ret;
}


int EasyLiteEngine::insert(const std::string &collection_name, const std::string &json_str) {
#define SCRIPT_INSERT_PREFIX "db_store($collection_name, "

#define SCRIPT_INSERT_MIDDLE ");"

#define SCRIPT_INSERT_SUFFIX "$last_id = db_last_record_id($collection_name);"

    std::string ad_json_str = json_str;
    if(ad_json_str.empty()){
        return -1;
    }

//    StaticJsonBuffer<JSON_BUFF_SIZE> jsonBuffer;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(ad_json_str.c_str());

    if(&root == NULL){
        return -1;
    }

    std::string jstr;
    root.printTo(jstr);

    ostringstream script_stream;
    script_stream << SCRIPT_DBSTORE_COMMON;
    script_stream << SCRIPT_INSERT_PREFIX;
    script_stream << jstr;
    script_stream << SCRIPT_INSERT_MIDDLE;
    script_stream << SCRIPT_INSERT_SUFFIX;

    std::cout << script_stream.str() << std::endl;

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pObject;

    rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }

//    unqlite_commit(pDb);
    pObject = unqlite_vm_extract_variable(pVm,"last_id");
    long ret = -1;
    if( pObject && unqlite_value_is_numeric(pObject) ) {
        ret = unqlite_value_to_int64(pObject);
//         unqlite_array_walk(pObject, JsonObjectWalker, 0);
        unqlite_vm_release_value(pVm, pObject);
    }

    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release(pVm);

    return (int)ret;

}


bool EasyLiteEngine::update(const std::string &collection_name, int id, const std::string &json_str) {
#define SCRIPT_UPDATE_PREFIX "$data = db_update_record($collection_name, $record_id, "

#define SCRIPT_UPDATE_MIDDLE ");"

    std::string ad_json_str = json_str;
    if(ad_json_str.empty()){
        return -1;
    }

//    StaticJsonBuffer<JSON_BUFF_SIZE> jsonBuffer;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(ad_json_str.c_str());

    if(&root == NULL){
        return -1;
    }

    std::string jstr;
    root.printTo(jstr);

    ostringstream script_stream;
    script_stream << SCRIPT_DBSTORE_COMMON;
    script_stream << SCRIPT_UPDATE_PREFIX;
    script_stream << jstr;
    script_stream << SCRIPT_UPDATE_MIDDLE;

    std::cout << script_stream.str() << std::endl;

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pScalar2, *pObject;

    rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }



    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }


    pScalar2 = unqlite_vm_new_scalar(pVm);
    if( pScalar2 == 0 ){
        throw EasyLiteException("Cannot create foreign variable $record_id", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_int64(pScalar2, (long)id);
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "record_id",
            pScalar2
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }


    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }

    unqlite_commit(pDb);
    pObject = unqlite_vm_extract_variable(pVm,"data");

    bool ret = false;

    if( pObject && unqlite_value_is_bool(pObject) ) {
        ret = unqlite_value_to_bool(pObject);
        unqlite_vm_release_value(pVm, pObject);
    }

    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release_value(pVm, pScalar2);
    unqlite_vm_release(pVm);
    return ret;
}


int EasyLiteEngine::fetchById(const std::string &collection_name, const int id, std::string &out_json_str) {

    ostringstream script_stream;
    script_stream << SCRIPT_FETCHBYID;

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pScalar2, *pObject;

    rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }


    pScalar2 = unqlite_vm_new_scalar(pVm);
    if( pScalar2 == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_int64(pScalar2, (long)id);
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "record_id",
            pScalar2
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }

    pObject = unqlite_vm_extract_variable(pVm,"data");

    int ret = 0;
    if( pObject && unqlite_value_is_json_object(pObject) ) {
        out_json_str = unqlite_value_to_string(pObject, 0);
        ret = 1;
        unqlite_vm_release_value(pVm, pObject);
//         unqlite_array_walk(pObject, JsonObjectWalker, 0);
    }
    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release_value(pVm, pScalar2);
    unqlite_vm_release(pVm);
    return ret;
}


bool EasyLiteEngine::deleteById(const std::string &collection_name, const int id) {
#define SCRIPT_DROPBYID \
    "$data = db_drop_record($collection_name, $record_id);" \

    ostringstream script_stream;
    script_stream << SCRIPT_DROPBYID;

    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pScalar2, *pObject;

    rc = unqlite_compile(pDb,script_stream.str().c_str(),script_stream.str().size()-1,&pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }


    pScalar2 = unqlite_vm_new_scalar(pVm);
    if( pScalar2 == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_int64(pScalar2, (long)id);
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "record_id",
            pScalar2
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $record_id", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }
    unqlite_commit(pDb);
    pObject = unqlite_vm_extract_variable(pVm,"data");

    bool ret = false;
    if( pObject && unqlite_value_is_bool(pObject) ) {
        ret = unqlite_value_to_bool(pObject);
        unqlite_vm_release_value(pVm, pObject);
    }
    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release_value(pVm, pScalar2);
    unqlite_vm_release(pVm);
    return ret;

}


int EasyLiteEngine::fetchAll(const std::string &collection_name, std::vector<std::string> &out_json,
                     string condition_str) {

//#define SCRIPT_FETCH_ALL_FACE \
// "$data = db_fetch_all($collection_name,$zCallback);" \
// "$zCallback = function($rec){" \
// "    return TRUE;"\
// "};"\

#define COND_FUNC \
"$zCallback = " \

#define SCRIPT_FETCH_ALL_FACE \
 "$data = db_fetch_all($collection_name,$zCallback);" \


    out_json.clear();

    std::string condf;
    if(condition_str.empty()){
        condf = "function($rec){return TRUE;}";
    }else{
        condf = condition_str;
    }

    stringstream script_stream;
    script_stream << COND_FUNC;
    script_stream << condf;
    script_stream << ";";
    script_stream << SCRIPT_FETCH_ALL_FACE;


    std::string ct = script_stream.str();


    int rc;
    unqlite_vm *pVm;
    unqlite_value *pScalar, *pScalar2, *pObject;

    rc = unqlite_compile(pDb,script_stream.str().c_str(), script_stream.str().size()-1, &pVm);
    if( rc != UNQLITE_OK ){
        /* Compile error, extract the compiler error log */
        const char *zBuf;
        int iLen;
        /* Extract error log */
        unqlite_config(pDb,UNQLITE_CONFIG_JX9_ERR_LOG,&zBuf,&iLen);
        if( iLen > 0 ){
            puts(zBuf);
        }
        throw EasyLiteException("Nosql compile error", EASYLITE_ERR_NOSQL_COMPILE);
    }

    pScalar = unqlite_vm_new_scalar(pVm);
    if( pScalar == 0 ){
        throw EasyLiteException("Cannot create foreign variable $collection_name", EASYLITE_ERR_CREATE_FOREIGN_VARIABLE);
    }

    unqlite_value_string(pScalar, collection_name.c_str(), collection_name.size());
    rc = unqlite_vm_config(
            pVm,
            UNQLITE_VM_CONFIG_CREATE_VAR,
            "collection_name",
            pScalar
    );
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Error while installing variable $collection_name", EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE);
    }

    rc = unqlite_vm_exec(pVm);
    if( rc != UNQLITE_OK ){
        throw EasyLiteException("Execute nosql db error", EASYLITE_ERR_NOSQL_EXECUTE);
    }

    pObject = unqlite_vm_extract_variable(pVm,"data");

    fetch_all_tmp.clear();

    if( pObject) {
        unqlite_array_walk(pObject, JsonObjectWalker, 0);
        unqlite_vm_release_value(pVm, pObject);

    }

    unqlite_vm_release_value(pVm, pScalar);
    unqlite_vm_release(pVm);

    out_json.reserve(fetch_all_tmp.size());
    out_json.assign(fetch_all_tmp.begin(), fetch_all_tmp.end());

    long ret = out_json.size();

    if(true){
        fetch_all_tmp.clear();
        std::vector<std::string> free_fetch_all_tmp;
        fetch_all_tmp.swap(free_fetch_all_tmp);
    }

    return (int)ret;

}



bool EasyLiteEngine::kv_store(const std::string &key, std::string value){
    int rc;
    rc = unqlite_kv_store(pDb, key.c_str(), key.size(), value.c_str(), value.size());
    if( rc != UNQLITE_OK ){
        return false;
    }
    return true;
}

bool EasyLiteEngine::kv_fetch(const std::string &key, std::string &out_value) {
	int rc;
	unqlite_int64 nBytes;  //Data length
	char *zBuf;

    rc = unqlite_kv_fetch(pDb, (void*)key.c_str(), -1, NULL, &nBytes);
    if( rc != UNQLITE_OK ){
        return false;
    }

    //Allocate a buffer big enough to hold the record content
    zBuf = (char *)malloc(nBytes);
    if( zBuf == NULL ){ return false; }

    //Copy record content in our buffer
    unqlite_kv_fetch(pDb, key.c_str(), -1, zBuf, &nBytes);
    out_value.assign(zBuf, nBytes);
    free(zBuf);
    return true;
}


bool EasyLiteEngine::kv_delete(const std::string &key){
    int rc;
    rc = unqlite_kv_delete(pDb,key.c_str(),-1);
    if( rc != UNQLITE_OK ){
//        const char *zBuf;
//        int iLen;
//        /* Extract database error log */
//        unqlite_config(pDb,UNQLITE_CONFIG_ERR_LOG,&zBuf,&iLen);
//        if( iLen > 0 ){
//            puts(zBuf);
//        }
        if( rc != UNQLITE_BUSY && rc != UNQLITE_NOTFOUND ){
            /* Rollback */
            unqlite_rollback(pDb);
        }
        return false;
    }
    return true;
}


#ifdef __cplusplus
extern "C" {
EASYLITE_EXPORTS EasyLite* create_easylite(){
//        return FaceDetect::GetInstance(model_path);
    return new EasyLiteEngine();
};
EASYLITE_EXPORTS void destroy_easylite(EasyLite *p){
    delete p;
};

}
#endif
