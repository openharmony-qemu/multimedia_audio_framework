/*
 * Stub header for x86 host build.
 * Provides no-op logging macros to replace OHOS hilog.
 */
#ifndef AUDIO_ENGINE_LOG_H
#define AUDIO_ENGINE_LOG_H

#include <cstdio>

#define AUDIO_DEBUG_LOG(fmt, ...)
#define AUDIO_INFO_LOG(fmt, ...)
#define AUDIO_WARNING_LOG(fmt, ...)
#define AUDIO_ERR_LOG(fmt, ...)

#define CHECK_AND_RETURN(cond) do { if (!(cond)) return; } while(0)
#define CHECK_AND_RETURN_LOG(cond, fmt, ...) do { if (!(cond)) return; } while(0)
#define CHECK_AND_RETURN_RET(cond, ret) do { if (!(cond)) return ret; } while(0)
#define CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...) do { if (!(cond)) return ret; } while(0)
#define CHECK_AND_BREAK_LOG(cond, fmt, ...) do { if (!(cond)) break; } while(0)

// OHOS test macro compatibility for host build
namespace testing { namespace ext {
    struct TestSize { static const int Level0 = 0; };
}}
#define HWTEST_F(test_fixture, test_name, level) TEST_F(test_fixture, test_name)

#endif  // AUDIO_ENGINE_LOG_H
