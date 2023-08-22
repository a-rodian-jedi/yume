#pragma once

#include "defines.h"

// Disable assertions by commenting this out
#define YASSERTIONS_ENABLED

#ifdef YASSERTIONS_ENABLED
    #if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
    #else
    #define debugBreak() __builtin_trap()
    #endif

    YAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

    #define YASSERT(expr)                                                       \
        {                                                                       \
            if (expr) {                                                         \
                                                                                \
            } else {                                                            \
                report_assertion_failure(#expr, "", __FILE__, __LINE__);        \
                debugBreak();                                                   \
            }                                                                   \
        }

    #define YASSERT_MSG(expr, message)                                              \
        {                                                                           \
            if (expr) {                                                             \
                                                                                    \
            } else {                                                                \
                report_assertion_failure(#expr, message, __FILE__, __LINE__);       \
                debugBreak();                                                       \
            }                                                                       \
        }

    #ifdef _DEBUG
    #define YASSERT_DEBUG(expr, message)                                        \
        {                                                                       \
            if (expr) {                                                         \
                                                                                \
            } else {                                                            \
                report_assertion_failure(#expr, "", __FILE__, __LINE__);        \
                debugBreak();                                                   \
            }                                                                   \
        }
    #else
    #define YASSERT_DEBUG(expr)
    #endif
#else
    #define YASSERT(expr)
    #define YASSERT_MSG(expr)
    #define YASSERT_DEBUG(expr)
#endif
