#ifndef POSIX_EINTER_H_ECD80E6F_FCFA_45D2_9066_6DC285678DD5
#define POSIX_EINTER_H_ECD80E6F_FCFA_45D2_9066_6DC285678DD5

//chromium base/posix/eintr_wrapper.h
#include <errno.h>

#if defined(NDEBUG)

//http://stackoverflow.com/questions/5313536/whats-the-difference-between-gnu99-and-c99-clang 
#define HANDLE_EINTR(x) ({ \
  __typeof__(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR); \
  eintr_wrapper_result; \
})

#else

#define HANDLE_EINTR(x) ({ \
  int eintr_wrapper_counter = 0; \
  __typeof__(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR && \
           eintr_wrapper_counter++ < 100); \
  eintr_wrapper_result; \
})

#endif  // NDEBUG


#endif /* POSIX_EINTER_H_ECD80E6F_FCFA_45D2_9066_6DC285678DD5 */

