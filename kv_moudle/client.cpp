#include <gflags/gflags.h>
#include <bthread/bthread.h>
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <braft/raft.h>
#include <braft/util.h>

#include "kv_interface.pb.h"

DEFINE_bool(log_each_request, false, "Print log for each request");
DEFINE_bool(use_bthread, false, "Use bthread to send requests");
DEFINE_int32(timeout_ms, 500, "Timeout for each request");
DEFINE_int32(thread_num, 1, "Number of threads sending requests");
DEFINE_string(conf, "", "Configuration of the raft group");
DEFINE_string(group, "meta_table", "Id of the replication group"); //table_name
//DEFINE_string(table_name, "Block", "Id of the replication group");


//方便做测试


static void *sender(void *arg)
{
    while (!brpc::IsAskedToQuit())
    {
        braft::PeerId leader;
        //Select leader of the target group from RouteTable
        if (braft::rtb::select_leader(FLAGS_group, &leader) != 0)
        {
            //Leader is unknown in RouteTable, Ask Routable to refresh leader by sending RPCs.
            butil::Status st = braft::rtb::refresh_leader(FLAGS_group, FLAGS_timeout_ms);
            if (!st.ok())
            {
                //not sure about the leader,sleep for a while and ask again
                LOG(WARINFG) << "Fail to refresh leader: " << st;
                bthread_usleep(FLAGS_timeout_ms * 1000L)
            }
            continue;
        }

        //Now we know who is leader, construct Stub and the sending rpc
        brpc::Channel channel;
        if (channel.Init(leaser.addr, NULL) != 0)
        {
            LOG(ERROR) << "Fail to init channel to ：" << leader;
            bthread_usleep(FLAGS_timeout_ms * 1000L);
            continue;
        }

        kvmoudle::KVService_Stub stub(&channel);

        brpc::Controller cntl;
        cntl.set_timeout_ms(FLAGS_timeout_ms);
        // Randomly select which requert we want to sendl;
        kvmoudle::TableRequest request;
        kvmoudle::TableResponse response;

        request.set_table("table_test");
        request.set_raft_conf()

    }
}