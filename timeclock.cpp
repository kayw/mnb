#include "timeclock.h"

#include <sys/time.h>
namespace mnb {

void TimeTick::startTick() {
  struct timeval tv;
  struct timezone tz = { 0, 0 };  // UTC
  if (gettimeofday(&tv, &tz) != 0) {
    //DCHECK(0) << "Could not determine time of day";
    //LOG_ERRNO(ERROR) << "Call to gettimeofday failed.";
    // Return null instead of uninitialized |tv| value, which contains random
    // garbage data. This may result in the crash seen in crbug.com/147570.
    return;
  }
  us_ = tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec;
}

int64_t TimeTick::elapseInMicroseconds() const {
  int64_t nowsec, nowusec;
  getNowTime(nowsec, nowusec);
  int64_t delta = nowsec * kMicrosecondsPerSecond + nowusec - us_;
  return delta;
}

void TimeTick::getNowTime(int64_t& nowsec, int64_t& nowusec) {
  struct timeval tv;
  struct timezone tz = { 0, 0 };  // UTC
  if (gettimeofday(&tv, &tz) != 0) {
    //DCHECK(0) << "Could not determine time of day";
    //LOG_ERRNO(ERROR) << "Call to gettimeofday failed.";
    // Return null instead of uninitialized |tv| value, which contains random
    // garbage data. This may result in the crash seen in crbug.com/147570.
    return;
  }
  nowsec = tv.tv_sec;
  nowusec = tv.tv_usec;
}

}
