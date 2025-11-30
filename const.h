#ifndef VERSION
    #define VERSION "unknown"
#endif

// Handle Compilers
#if defined(__GNUC__) || defined(__clang__)
    #define UDU_SI static inline __attribute__((always_inline))
#elif __STDC_VERSION__ >= 199901L
    #define UDU_SI static inline
#else
    #define UDU_SI static
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define UDU_THD __thread
#elif __STDC_VERSION__ >= 201112L
    #define UDU_THD _Thread_local
#else
    #define UDU_THD
#endif
