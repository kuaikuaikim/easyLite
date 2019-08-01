//
// Created by asy on 10/19/18.
//

#ifndef EASYLITE_DEF_H
#define EASYLITE_DEF_H


#if (defined WIN32 || defined _WIN32 || defined WINCE || defined __CYGWIN__)
#  define EASYLITE_EXPORTS _declspec(dllexport)
#elif defined __GNUC__ && __GNUC__ >= 4
#  define EASYLITE_EXPORTS __attribute__ ((visibility ("default")))
#else
#  define EASYLITE_EXPORTS
#endif

#ifndef EASYLITE_EXTERN_C
#  ifdef __cplusplus
#    define EASYLITE_EXTERN_C extern "C"
#  else
#    define EASYLITE_EXTERN_C
#  endif
#endif


#endif //DFACE_DEF_H
