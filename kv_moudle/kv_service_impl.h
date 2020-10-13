//
// Created by root on 2019/8/22.
//

#ifndef KV_SERVICE_IMPL_H
#define KV_SERIVCE_IMPL_H

#include <gflags/gflags.h>   // DEFINE_*
#include <brpc/controller.h> // brpc::Controller
#include <brpc/server.h>     // brpc::Server

#include "kv_interface.pb.h" // BlockService
#include "kv_state_machine.h"
#include "common/kv_error.h"

// Implements example::BlockService if you are using brpc
namespace kvmoudle
{

class KVServiceImpl : public KVService
{
public:
    //注意：这里没有构造和析构函数
    int AddTable(const std::string &table_name, const std::string &raft_conf);
    // Add all existing tables, ignore all error
    virtual void CreateTable(::google::protobuf::RpcController *controller,
                             const ::kvmoudle::TableRequest *request,
                             ::kvmoudle::TableResponse *response,
                             ::google::protobuf::Closure *done);

    virtual void Get(::google::protobuf::RpcController *controller,
                     const ::kvmoudle::KVRequest *request,
                     ::kvmoudle::KVResponse *response,
                     ::google::protobuf::Closure *done);

    virtual void Put(::google::protobuf::RpcController *controller,
                     const ::kvmoudle::KVRequest *request,
                     ::kvmoudle::KVResponse *response,
                     ::google::protobuf::Closure *done);
    virtual void Delete(::google::protobuf::RpcController *controller,
                        const ::kvmoudle::KVRequest *request,
                        ::kvmoudle::KVResponse *response,
                        ::google::protobuf::Closure *done);

private:
    std::map<std::string, std::shared_ptr<KvRaft>> kv_tables_;
};

} // namespace kvmoudle

#endif //RAPTOR_SERIMPL_H
