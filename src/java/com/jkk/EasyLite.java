package com.jkk;

import java.util.List;

public class EasyLite {
    public native int identify();

    public native boolean initLoad();

    public native boolean uninitLoad();

    /**
     * 打开数据库
     * @param db_file 数据库文件路径
     * @return 是否打开成功
     * @note  (如果数据库文件不存在,系统会自动创建)
     */
    public native boolean openDB(String db_file);

    /**
     * 关闭数据库
     * @return 是否关闭成功
     * @note  (程序退出，关闭数据库)
     */
    public native boolean closeDB();

    /**
     * 查询数据库是否存在某个集合(数据表)
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @return 是否存在
     * @note  ()
     */
    public native boolean dbExsitCollection(String collection_name);

    /**
     * 在数据库中创建集合(数据表)
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @return 是否创建成功
     * @note  ()
     */
    public native boolean dbCreateCollection(String collection_name);


    /**
     * 查询某个集合的总记录数
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @return 总记录数
     * @note  ()
     */
    public native int totalCollectionRecord(String collection_name);


    /**
     * 插入数据
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @param json 数据json字符串，系统会自动解析json字符串，以动态字段的形式添加到集合中 <br/>
     * 例如: {"name":"xiaokong", "age":25, "sex": 0} <br/>
     * c/c++ json库推荐 ArduinoJson (https://github.com/bblanchon/ArduinoJson), 非常适合嵌入式物联网的json库 <br/>
     * StaticJsonBuffer<200> jsonBuffer; <br/>
     * JsonObject& root = jsonBuffer.parseObject(json); <br/>
     * const char* sensor = root["sensor"]; <br/>
     * long time          = root["time"]; <br/>
     * @return 返回插入数据库后对应的id
     * @note  (如果返回-1,则表示插入失败)
     */
    public native int insert(String collection_name, String json);


    /**
     * 查询数据
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @param id 数据id
     * @return 查询返回的条数
     * @note  (如果返回0，则表示查询结果为空)
     */
    public native String fetchById(String collection_name, int id);

    /**
     * 删除数据
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @param id 数据id
     * @return 是否删除成功
     * @note  ()
     */
    public native boolean deleteById(String collection_name, int id);


    /**
     * 查询某个集合的所有记录
     * @param collection_name 集合名称(类似于数据表)，可以任意字符串，例如 white_list(白名单), black_list(黑名单), vip等
     * @param condition_func_str 条件查询函数脚本字符串
     * 脚本类似于PHP，一般通用格式: <br/>
     * function($rec){ if( $rec.__id == 800 ){ return TRUE; } return FALSE; }  <br/>
     * @return 加载记录数
     * @note  ()
     */
    public native List<String> fetchAll(String collection_name,  String condition_func_str);

}

