﻿//
// Created by asy on 10/20/18.
//

#ifndef EASYLITE_SYMBOL_H
#define EASYLITE_SYMBOL_H

#include "./easylite.h"
#include "./exception/easylite_exceptio.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef _WIN32
//加载动态库
static HMODULE db_handle = LoadLibraryEx("libeasylite.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
//加载动态库
static void *db_handle = dlopen("libeasylite.so", RTLD_LAZY);
#endif


/**
* 创建数据库
*/
inline EasyLite* load_easylite(){
#ifdef _WIN32
    if (db_handle == NULL)
    {
        throw "libeasylite.dll library failed to load";
    }
    //获得数据库工厂函数
    CreateEasyLite* createEasyLite = (CreateEasyLite*)GetProcAddress(db_handle, "create_easylite");
    if (createEasyLite == NULL) {
		throw "EasyLite failed to create";
    }
#else
    if (!db_handle)
    {
        throw "libeasylite.so library failed to load";
    }
    //获得数据库工厂函数
    CreateEasyLite* createEasyLite = (CreateEasyLite*)dlsym(db_handle, "create_easylite");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        throw "EasyLite failed to create";
    }
#endif
    return createEasyLite();
}

/**
* 销毁数据库
* @param p 数据库指针
*/
inline void destory_easylite(EasyLite* p){
#ifdef _WIN32
    //获得数据库通道工厂销毁函数
    DestroyEasyLite* destroyEasyLite = (DestroyEasyLite*)GetProcAddress(db_handle, "destroy_easylite");
    if (destroyDfaceDB == NULL) {
        throw "EasyLite failed to destory";
    }
#else
    //获得数据库通道工厂销毁函数
    DestroyEasyLite* destroyEasyLite = (DestroyEasyLite*)dlsym(db_handle, "destroy_easylite");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        throw "EasyLite failed to destory";
    }
#endif
    //销毁数据库通道
    destroyEasyLite(p);
}



#endif //EasyLite_SYMBOL_H
