
#if _WIN64 || __amd64__
#define BITS_64
#endif

#if defined(__MINGW32__) && !defined(WINDOWS)
#define WINDOWS (1)
#endif

#if defined _WIN32 || defined _WIN64 || defined WIN32 || defined _WIN32 || defined WINDOWS
#define WINDOWS (1)
#elif defined __linux__
#define LINUX
#elif defined TARGET_OS_MAC || defined __APPLE__
#define OSX
#else
#error Operating system cannot be determined!
#endif
