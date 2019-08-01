//
// Created by asy on 11/24/18.
//

#ifndef EASYLITE_EXCEPTIO_H
#define EASYLITE_EXCEPTIO_H
#include <iostream>
#include <exception>
#include <unistd.h>
#include "../def.h"
using namespace std;

/**
* @brief EasyLite异常类
*
*/
class EASYLITE_EXPORTS EasyLiteException : public std::exception {
public:
/**
* @brief 错误码
*/
const int code;
/**
* @brief 错误信息
*/
const char* msg;

public:
EasyLiteException(const char* msg, const int code);
const char* what () const throw ();
/**
* 返回错误信息
*/
const char* getMsg() const;
/**
* 返回错误码
*/
const int getCode() const;

};

inline EasyLiteException::EasyLiteException(const char *msg, const int code):msg(msg), code(code){

}

inline const char* EasyLiteException::what() const throw(){
#ifdef _WIN32
	return msg;
#else // _WIN32
    int len = sizeof(msg) + sizeof(code) + 10;
    char* buff = (char*)malloc(len);
    sprintf(buff, "%s, %d", msg, code);
    return buff;
#endif
}

inline const int EasyLiteException::getCode() const {
    return code;
}

inline const char* EasyLiteException::getMsg() const {
    return msg;
}



#endif //EASYLITE_EXCEPTIO_H
