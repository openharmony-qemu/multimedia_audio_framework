/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Timeline_napi.h"
#include <string>
#include "/utils/Constant.h"
#include "utils/Utils.h"


std::vector<long> ParseLongArray(napi_env env, napi_value arrayValue, uint32_t trackIds_length)
{
    std::vector<long> trackIds;
    napi_valuetype type;
    for (uint32_t i = 0; i < trackIds_length; i++) {
        napi_value element;
        napi_get_element(env, arrayValue, i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_number) {
            napi_throw_type_error(env, "EINVAL", "indexs must contain only longs");
            return {};
        }
        long tempLong;
        napi_status status = napi_get_value_int64(env, element, &tempLong);
        trackIds.push_back(tempLong);
    }
    return trackIds;
}

napi_value AddAudioTrack(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    bool isSilent = false;
    status = napi_get_value_bool(env, argv[NAPI_ARGV_INDEX_1], &isSilent);
    delete [] argv;

    AudioTrack track{trackId, isSilent, {}};
    bool ret = Timeline::GetInstance().AddAudioTrack(track);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value DeleteAudioTrack(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    delete [] argv;

    bool ret = Timeline::GetInstance().DeleteAudioTrack(trackId);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

bool IsTypeOrArray(napi_env &env, napi_value *&argv, napi_valuetype &type, bool &isArray)
{
    napi_typeof(env, argv[NAPI_ARGV_INDEX_1], &type);
    if (type != napi_object) {
        napi_throw_type_error(env, "EINVAL", "isSilents must be an array");
        delete[] argv;
        return true;
    }
    napi_is_array(env, argv[NAPI_ARGV_INDEX_1], &isArray);
    if (!isArray) {
        napi_throw_type_error(env, "EINVAL", "isSilents must be an array");
        delete[] argv;
        return true;
    }
    return false;
}

bool ProcessIsArray(napi_env &env, napi_value *&argv, napi_valuetype &type, bool &isArray)
{
    if (type != napi_object) {
        napi_throw_type_error(env, "EINVAL", "trackIds must be an array");
        delete[] argv;
        return true;
    }
    napi_is_array(env, argv[NAPI_ARGV_INDEX_0], &isArray);
    if (!isArray) {
        napi_throw_type_error(env, "EINVAL", "trackIds must be an array");
        delete[] argv;
        return true;
    }
    return false;
}
bool IsNotEquels(napi_env &env, napi_value *&argv, uint32_t &trackIdsLength, uint32_t &isSilentLength)
{
    if (trackIdsLength != isSilentLength) {
        napi_throw_error(env, "EINVAL", "Arrays must be the same length");
        delete[] argv;
        return true;
    }
    return false;
}
napi_value SetAudioTrackSilent(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_valuetype type;
    napi_typeof(env, argv[NAPI_ARGV_INDEX_0], &type);
    bool isArray;
    if (ProcessIsArray(env, argv, type, isArray)) { return nullptr; }
    uint32_t trackIdsLength;
    napi_get_array_length(env, argv[NAPI_ARGV_INDEX_0], &trackIdsLength);
    std::vector<std::string> trackIds;
    for (uint32_t i = 0; i < trackIdsLength; i++) {
        napi_value element;
        napi_get_element(env, argv[NAPI_ARGV_INDEX_0], i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_string) {
            napi_throw_type_error(env, "EINVAL", "trackIds must contain only strings");
            delete [] argv;
            return nullptr;
        }
        char str[1024];
        size_t strLength;
        napi_get_value_string_utf8(env, element, str, sizeof(str), &strLength);
        trackIds.push_back(std::string(str, strLength));
    }
    if (IsTypeOrArray(env, argv, type, isArray)) { return nullptr; }
    uint32_t isSilentLength;
    napi_get_array_length(env, argv[NAPI_ARGV_INDEX_1], &isSilentLength);
    if (IsNotEquels(env, argv, trackIdsLength, isSilentLength)) { return nullptr; }
    std::vector<bool> isSilents;
    for (uint32_t i = 0; i < isSilentLength; i++) {
        napi_value element;
        napi_get_element(env, argv[NAPI_ARGV_INDEX_1], i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_boolean) {
            napi_throw_type_error(env, "EINVAL", "isSilents must contain only booleans");
            delete [] argv;
            return nullptr;
        }
        bool value;
        napi_get_value_bool(env, element, &value);
        isSilents.push_back(value);
    }
    delete [] argv;
    bool ret = Timeline::GetInstance().SetAudioTrackSilent(trackIds, isSilents);
    napi_value result;
    napi_get_boolean(env, ret, &result);
    return result;
}

napi_value AddAudioAsset(napi_env env, napi_callback_info info)
{
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long oldStartTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &oldStartTime);
    long newStartTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_2], &newStartTime);
    napi_valuetype type;
    napi_typeof(env, argv[NAPI_ARGV_INDEX_3], &type);
    if (type != napi_object) {
        napi_throw_type_error(env, "EINVAL", "indexs must be an object");
        delete [] argv;
        return {};
    }
    bool isArray;
    napi_is_array(env, argv[NAPI_ARGV_INDEX_3], &isArray);
    if (!isArray) {
        napi_throw_type_error(env, "EINVAL", "indexs must be an array");
        delete [] argv;
        return {};
    }
    uint32_t trackIdsLength;
    napi_get_array_length(env, argv[NAPI_ARGV_INDEX_3], &trackIdsLength);
    std::vector<long> indexs = ParseLongArray(env, argv[NAPI_ARGV_INDEX_3], trackIdsLength);
    if (indexs.empty() || indexs.size() < UINT_2) {
        napi_throw_type_error(env, "EINVAL", "Failed to parse indexs");
        delete [] argv;
        return {};
    }
    long startIndex = indexs[ARG_0];
    long endIndex = indexs[ARG_1];
    //平铺复制标记
    bool isCopyMultiple = false;
    status = napi_get_value_bool(env, argv[NAPI_ARGV_INDEX_4], &isCopyMultiple);
    delete [] argv;

    AudioAsset asset{
        startTime: newStartTime,
        pcmBufferLength: endIndex - startIndex,
    };
    bool ret = Timeline::GetInstance().AddAudioAsset(trackId, asset, oldStartTime, indexs, isCopyMultiple);
    ret = AddWriteDataBuffer(trackId, oldStartTime, asset.startTime, indexs, isCopyMultiple);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value UpdateAudioAsset(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long startTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &startTime);
    long startIndex = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_2], &startIndex);
    long endIndex = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_3], &endIndex);
    delete [] argv;

    AudioAsset asset{
        startTime: startTime,
        pcmBufferLength: endIndex - startIndex,
    };
    bool ret = Timeline::GetInstance().UpdateAudioAsset(trackId, asset, startIndex, endIndex);
    ret = UpdateWriteDataBuffer(trackId, asset.startTime, startIndex, endIndex);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value DeleteAudioAsset(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long startTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &startTime);
    delete [] argv;

    bool ret = Timeline::GetInstance().DeleteAudioAsset(trackId, startTime);
    ret = DeleteWriteDataBuffer(trackId, startTime);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value SetAudioAssetStartTime(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long originStartTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &originStartTime);
    long newStartTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_2], &newStartTime);
    delete [] argv;

    bool ret = Timeline::GetInstance().SetAudioAssetStartTime(trackId, originStartTime, newStartTime);
    ret = SetWriteDataBuffer(trackId, originStartTime, newStartTime);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value SetAudioAssetPcmBufferLength(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long startTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &startTime);
    long pcmBufferLength = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_2], &pcmBufferLength);
    delete [] argv;

    bool ret = Timeline::GetInstance().SetAudioAssetPcmBufferLength(trackId, startTime, pcmBufferLength);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value AddAudioAssetEffectNode(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long startTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &startTime);
    std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);
    delete [] argv;

    bool ret = Timeline::GetInstance().AddAudioAssetEffectNode(trackId, startTime, effectNodeId);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value DeleteAudioAssetEffectNode(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string trackId;
    napi_status status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], trackId);
    long startTime = 0;
    status = napi_get_value_int64(env, argv[NAPI_ARGV_INDEX_1], &startTime);
    std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);
    delete [] argv;

    bool ret = Timeline::GetInstance().DeleteAudioAssetEffectNode(trackId, startTime, effectNodeId);
    napi_value result;
    status = napi_get_boolean(env, ret, &result);
    return result;
}

napi_value ClearTimeline(napi_env env, napi_callback_info info)
{
    (void)info;
    Timeline::GetInstance().Clear();
    napi_value result;
    napi_status status = napi_get_undefined(env, &result);
    return result;
}
