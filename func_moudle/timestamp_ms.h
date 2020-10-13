#ifndef _TIMESTAMP_MS_H_
#define _TIMESTAMP_MS_H_

#include <stdint.h>
#include <chrono>

namespace utils {
//返回当前时间，秒
inline uint64_t now_timestamp_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();
}
//返回当前时间，毫秒
inline uint64_t now_timestamp_us() {
  return std::chrono::duration_cast<std::chrono::microseconds>
      (std::chrono::system_clock::now().time_since_epoch()).count();
}
}



#endif 