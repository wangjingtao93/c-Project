//
// Created by root on 2019/8/22.
//

#include <iostream>
#include <sys/types.h>           // O_CREAT
#include <fcntl.h>               // open
#include <gflags/gflags.h>       // DEFINE_*
#include <butil/sys_byteorder.h> // butil::NetToHost32
#include <brpc/controller.h>     // brpc::Controller
#include <brpc/server.h>         // brpc::Server
#include <braft/raft.h>          // braft::Node braft::StateMachine
#include <braft/storage.h>       // braft::SnapshotWriter
#include <braft/util.h>          // braft::AsyncClosureGuard

#include "kv_state_machine.h"
#include "kv_service_impl.h"
#include "kv_interface.pb.h"

DEFINE_bool(check_term, true, "Check if the leader changed to another term");
DEFINE_bool(disable_cli, false, "Don't allow raft_cli access this node");
//DEFINE_bool(log_applied_task, false, "Print notice log when a task is applied");
DEFINE_int32(election_timeout_ms, 5000,
             "Start election in such milliseconds if disconnect with the leader");
DEFINE_string(listen_ip, "192.168.56.117", "Serving IP address");
DEFINE_int32(port, 8200, "Listen port of this peer");
DEFINE_int32(snapshot_interval, 30, "Interval between each snapshot");
DEFINE_string(raft_conf, "192.168.56.117:8200:0,192.168.56.117:8201:0,192.168.56.117:8202:0", "Initial configuration of the replication group");
DEFINE_string(data_path, "./data", "Path of data stored on");
DEFINE_string(meta_table_name, "meta_table", "MetaTable name. Do NOT specify it, the program will override this value");//group
// DEFINE_string(group, "KvRaft", "Id of the replication group");
// DEFINE_string(crash_on_fatal, "true", "Crash on fatal log");//wjt添加

int main(int argc, char *argv[])
{
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);
    butil::AtExitManager exit_manager;

    //一般只需要一个server
    brpc::Server server_rpc;
    kvmoudle::KVServiceImpl service_impl;

    int ret = 0;
    //例如：ipport = "127.0.0.1:21231",这么修改有什么意义呢？？不知道，把ip和port绑在了一块
    std::string ipport = butil::string_printf("%s:%d", FLAGS_listen_ip.c_str(), FLAGS_port);

    //add service_impl to RPC server_rpc
    ret = server_rpc.AddService(&service_impl, brpc::SERVER_DOESNT_OWN_SERVICE);
    if (ret != 0)
    {
        LOG(ERROR) << "Fail to add service_impl";
        return ret;
    }
    //raft 可以共享一个RPC server。注意第二个参数，因为不允许将service加入到一个
    // 正在运行的server，并且在server starts之前获得server的监听地址是不可能的。所以必须
    // 指定server的address。
    ret = braft::add_service(&server_rpc, ipport.c_str());
    if (ret != 0)
    {
        LOG(ERROR) << "Fail to add raft service_impl";
        return ret;
    }
    // It's recommended to start the server_rpc before KvRaft is started to avoid
    // the case that it becomes the leader while the service_impl is unreacheable by
    // clients.
    // Notice that default options of server_rpc are used here. Check out details
    // from the doc of brpc if you would like change some options
    // 要求在block starts之前 start server，以避免他成为leader，然而service
    // 对client来说还是unreachable。
    // 这里server使用默认的options
    if (server_rpc.Start(FLAGS_port, NULL) != 0)
    {
        LOG(ERROR) << "Fail to start Server";
        return ret;
    }
    /*wjt chanage, 修改成在create meta_table时start,因为有个Init*/

    // if (kvraft.Start() != 0)
    // {
    //     LOG(ERROR) << "Fail to start KvRaft";
    //     return ret;
    // }
    //It's ok to start KvRaft,通过KVServiceImpl::addtable()函数启动
    ret = service_impl.AddTable(FLAGS_meta_table_name, FLAGS_raft_conf);
    if (ret != kvmoudle::OK)
    {
        LOG(ERROR) << "Meta Data Table failed to start, ret = " << ret;
        return ret;
    }

    LOG(INFO) << "KvRaft service_impl is running on " << server_rpc.listen_address();
    // Wait until 'CTRL-C' is pressed. then Stop() and Join() the service_impl
    while (!brpc::IsAskedToQuit())
    {
        sleep(1);
    }
    LOG(INFO) << "KvRaft service_impl is going to quit";

    // Stop block before server_rpc
    // block.shutdown();
    server_rpc.Stop(0);

    // Wait until all the processing tasks are over.
    // block.join();
    server_rpc.Join();
    return 0;
}