/*
 * Stub header for x86 host build.
 * Provides minimal AudioSampleFormat enum without OHOS parcel.h dependency.
 */
#ifndef AUDIO_STREAM_INFO_H
#define AUDIO_STREAM_INFO_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {

enum AudioSampleFormat : uint32_t {
    SAMPLE_U8 = 0,
    SAMPLE_S16LE = 1,
    SAMPLE_S24LE = 2,
    SAMPLE_S32LE = 3,
    SAMPLE_F32LE = 4,
    INVALID_WIDTH = 0xFFFFFFFF,
};

}  // namespace AudioStandard
}  // namespace OHOS

#endif  // AUDIO_STREAM_INFO_H
