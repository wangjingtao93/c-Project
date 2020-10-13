//
// Created by liuzirui on 2018-12-17.
//

#ifndef KV_ERROR_H
#define KV_ERROR_H

namespace kvmoudle {

constexpr int OK = 0;

constexpr int LIBRPC_OK = 0; // external library definition of success and is subject to change
constexpr int LIBRPC_ERR = -1;

// table/db errors
constexpr int ERR_TABLE_NOT_EXISTS = 10001;
constexpr int ERR_TABLE_ALREADY_EXISTS = 10002;
constexpr int ERR_TABLE_NOT_CREATED = 10003;
constexpr int ERR_DB_NOT_INITIALIZED = 10004;
constexpr int ERR_CLIENT_NOT_INITIALIZED = 10005;
constexpr int ERR_DB_BACKUP_FAILED = 10006;
constexpr int ERR_TABLE_DELETE_FAILED = 10007;
constexpr int ERR_TABLE_OP_NOTSUPPORT = 10008;
constexpr int ERR_TABLE_IS_DELETING = 10009;
constexpr int ERR_TABLE_NO_RAFT_CONF = 10010;
constexpr int ERR_TABLE_CHECKPOINT_FAILED = 10011;

// operations
constexpr int ERR_KEY_PUT_FAILED = 11001;
constexpr int ERR_KEY_DEL_FAILED = 11002;
constexpr int ERR_KEY_GET_FAILED = 11003;
constexpr int ERR_KEY_PUT_NO_VALUE = 11004;
constexpr int ERR_KEY_NOT_EXISTS = 11005;
constexpr int ERR_LIST_FAILED = 11006;
constexpr int ERR_BATCH_INVALID = 11007;
constexpr int ERR_BATCH_FAILED = 11008;
constexpr int ERR_KEY_UPDATE_TS_FAILED = 11009;

constexpr int ERR_INVALID_OP = 11100;
constexpr int ERR_NOT_IMPLEMENTED = 111101;

// blockmaster bn errors
constexpr int ERR_BM_BN_NOT_INITIALIZED = 12001;
constexpr int ERR_BM_BN_INTERNAL_TESTING = 12002;
constexpr int ERR_BM_BN_STAT_REMOVE_FAILED = 12003;

// file/dir errors
constexpr int ERR_FILE_NOT_EXISTS = 20001;
constexpr int ERR_DIR_NOT_EXISTS = 20002;

// raft Errors
constexpr int ERR_RAFT_INIT_FAILED = 30001;
constexpr int ERR_RAFT_NOT_LEADER = 30002;
constexpr int ERR_RAFT_LOG_ERROR = 30003;
constexpr int ERR_RAFT_PEER_NOT_IN_CONF = 30004;

// file/dir errors
constexpr int ERR_MEM_ALLOCATION = 80001;
// unknown errors
constexpr int ERR_INVALID_ARG = 90001;

}

#endif //KV_ERROR_H
