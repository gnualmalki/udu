#ifndef VERSION
    #define VERSION "unknown"
#endif

// Compilers
#if defined(__GNUC__) || defined(__clang__) || defined(__xlC__) ||             \
  defined(__IBMC__)
    #define UDU_SI static inline __attribute__((always_inline))
#elif __STDC_VERSION__ >= 199901L
    #define UDU_SI static inline
#else
    #define UDU_SI static
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(__SUNPRO_C) ||          \
  defined(__SUNPRO_CC) || defined(__xlC__) || defined(__IBMC__) ||             \
  defined(__HP_cc) || defined(__HP_aCC)
    #define UDU_THD __thread
#elif __STDC_VERSION__ >= 201112L
    #define UDU_THD _Thread_local
#else
    #define UDU_THD
#endif
