//
// Created by asy on 11/24/18.
//

#ifndef EASYLITE_COMMON_H
#define EASYLITE_COMMON_H

#include <stdio.h>
#include <vector>

/**
* @brief EASYLITE错误码
*/
typedef enum EASYLITE_ERROR_CODES {
            EASYLITE_SUCCESS = 0, /**< 成功 */
            EASYLITE_ERR_OPEN_DB = 7, /**< 打开数据库出错 */
            EASYLITE_ERR_CREATE_FOREIGN_VARIABLE = 8, /**< 数据库内置错误,创建脚本变量出错 */
            EASYLITE_ERR_INSTALL_FOREIGN_VARIABLE = 9, /**< 数据库内置错误,装载脚本变量出错 */
            EASYLITE_ERR_NOSQL_COMPILE = 10, /**< 数据库内置错误, 编译nosql脚本出错 */
            EASYLITE_ERR_NOSQL_EXECUTE = 11 /**< 数据库内置错误,执行nosql脚本出错 */
}EASYLITE_STATUS;



#endif //EASYLITE_COMMON_H
