//
// Created by root on 2019/8/22.
//
#include <iostream>
#include "kv_service_impl.h"

namespace kvmoudle
{

/********************************************************************
*Function:       
*Description:    AddTable(),可作为启动函数，服务启动时会立刻调用一次，创建个meta_table
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void KVServiceImpl::CreateTable(::google::protobuf::RpcController *controller,
                             const ::kvmoudle::TableRequest *request,
                             ::kvmoudle::TableResponse *response,
                             ::google::protobuf::Closure *done)
{
    brpc::ClosureGuard done_guard(done);

    int ret = AddTable(request->table(), request->raft_conf()); //关键要调用Addtable()函数，完成Createable
    response->set_status(ret);

    return;
}

int KVServiceImpl::AddTable(const std::string &table_name, const std::string &raft_conf)
{

    //instance实例,重要，作为模板学习用法,就是说效率高，会自动销毁
    std::shared_ptr<KvRaft> kv_instance = std::make_shared<KvRaft>(table_name, raft_conf);

    int ret = kv_instance->Init(); //先初始化，这个函数里调用RocksDB open这个Table(就是engine/Raptor_meta/这个目录)
    if (ret != OK)
    {
        LOG(ERROR) << "Fail to init instance: " << table_name;
        return ret;
    }

    //为什么要start，start函数的作用还没搞懂
    ret = kv_instance->Start();
    if (ret != OK)
    {
        LOG(ERROR) << "Fail to start instance: " << table_name;
        return ret;
    }

    LOG(INFO) << "Added table: " << table_name;

    //kv_tables_是一个map表，具体有什么作用呢？调用的地方挺多的。一个BM一张表，
    kv_tables_.insert(std::make_pair(table_name, kv_instance));


    return OK;
}

/********************************************************************
*Function:       
*Description:    .
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void KVServiceImpl::Get(::google::protobuf::RpcController *controller, const ::kvmoudle::KVRequest *request,
                        ::kvmoudle::KVResponse *response, ::google::protobuf::Closure *done)
{
    brpc::ClosureGuard done_guard(done);

    // auto kv_table = kv_tables_.find(request->table());
    // if (kv_table == kv_tables_.end())
    // {
    //     response->set_status(ERR_TABLE_NOT_EXISTS);
    // }
    // else
    // {
    //     kv_table->second->Get(request, response);
    // }
    return;
}

void KVServiceImpl::Put(::google::protobuf::RpcController *controller,
                        const ::kvmoudle::KVRequest *request,
                        ::kvmoudle::KVResponse *response,
                        ::google::protobuf::Closure *done)
{

    // std::cout << "KVServiceImpl::Put() [begin]" << std::endl;
    // std::cout << "Prase Request" << std::endl;

    brpc::ClosureGuard done_guard(done);

    // //先判断req的内容是否正确合法，比如valuez值存不存在，table_name存不存在
    // if (request->op() != OP_PUT)
    // {
    //     response->set_status(ERR_INVALID_OP);
    //     return;
    // }
    // if (!request->has_value())
    // {
    //     response->set_status(ERR_KEY_PUT_NO_VALUE);
    //     return;
    // }
    // auto kv_table = kv_tables_.find(request->table());
    // if (kv_table == kv_tables_.end())
    // {
    //     response->set_status(ERR_TABLE_NOT_EXISTS);
    // }
    // else
    // {
    //     //重点来了
    //     kv_table->second->Modify(request, response, done_guard.release());
    // }

    // std::cout << "KVServiceImpl::Put() [end]" << std::endl;

    return;
}

void KVServiceImpl::Delete(::google::protobuf::RpcController *controller,
                           const ::kvmoudle::KVRequest *request,
                           ::kvmoudle::KVResponse *response,
                           ::google::protobuf::Closure *done)
{
    brpc::ClosureGuard done_guard(done);

    // if (request->op() != OP_DELETE)
    // {
    //     response->set_status(ERR_INVALID_OP);
    //     return;
    // }
    // auto kv_table = kv_tables_.find(request->table());
    // if (kv_table == kv_tables_.end())
    // {
    //     response->set_status(ERR_TABLE_NOT_EXISTS);
    // }
    // else
    // {
    //     kv_table->second->Modify(request, response, done_guard.release());
    // }
    return;
}

}//end namespace kvmoudle