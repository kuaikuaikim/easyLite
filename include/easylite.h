//
// Created by kkkim on 11/24/18.
//

#ifndef EASYLITE_DATABASE_H
#define EASYLITE_DATABASE_H

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

/**
* @brief EasyLite数据库API抽象层(EasyLite)
* 非关系nosql人脸数据库(单文件)，支持动态添加属性，适应任意应用场景，内置图灵完备微内核虚拟机，支持脚本导入导出数据
*
*/
class EasyLite{
public:
    EasyLite() {};
    virtual ~EasyLite() { return; };

    /**
    * 打开数据库
    * @param[in] db_file 数据库文件路径
    * @return 是否打开成功
    * @note  (如果数据库文件不存在,系统会自动创建)
    */
    virtual bool openDB(const std::string &db_file) = 0;

    /**
    * 关闭数据库
    * @return 是否关闭成功
    * @note  (程序退出，关闭数据库)
    */
    virtual bool closeDB() = 0;

    /**
    * 查询数据库是否存在某个集合(数据表)
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @return 是否存在
    * @note  ()
    */
    virtual bool dbExsitCollection(const string &collection_name) = 0;

    /**
    * 在数据库中创建集合(数据表)
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @return 是否创建成功
    * @note  ()
    */
    virtual bool dbCreateCollection(const string &collection_name) = 0;


    /**
    * 查询某个集合的总记录数
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @return 总记录数
    * @note  ()
    */
    virtual int totalCollectionRecord(const std::string &collection_name) = 0;


    /**
    * 插入数据
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @param[in] json_str 数据json字符串，系统会自动解析json字符串，以动态字段的形式添加到集合中 <br/>
    * 例如: {"name":"xiaokong", "age":25, "sex": 0} <br/>
    * c/c++ json库推荐 ArduinoJson (https://github.com/bblanchon/ArduinoJson), 非常适合嵌入式物联网的json库 <br/>
    * StaticJsonBuffer<200> jsonBuffer; <br/>
    * JsonObject& root = jsonBuffer.parseObject(json); <br/>
    * const char* sensor = root["sensor"]; <br/>
    * long time          = root["time"]; <br/>
    * @return 返回插入数据库后对应的id
    * @note  (如果返回-1,则表示插入失败)
    */
    virtual int insert(const std::string &collection_name, const std::string &json_str) = 0;


    /**
    * 更新数据
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @param[in] id 数据id(插入数据库返回的id)
    * @param[in] json_str 数据json字符串，系统会自动解析json字符串，以动态字段的形式添加到集合中 <br/>
    * 例如: {"name":"xiaokong", "age":25, "sex": 0} <br/>
    * c/c++ json库推荐 ArduinoJson (https://github.com/bblanchon/ArduinoJson), 非常适合嵌入式物联网的json库 <br/>
    * StaticJsonBuffer<200> jsonBuffer; <br/>
    * JsonObject& root = jsonBuffer.parseObject(json); <br/>
    * const char* sensor = root["sensor"]; <br/>
    * long time          = root["time"]; <br/>
    * @return 返回成功标志
    * @note  ()
    */
    virtual bool update(const std::string &collection_name, int id, const std::string &json_str) = 0;


    /**
    * 查询数据
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @param[in] id 数据id(一般为数据库插入返回的id)
    * @param[out] out_feature [out]输出的的人脸特征
    * @param[out] out_json_str [out]输出的json字符串
    * @return 查询返回的条数
    * @note  (如果返回0，则表示查询结果为空)
    */
    virtual int fetchById(const std::string &collection_name, const int id, std::string &out_json_str) = 0;

    /**
    * 删除数据
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @param[in] id 数据id
    * @return 是否删除成功
    * @note  ()
    */
    virtual bool deleteById(const std::string &collection_name, const int id) = 0;


    /**
    * 查询某个集合的所有记录
    * @param[in] collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
    * @param[out] out_json [out]输出的json字符串数组
    * @param[in] condition_func_str 条件查询函数脚本字符串  <br/>
    * 脚本类似于PHP，一般通用格式: <br/>
    * function($rec){ if( $rec.__id == 800 ){ return TRUE; } return FALSE; }  <br/>
    * @return 加载记录数
    * @note  ()
    */
    virtual int fetchAll(const std::string &collection_name, std::vector<std::string> &out_json, std::string condition_func_str = "") = 0;


    /**
    * key-value 键值对保存
    * @param[in] key 键名称
    * @param[in] value 值
    * @return 是否保存成功标志
    * @note()
    */
    virtual bool kv_store(const std::string &key, std::string value) = 0;


    /**
　　 * 键值对查询
　　 * @param[in] key 键名称
　　 * @param[out] out_value 查询到的值
　　 * @return 是否查询成功标志
　　 * @note()
　　 */
   virtual bool kv_fetch(const std::string &key, std::string &out_value) = 0;


    /**
　　 * 删除键值对
　　 * @param[in] key 键名称
　　 * @return 是否删除成功标志
　　 * @note()
　　 */
   virtual bool kv_delete(const std::string &key) = 0;

};


/**
* 创建人脸人脸数据库通道
*/
typedef EasyLite* CreateEasyLite();

/**
* 销毁人脸数据库通道
* @param[in] db 人脸数据库通道
*/
typedef void DestroyEasyLite(EasyLite* db);


#endif //EasyLite_DATABASE_H
