cmake_minimum_required(VERSION 3.6)
project(easylite)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -pthread -fopenmp")

if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)


option(GEN_JAVA_API "Generate Java API" ON)

SET(JDK_PATH /usr/local/java/jdk1.8.0_181/)

SET(EASY_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(EASY_INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET(EASYLITE_SRCS ${EASY_SOURCE_ROOT}/easylite_engine.cpp ${EASY_INCLUDE_ROOT}/unqlite.c ${EASY_INCLUDE_ROOT}/jx9.c)

include_directories(${EASY_INCLUDE_ROOT})

add_library(easylite SHARED ${EASYLITE_SRCS})

if(GEN_JAVA_API)
    include_directories(${JDK_PATH}/include)
    include_directories(${JDK_PATH}/include/linux)
    add_library(easylitejni SHARED ${EASY_SOURCE_ROOT}/jni/easylite_jni.cpp ${EASYLITE_SRCS})
endif()


if(GEN_JAVA_API)
    install(TARGETS easylitejni LIBRARY DESTINATION lib)
endif()

install(TARGETS easylite LIBRARY DESTINATION lib)

install(FILES
        ${DFACE_INCLUDE_ROOT}/ArduinoJson-v5.13.3.h
        ${DFACE_INCLUDE_ROOT}/common.h
        ${DFACE_INCLUDE_ROOT}/def.h
        ${DFACE_INCLUDE_ROOT}/easylite.h
        ${DFACE_INCLUDE_ROOT}/symbol.h
        ${DFACE_INCLUDE_ROOT}/easylite_api.h
        DESTINATION include/easylite
        )

