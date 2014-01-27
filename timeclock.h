#ifndef TIMECLOCK_H_61D7F1C8_356C_4EF9_86DA_DC48B3E89037
#define TIMECLOCK_H_61D7F1C8_356C_4EF9_86DA_DC48B3E89037

#include "mmacros.h"

namespace mnb {

class TimeTick {
  public:
    TimeTick() : us_(0) { startTick(); }

    void startTick();
    int64_t elapseInMicroseconds() const;

  private:
    static const int64_t kMillisecondsPerSecond = 1000;
    static const int64_t kMicrosecondsPerMillisecond = 1000;
    static const int64_t kMicrosecondsPerSecond = kMicrosecondsPerMillisecond *
                                              kMillisecondsPerSecond;
    static void getNowTime(int64_t& nowsec, int64_t& nowusec);
    // time in microseconds.
    int64_t us_;
};

}

#endif /* TIMECLOCK_H_61D7F1C8_356C_4EF9_86DA_DC48B3E89037 */

