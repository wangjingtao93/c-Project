cmake_minimum_required(VERSION 2.8.10)
project(KvMoudle)

#新增
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EXAMPLE_LINK_SO "Whether examples are linked dynamically" OFF)
option(LINK_TCMALLOC "Link tcmalloc if possible" ON)

# 新增，LINK_JEMALLOC貌似用不到
option(LINK_JEMALLOC "Link jemalloc if possible" OFF)
message("LINK_TCMALLOC" ${LINK_TCMALLOC})

# 不需要
#execute_process(
#    COMMAND bash -c "find ${CMAKE_SOURCE_DIR}/../.. -type d -path \"*output/include/braft\" | xargs dirname | xargs dirname | tr -d '\n'"
#    OUTPUT_VARIABLE OUTPUT_PATH
#)

#set(CMAKE_PREFIX_PATH ${OUTPUT_PATH})

include(FindThreads)
include(FindProtobuf)

if (NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_filename_component(PROTO_LIB_DIR ${PROTOBUF_LIBRARY} DIRECTORY)
    set (PROTOBUF_PROTOC_EXECUTABLE "${PROTO_LIB_DIR}/../bin/protoc")
endif()

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER common/kv_interface.proto)
# include PROTO_HEADER
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 新增GLOG
find_path(GLOG_INCLUDE_PATH NAMES glog/logging.h)
find_library(GLOG_LIB NAMES glog)
if((NOT GLOG_INCLUDE_PATH) OR (NOT GLOG_LIB))
    message(FATAL_ERROR "Fail to find glog")
endif()
include_directories(${GLOG_INCLUDE_PATH})

find_path(BRPC_INCLUDE_PATH NAMES brpc/server.h)
if(EXAMPLE_LINK_SO)
    find_library(BRPC_LIB NAMES brpc)
    find_library(BRAFT_LIB NAMES braft)
    find_library(YAML_CPP_LIB NAMES yaml-cpp)#新增
else()
    find_library(BRPC_LIB NAMES libbrpc.a brpc)
    find_library(BRAFT_LIB NAMES libbraft.a braft)
    find_library(YAML_CPP_LIB NAMES yaml-cpp)#新增
endif()

if((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
    message(FATAL_ERROR "Fail to find brpc")
endif()
include_directories(${BRPC_INCLUDE_PATH})

# 新增yaml
find_path(YAML_CPP_INCLUDE_PATH NAMES yaml-cpp/yaml.h)
if ((NOT YAML_CPP_INCLUDE_PATH) OR (NOT YAML_CPP_LIB))
    message (FATAL_ERROR "Fail to find yaml-cpp")
endif()
include_directories(${YAML_CPP_INCLUDE_PATH})

find_path(GLOG_INCLUDE_PATH NAMES glog/logging.h)
find_library(GLOG_LIB NAMES glog)
if((NOT GLOG_INCLUDE_PATH) OR (NOT GLOG_LIB))
    message(FATAL_ERROR "Fail to find glog")
endif()
include_directories(${GLOG_INCLUDE_PATH})

find_path(BRAFT_INCLUDE_PATH NAMES braft/raft.h)
if ((NOT BRAFT_INCLUDE_PATH) OR (NOT BRAFT_LIB))
    message (FATAL_ERROR "Fail to find braft")
endif()
include_directories(${BRAFT_INCLUDE_PATH})

find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif()
include_directories(${GFLAGS_INCLUDE_PATH})

execute_process(
    COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
    OUTPUT_VARIABLE GFLAGS_NS
)
if(${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
    execute_process(
        COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
        OUTPUT_VARIABLE GFLAGS_NS
    )
endif()

if (LINK_TCMALLOC)
    find_path(GPERFTOOLS_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
    find_library(GPERFTOOLS_LIBRARIES NAMES tcmalloc_and_profiler)
    if (GPERFTOOLS_INCLUDE_DIR AND GPERFTOOLS_LIBRARIES)
        set(CMAKE_CXX_FLAGS "-DBRPC_ENABLE_CPU_PROFILER")
        include_directories(${GPERFTOOLS_INCLUDE_DIR})
    else ()
        set (GPERFTOOLS_LIBRARIES "")
    endif ()
endif ()

set(CMAKE_CPP_FLAGS "-DGFLAGS_NS=${GFLAGS_NS} -DRAPTOR_VERSION=\\\"${MY_VER}\\\"")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CPP_FLAGS} -g -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # require at least gcc 4.8
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC is too old, please install a newer version supporting C++11")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # require at least clang 3.3
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "Clang is too old, please install a newer version supporting C++11")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()

# 用不到
#if(CMAKE_VERSION VERSION_LESS "3.1.3")
#    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#    endif()
#    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#    endif()
#else()
#    set(CMAKE_CXX_STANDARD 11)
#    set(CMAKE_CXX_STANDARD_REQUIRED ON)
#endif()

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif()
include_directories(${LEVELDB_INCLUDE_PATH})

#新增RocksDB
find_path(ROCKSDB_INCLUDE_PATH NAMES rocksdb/db.h)
find_library(ROCKSDB_LIB NAMES rocksdb)
if ((NOT ROCKSDB_INCLUDE_PATH) OR (NOT ROCKSDB_LIB))
    message(FATAL_ERROR "Fail to find rocksdb")
endif()
include_directories(${ROCKSDB_INCLUDE_PATH})

add_executable(server server.cpp ${PROTO_SRC} ${PROTO_HEADER}
             kv_state_machine.h kv_state_machine.cpp
             kv_service_impl.h kv_service_impl.cpp
             common/kv_error.h)

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLOG_LIB}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${GPERFTOOLS_LIBRARIES}
    ${LEVELDB_LIB}
    ${BRAFT_LIB}
    ${YAML_CPP_LIB}
    ${ROCKSDB_LIB}
    ${BRPC_LIB}
    ${GLOG_LIB}
    rt
    ssl
    crypto
    dl
    z
    )


target_link_libraries(server
                      "-Xlinker \"-(\""
                      ${DYNAMIC_LIB}
                      "-Xlinker \"-)\"")
