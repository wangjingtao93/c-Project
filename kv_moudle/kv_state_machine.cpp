//
// Created by root on 2019/8/22.
//

#include "kv_state_machine.h"
#include "common/kv_error.h"

DECLARE_bool(check_term);
DECLARE_bool(disable_cli);
// DECLARE_bool(log_applied_task);
DECLARE_int32(port);
DECLARE_int32(election_timeout_ms);
DECLARE_int32(snapshot_interval);
DECLARE_string(conf);
DECLARE_string(data_path);
DECLARE_string(group);
DECLARE_string(listen_ip);

DEFINE_bool(kv_engine_error_injection, false, "Inject random non-retryable error in KV engine");

namespace kvmoudle
{

KvRaft::KvRaft(const std::string &table_name, const std::string &raft_conf)
    : node_(NULL),
      leader_term_(-1),
      rocksdb_(NULL),
      table_name_(table_name),
      group_name_(table_name),
      raft_conf_string_(raft_conf) {}

KvRaft::~KvRaft()
{
    delete node_;
}

/********************************************************************
*Function:       
*Description:   
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
int KvRaft::Init()
{
    // 一些前置判断
    if (table_name_.size() < 1)
    {
        LOG(ERROR) << "table name is empty";
        return ERR_INVALID_ARG;
    }

    //会创建一个路径db_path，rockdb会open这个路径
    std::string db_path = FLAGS_data_path + "/engine/" + table_name_;
    if (!butil::CreateDirectory(butil::FilePath(db_path))) //wjt 创建文件夹函数，好用
    {
        LOG(ERROR) << "Fail to create directory " << FLAGS_data_path;
        return ERR_FILE_NOT_EXISTS;
    }

    //注意RocksDB的调用方式，以后封装函数可能还会用到
    db_options_.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(1));
    db_options_.IncreaseParallelism();
    // db_options_.OptimizeForSmallDb();
    // create the DB if it's not already present，不存在则创建
    db_options_.create_if_missing = true;

    // open DB
    std::cout << "open DB" << std::endl;
    rocksdb::DB *rocksdb_ptr;
    rocksdb::Status s = rocksdb::DB::Open(db_options_, db_path, &rocksdb_ptr);
    if (!s.ok())
    {
        LOG(ERROR) << "Fail to open rocksdb " << db_path;
        return ERR_TABLE_NOT_CREATED;
    }

    //整个类都用这个RocksDB指针，而不是仅这个函数。这是个只能指针，会自动销毁
    rocksdb_ = std::shared_ptr<rocksdb::DB>(rocksdb_ptr);

    LOG(INFO) << "DB: " << db_path << " opened";

    return OK;
}

int KvRaft::Start()
{
    /***wjt delete,此处像是可以自由添加的东西****/
    // if (!butil::CreateDirectory(butil::FilePath(FLAGS_data_path)))
    // {
    //     LOG(ERROR) << "Fail to create directory " << FLAGS_data_path;
    //     return -1;
    // }
    // std::string data_path = FLAGS_data_path + "/data";
    // int fd = ::open(data_path.c_str(), O_CREAT | O_RDWR, 0644);
    // if (fd < 0)
    // {
    //     PLOG(ERROR) << "Fail to open " << data_path;
    //     return -1;
    // }
    // _fd = new SharedFD(fd);

    //wjt 换下面一种方式
    // butil::EndPoint addr(butil::my_ip(), FLAGS_port);
    // braft::NodeOptions node_options;
    // if (node_options.initial_conf.parse_from(FLAGS_conf) != 0)
    // {
    //     LOG(ERROR) << "Fail to parse configuration `" << FLAGS_conf << '\'';
    //     return -1;
    // }

    butil::EndPoint addr;
    butil::str2endpoint(FLAGS_listen_ip.c_str(), FLAGS_port, &addr);
    braft::PeerId myself(addr);

    braft::NodeOptions node_options;
    if (node_options.initial_conf.parse_from(raft_conf_string_) != 0)
    {
        LOG(ERROR) << "Fail to parse configuration `" << raft_conf_string_ << '\'';
        return ERR_RAFT_INIT_FAILED;
    }
    if (!node_options.initial_conf.contains(myself))
    {
        LOG(ERROR) << "This node [" << addr << "] is NOT in the group [" << raft_conf_string_ << "]";
        return ERR_RAFT_PEER_NOT_IN_CONF;
    }

    node_options.election_timeout_ms = FLAGS_election_timeout_ms;
    node_options.fsm = this;
    node_options.node_owns_fsm = false;
    node_options.snapshot_interval_s = FLAGS_snapshot_interval;
    std::string prefix = "local://" + FLAGS_data_path;                 //data0/bm_data
    node_options.log_uri = prefix + "/log/" + table_name_;             // data0/bm_data/raft_meta/Raptor_meta
    node_options.raft_meta_uri = prefix + "/raft_meta/" + table_name_; // data0/bm_data/raft_meta/Raptor_meta
    node_options.snapshot_uri = prefix + "/snapshot/" + table_name_;   // data0/bm_data/snapshot/Raptor_meta
    node_options.disable_cli = FLAGS_disable_cli;

    //构造node(就是指meta_table这张表(或者是其他table)，ip:port:index)
    braft::Node *node = new braft::Node(group_name_, myself);         //注意group_name,很棒的东西
    if (node->init(node_options) != 0)
    {
        LOG(ERROR) << "Fail to init raft node";
        delete node;
        return ERR_RAFT_INIT_FAILED;
    }
    node_ = node;

    return 0;
}

/********************************************************************
*Function:       
*Description:    接收rpc,解析完req之后调用addtable
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void KvRaft::on_apply(braft::Iterator &iter)
{
    KVRequest request_on_stack; //wjt add
    // A batch（批） of tasks are committed, which must be processed through
    // |iter|
    for (; iter.valid(); iter.next())
    {
        KVRequest *request = NULL; //wjt add
        KVResponse *response = NULL;
        // This guard helps invoke（调用） iter.done()->Run() asynchronously to
        // avoid that callback blocks the StateMachine
        braft::AsyncClosureGuard closure_guard(iter.done());

        // butil::IOBuf data; //wjt delete
        // off_t offset = 0;  //wjt dellete

        //判断是从request中解析还是从log中解析
        if (iter.done())
        {
            // This task is applied by this node, get value from this
            // closure to avoid additional parsing.
            KVRequestClosure *c = dynamic_cast<KVRequestClosure *>(iter.done());
            // offset = c->request()->offset(); //wjt delete
            // data.swap(*(c->data())); //wjt delete

            request = const_cast<KVRequest *>(c->request()); //wjt add,这里就牛逼了,完全看不懂

            response = c->response();
        }
        else
        {
            /**********wjt delete *****/
            // // Have to parse KVRequest from this log.
            // uint32_t meta_size = 0;
            // butil::IOBuf saved_log = iter.data();
            // saved_log.cutn(&meta_size, sizeof(uint32_t));
            // // Remember that meta_size is in network order which hould be
            // // covert to host order网络序 和主机序列
            // meta_size = butil::NetToHost32(meta_size);
            // butil::IOBuf meta;
            // saved_log.cutn(&meta, meta_size);
            // butil::IOBufAsZeroCopyInputStream wrapper(meta);
            // KVRequest request;
            // CHECK(request.ParseFromZeroCopyStream(&wrapper));
            // data.swap(saved_log);
            // offset = request.offset();

            /****wjt add****/
            // Have to parse FetchAddRequest from this log.
            butil::IOBufAsZeroCopyInputStream wrapper(iter.data());
            // TODO check if this on stack var works well!
            CHECK(request_on_stack.ParseFromZeroCopyStream(&wrapper));
            request = &request_on_stack;
        }

        //wjt add，干什么用呢，不知道
        if (__glibc_unlikely(FLAGS_kv_engine_error_injection))
        {
            // TODO Inject Error occasionally !!!
        }

        //这是具体操作的实现？？可以在这封装个函数？
        int status = 0;
        status = apply_one(request, response);

        if (response)
        {
            response->set_status(status);
        }

        /*****wjt add, 不知道是否是必须的，有啥作用呢*/
        if (status != OK)
        {
            // Let raft run this closure.
            closure_guard.release();
            // Some disk error occurred, notify raft and never apply any data
            // ever after
            iter.set_error_and_rollback();
            //kv_on_apply_error_count_ << 1; // wjt 暂时注释掉
            return;
        }
        // The purpose of following logs is to help you understand the way
        // this StateMachine works.
        // Remove these logs in performance-sensitive servers.
        // LOG_IF(INFO, FLAGS_log_applied_task)
        //     << "Write " << data.size() << " bytes"
        //     << " from offset=" << offset
        //     << " at log_index=" << iter.index();
    }
}

int KvRaft::apply_one(const KVRequest *request, KVResponse *response)
{
    return OK;
}

/********************************************************************
*Function:       
*Description:    快照，这个rocksDB的快照是咋应用到raft的呢，that's a question
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void *KvRaft::save_snapshot(void *arg)
{
    SnapshotArg *sa = (SnapshotArg *)arg;
    std::unique_ptr<SnapshotArg> arg_guard(sa);
    // Serialize StateMachine to the snapshot
    brpc::ClosureGuard done_guard(sa->done);

    //wjt change
    //std::string snapshot_path = sa->writer->get_path() + "/data";
    std::string snapshot_file = sa->table_name + "_dump.sst";
    std::string snapshot_path = sa->writer->get_path() + "/" + snapshot_file;

    LOG(INFO) << "Saving snapshot to " << snapshot_path;

    /**wjt 此处需要大改，根据自己的方法**/
    // // Sync buffered data before
    // int rc = 0;
    // for (; (rc = ::fdatasync(sa->fd->fd())) < 0 && errno == EINTR;)
    // {
    // }
    // if (rc < 0)
    // {
    //     sa->done->status().set_error(EIO, "Fail to sync fd=%d : %m",
    //                                  sa->fd->fd());
    //     return NULL;
    // }
    // std::string data_path = FLAGS_data_path + "/data";
    // if (link_overwrite(data_path.c_str(), snapshot_path.c_str()) != 0)
    // {
    //     sa->done->status().set_error(EIO, "Fail to link data : %m");
    //     return NULL;
    // }

    // // Snapshot is a set of files in raft. Add the only file into the
    // // writer here.
    // if (sa->writer->add_file("data") != 0)
    // {
    //     sa->done->status().set_error(EIO, "Fail to add file to writer");
    //     return NULL;
    // }
    // return NULL;

    /***wjt add***/
    rocksdb::ReadOptions snapopt;
    snapopt.snapshot = sa->db_ptr->GetSnapshot();
    rocksdb::Iterator *it = sa->db_ptr->NewIterator(snapopt);

    rocksdb::Options dummy_options;

    rocksdb::SstFileWriter sst_file_writer(rocksdb::EnvOptions(), dummy_options);

    // Open the file for writing
    rocksdb::Status s = sst_file_writer.Open(snapshot_path);
    if (!s.ok())
    {
        LOG(ERROR) << "Error while opening file " << snapshot_path << ", Error: " << s.ToString();
        sa->done->status().set_error(EIO, "Fail to open sst writer");

        //*(sa->kv_on_snapshot_save_error_count) << 1; // wjt
        return nullptr;
    }

    // Insert rows into the SST file, note that inserted keys must be
    // strictly increasing (based on options.comparator)
    int i = 0;

    // TODO: no matter what, put one empty kv pair in sst
    s = sst_file_writer.Put(" ", "no_exists");
    if (!s.ok())
    {
        LOG(ERROR) << "Error while adding numb key, Error: " << s.ToString();
        sa->done->status().set_error(EIO, "Fail to write KV pair");

        //*(sa->kv_on_snapshot_save_error_count) << 1;
        return nullptr;
    }

    for (it->SeekToFirst();
         it->Valid();
         it->Next())
    {
        if (it->key() == " ")
        {
            continue;
        }
        i++;
        s = sst_file_writer.Put(it->key(), it->value());
        if (!s.ok())
        {
            LOG(ERROR) << "Error while adding Key: " << it->key().ToString() << ", Error: " << s.ToString();
            sa->done->status().set_error(EIO, "Fail to write KV pair");

            //*(sa->kv_on_snapshot_save_error_count) << 1;
            return nullptr;
        }
        VLOG(9) << "Snapshot: saving key:" << it->key().ToString() << "  value:" << it->value().ToString();
    }

    delete it;

    // Close the file
    s = sst_file_writer.Finish();
    if (!s.ok())
    {
        LOG(ERROR) << "Error while finishing file " << snapshot_path << ", Error: " << s.ToString().c_str();
        sa->done->status().set_error(EIO, "Fail to finish sst writer");

        // *(sa->kv_on_snapshot_save_error_count) << 1;//干啥用的，不知道
        return nullptr;
    }

    // TODO: clean up on errors
    sa->db_ptr->ReleaseSnapshot(snapopt.snapshot);

    VLOG(1) << "Wrote " << i << " records to " << snapshot_path;
    // Snapshot is a set of files in raft. Add the only file into the
    // writer here.
    if (sa->writer->add_file(snapshot_file) != 0)
    {
        sa->done->status().set_error(EIO, "Fail to add file to writer");

        // *(sa->kv_on_snapshot_save_error_count) << 1;//干啥用的，不知道
        return nullptr;
    }

    // elasped_time_ms = butil::gettimeofday_ms() - start_time_ms;
    // *(sa->kv_on_snapshot_save_latency_recorder) << elasped_time_ms;

    return nullptr;
}

void KvRaft::on_snapshot_save(braft::SnapshotWriter *writer, braft::Closure *done)
{
    // Save current StateMachine in memory and starts a new bthread to avoid
    // blocking StateMachine since it's a bit slow to write data to disk
    // file.
    SnapshotArg *arg = new SnapshotArg;

    //wjt add
    arg->table_name = table_name_;
    arg->db_ptr = rocksdb_;

    arg->writer = writer;
    arg->done = done;
    bthread_t tid;
    bthread_start_urgent(&tid, NULL, save_snapshot, arg);
}

int KvRaft::on_snapshot_load(braft::SnapshotReader *reader)
{
    // Load snasphot from reader, replacing the running StateMachine
    CHECK(!is_leader()) << "Leader is not supposed to load snapshot";
    if (reader->get_file_meta("data", NULL) != 0)
    {
        LOG(ERROR) << "Fail to find `data' on " << reader->get_path();
        return -1;
    }

    /*****wjt chanage，此处根据需要修改*****/
    // std::string snapshot_path = reader->get_path() + "/data";
    // std::string data_path = FLAGS_data_path + "/data";
    // if (link_overwrite(snapshot_path.c_str(), data_path.c_str()) != 0)
    // {
    //     PLOG(ERROR) << "Fail to link data";
    //     return -1;
    // }
    // // Reopen this file
    // int fd = ::open(data_path.c_str(), O_RDWR, 0644);
    // if (fd < 0)
    // {
    //     PLOG(ERROR) << "Fail to open " << data_path;
    //     return -1;
    // }
    // _fd = new SharedFD(fd);

    /*****wjt add****/
    std::string snapshot_file = table_name_ + "_dump.sst";
    std::string snapshot_path = reader->get_path() + "/" + snapshot_file;
    LOG(INFO) << "Load Snapshot from " << snapshot_path;

    //if (reader->get_file_meta(snapshot_file, NULL) != 0) {
    //  LOG(ERROR) << "Fail to find snapshot on " << reader->get_path();
    //  return -1;
    //}

    butil::File::Info info;
    butil::GetFileInfo(butil::FilePath(snapshot_path), &info); // wjt 这是标准的从快照恢复的函数吗？？？不知道
    //if (info.size < 10) {
    //  LOG(ERROR) << "Snapshot is empty, do nothing";
    //  return 0;
    //}

    rocksdb::IngestExternalFileOptions ifo;
    // Ingest
    rocksdb::Status s = rocksdb_->IngestExternalFile({snapshot_path}, ifo);
    if (!s.ok())
    {
        LOG(ERROR) << "Error while adding file , Error: " << s.ToString();

        // kv_on_snapshot_load_error_count_ << 1;
        // TODO
        return -1;
    }

    // elasped_time_ms = butil::gettimeofday_ms() - start_time_ms;//干什么用的，不知道
    // kv_on_snapshot_load_latency_recorder_ << elasped_time_ms;

    return 0;
}

/********************************************************************
*Function:       
*Description:    这些地方基本不修改
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void KvRaft::shutdown()
{
    if (node_)
    {
        node_->shutdown(NULL);
    }
}

void KvRaft::join()
{
    if (node_)
    {
        node_->join();
    }
}

bool KvRaft::is_leader() const
{
    return leader_term_.load(butil::memory_order_acquire) > 0;
}

void KvRaft::on_leader_start(int64_t term)
{
    leader_term_.store(term, butil::memory_order_release);
    LOG(INFO) << "Node becomes leader";
}

void KvRaft::on_leader_stop(const butil::Status &status)
{
    leader_term_.store(-1, butil::memory_order_release);
    LOG(INFO) << "Node stepped down : " << status;
}

void KvRaft::on_shutdown()
{
    LOG(INFO) << "This node is down";
}

void KvRaft::on_error(const ::braft::Error &e)
{
    LOG(ERROR) << "Met raft error " << e;
}

void KvRaft::on_configuration_committed(const ::braft::Configuration &conf)
{
    LOG(INFO) << "Configuration of this group is " << conf;
}

void KvRaft::on_start_following(const ::braft::LeaderChangeContext &ctx)
{
    LOG(INFO) << "Node start following " << ctx;
}

/********************************************************************
*Function:       
*Description:    redirect根据proto文件修改
*Calls:          
*Table Accessed: 
*Table Updated: 
*Input:                 
*Output:         
*Return:         
*Others:        
*********************************************************************/
void KvRaft::redirect(KVResponse *response)
{
    // response->set_success(false);//wjt proto文件已经修改了
    response->set_status(ERR_RAFT_NOT_LEADER);
    if (node_)
    {
        braft::PeerId leader = node_->leader_id();
        if (!leader.is_empty())
        {
            response->set_redirect(leader.to_string());
        }
        //wjt 新增,删除了会咋样呢
        if (!raft_conf_string_.empty())
        {
            response->set_raft_conf(raft_conf_string_);
        }
    }
}

void KVRequestClosure::Run()
{
    // Auto delete this after Run()
    std::unique_ptr<KVRequestClosure> self_guard(this);
    // Repsond this RPC.
    brpc::ClosureGuard done_guard(_done);
    if (status().ok())
    {
        return;
    }
    // Try redirect if this request failed.
    kv_instance_->redirect(_response);
}
} // namespace kvmoudle
