#ifndef EASYBOT__VISIBILITY_CONTROL_H_
#define EASYBOT__VISIBILITY_CONTROL_H_

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define EASYBOT_EXPORT __attribute__ ((dllexport))
    #define EASYBOT_IMPORT __attribute__ ((dllimport))
  #else
    #define EASYBOT_EXPORT __declspec(dllexport)
    #define EASYBOT_IMPORT __declspec(dllimport)
  #endif
  #ifdef EASYBOT_BUILDING_LIBRARY
    #define EASYBOT_PUBLIC EASYBOT_EXPORT
  #else
    #define EASYBOT_PUBLIC EASYBOT_IMPORT
  #endif
  #define EASYBOT_PUBLIC_TYPE EASYBOT_PUBLIC
  #define EASYBOT_LOCAL
#else
  #define EASYBOT_EXPORT __attribute__ ((visibility("default")))
  #define EASYBOT_IMPORT
  #if __GNUC__ >= 4
    #define EASYBOT_PUBLIC __attribute__ ((visibility("default")))
    #define EASYBOT_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define EASYBOT_PUBLIC
    #define EASYBOT_LOCAL
  #endif
  #define EASYBOT_PUBLIC_TYPE
#endif

#endif  // EASYBOT__VISIBILITY_CONTROL_H_
