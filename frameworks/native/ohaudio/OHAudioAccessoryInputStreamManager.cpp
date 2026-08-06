/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "OHAudioAccessoryInputStreamManager"
#endif

#include "OHAudioAccessoryInputStreamManager.h"

#include "audio_common_log.h"

using OHOS::AudioStandard::AudioAccessoryInputStream;
using OHOS::AudioStandard::OHAudioAccessoryInputStreamManager;

namespace {
AudioAccessoryInputStream *ConvertStream(OH_AudioAccessoryInputStream *stream)
{
    return reinterpret_cast<AudioAccessoryInputStream *>(stream);
}
}

namespace OHOS {
namespace AudioStandard {

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::RegisterStartCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_StartCallback callback)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->RegisterStartCallback(callback);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::RegisterStopCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_StopCallback callback)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->RegisterStopCallback(callback);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::RegisterReleaseCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_ReleaseCallback callback)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->RegisterReleaseCallback(callback);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::RegisterLatencyCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_GetLatencyCallback callback)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->RegisterLatencyCallback(callback);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::RegisterFramePositionCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_GetFramePositionCallback callback)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->RegisterFramePositionCallback(callback);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::Write(OH_AudioAccessoryInputStream *stream,
    const uint8_t *data, uint32_t dataSize)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->Write(data, dataSize);
}

OH_AudioCommon_Result OHAudioAccessoryInputStreamManager::GetWritableSize(OH_AudioAccessoryInputStream *stream,
    uint32_t *writableSize)
{
    auto inputStream = ConvertStream(stream);
    CHECK_AND_RETURN_RET_LOG(inputStream != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "stream is null");
    return inputStream->GetWritableSize(writableSize);
}

} // namespace AudioStandard
} // namespace OHOS

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterStartCallback(OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_StartCallback callback)
{
    return OHAudioAccessoryInputStreamManager::RegisterStartCallback(stream, callback);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterStopCallback(OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_StopCallback callback)
{
    return OHAudioAccessoryInputStreamManager::RegisterStopCallback(stream, callback);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterReleaseCallback(OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_ReleaseCallback callback)
{
    return OHAudioAccessoryInputStreamManager::RegisterReleaseCallback(stream, callback);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterLatencyCallback(OH_AudioAccessoryInputStream *stream,
    OH_AudioAccessoryInputStream_GetLatencyCallback callback)
{
    return OHAudioAccessoryInputStreamManager::RegisterLatencyCallback(stream, callback);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_RegisterFramePositionCallback(
    OH_AudioAccessoryInputStream *stream, OH_AudioAccessoryInputStream_GetFramePositionCallback callback)
{
    return OHAudioAccessoryInputStreamManager::RegisterFramePositionCallback(stream, callback);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_Write(OH_AudioAccessoryInputStream *stream,
    const uint8_t *data, uint32_t dataSize)
{
    return OHAudioAccessoryInputStreamManager::Write(stream, data, dataSize);
}

OH_AudioCommon_Result OH_AudioAccessoryInputStreamManager_GetWritableSize(OH_AudioAccessoryInputStream *stream,
    uint32_t *writableSize)
{
    return OHAudioAccessoryInputStreamManager::GetWritableSize(stream, writableSize);
}
