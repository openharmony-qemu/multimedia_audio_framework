#include "audio_tool_calculate.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
namespace OHOS{
namespace AudioStandard {
#if USE_ARM_NEON == 1
// constexpr int ALIGIN_FLOAT_SIZE = 8;
#endif

inline bool Is16ByteAligned(const void *ptr) {
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (arrress & 0xF) == 0;
}

template <>
}
}