//
// Created by root on 2019/8/22.
//

#ifndef KV_STATE_MACHINE_H
#define KV_STATE_MACHINE_H

#include <rocksdb/db.h>
#include <rocksdb/slice_transform.h>
#include <sys/types.h>           // O_CREAT
#include <fcntl.h>               // open
#include <gflags/gflags.h>       // DEFINE_*
#include <butil/sys_byteorder.h> // butil::NetToHost32
#include <brpc/controller.h>     // brpc::Controller
#include <brpc/server.h>         // brpc::Server
#include <braft/raft.h>          // braft::Node braft::StateMachine
#include <braft/storage.h>       // braft::SnapshotWriter
#include <braft/util.h>          // braft::AsyncClosureGuard

#include "kv_interface.pb.h"     // rpc_impl

namespace kvmoudle
{

// Implementation of example::KvRaft as a braft::StateMachine.
class KvRaft : public braft::StateMachine
{
public:
    KvRaft(const std::string &table_name, const std::string &raft_conf);
    virtual ~KvRaft();

    // Init and Starts this node
    int Init();
    int Start();
    //implements the methods

    // Shut this node down.
    void shutdown();

    // Blocking this thread until the node is eventually down.
    void join();

    bool is_leader() const;

    void redirect(KVResponse *response);

    // @braft::StateMachine
    // 这个是必须要实现的
    // 会在一条或者多条日志被多数节点持久化之后调用。通知用户将这些日志所表示的操作应用到业务状态机中
    // 通过iter，可以从遍历所有未处理但是已经提交的日志，如果你的状态机支持批量更新，可以一次性获取
    // 多条日志,提高状态机的吞吐
    virtual int apply_one(const KVRequest *request, KVResponse *response);

    void on_apply(braft::Iterator &iter);

    static void *save_snapshot(void *arg);
    void on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done);
    int on_snapshot_load(braft::SnapshotReader *reader);

    void on_leader_start(int64_t term);
    void on_leader_stop(const butil::Status &status);
    void on_shutdown();
    void on_error(const ::braft::Error &e);
    void on_configuration_committed(const ::braft::Configuration &conf);
    void on_start_following(const ::braft::LeaderChangeContext &ctx);

public:
    struct SnapshotArg
    {
        braft::SnapshotWriter *writer;
        braft::Closure *done;
        std::shared_ptr<rocksdb::DB> db_ptr;
        std::string table_name;
    };

protected:
    braft::Node *volatile node_;
    butil::atomic<int64_t> leader_term_;

    std::shared_ptr<rocksdb::DB> rocksdb_;
    rocksdb::Options db_options_;

    std::string table_name_;
    std::string group_name_;
    std::string raft_conf_string_;
};// end of @braft::StateMachine

// Implements Closure（关闭，使终止） which enclosed（围绕，装入，放入封套） RPC stuff（东西，塞满，填塞）
// 实现包含RPC内容的关闭
class KVRequestClosure : public braft::Closure
{
public:
    KVRequestClosure(KvRaft *kv_instance,
                     const KVRequest *request,
                     KVResponse *response,
                     butil::IOBuf *data,
                     google::protobuf::Closure *done)
        : kv_instance_(kv_instance), _request(request), _response(response), _data(data), _done(done) {}
    ~KVRequestClosure() {}

    const KVRequest *request() const { return _request; }
    KVResponse *response() const { return _response; }
    void Run();
    butil::IOBuf *data() const { return _data; }

private:
    // Disable explicitly delete
    KvRaft *kv_instance_;
    const KVRequest *_request;
    KVResponse *_response;
    butil::IOBuf *_data;
    google::protobuf::Closure *_done;
};
} // namespace kvmoudle
#endif //RAPTOR_SERVER_H
