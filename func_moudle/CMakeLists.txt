cmake_minimum_required(VERSION 3.5)
project(c++project)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CPP_FLAGS} -g -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # require at least gcc 4.8
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC is too old, please install a newer version supporting C++11")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    # require at least clang 3.3
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "Clang is too old, please install a newer version supporting C++11")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()

add_executable(test main.cpp timestamp_ms.h raptorkv.h raptorkv.cpp my_stl.h my_stl.cpp com_func.h com_func.cpp)

#说如果缺少动态库，就加上这些
set(DYNAMIC_LIB
        rt
        ssl
        crypto
        dl
        z
        )

target_link_libraries(test
        "-Xlinker \"-(\""
        ${DYNAMIC_LIB}
        "-Xlinker \"-)\"")
