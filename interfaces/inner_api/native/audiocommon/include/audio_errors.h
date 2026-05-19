/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

/**
 * @addtogroup MultiMedia_AudioCommon
 * @{
 *
 * @brief Provides data types and audio formats required for recording and playing and recording audio.
 *
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file audio_errors.h
 *
 * @brief Declares the <b>audio_errors</b> class to define errors that may occur during audio operations.
 *
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef AUDIO_ERRORS_H
#define AUDIO_ERRORS_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
constexpr int MODULE_AUDIO = 1;
constexpr int SUBSYS_AUDIO = 30;
constexpr uint32_t OPEN_PORT_FAILURE = (uint32_t) -1;

using ErrCode = int32_t;
constexpr int SUBSYSTEM_BIT_NUM = 21;
constexpr int MODULE_BIT_NUM = 16;
constexpr int PROBLEM_CATEGORY_BIT_NUM = 24;
constexpr int OPERATION_TYPE_BIT_NUM = 16;
constexpr int BUSINESS_SCENARIO_BIT_NUM = 8;

constexpr ErrCode ErrCodeOffset(unsigned int subsystem, unsigned int module = 0)
{
    return (subsystem << SUBSYSTEM_BIT_NUM) | (module << MODULE_BIT_NUM);
}

constexpr int32_t BASE_AUDIO_ERR_OFFSET = -ErrCodeOffset(SUBSYS_AUDIO, MODULE_AUDIO);

/** Success */
const int32_t  SUCCESS = 0;

/** Fail */
const int32_t  ERROR = BASE_AUDIO_ERR_OFFSET; // base is -62980096

/** Status error */
const int32_t  ERR_ILLEGAL_STATE = BASE_AUDIO_ERR_OFFSET - 1;

/** Invalid parameter */
const int32_t  ERR_INVALID_PARAM = BASE_AUDIO_ERR_OFFSET - 2;

/** Early media preparation */
const int32_t  ERR_EARLY_PREPARE = BASE_AUDIO_ERR_OFFSET - 3;

/** Invalid operation */
const int32_t  ERR_INVALID_OPERATION = BASE_AUDIO_ERR_OFFSET - 4;

/** error operation failed */
const int32_t  ERR_OPERATION_FAILED = BASE_AUDIO_ERR_OFFSET - 5;

/** Buffer reading failed */
const int32_t  ERR_READ_BUFFER = BASE_AUDIO_ERR_OFFSET - 6;

/** Buffer writing failed */
const int32_t  ERR_WRITE_BUFFER = BASE_AUDIO_ERR_OFFSET - 7;

/**  Device not started */
const int32_t  ERR_NOT_STARTED = BASE_AUDIO_ERR_OFFSET - 8;

/**  Invalid Device handle */
const int32_t  ERR_INVALID_HANDLE = BASE_AUDIO_ERR_OFFSET - 9;

/**  unsupported operation */
const int32_t  ERR_NOT_SUPPORTED = BASE_AUDIO_ERR_OFFSET - 10;

/**  unsupported device */
const int32_t  ERR_DEVICE_NOT_SUPPORTED = BASE_AUDIO_ERR_OFFSET - 11;

/**  write operation failed */
const int32_t  ERR_WRITE_FAILED = BASE_AUDIO_ERR_OFFSET - 12;

/**  read operation failed */
const int32_t  ERR_READ_FAILED = BASE_AUDIO_ERR_OFFSET - 13;

/**  device init failed */
const int32_t  ERR_DEVICE_INIT = BASE_AUDIO_ERR_OFFSET - 14;

/** Invalid data size that has been read */
const int32_t  ERR_INVALID_READ = BASE_AUDIO_ERR_OFFSET - 15;

/** Invalid data size that has been written */
const int32_t  ERR_INVALID_WRITE = BASE_AUDIO_ERR_OFFSET - 16;

/** set invalid index < 0 */
const int32_t  ERR_INVALID_INDEX = BASE_AUDIO_ERR_OFFSET - 17;

/** focus request denied */
const int32_t  ERR_FOCUS_DENIED = BASE_AUDIO_ERR_OFFSET - 18;

/** incorrect render/capture mode */
const int32_t  ERR_INCORRECT_MODE = BASE_AUDIO_ERR_OFFSET - 19;

/** incorrect render/capture mode */
const int32_t  ERR_PERMISSION_DENIED = BASE_AUDIO_ERR_OFFSET - 20;

/** Memory alloc failed */
const int32_t  ERR_MEMORY_ALLOC_FAILED = BASE_AUDIO_ERR_OFFSET - 21;

/** microphone is disabled by EDM */
const int32_t ERR_MICROPHONE_DISABLED_BY_EDM = BASE_AUDIO_ERR_OFFSET - 22;

/** system permission denied */
const int32_t ERR_SYSTEM_PERMISSION_DENIED = BASE_AUDIO_ERR_OFFSET - 23;

/** callback not registered */
const int32_t ERR_CALLBACK_NOT_REGISTERED = BASE_AUDIO_ERR_OFFSET - 24;

/** need not switch device */
const int32_t ERR_NEED_NOT_SWITCH_DEVICE = BASE_AUDIO_ERR_OFFSET - 25;

const int32_t ERR_CONCEDE_INCOMING_STREAM = BASE_AUDIO_ERR_OFFSET - 26;

const int32_t ERR_RENDERER_IN_SERVER_UNDERRUN = BASE_AUDIO_ERR_OFFSET - 27;

/**exceed max audio stream cnt*/
const int32_t ERR_EXCEED_MAX_STREAM_CNT = BASE_AUDIO_ERR_OFFSET - 28;

/**exceed max audio stream cnt per uid*/
const int32_t ERR_EXCEED_MAX_STREAM_CNT_PER_UID = BASE_AUDIO_ERR_OFFSET - 29;

/**null pointer*/
const int32_t ERR_NULL_POINTER = BASE_AUDIO_ERR_OFFSET - 30;
 
/**multimode subscription failed*/
const int32_t ERR_MMI_SUBSCRIBE = BASE_AUDIO_ERR_OFFSET - 31;
 
/**multimode creation failed*/
const int32_t ERR_MMI_CREATION = BASE_AUDIO_ERR_OFFSET - 32;

/** set volume failed for not complying with safe volume regulation */
const int32_t ERR_SET_VOL_FAILED_BY_SAFE_VOL = BASE_AUDIO_ERR_OFFSET - 33;

/** add capture over limit */
const int32_t ERR_ADD_CAPTURE_OVER_LIMIT = BASE_AUDIO_ERR_OFFSET - 34;

/** add capture over limit */
const int32_t ERR_CONFIG_NAME_ERROR = BASE_AUDIO_ERR_OFFSET - 35;

/** retry in client */
const int32_t ERR_RETRY_IN_CLIENT = BASE_AUDIO_ERR_OFFSET - 36;

/** stream register num exceed max */
const int32_t ERR_AUDIO_STREAM_REGISTER_EXCEED_MAX = BASE_AUDIO_ERR_OFFSET - 37;

/** stream register repeat */
const int32_t ERR_AUDIO_STREAM_REGISTER_REPEAT = BASE_AUDIO_ERR_OFFSET - 38;

/** pro streams do not support for setting loudness*/
const int32_t ERR_PRO_STREAM_NOT_SUPPORTED = BASE_AUDIO_ERR_OFFSET - 39;

/** failed to set volume because isVolumeControlDisabled is true */
const int32_t ERR_SET_VOL_FAILED_BY_VOLUME_CONTROL_DISABLED = BASE_AUDIO_ERR_OFFSET - 40;

/** unsupported audio format, such as unsupported sample, channelCount format etc. */
const int32_t ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT = BASE_AUDIO_ERR_OFFSET - 41;

/** audio engine not exist. */
const int32_t ERR_AUDIO_SUITE_ENGINE_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 42;

/** audio pipeline not exist. */
const int32_t ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 43;

/** audio node not exist. */
const int32_t ERR_AUDIO_SUITE_NODE_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 44;

/** the connect or disconnect betwen the nodes is unsupported. */
const int32_t ERR_AUDIO_SUITE_UNSUPPORT_CONNECT = BASE_AUDIO_ERR_OFFSET - 45;

/** The number of created pipelines or nodes exceeds the system specification. */
const int32_t ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS = BASE_AUDIO_ERR_OFFSET - 46;

/** latency fetcher returns a default value without caching */
const int32_t ERR_LATENCY_DEFAULT_VALUE = BASE_AUDIO_ERR_OFFSET - 47;

/** Audio suite function timed out during execution. */
const int32_t ERR_AUDIO_SUITE_TIMEOUT = BASE_AUDIO_ERR_OFFSET - 47;

/** no memory to allocate */
const int32_t ERR_NO_MEMORY = BASE_AUDIO_ERR_OFFSET - 48;

/** object init failed */
const int32_t ERR_INIT_FAILED = BASE_AUDIO_ERR_OFFSET - 49;

/** operation timeout */
const int32_t ERR_TIMEOUT = BASE_AUDIO_ERR_OFFSET - 51;

/** service unavailable */
const int32_t ERR_SERVICE_UNAVAILABLE = BASE_AUDIO_ERR_OFFSET - 52;

/** result invalid */
const int32_t ERR_RESULT_INVALID = BASE_AUDIO_ERR_OFFSET - 53;

/** callback register failed */
const int32_t ERR_CALLBACK_REGISTER_FAILED = BASE_AUDIO_ERR_OFFSET - 54;

/** callback unregister failed */
const int32_t ERR_CALLBACK_UNREGISTER_FAILED = BASE_AUDIO_ERR_OFFSET - 55;

/** callback already registered */
const int32_t ERR_CALLBACK_ALREADY_REGISTERED = BASE_AUDIO_ERR_OFFSET - 56;

/** callback not found */
const int32_t ERR_CALLBACK_NOT_FOUND = BASE_AUDIO_ERR_OFFSET - 57;

/** device not found */
const int32_t ERR_DEVICE_NOT_FOUND = BASE_AUDIO_ERR_OFFSET - 58;

/** device mismatch */
const int32_t ERR_DEVICE_MISMATCH = BASE_AUDIO_ERR_OFFSET - 59;

/** device unavailable */
const int32_t ERR_DEVICE_UNAVAILABLE = BASE_AUDIO_ERR_OFFSET - 60;

/** privacy auth failed */
const int32_t ERR_PRIVACY_AUTH_FAILED = BASE_AUDIO_ERR_OFFSET - 61;

/** session unavailable */
const int32_t ERR_SESSION_UNAVAILABLE = BASE_AUDIO_ERR_OFFSET - 62;

/** Buffer too small */
const int32_t ERR_BUFFER_TOO_SMALL = BASE_AUDIO_ERR_OFFSET - 63;

/** Audio suite write failed. */
const int32_t ERR_AUDIO_SUITE_WRITE_FAILED = BASE_AUDIO_ERR_OFFSET - 64;

/** AVSession service not exist */
const int32_t ERR_SERVICE_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 65;

/** AVSession not exist */
const int32_t ERR_SESSION_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 66;

/** AVSession command not support */
const int32_t ERR_COMMAND_NOT_SUPPORT = BASE_AUDIO_ERR_OFFSET - 67;

/** AVSession controller not exist */
const int32_t ERR_CONTROLLER_NOT_EXIST = BASE_AUDIO_ERR_OFFSET - 68;

/** AVSession no permission */
const int32_t ERR_NO_PERMISSION = BASE_AUDIO_ERR_OFFSET - 69;

/** AVSession deactive */
const int32_t ERR_SESSION_DEACTIVE = BASE_AUDIO_ERR_OFFSET - 70;

/** AVSession controller is exist */
const int32_t ERR_CONTROLLER_IS_EXIST = BASE_AUDIO_ERR_OFFSET - 71;

/** AVSession command send exceed max */
const int32_t ERR_COMMAND_SEND_EXCEED_MAX = BASE_AUDIO_ERR_OFFSET - 72;

/** AVSession is exist */
const int32_t ERR_SESSION_IS_EXIST = BASE_AUDIO_ERR_OFFSET - 73;

/** AVSession desktop lyric not support */
const int32_t ERR_DESKTOPLYRIC_NOT_SUPPORT = BASE_AUDIO_ERR_OFFSET - 74;

/** AVSession desktop lyric not enable */
const int32_t ERR_DESKTOPLYRIC_NOT_ENABLE = BASE_AUDIO_ERR_OFFSET - 75;

/** Time out when saving HRTF on disk */
const int32_t ERR_SAVE_HRTF_TIMEOUT = BASE_AUDIO_ERR_OFFSET - 76;

/** Fail to save HRTF on disk. */
const int32_t ERR_SAVE_HRTF_FAIL = BASE_AUDIO_ERR_OFFSET - 77;

/** Ipc related error */
const int32_t ERR_IPC = BASE_AUDIO_ERR_OFFSET - 100;

/** Unknown error */
const int32_t ERR_UNKNOWN = BASE_AUDIO_ERR_OFFSET - 200;

/** success but not continue */
const int32_t SUCCESS_BUT_NOT_CONTINUE = BASE_AUDIO_ERR_OFFSET - 255;

const std::unordered_map<int32_t, std::string> ErrCode2ErrMessage = {
    {SUCCESS, "success"},
    {ERROR, "system error"},
    {ERR_ILLEGAL_STATE, "illegal state"},
    {ERR_INVALID_PARAM, "invalid parameter"},
    {ERR_PERMISSION_DENIED, "permission denied"},
};

inline std::string ErrMessage(int32_t ec)
{
    auto it = ErrCode2ErrMessage.find(ec);
    return it != ErrCode2ErrMessage.end() ? it->second : "unknown error";
}

enum class ProblemCategory : uint8_t {
    FAULT_GENERAL = 0x00,
    FAULT_NO_SOUND = 0x01,
    FAULT_LOW_VOLUME = 0x02,
    FAULT_STUTTER_NOISE = 0x03,
    FAULT_AUTO_STOP = 0x04,
    FAULT_VOLUME_ABNORMAL = 0x05,
    FAULT_WRONG_DEVICE = 0x06,
    FAULT_AUTO_PLAY = 0x07,
    FAULT_CONTROL_ABNORMAL = 0x08,
    FAULT_DEVICE_SWITCH_FAIL = 0x09,
    FAULT_STATE_INCONSISTENT = 0x0A,
    FAULT_WRONG_AUDIO_EFFECT = 0x0B,
    FAULT_SESSION_LOCAL_SET = 0x0C,
    FAULT_SESSION_LOCAL_GET = 0x0D,
    FAULT_SESSION_CAST_SET = 0x0E,
    FAULT_SESSION_CAST_GET = 0x0F,
};

enum class OperationType : uint8_t {
    GENERAL = 0x00,
    PLAY = 0x01,
    RECORD = 0x02,
    CONTROL = 0x03,
    EDIT = 0x04,
    HDPLAY = 0x05,
    AVSESSION = 0x06,
};

enum class BusinessScenario : uint8_t {
    GENERAL = 0x00,
    CREATE = 0x01,
    START = 0x02,
    SEND_DATA = 0x03,
    DEVICE_SWITCH = 0x04,
    FOCUS_EVENT = 0x05,
    PAUSE = 0x06,
    STOP = 0x07,
    FLUSH = 0x08,
    RELEASE = 0x09,
    QUERY = 0x0A,
    CALLBACK = 0x0B,
    CONFIG = 0x0C,
    RENDER = 0x0D,
    CONTROL = 0x0E,
};

constexpr int32_t BuildErrorCode(ProblemCategory category, OperationType operationType,
    BusinessScenario scenario, int32_t detail)
{
    return static_cast<int32_t>((static_cast<uint32_t>(category) << PROBLEM_CATEGORY_BIT_NUM) |
        (static_cast<uint32_t>(operationType) << OPERATION_TYPE_BIT_NUM) |
        (static_cast<uint32_t>(scenario) << BUSINESS_SCENARIO_BIT_NUM) |
        (static_cast<uint32_t>(BASE_AUDIO_ERR_OFFSET - detail) & 0xFFu));
}

// PLAY_CREATE
constexpr int32_t PLAY_CREATE_MEMORY_ALLOC_FAILED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_MEMORY_ALLOC_FAILED);
constexpr int32_t PLAY_CREATE_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_OPERATION_FAILED);
constexpr int32_t PLAY_CREATE_INVALID_PARAM = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_INVALID_PARAM);
constexpr int32_t PLAY_CREATE_SERVICE_UNAVAILABLE = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_SERVICE_UNAVAILABLE);
constexpr int32_t PLAY_CREATE_PERMISSION_DENIED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_PERMISSION_DENIED);
constexpr int32_t PLAY_CREATE_CALLBACK_REGISTER_FAILED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_CALLBACK_REGISTER_FAILED);
constexpr int32_t PLAY_CREATE_INVALID_INDEX = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_INVALID_INDEX);
constexpr int32_t PLAY_CREATE_EXCEED_MAX_STREAM_CNT = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_EXCEED_MAX_STREAM_CNT);
constexpr int32_t PLAY_CREATE_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::CREATE, ERR_NULL_POINTER);

// PLAY_START
constexpr int32_t PLAY_START_PERMISSION_DENIED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::START, ERR_PERMISSION_DENIED);
constexpr int32_t PLAY_START_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT,
    OperationType::PLAY, BusinessScenario::START, ERR_ILLEGAL_STATE);
constexpr int32_t PLAY_START_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::START, ERR_NULL_POINTER);
constexpr int32_t PLAY_START_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::START, ERR_OPERATION_FAILED);
constexpr int32_t PLAY_START_TIMEOUT = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::START, ERR_TIMEOUT);

// PLAY_SEND_DATA
constexpr int32_t PLAY_SEND_DATA_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::SEND_DATA, ERR_NULL_POINTER);

// PLAY_PAUSE
constexpr int32_t PLAY_PAUSE_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT,
    OperationType::PLAY, BusinessScenario::PAUSE, ERR_ILLEGAL_STATE);
constexpr int32_t PLAY_PAUSE_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::PAUSE, ERR_OPERATION_FAILED);
constexpr int32_t PLAY_PAUSE_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::PAUSE, ERR_NULL_POINTER);
constexpr int32_t PLAY_PAUSE_TIMEOUT = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::PAUSE, ERR_TIMEOUT);

// PLAY_STOP
constexpr int32_t PLAY_STOP_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT,
    OperationType::PLAY, BusinessScenario::STOP, ERR_ILLEGAL_STATE);
constexpr int32_t PLAY_STOP_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::STOP, ERR_NULL_POINTER);
constexpr int32_t PLAY_STOP_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::STOP, ERR_OPERATION_FAILED);
constexpr int32_t PLAY_STOP_TIMEOUT = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::STOP, ERR_TIMEOUT);

// PLAY_FLUSH
constexpr int32_t PLAY_FLUSH_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT,
    OperationType::PLAY, BusinessScenario::FLUSH, ERR_ILLEGAL_STATE);
constexpr int32_t PLAY_FLUSH_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::FLUSH, ERR_NULL_POINTER);
constexpr int32_t PLAY_FLUSH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_CONTROL_ABNORMAL,
    OperationType::PLAY, BusinessScenario::FLUSH, ERR_OPERATION_FAILED);

// PLAY_RELEASE
constexpr int32_t PLAY_RELEASE_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_NO_SOUND,
    OperationType::PLAY, BusinessScenario::RELEASE, ERR_NULL_POINTER);
constexpr int32_t PLAY_RELEASE_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::RELEASE, ERR_OPERATION_FAILED);

// PLAY_QUERY
constexpr int32_t PLAY_QUERY_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::QUERY, ERR_NULL_POINTER);

// PLAY_DEVICE_SWITCH
constexpr int32_t PLAY_DEVICE_SWITCH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_OPERATION_FAILED);

// PLAY_CONFIG
constexpr int32_t PLAY_CONFIG_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_GENERAL,
    OperationType::PLAY, BusinessScenario::CONFIG, ERR_NULL_POINTER);
constexpr int32_t PLAY_CONFIG_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT,
    OperationType::PLAY, BusinessScenario::CONFIG, ERR_ILLEGAL_STATE);

// HDPLAY Error Codes
// CREATE scenario
constexpr int32_t HDPLAY_CREATE_INVALID_HANDLE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::HDPLAY, BusinessScenario::CREATE, ERR_INVALID_HANDLE);

constexpr int32_t HDPLAY_CREATE_NOT_STARTED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::HDPLAY, BusinessScenario::CREATE, ERR_NOT_STARTED);

// START scenario
constexpr int32_t HDPLAY_START_NOT_STARTED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::HDPLAY, BusinessScenario::START, ERR_NOT_STARTED);

// SEND_DATA scenario
constexpr int32_t HDPLAY_SEND_DATA_WRITE_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::HDPLAY, BusinessScenario::SEND_DATA, ERR_WRITE_FAILED);

// PAUSE scenario
constexpr int32_t HDPLAY_PAUSE_NOT_STARTED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::PAUSE, ERR_NOT_STARTED);

// STOP scenario
constexpr int32_t HDPLAY_STOP_NOT_STARTED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::STOP, ERR_NOT_STARTED);

// FLUSH scenario
constexpr int32_t HDPLAY_FLUSH_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::FLUSH, ERR_OPERATION_FAILED);

// QUERY scenario
constexpr int32_t HDPLAY_QUERY_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::QUERY, ERR_OPERATION_FAILED);

// CALLBACK scenario
constexpr int32_t HDPLAY_CALLBACK_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::CALLBACK, ERR_OPERATION_FAILED);

// CONFIG scenario
constexpr int32_t HDPLAY_CONFIG_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);

constexpr int32_t HDPLAY_CONFIG_INVALID_HANDLE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::HDPLAY, BusinessScenario::CONFIG, ERR_INVALID_HANDLE);

constexpr int32_t HDPLAY_CONFIG_VOLUME_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::HDPLAY, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);

constexpr int32_t RECORD_CREATE_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE, ERR_NULL_POINTER);
constexpr int32_t RECORD_CREATE_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_CREATE_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE, ERR_INVALID_PARAM);
constexpr int32_t RECORD_CREATE_INVALID_INDEX = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE, ERR_INVALID_INDEX);
constexpr int32_t RECORD_CREATE_MEMORY_ALLOC_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CREATE, ERR_MEMORY_ALLOC_FAILED);
constexpr int32_t RECORD_CREATE_PERMISSION_DENIED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE, ERR_PERMISSION_DENIED);
constexpr int32_t RECORD_CREATE_INVALID_HANDLE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CREATE, ERR_INVALID_HANDLE);
constexpr int32_t RECORD_CREATE_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CREATE, ERR_ILLEGAL_STATE);

constexpr int32_t RECORD_START_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_CONTROL_ABNORMAL, OperationType::RECORD, BusinessScenario::START, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_START_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_START_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START, ERR_INVALID_PARAM);

constexpr int32_t RECORD_SEND_DATA_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::SEND_DATA, ERR_INVALID_PARAM);
constexpr int32_t RECORD_SEND_DATA_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::SEND_DATA, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_SEND_DATA_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::SEND_DATA, ERR_ILLEGAL_STATE);

constexpr int32_t RECORD_DEVICE_SWITCH_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_DEVICE_SWITCH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_DEVICE_SWITCH_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_INVALID_PARAM);
constexpr int32_t RECORD_DEVICE_SWITCH_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_NULL_POINTER);
constexpr int32_t RECORD_DEVICE_SWITCH_DEVICE_MISMATCH = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_DEVICE_MISMATCH);
constexpr int32_t RECORD_DEVICE_SWITCH_MEMORY_ALLOC_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_MEMORY_ALLOC_FAILED);

constexpr int32_t RECORD_PAUSE_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_CONTROL_ABNORMAL, OperationType::RECORD, BusinessScenario::PAUSE, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_PAUSE_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::PAUSE, ERR_OPERATION_FAILED);

constexpr int32_t RECORD_STOP_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_CONTROL_ABNORMAL, OperationType::RECORD, BusinessScenario::STOP, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_STOP_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::STOP, ERR_OPERATION_FAILED);

constexpr int32_t RECORD_FLUSH_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::FLUSH, ERR_ILLEGAL_STATE);

constexpr int32_t RECORD_RELEASE_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::RELEASE, ERR_ILLEGAL_STATE);

constexpr int32_t RECORD_QUERY_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::QUERY, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_QUERY_INVALID_HANDLE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::QUERY, ERR_INVALID_HANDLE);
constexpr int32_t RECORD_QUERY_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::QUERY, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_QUERY_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::QUERY, ERR_INVALID_PARAM);

constexpr int32_t RECORD_CALLBACK_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_CALLBACK_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_INVALID_PARAM);
constexpr int32_t RECORD_CALLBACK_MEMORY_ALLOC_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_MEMORY_ALLOC_FAILED);
constexpr int32_t RECORD_CALLBACK_INVALID_HANDLE = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_INVALID_HANDLE);
constexpr int32_t RECORD_CALLBACK_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_CALLBACK_TIMEOUT = BuildErrorCode(
    ProblemCategory::FAULT_CONTROL_ABNORMAL, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_TIMEOUT);
constexpr int32_t RECORD_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_NULL_POINTER);

constexpr int32_t RECORD_CONFIG_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CONFIG, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_CONFIG_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CONFIG, ERR_INVALID_PARAM);
constexpr int32_t RECORD_CONFIG_NOT_SUPPORTED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CONFIG, ERR_NOT_SUPPORTED);
constexpr int32_t RECORD_CONFIG_INVALID_OPERATION = BuildErrorCode(
    ProblemCategory::FAULT_CONTROL_ABNORMAL, OperationType::RECORD, BusinessScenario::CONFIG, ERR_INVALID_OPERATION);
constexpr int32_t RECORD_CONFIG_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_GENERAL, OperationType::RECORD, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);

// SESSION Error Codes (PLAY type for AudioSession)
constexpr int32_t SESSION_CALLBACK_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_INVALID_PARAM);
constexpr int32_t SESSION_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t SESSION_CALLBACK_REGISTER_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_CALLBACK_REGISTER_FAILED);
constexpr int32_t SESSION_CALLBACK_UNREGISTER_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_CALLBACK_UNREGISTER_FAILED);
constexpr int32_t SESSION_START_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_ILLEGAL_STATE);
constexpr int32_t SESSION_START_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_OPERATION_FAILED);
constexpr int32_t SESSION_START_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_INVALID_PARAM);
constexpr int32_t SESSION_START_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_NULL_POINTER);
constexpr int32_t SESSION_STOP_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_ILLEGAL_STATE);
constexpr int32_t SESSION_STOP_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_OPERATION_FAILED);
constexpr int32_t SESSION_STOP_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_INVALID_PARAM);
constexpr int32_t SESSION_STOP_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_NULL_POINTER);
constexpr int32_t SESSION_CONFIG_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_ILLEGAL_STATE);
constexpr int32_t SESSION_CONFIG_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);
constexpr int32_t SESSION_CONFIG_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_INVALID_PARAM);
constexpr int32_t SESSION_CONFIG_NOT_SUPPORTED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_NOT_SUPPORTED);
constexpr int32_t SESSION_CONFIG_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_NULL_POINTER);
constexpr int32_t SESSION_QUERY_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::QUERY, ERR_ILLEGAL_STATE);
constexpr int32_t SESSION_QUERY_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::QUERY, ERR_OPERATION_FAILED);
constexpr int32_t SESSION_QUERY_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::QUERY, ERR_NULL_POINTER);
constexpr int32_t SESSION_FOCUS_EVENT_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::FOCUS_EVENT, ERR_OPERATION_FAILED);
constexpr int32_t SESSION_FOCUS_EVENT_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::FOCUS_EVENT, ERR_NULL_POINTER);

// INTERRUPT Error Codes (PLAY type for AudioInterrupt playback)
constexpr int32_t INTERRUPT_CALLBACK_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_INVALID_PARAM);
constexpr int32_t INTERRUPT_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t INTERRUPT_CALLBACK_UNREGISTER_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CALLBACK, ERR_CALLBACK_UNREGISTER_FAILED);
constexpr int32_t INTERRUPT_START_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_OPERATION_FAILED);
constexpr int32_t INTERRUPT_START_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_INVALID_PARAM);
constexpr int32_t INTERRUPT_START_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_NULL_POINTER);
constexpr int32_t INTERRUPT_START_FOCUS_DENIED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::START, ERR_FOCUS_DENIED);
constexpr int32_t INTERRUPT_STOP_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_NULL_POINTER);
constexpr int32_t INTERRUPT_STOP_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::STOP, ERR_INVALID_PARAM);
constexpr int32_t INTERRUPT_CONFIG_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_ILLEGAL_STATE);
constexpr int32_t INTERRUPT_CONFIG_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);
constexpr int32_t INTERRUPT_CONFIG_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::CONFIG, ERR_INVALID_PARAM);
constexpr int32_t INTERRUPT_QUERY_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::QUERY, ERR_NULL_POINTER);
constexpr int32_t INTERRUPT_FOCUS_EVENT_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::PLAY, BusinessScenario::FOCUS_EVENT, ERR_OPERATION_FAILED);

// RECORD_INTERRUPT Error Codes (RECORD type for AudioInterrupt recording)
constexpr int32_t RECORD_INTERRUPT_CALLBACK_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_INVALID_PARAM);
constexpr int32_t RECORD_INTERRUPT_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t RECORD_INTERRUPT_START_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::START, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_INTERRUPT_START_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::START, ERR_INVALID_PARAM);
constexpr int32_t RECORD_INTERRUPT_START_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::START, ERR_NULL_POINTER);
constexpr int32_t RECORD_INTERRUPT_START_FOCUS_DENIED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::START, ERR_FOCUS_DENIED);
constexpr int32_t RECORD_INTERRUPT_STOP_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::STOP, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_INTERRUPT_STOP_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::STOP, ERR_NULL_POINTER);
constexpr int32_t RECORD_INTERRUPT_STOP_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::STOP, ERR_INVALID_PARAM);
constexpr int32_t RECORD_INTERRUPT_CONFIG_ILLEGAL_STATE = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CONFIG, ERR_ILLEGAL_STATE);
constexpr int32_t RECORD_INTERRUPT_CONFIG_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_INTERRUPT_CONFIG_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CONFIG, ERR_INVALID_PARAM);
constexpr int32_t RECORD_INTERRUPT_CONFIG_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::CONFIG, ERR_NULL_POINTER);
constexpr int32_t RECORD_INTERRUPT_QUERY_OPERATION_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::QUERY, ERR_OPERATION_FAILED);
constexpr int32_t RECORD_INTERRUPT_QUERY_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_AUTO_STOP, OperationType::RECORD, BusinessScenario::QUERY, ERR_NULL_POINTER);

// GENERAL_DEVICE_SWITCH
constexpr int32_t ERR_DEVICE_SWITCH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::GENERAL, BusinessScenario::DEVICE_SWITCH, ERR_OPERATION_FAILED);

// PLAY_DEVICE_SWITCH
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_OPERATION_FAILED);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_NULL_POINTER);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_INVALID_PARAM = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_INVALID_PARAM);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_DEVICE_NOT_FOUND = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_DEVICE_NOT_FOUND);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_PERMISSION_DENIED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::PLAY, BusinessScenario::DEVICE_SWITCH, ERR_SYSTEM_PERMISSION_DENIED);

// PLAY_DEVICE_SWITCH_CALLBACK
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_CALLBACK_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::PLAY,
    BusinessScenario::CALLBACK, ERR_OPERATION_FAILED);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::PLAY,
    BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t ERR_PLAY_DEVICE_SWITCH_CALLBACK_REGISTER_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::PLAY,
    BusinessScenario::CALLBACK, ERR_CALLBACK_REGISTER_FAILED);

// RECORD_DEVICE_SWITCH
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_OPERATION_FAILED);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_NULL_POINTER);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_INVALID_PARAM = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_INVALID_PARAM);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_ILLEGAL_STATE = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::DEVICE_SWITCH, ERR_ILLEGAL_STATE);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_PERMISSION_DENIED = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::RECORD,
    BusinessScenario::DEVICE_SWITCH, ERR_SYSTEM_PERMISSION_DENIED);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_MEMORY_ALLOC_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::RECORD,
    BusinessScenario::DEVICE_SWITCH, ERR_MEMORY_ALLOC_FAILED);

// RECORD_DEVICE_SWITCH_CALLBACK
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_CALLBACK_FAILED = BuildErrorCode(ProblemCategory::FAULT_DEVICE_SWITCH_FAIL,
    OperationType::RECORD, BusinessScenario::CALLBACK, ERR_OPERATION_FAILED);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_CALLBACK_NULL_POINTER = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::RECORD,
    BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t ERR_RECORD_DEVICE_SWITCH_CALLBACK_REGISTER_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_DEVICE_SWITCH_FAIL, OperationType::RECORD,
    BusinessScenario::CALLBACK, ERR_CALLBACK_REGISTER_FAILED);

// VOLUME_SET
constexpr int32_t ERR_SET_VOLUME_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY, BusinessScenario::CONFIG, ERR_OPERATION_FAILED);
constexpr int32_t ERR_SET_VOLUME_INVALID_PARAM = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY, BusinessScenario::CONFIG, ERR_INVALID_PARAM);
constexpr int32_t ERR_SET_VOLUME_NOT_SUPPORTED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY, BusinessScenario::CONFIG, ERR_NOT_SUPPORTED);
constexpr int32_t ERR_SET_VOLUME_PERMISSION_DENIED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY,
    BusinessScenario::CONFIG, ERR_SYSTEM_PERMISSION_DENIED);

// VOLUME_GET
constexpr int32_t ERR_GET_VOLUME_FAILED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY, BusinessScenario::QUERY, ERR_OPERATION_FAILED);
constexpr int32_t ERR_GET_VOLUME_NOT_SUPPORTED = BuildErrorCode(
    ProblemCategory::FAULT_VOLUME_ABNORMAL, OperationType::PLAY, BusinessScenario::QUERY, ERR_NOT_SUPPORTED);

// VOLUME_CALLBACK
constexpr int32_t ERR_VOLUME_CALLBACK_FAILED = BuildErrorCode(ProblemCategory::FAULT_VOLUME_ABNORMAL,
    OperationType::PLAY, BusinessScenario::CALLBACK, ERR_OPERATION_FAILED);
constexpr int32_t ERR_VOLUME_CALLBACK_NULL_POINTER = BuildErrorCode(ProblemCategory::FAULT_VOLUME_ABNORMAL,
    OperationType::PLAY, BusinessScenario::CALLBACK, ERR_NULL_POINTER);
constexpr int32_t ERR_VOLUME_CALLBACK_REGISTER_FAILED = BuildErrorCode(ProblemCategory::FAULT_VOLUME_ABNORMAL,
    OperationType::PLAY, BusinessScenario::CALLBACK, ERR_CALLBACK_REGISTER_FAILED);
constexpr int32_t ERR_VOLUME_CALLBACK_UNREGISTER_FAILED = BuildErrorCode(ProblemCategory::FAULT_VOLUME_ABNORMAL,
    OperationType::PLAY, BusinessScenario::CALLBACK, ERR_CALLBACK_UNREGISTER_FAILED);
	
// AVSESSION_CONTROL Error Codes
constexpr int32_t AVSESSION_CONTROL_NO_MEMORY_LOCAL_SET = BuildErrorCode(ProblemCategory::FAULT_SESSION_LOCAL_SET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_NO_MEMORY);
constexpr int32_t AVSESSION_CONTROL_NO_MEMORY_CAST_SET = BuildErrorCode(ProblemCategory::FAULT_SESSION_CAST_SET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_NO_MEMORY);

constexpr int32_t AVSESSION_CONTROL_INVALID_PARAM_LOCAL_SET = BuildErrorCode(ProblemCategory::FAULT_SESSION_LOCAL_SET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_INVALID_PARAM);
constexpr int32_t AVSESSION_CONTROL_INVALID_PARAM_LOCAL_GET = BuildErrorCode(ProblemCategory::FAULT_SESSION_LOCAL_GET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_INVALID_PARAM);
constexpr int32_t AVSESSION_CONTROL_INVALID_PARAM_CAST_SET = BuildErrorCode(ProblemCategory::FAULT_SESSION_CAST_SET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_INVALID_PARAM);
constexpr int32_t AVSESSION_CONTROL_INVALID_PARAM_CAST_GET = BuildErrorCode(ProblemCategory::FAULT_SESSION_CAST_GET,
    OperationType::AVSESSION, BusinessScenario::CONTROL, ERR_INVALID_PARAM);

constexpr int32_t AVSESSION_CONTROL_SERVICE_NOT_EXIST_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SERVICE_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SERVICE_NOT_EXIST_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SERVICE_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SERVICE_NOT_EXIST_CAST_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SERVICE_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SERVICE_NOT_EXIST_CAST_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SERVICE_NOT_EXIST);

constexpr int32_t AVSESSION_CONTROL_SESSION_NOT_EXIST_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SESSION_NOT_EXIST_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SESSION_NOT_EXIST_CAST_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_SESSION_NOT_EXIST_CAST_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_NOT_EXIST);

constexpr int32_t AVSESSION_CONTROL_COMMAND_NOT_SUPPORT_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_COMMAND_NOT_SUPPORT);
constexpr int32_t AVSESSION_CONTROL_COMMAND_NOT_SUPPORT_CAST_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_COMMAND_NOT_SUPPORT);

constexpr int32_t AVSESSION_CONTROL_CONTROLLER_NOT_EXIST_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_CONTROLLER_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_CONTROLLER_NOT_EXIST_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_CONTROLLER_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_CONTROLLER_NOT_EXIST_CAST_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_CONTROLLER_NOT_EXIST);
constexpr int32_t AVSESSION_CONTROL_CONTROLLER_NOT_EXIST_CAST_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_CAST_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_CONTROLLER_NOT_EXIST);

constexpr int32_t AVSESSION_CONTROL_NO_PERMISSION_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_NO_PERMISSION);
constexpr int32_t AVSESSION_CONTROL_NO_PERMISSION_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_NO_PERMISSION);

constexpr int32_t AVSESSION_CONTROL_SESSION_DEACTIVE_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_DEACTIVE);

constexpr int32_t AVSESSION_CONTROL_CONTROLLER_IS_EXIST_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_CONTROLLER_IS_EXIST);

constexpr int32_t AVSESSION_CONTROL_COMMAND_SEND_EXCEED_MAX_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_COMMAND_SEND_EXCEED_MAX);
constexpr int32_t AVSESSION_CONTROL_COMMAND_SEND_EXCEED_MAX_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_COMMAND_SEND_EXCEED_MAX);

constexpr int32_t AVSESSION_CONTROL_SESSION_IS_EXIST_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_SESSION_IS_EXIST);

constexpr int32_t AVSESSION_CONTROL_PERMISSION_DENIED_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_PERMISSION_DENIED);
constexpr int32_t AVSESSION_CONTROL_PERMISSION_DENIED_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_PERMISSION_DENIED);

constexpr int32_t AVSESSION_CONTROL_DESKTOPLYRIC_NOT_SUPPORT_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_DESKTOPLYRIC_NOT_SUPPORT);
constexpr int32_t AVSESSION_CONTROL_DESKTOPLYRIC_NOT_SUPPORT_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_DESKTOPLYRIC_NOT_SUPPORT);

constexpr int32_t AVSESSION_CONTROL_DESKTOPLYRIC_NOT_ENABLE_LOCAL_SET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_SET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_DESKTOPLYRIC_NOT_ENABLE);
constexpr int32_t AVSESSION_CONTROL_DESKTOPLYRIC_NOT_ENABLE_LOCAL_GET = BuildErrorCode(
    ProblemCategory::FAULT_SESSION_LOCAL_GET, OperationType::AVSESSION,
    BusinessScenario::CONTROL, ERR_DESKTOPLYRIC_NOT_ENABLE);
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_ERRORS_H
