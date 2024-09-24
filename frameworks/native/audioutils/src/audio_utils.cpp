/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioUtils"
#endif

#include "audio_utils.h"
#include <cinttypes>
#include <ctime>
#include <sstream>
#include <ostream>
#include <climits>
#include <string>
#include "audio_utils_c.h"
#include "audio_errors.h"
#include "audio_log.h"
#ifdef FEATURE_HITRACE_METER
#include "hitrace_meter.h"
#endif
#include "parameter.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include "securec.h"
#include "privacy_error.h"

using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t UID_MSDP_SA = 6699;
constexpr int32_t UID_INTELLIGENT_VOICE_SA = 1042;
constexpr int32_t UID_CAAS_SA = 5527;
constexpr int32_t UID_DISTRIBUTED_AUDIO_SA = 3055;
constexpr int32_t UID_FOUNDATION_SA = 5523;
constexpr int32_t UID_DISTRIBUTED_CALL_SA = 3069;
constexpr int32_t UID_TELEPHONY_SA = 1001;
constexpr int32_t TIME_OUT_SECONDS = 10;

constexpr size_t FIRST_CHAR = 1;
constexpr size_t MIN_LEN = 8;
constexpr size_t HEAD_STR_LEN = 2;
constexpr size_t TAIL_STR_LEN = 5;

const int32_t DATA_INDEX_0 = 0;
const int32_t DATA_INDEX_1 = 1;
const int32_t DATA_INDEX_2 = 2;
const int32_t DATA_INDEX_3 = 3;
const int32_t DATA_INDEX_4 = 4;
const int32_t DATA_INDEX_5 = 5;
const int32_t STEREO_CHANNEL_COUNT = 2;

const std::set<int32_t> RECORD_ALLOW_BACKGROUND_LIST = {
#ifdef AUDIO_BUILD_VARIANT_ROOT
    0, // UID_ROOT
#endif
    UID_MSDP_SA,
    UID_INTELLIGENT_VOICE_SA,
    UID_CAAS_SA,
    UID_DISTRIBUTED_AUDIO_SA,
    UID_FOUNDATION_SA,
    UID_DISTRIBUTED_CALL_SA,
    UID_TELEPHONY_SA // used in distributed communication call
};

const std::set<SourceType> NO_BACKGROUND_CHECK_SOURCE_TYPE = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_REMOTE_CAST
};
}
int64_t ClockTime::GetCurNano()
{
    int64_t result = -1; // -1 for bad result.
    struct timespec time;
    clockid_t clockId = CLOCK_MONOTONIC;
    int ret = clock_gettime(clockId, &time);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, result,
        "GetCurNanoTime fail, result:%{public}d", ret);
    result = (time.tv_sec * AUDIO_NS_PER_SECOND) + time.tv_nsec;
    return result;
}

int32_t ClockTime::AbsoluteSleep(int64_t nanoTime)
{
    int32_t ret = -1; // -1 for bad result.
    CHECK_AND_RETURN_RET_LOG(nanoTime > 0, ret,
        "AbsoluteSleep invalid sleep time :%{public}" PRId64 " ns", nanoTime);
    struct timespec time;
    time.tv_sec = nanoTime / AUDIO_NS_PER_SECOND;
    time.tv_nsec = nanoTime - (time.tv_sec * AUDIO_NS_PER_SECOND); // Avoids % operation.

    clockid_t clockId = CLOCK_MONOTONIC;
    ret = clock_nanosleep(clockId, TIMER_ABSTIME, &time, nullptr);
    if (ret != 0) {
        AUDIO_WARNING_LOG("AbsoluteSleep may failed, ret is :%{public}d", ret);
    }

    return ret;
}

int32_t ClockTime::RelativeSleep(int64_t nanoTime)
{
    int32_t ret = -1; // -1 for bad result.
    CHECK_AND_RETURN_RET_LOG(nanoTime > 0, ret,
        "AbsoluteSleep invalid sleep time :%{public}" PRId64 " ns", nanoTime);
    struct timespec time;
    time.tv_sec = nanoTime / AUDIO_NS_PER_SECOND;
    time.tv_nsec = nanoTime - (time.tv_sec * AUDIO_NS_PER_SECOND); // Avoids % operation.

    clockid_t clockId = CLOCK_MONOTONIC;
    const int relativeFlag = 0; // flag of relative sleep.
    ret = clock_nanosleep(clockId, relativeFlag, &time, nullptr);
    if (ret != 0) {
        AUDIO_WARNING_LOG("RelativeSleep may failed, ret is :%{public}d", ret);
    }

    return ret;
}

void Trace::Count(const std::string &value, int64_t count)
{
#ifdef FEATURE_HITRACE_METER
    CountTrace(HITRACE_TAG_ZAUDIO, value, count);
#endif
}

void Trace::CountVolume(const std::string &value, uint8_t data)
{
#ifdef FEATURE_HITRACE_METER
    if (data == 0) {
        CountTrace(HITRACE_TAG_ZAUDIO, value, PCM_MAYBE_SILENT);
    } else {
        CountTrace(HITRACE_TAG_ZAUDIO, value, PCM_MAYBE_NOT_SILENT);
    }
#endif
}

Trace::Trace(const std::string &value)
{
    value_ = value;
    isFinished_ = false;
#ifdef FEATURE_HITRACE_METER
    StartTrace(HITRACE_TAG_ZAUDIO, value_);
#endif
}

void Trace::End()
{
#ifdef FEATURE_HITRACE_METER
    if (!isFinished_) {
        FinishTrace(HITRACE_TAG_ZAUDIO);
        isFinished_ = true;
    }
#endif
}

Trace::~Trace()
{
    End();
}

AudioXCollie::AudioXCollie(const std::string &tag, uint32_t timeoutSeconds,
    std::function<void(void *)> func, void *arg, uint32_t flag)
{
    AUDIO_DEBUG_LOG("Start AudioXCollie, tag: %{public}s, timeoutSeconds: %{public}u, flag: %{public}u",
        tag.c_str(), timeoutSeconds, flag);
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(tag, timeoutSeconds, func, arg, flag);
    tag_ = tag;
    isCanceled_ = false;
}

AudioXCollie::~AudioXCollie()
{
    CancelXCollieTimer();
}

void AudioXCollie::CancelXCollieTimer()
{
    if (!isCanceled_) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        isCanceled_ = true;
        AUDIO_DEBUG_LOG("CancelXCollieTimer: cancel timer %{public}s", tag_.c_str());
    }
}

bool PermissionUtil::VerifyIsShell()
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenTypeFlag == Security::AccessToken::TOKEN_SHELL) {
        return true;
    }
    return false;
}

bool PermissionUtil::VerifyIsSystemApp()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    bool tmp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_ERR_LOG("Check system app permission reject");
    return false;
}

bool PermissionUtil::VerifySelfPermission()
{
    Security::AccessToken::FullTokenID selfToken = IPCSkeleton::GetSelfTokenID();

    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(static_cast<uint32_t>(selfToken));

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_NATIVE, true);

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_SHELL, true);

    bool tmp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken);
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_ERR_LOG("Check self app permission reject");
    return false;
}

bool PermissionUtil::VerifySystemPermission()
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenTypeFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);

    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_NATIVE, true);
#ifdef AUDIO_BUILD_VARIANT_ROOT
    CHECK_AND_RETURN_RET(tokenTypeFlag != Security::AccessToken::TOKEN_SHELL, true);
#endif
    bool tmp = VerifyIsSystemApp();
    CHECK_AND_RETURN_RET(!tmp, true);

    AUDIO_ERR_LOG("Check system permission reject");
    return false;
}

bool PermissionUtil::VerifyPermission(const std::string &permissionName, uint32_t tokenId)
{
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    CHECK_AND_RETURN_RET_LOG(res == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, "Permission denied [%{public}s]", permissionName.c_str());

    return true;
}

bool PermissionUtil::NeedVerifyBackgroundCapture(int32_t callingUid, SourceType sourceType)
{
    if (RECORD_ALLOW_BACKGROUND_LIST.count(callingUid)) {
        AUDIO_INFO_LOG("internal sa(%{public}d) user directly recording", callingUid);
        return false;
    }
    if (NO_BACKGROUND_CHECK_SOURCE_TYPE.count(sourceType)) {
        AUDIO_INFO_LOG("sourceType %{public}d", sourceType);
        return false;
    }
    return true;
}

bool PermissionUtil::VerifyBackgroundCapture(uint32_t tokenId, uint64_t fullTokenId)
{
    Trace trace("PermissionUtil::VerifyBackgroundCapture");
    if (Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        AUDIO_INFO_LOG("system app recording");
        return true;
    }

    bool ret = Security::AccessToken::PrivacyKit::IsAllowedUsingPermission(tokenId, MICROPHONE_PERMISSION);
    if (!ret) {
        AUDIO_ERR_LOG("failed: not allowed!");
    }
    return ret;
}

bool PermissionUtil::NotifyPrivacy(uint32_t targetTokenId, AudioPermissionState state)
{
    AudioXCollie audioXCollie("PermissionUtil::NotifyPrivacy", TIME_OUT_SECONDS);
    if (state == AUDIO_PERMISSION_START) {
        Trace trace("PrivacyKit::StartUsingPermission");
        int res = Security::AccessToken::PrivacyKit::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        if (res != 0 && res != Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
            AUDIO_ERR_LOG("StartUsingPermission for tokenId %{public}u!, The PrivacyKit error code is %{public}d",
                targetTokenId, res);
            return false;
        }
        res = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(targetTokenId, MICROPHONE_PERMISSION, 1, 0);
        if (res != 0 && res != Security::AccessToken::ERR_PERMISSION_ALREADY_START_USING) {
            AUDIO_ERR_LOG("AddPermissionUsedRecord for tokenId %{public}u! The PrivacyKit error code is "
                "%{public}d", targetTokenId, res);
            return false;
        }
    } else if (state == AUDIO_PERMISSION_STOP) {
        Trace trace("PrivacyKit::StopUsingPermission");
        int res = Security::AccessToken::PrivacyKit::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        if (res != 0) {
            AUDIO_ERR_LOG("StopUsingPermission for tokenId %{public}u!, The PrivacyKit error code is %{public}d",
                targetTokenId, res);
            return false;
        }
    }
    return true;
}

void AdjustStereoToMonoForPCM8Bit(int8_t *data, uint64_t len)
{
    // the number 2: stereo audio has 2 channels
    uint64_t count = len / 2;

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustStereoToMonoForPCM16Bit(int16_t *data, uint64_t len)
{
    uint64_t count = len / 2 / 2;
    // first number 2: stereo audio has 2 channels
    // second number 2: the bit depth of PCM16Bit is 16 bits (2 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustStereoToMonoForPCM24Bit(uint8_t *data, uint64_t len)
{
    uint64_t count = len / STEREO_CHANNEL_COUNT / 3; // 3: the bit depth of PCM24Bit is 24 bits (3 bytes)

    while (count > 0) {
        uint32_t leftData = (static_cast<uint32_t>(data[DATA_INDEX_2]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_1]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_0]));
        uint32_t rightData = (static_cast<uint32_t>(data[DATA_INDEX_5]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_4]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_3]));

        leftData = static_cast<uint32_t>(static_cast<int32_t>(leftData << BIT_8) / STEREO_CHANNEL_COUNT +
            static_cast<int32_t>(rightData << BIT_8) / STEREO_CHANNEL_COUNT) >> BIT_8;
        rightData = leftData;

        data[DATA_INDEX_0] = static_cast<uint8_t>(leftData);
        data[DATA_INDEX_1] = static_cast<uint8_t>(leftData >> BIT_8);
        data[DATA_INDEX_2] = static_cast<uint8_t>(leftData >> BIT_16);
        data[DATA_INDEX_3] = static_cast<uint8_t>(rightData);
        data[DATA_INDEX_4] = static_cast<uint8_t>(rightData >> BIT_8);
        data[DATA_INDEX_5] = static_cast<uint8_t>(rightData >> BIT_16);
        data += 6; // 6: 2 channels, 24 bits (3 bytes), 2 * 3 = 6
        count--;
    }
}

void AdjustStereoToMonoForPCM32Bit(int32_t *data, uint64_t len)
{
    uint64_t count = len / 2 / 4;
    // first number 2: stereo audio has 2 channels
    // second number 4: the bit depth of PCM32Bit is 32 bits (4 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] = data[0] / 2 + data[1] / 2;
        data[1] = data[0];
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM8Bit(int8_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2;
    // the number 2: stereo audio has 2 channels

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM16Bit(int16_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2 / 2;
    // first number 2: stereo audio has 2 channels
    // second number 2: the bit depth of PCM16Bit is 16 bits (2 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

void AdjustAudioBalanceForPCM24Bit(uint8_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / STEREO_CHANNEL_COUNT / 3; // 3: the bit depth of PCM24Bit is 24 bits (3 bytes)

    while (count > 0) {
        uint32_t leftData = (static_cast<uint32_t>(data[DATA_INDEX_2]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_1]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_0]));
        int32_t leftTemp = static_cast<int32_t>(leftData << BIT_8);
        leftTemp *= left;
        leftData = static_cast<uint32_t>(leftTemp) >> BIT_8;
        data[DATA_INDEX_0] = static_cast<uint8_t>(leftData);
        data[DATA_INDEX_1] = static_cast<uint8_t>(leftData >> BIT_8);
        data[DATA_INDEX_2] = static_cast<uint8_t>(leftData >> BIT_16);

        uint32_t rightData = (static_cast<uint32_t>(data[DATA_INDEX_5]) << BIT_16) |
            (static_cast<uint32_t>(data[DATA_INDEX_4]) << BIT_8) |
            (static_cast<uint32_t>(data[DATA_INDEX_3]));
        int32_t rightTemp = static_cast<int32_t>(rightData << BIT_8);
        rightTemp *= right;
        rightData = static_cast<uint32_t>(rightTemp) >> BIT_8;
        data[DATA_INDEX_3] = static_cast<uint8_t>(rightData);
        data[DATA_INDEX_4] = static_cast<uint8_t>(rightData >> BIT_8);
        data[DATA_INDEX_5] = static_cast<uint8_t>(rightData >> BIT_16);
        data += 6; // 6: 2 channels, 24 bits (3 bytes), 2 * 3 = 6
        count--;
    }
}

void AdjustAudioBalanceForPCM32Bit(int32_t *data, uint64_t len, float left, float right)
{
    uint64_t count = len / 2 / 4;
    // first number 2: stereo audio has 2 channels
    // second number 4: the bit depth of PCM32Bit is 32 bits (4 bytes)

    while (count > 0) {
        // the number 2 is the count of stereo audio channels
        data[0] *= left;
        data[1] *= right;
        data += 2;
        count--;
    }
}

uint32_t Read24Bit(const uint8_t *p)
{
    return ((uint32_t) p[BIT_DEPTH_TWO] << BIT_16) | ((uint32_t) p[1] << BIT_8) | ((uint32_t) p[0]);
}

void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = (uint8_t) (u >> BIT_16);
    p[1] = static_cast<uint8_t>(u >> BIT_8);
    p[0] = static_cast<uint8_t>(u);
}

void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

float CapMax(float v)
{
    float value = v;
    if (v > 1.0f) {
        value = 1.0f - FLOAT_EPS;
    } else if (v < -1.0f) {
        value = -1.0f + FLOAT_EPS;
    }
    return value;
}

void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        Write24Bit(b, (static_cast<int32_t>(v)) >> BIT_8);
        b += OFFSET_BIT_24;
    }
}

void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        *(b++) = static_cast<int32_t>(v);
    }
}

float UpdateMaxAmplitude(ConvertHdiFormat adapterFormat, char *frame, uint64_t replyBytes)
{
    switch (adapterFormat) {
        case SAMPLE_U8_C: {
            return CalculateMaxAmplitudeForPCM8Bit(reinterpret_cast<int8_t *>(frame), replyBytes);
        }
        case SAMPLE_S16_C: {
            return CalculateMaxAmplitudeForPCM16Bit(reinterpret_cast<int16_t *>(frame),
                (replyBytes / sizeof(int16_t)));
        }
        case SAMPLE_S24_C: {
            return CalculateMaxAmplitudeForPCM24Bit(frame, (replyBytes / 3)); // 3 bytes
        }
        case SAMPLE_S32_C: {
            return CalculateMaxAmplitudeForPCM32Bit(reinterpret_cast<int32_t *>(frame),
                (replyBytes / sizeof(int32_t)));
        }
        default: {
            AUDIO_INFO_LOG("getMaxAmplitude: Unsupported audio format: %{public}d", adapterFormat);
            return 0;
        }
    }
}

float CalculateMaxAmplitudeForPCM8Bit(int8_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int8_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / SCHAR_MAX;
}

float CalculateMaxAmplitudeForPCM16Bit(int16_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int16_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / SHRT_MAX;
}

float CalculateMaxAmplitudeForPCM24Bit(char *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = 0; i < nSamples; ++i) {
        char *curPos = frame + (i * 3); // 3 bytes
        int curValue = 0;
        for (int j = 0; j < 3; ++j) { // 3 bytes
            curValue += (*(curPos + j) << (BIT_8 * j));
        }
        if (curValue < 0) {
            curValue = -curValue;
        }
        if (curMaxAmplitude < curValue) {
            curMaxAmplitude = curValue;
        }
    }
    return float(curMaxAmplitude) / MAX_VALUE_OF_SIGNED_24_BIT;
}

float CalculateMaxAmplitudeForPCM32Bit(int32_t *frame, uint64_t nSamples)
{
    int curMaxAmplitude = 0;
    for (uint32_t i = nSamples; i > 0; --i) {
        int32_t value = *frame++;
        if (value < 0) {
            value = -value;
        }
        if (curMaxAmplitude < value) {
            curMaxAmplitude = value;
        }
    }
    return float(curMaxAmplitude) / LONG_MAX;
}

template <typename T>
bool GetSysPara(const char *key, T &value)
{
    CHECK_AND_RETURN_RET_LOG(key != nullptr, false, "key is nullptr");
    char paraValue[30] = {0}; // 30 for system parameter
    auto res = GetParameter(key, "-1", paraValue, sizeof(paraValue));

    CHECK_AND_RETURN_RET_LOG(res > 0, false, "GetSysPara fail, key:%{public}s res:%{public}d", key, res);
    AUDIO_DEBUG_LOG("key:%{public}s value:%{public}s", key, paraValue);
    std::stringstream valueStr;
    valueStr << paraValue;
    valueStr >> value;
    return true;
}

template bool GetSysPara(const char *key, int32_t &value);
template bool GetSysPara(const char *key, uint32_t &value);
template bool GetSysPara(const char *key, int64_t &value);
template bool GetSysPara(const char *key, std::string &value);

std::map<std::string, std::string> DumpFileUtil::g_lastPara = {};

FILE *DumpFileUtil::OpenDumpFileInner(std::string para, std::string fileName, AudioDumpFileType fileType)
{
    std::string filePath;
    switch (fileType) {
        case AUDIO_APP:
            filePath = DUMP_APP_DIR + fileName;
            break;
        case OTHER_NATIVE_SERVICE:
            filePath = DUMP_SERVICE_DIR + fileName;
            break;
        case AUDIO_PULSE:
            filePath = DUMP_PULSE_DIR + fileName;
            break;
        default:
            AUDIO_ERR_LOG("Invalid AudioDumpFileType");
            break;
    }
    std::string dumpPara;
    FILE *dumpFile = nullptr;
    bool res = GetSysPara(para.c_str(), dumpPara);
    if (!res || dumpPara.empty()) {
        AUDIO_INFO_LOG("%{public}s is not set, dump audio is not required", para.c_str());
        g_lastPara[para] = dumpPara;
        return dumpFile;
    }
    AUDIO_DEBUG_LOG("%{public}s = %{public}s", para.c_str(), dumpPara.c_str());
    if (dumpPara == "w") {
        dumpFile = fopen(filePath.c_str(), "wb+");
        CHECK_AND_RETURN_RET_LOG(dumpFile != nullptr, dumpFile,
            "Error opening pcm dump file:%{public}s", filePath.c_str());
    } else if (dumpPara == "a") {
        dumpFile = fopen(filePath.c_str(), "ab+");
        CHECK_AND_RETURN_RET_LOG(dumpFile != nullptr, dumpFile,
            "Error opening pcm dump file:%{public}s", filePath.c_str());
    }
    if (dumpFile != nullptr) {
        AUDIO_INFO_LOG("Dump file path: %{public}s", filePath.c_str());
    }
    g_lastPara[para] = dumpPara;
    return dumpFile;
}

void DumpFileUtil::WriteDumpFile(FILE *dumpFile, void *buffer, size_t bufferSize)
{
    if (dumpFile == nullptr) {
        return;
    }
    CHECK_AND_RETURN_LOG(buffer != nullptr, "Invalid write param");
    size_t writeResult = fwrite(buffer, 1, bufferSize, dumpFile);
    CHECK_AND_RETURN_LOG(writeResult == bufferSize, "Failed to write the file.");
}

void DumpFileUtil::CloseDumpFile(FILE **dumpFile)
{
    if (*dumpFile) {
        fclose(*dumpFile);
        *dumpFile = nullptr;
    }
}

void DumpFileUtil::ChangeDumpFileState(std::string para, FILE **dumpFile, std::string filePath)
{
    CHECK_AND_RETURN_LOG(*dumpFile != nullptr, "Invalid file para");
    CHECK_AND_RETURN_LOG(g_lastPara[para] == "w" || g_lastPara[para] == "a", "Invalid input para");
    std::string dumpPara;
    bool res = GetSysPara(para.c_str(), dumpPara);
    if (!res || dumpPara.empty()) {
        AUDIO_WARNING_LOG("get %{public}s fail", para.c_str());
    }
    if (g_lastPara[para] == "w" && dumpPara == "w") {
        return;
    }
    CloseDumpFile(dumpFile);
    OpenDumpFile(para, filePath, dumpFile);
}

void DumpFileUtil::OpenDumpFile(std::string para, std::string fileName, FILE **file)
{
    if (*file != nullptr) {
        DumpFileUtil::ChangeDumpFileState(para, file, fileName);
        return;
    }

    if (para == DUMP_SERVER_PARA) {
        *file = DumpFileUtil::OpenDumpFileInner(para, fileName, AUDIO_PULSE);
    } else {
        *file = DumpFileUtil::OpenDumpFileInner(para, fileName, AUDIO_APP);
        if (*file == nullptr) {
            *file = DumpFileUtil::OpenDumpFileInner(para, fileName, OTHER_NATIVE_SERVICE);
        }
    }
}

static void MemcpyToI32FromI16(int16_t *src, int32_t *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        *(dst + i) = static_cast<int32_t>(*(src + i));
    }
}

static void MemcpyToI32FromI24(uint8_t *src, int32_t *dst, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        uint8_t *tmp = src + 3 * i; // 3 is byte size of SAMPLE_S24LE;
        *(dst + i) = static_cast<int32_t>(tmp[2] << (2 * sizeof(uint8_t))) |
            static_cast<int32_t>(tmp[1] << sizeof(uint8_t)) | static_cast<int32_t>(tmp[0]);
    }
}

bool NearZero(int16_t number)
{
    return number >= -DETECTED_ZERO_THRESHOLD && number <= DETECTED_ZERO_THRESHOLD;
}

std::string GetTime()
{
    std::string curTime;
    struct timeval tv;
    struct timezone tz;
    struct tm *t;
    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    curTime += std::to_string(YEAR_BASE + t->tm_year);
    curTime += (1 + t->tm_mon < DECIMAL_EXPONENT ? "0" + std::to_string(1 + t->tm_mon) :
        std::to_string(1 + t->tm_mon));
    curTime += (t->tm_mday < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_mday) :
        std::to_string(t->tm_mday));
    curTime += (t->tm_hour < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_hour) :
        std::to_string(t->tm_hour));
    curTime += (t->tm_min < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_min) :
        std::to_string(t->tm_min));
    curTime += (t->tm_sec < DECIMAL_EXPONENT ? "0" + std::to_string(t->tm_sec) :
        std::to_string(t->tm_sec));
    int64_t mSec = static_cast<int64_t>(tv.tv_usec / AUDIO_MS_PER_SECOND);
    curTime += (mSec < (DECIMAL_EXPONENT * DECIMAL_EXPONENT) ? (mSec < DECIMAL_EXPONENT ? "00" : "0")
        + std::to_string(mSec) : std::to_string(mSec));
    return curTime;
}

int32_t GetFormatByteSize(int32_t format)
{
    int32_t formatByteSize;
    switch (format) {
        case SAMPLE_S16LE:
            formatByteSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            formatByteSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            formatByteSize = 4; // size is 4
            break;
        default:
            formatByteSize = 2; // size is 2
            break;
    }
    return formatByteSize;
}

bool SignalDetectAgent::CheckAudioData(uint8_t *buffer, size_t bufferLen)
{
    CHECK_AND_RETURN_RET_LOG(formatByteSize_ != 0, false, "LatencyMeas checkAudioData failed, "
        "formatByteSize_ %{public}d", formatByteSize_);
    frameCountIgnoreChannel_ = bufferLen / static_cast<uint32_t>(formatByteSize_);
    if (cacheAudioData_.capacity() < frameCountIgnoreChannel_) {
        cacheAudioData_.clear();
        cacheAudioData_.reserve(frameCountIgnoreChannel_);
    }
    int32_t *cache = cacheAudioData_.data();
    if (sampleFormat_ == SAMPLE_S32LE) {
        int32_t ret = memcpy_s(cache, sizeof(int32_t) * cacheAudioData_.capacity(), buffer, bufferLen);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "LatencyMeas checkAudioData failed, dstSize "
            "%{public}zu, srcSize %{public}zu", sizeof(int32_t) * cacheAudioData_.capacity(), bufferLen);
    } else if (sampleFormat_ == SAMPLE_S24LE) {
        MemcpyToI32FromI24(buffer, cache, frameCountIgnoreChannel_);
    } else {
        int16_t *cp = reinterpret_cast<int16_t*>(buffer);
        MemcpyToI32FromI16(cp, cache, frameCountIgnoreChannel_);
    }
    if (DetectSignalData(cache, frameCountIgnoreChannel_)) {
        ResetDetectResult();
        return true;
    }
    return false;
}

bool SignalDetectAgent::DetectSignalData(int32_t *buffer, size_t bufferLen)
{
    std::string curTime = GetTime();
    uint32_t rightZeroSignal = 0;
    int32_t currentPeakIndex = -1;
    int32_t currentPeakSignal = SHRT_MIN;
    bool hasNoneZero = false;
    size_t frameCount = bufferLen / static_cast<size_t>(channels_);
    for (size_t index = 0; index < frameCount; index++) {
        int32_t tempMax = SHRT_MIN;
        int32_t tempMin = SHRT_MAX;
        for (uint32_t channel = 0; channel < static_cast<uint32_t>(channels_); channel++) {
            int32_t temp = buffer[index * static_cast<uint32_t>(channels_) + channel];
            tempMax = temp > tempMax ? temp : tempMax;
            tempMin = temp < tempMin ? temp : tempMin;
        }
        if (!NearZero(tempMax) || !NearZero(tempMin)) {
            rightZeroSignal = index + 1;
            hasNoneZero = true;
            if (currentPeakIndex == -1 || tempMax > currentPeakSignal) {
                currentPeakIndex = static_cast<int32_t>(index);
                currentPeakSignal = tempMax;
            }
        }
    }
    if (!hasNoneZero) {
        blankPeriod_ += static_cast<int32_t>(frameCount);
    } else {
        if (!hasFirstNoneZero_) {
            lastPeakBufferTime_ = curTime;
            hasFirstNoneZero_ = true;
        }
        if (currentPeakSignal > lastPeakSignal_) {
            lastPeakSignal_ = currentPeakSignal;
            lastPeakSignalPos_ = currentPeakIndex;
        }
        blankHaveOutput_ = false;
        blankPeriod_ = static_cast<int32_t>(frameCount - rightZeroSignal);
    }
    int32_t thresholdBlankPeriod = BLANK_THRESHOLD_MS * sampleRate_ / MILLISECOND_PER_SECOND;
    if (blankPeriod_ > thresholdBlankPeriod) {
        return !blankHaveOutput_;
    }
    return false;
}

void SignalDetectAgent::ResetDetectResult()
{
    blankHaveOutput_ = true;
    hasFirstNoneZero_ = false;
    lastPeakSignal_ = SHRT_MIN;
    signalDetected_ = true;
    dspTimestampGot_ = false;
    return;
}

bool AudioLatencyMeasurement::MockPcmData(uint8_t *buffer, size_t bufferLen)
{
    memset_s(buffer, bufferLen, 0, bufferLen);
    int16_t *signal = signalData_.get();
    size_t newlyMocked = bufferLen * MILLISECOND_PER_SECOND /
        static_cast<size_t>(channelCount_ * sampleRate_ * formatByteSize_);
    mockedTime_ += newlyMocked;
    if (mockedTime_ >= MOCK_INTERVAL) {
        mockedTime_ = 0;
        if (format_ == SAMPLE_S32LE) {
            MemcpyToI32FromI16(signal, reinterpret_cast<int32_t*>(buffer), SIGNAL_DATA_SIZE);
        } else {
            int32_t ret = memcpy_s(buffer, bufferLen, signal, SIGNAL_DATA_SIZE * sizeof(uint8_t));
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "LatencyMeas mockPcmData failed, dstSize "
                "%{public}zu, srcSize %{public}zu", bufferLen, SIGNAL_DATA_SIZE * sizeof(uint8_t));
        }
        return true;
    }
    return false;
}

AudioLatencyMeasurement::AudioLatencyMeasurement(const int32_t &sampleRate,
                                                 const int32_t &channelCount, const int32_t &sampleFormat,
                                                 const std::string &appName, const uint32_t &sessionId)
    :format_(sampleFormat),
     sampleRate_(sampleRate),
     channelCount_(channelCount),
     sessionId_(sessionId),
     appName_(appName)
{
    std::string appToMock = "com.example.null";
    GetSysPara("persist.multimedia.apptomock", appToMock);
    AUDIO_INFO_LOG("LatencyMeas appName:%{public}s, appToMock:%{public}s, g_sessionId:%{public}u",
        appName.c_str(), appToMock.c_str(), g_sessionToMock);
    if (appToMock == appName && g_sessionToMock == 0) {
        mockThisStream_ = true;
        g_sessionToMock = sessionId;
    }
    formatByteSize_ = GetFormatByteSize(sampleFormat);
    InitSignalData();
}

AudioLatencyMeasurement::~AudioLatencyMeasurement()
{
    if (mockThisStream_ && g_sessionToMock == sessionId_) {
        g_sessionToMock = 0;
    }
}

void AudioLatencyMeasurement::InitSignalData()
{
    signalData_ = std::make_unique<int16_t[]>(SIGNAL_DATA_SIZE);
    memset_s(signalData_.get(), SIGNAL_DATA_SIZE, 0, SIGNAL_DATA_SIZE);
    const int16_t channels = 2; // 2 channels
    const int16_t samplePerChannel = SIGNAL_DATA_SIZE / channels;
    int16_t *signalBuffer = signalData_.get();
    for (int16_t index = 0; index < samplePerChannel; index++) {
        signalBuffer[index * channels] = SIGNAL_THRESHOLD + static_cast<int16_t>(sinf(2.0f *
            static_cast<float>(M_PI) * index / samplePerChannel) * (SHRT_MAX - SIGNAL_THRESHOLD));
        for (int16_t k = 1; k < channels; k++) {
            signalBuffer[channels * index + k] = signalBuffer[channels * index];
        }
    }
    AUDIO_INFO_LOG("LatencyMeas signalData inited");
    return;
}

bool AudioLatencyMeasurement::CheckIfEnabled()
{
    int32_t latencyMeasure = -1;
    GetSysPara("persist.multimedia.audiolatency", latencyMeasure);
    return (latencyMeasure == 1);
}

LatencyMonitor& LatencyMonitor::GetInstance()
{
    static LatencyMonitor latencyMonitor_;
    return latencyMonitor_;
}

void LatencyMonitor::UpdateClientTime(bool isRenderer, std::string &timestamp)
{
    if (isRenderer) {
        rendererMockTime_ = timestamp;
    } else {
        capturerDetectedTime_ = timestamp;
    }
}

void LatencyMonitor::UpdateSinkOrSourceTime(bool isRenderer, std::string &timestamp)
{
    if (isRenderer) {
        sinkDetectedTime_ = timestamp;
    } else {
        sourceDetectedTime_ = timestamp;
    }
}

void LatencyMonitor::UpdateDspTime(std::string timestamp)
{
    dspDetectedTime_ = timestamp;
}

void LatencyMonitor::ShowTimestamp(bool isRenderer)
{
    if (extraStrLen_ == 0) {
        extraStrLen_ = dspDetectedTime_.find("20");
    }
    if (isRenderer) {
        if (dspDetectedTime_.length() == 0) {
            AUDIO_ERR_LOG("LatencyMeas GetExtraParameter failed!");
            AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, SinkDetectedTime:%{public}s",
                rendererMockTime_.c_str(), sinkDetectedTime_.c_str());
            return;
        }
        dspBeforeSmartPa_ = dspDetectedTime_.substr(extraStrLen_, DATE_LENGTH);
        dspAfterSmartPa_ = dspDetectedTime_.substr(extraStrLen_ + DATE_LENGTH + 1 +
            extraStrLen_, DATE_LENGTH);
        AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, SinkDetectedTime:%{public}s, "
                       "DspBeforeSmartPa:%{public}s, DspAfterSmartPa:%{public}s", rendererMockTime_.c_str(),
                       sinkDetectedTime_.c_str(), dspBeforeSmartPa_.c_str(), dspAfterSmartPa_.c_str());
    } else {
        if (dspDetectedTime_.length() == 0) {
            AUDIO_ERR_LOG("LatencyMeas GetExtraParam failed!");
            AUDIO_INFO_LOG("LatencyMeas CapturerDetectedTime:%{public}s, SourceDetectedTime:%{public}s",
                capturerDetectedTime_.c_str(), sourceDetectedTime_.c_str());
            return;
        }
        dspMockTime_ = dspDetectedTime_.substr(extraStrLen_ + DATE_LENGTH + extraStrLen_ + 1 +
            DATE_LENGTH + extraStrLen_ + 1, DATE_LENGTH);
        AUDIO_INFO_LOG("LatencyMeas CapturerDetectedTime:%{public}s, SourceDetectedTime:%{public}s, "
                       "DspMockTime:%{public}s", capturerDetectedTime_.c_str(), sourceDetectedTime_.c_str(),
                       dspMockTime_.c_str());
    }
}

void LatencyMonitor::ShowBluetoothTimestamp()
{
    AUDIO_INFO_LOG("LatencyMeas RendererMockTime:%{public}s, BTSinkDetectedTime:%{public}s",
        rendererMockTime_.c_str(), sinkDetectedTime_.c_str());
}

const std::string AudioInfoDumpUtils::GetStreamName(AudioStreamType streamType)
{
    std::string name;
    switch (streamType) {
        case STREAM_VOICE_ASSISTANT:
            name = "VOICE_ASSISTANT";
            break;
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
            name = "VOICE_CALL";

            break;
        case STREAM_SYSTEM:
            name = "SYSTEM";
            break;
        case STREAM_RING:
            name = "RING";
            break;
        case STREAM_MUSIC:
            name = "MUSIC";
            break;
        case STREAM_ALARM:
            name = "ALARM";
            break;
        case STREAM_NOTIFICATION:
            name = "NOTIFICATION";
            break;
        case STREAM_BLUETOOTH_SCO:
            name = "BLUETOOTH_SCO";
            break;
        case STREAM_DTMF:
            name = "DTMF";
            break;
        case STREAM_TTS:
            name = "TTS";
            break;
        case STREAM_ACCESSIBILITY:
            name = "ACCESSIBILITY";
            break;
        case STREAM_ULTRASONIC:
            name = "ULTRASONIC";
            break;
        case STREAM_WAKEUP:
            name = "WAKEUP";
            break;
        default:
            name = "UNKNOWN";
    }

    const std::string streamName = name;
    return streamName;
}

const std::string AudioInfoDumpUtils::GetDeviceTypeName(DeviceType deviceType)
{
    std::string device;
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
            device = "EARPIECE";
            break;
        case DEVICE_TYPE_SPEAKER:
            device = "SPEAKER";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            device = "WIRED_HEADSET";
            break;
        case DEVICE_TYPE_WIRED_HEADPHONES:
            device = "WIRED_HEADPHONES";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
             device = "BLUETOOTH_SCO";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            device = "BLUETOOTH_A2DP";
            break;
        case DEVICE_TYPE_MIC:
            device = "MIC";
            break;
        case DEVICE_TYPE_WAKEUP:
            device = "WAKEUP";
            break;
        case DEVICE_TYPE_NONE:
            device = "NONE";
            break;
        case DEVICE_TYPE_INVALID:
            device = "INVALID";
            break;
        default:
            device = "UNKNOWN";
    }

    const std::string deviceTypeName = device;
    return deviceTypeName;
}

const std::string AudioInfoDumpUtils::GetConnectTypeName(ConnectType connectType)
{
    std::string connectName;
    switch (connectType) {
        case OHOS::AudioStandard::CONNECT_TYPE_LOCAL:
            connectName = "LOCAL";
            break;
        case OHOS::AudioStandard::CONNECT_TYPE_DISTRIBUTED:
            connectName = "REMOTE";
            break;
        default:
            connectName = "UNKNOWN";
            break;
    }
    const std::string connectTypeName = connectName;
    return connectTypeName;
}

const std::string AudioInfoDumpUtils::GetSourceName(SourceType sourceType)
{
    std::string name;
    switch (sourceType) {
        case SOURCE_TYPE_INVALID:
            name = "INVALID";
            break;
        case SOURCE_TYPE_MIC:
            name = "MIC";
            break;
        case SOURCE_TYPE_VOICE_RECOGNITION:
            name = "VOICE_RECOGNITION";
            break;
        case SOURCE_TYPE_ULTRASONIC:
            name = "ULTRASONIC";
            break;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            name = "VOICE_COMMUNICATION";
            break;
        case SOURCE_TYPE_WAKEUP:
            name = "WAKEUP";
            break;
        default:
            name = "UNKNOWN";
    }

    const std::string sourceName = name;
    return sourceName;
}

const std::string AudioInfoDumpUtils::GetDeviceVolumeTypeName(DeviceVolumeType deviceType)
{
    std::string device;
    switch (deviceType) {
        case EARPIECE_VOLUME_TYPE:
            device = "EARPIECE";
            break;
        case SPEAKER_VOLUME_TYPE:
            device = "SPEAKER";
            break;
        case HEADSET_VOLUME_TYPE:
            device = "HEADSET";
            break;
        default:
            device = "UNKNOWN";
    }

    const std::string deviceTypeName = device;
    return deviceTypeName;
}

std::string GetEncryptStr(const std::string &src)
{
    if (src.empty()) {
        return std::string("");
    }

    size_t strLen = src.length();
    std::string dst;

    if (strLen < MIN_LEN) {
        // src: abcdef
        // dst: *bcdef
        dst = '*' + src.substr(FIRST_CHAR, strLen - FIRST_CHAR);
    } else {
        // src: 00:00:00:00:00:00
        // dst: 00**********00:00
        dst = src.substr(0, HEAD_STR_LEN);
        std::string tempStr(strLen - HEAD_STR_LEN - TAIL_STR_LEN, '*');
        dst += tempStr;
        dst += src.substr(strLen - TAIL_STR_LEN, TAIL_STR_LEN);
    }

    return dst;
}

AudioDump& AudioDump::GetInstance()
{
    static AudioDump mAudioDump;
    return mAudioDump;
}

void AudioDump::SetVersionType(const std::string& versionType)
{
    versionType_ = versionType;
}

std::string AudioDump::GetVersionType()
{
    return versionType_;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

struct CTrace {
    explicit CTrace(const char *traceName) : trace(OHOS::AudioStandard::Trace(traceName)) {};
    OHOS::AudioStandard::Trace trace;
};

CTrace *GetAndStart(const char *traceName)
{
    std::unique_ptr<CTrace> cTrace = std::make_unique<CTrace>(traceName);

    return cTrace.release();
}

void EndCTrace(CTrace *cTrace)
{
    if (cTrace != nullptr) {
        cTrace->trace.End();
    }
}

void CTraceCount(const char *traceName, int64_t count)
{
    OHOS::AudioStandard::Trace::Count(traceName, count);
}

void CallEndAndClear(CTrace **cTrace)
{
    if (cTrace != nullptr && *cTrace != nullptr) {
        EndCTrace(*cTrace);
        delete *cTrace;
        *cTrace = nullptr;
    }
}

#ifdef __cplusplus
}
#endif
