#ifndef __S_ENET_INTERNAL__
#define __S_ENET_INTERNAL__

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
    #include <windows.h>
    #define DELAY(ms) Sleep(ms)
#else  /* presume POSIX */
    #include <unistd.h>
    #define DELAY(ms) usleep(ms * 1000)
#endif

#define DEFAULT_PROCESS_TIME 100
//#define SENET_DEBUG

inline void debug (const char* data);

#endif // __S_ENET_INTERNAL__
