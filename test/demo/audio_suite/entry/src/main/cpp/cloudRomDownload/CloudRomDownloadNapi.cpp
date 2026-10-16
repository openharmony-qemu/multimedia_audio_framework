/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "CloudRomDownloadNapi.h"
#include "hilog/log.h"
#include <string>
#include <vector>
#include <cstring>

#include "ohaudiosuite/native_audio_suite_download_manager.h"
#include "ohaudio/native_audio_common.h"

const int GLOBAL_RESMGR = 0xFF00;
static const char *TAG = "[AudioEditTestApp_CloudRomDownloadNapi_cpp]";

// Global download manager handle (initialized once)
static OH_AudioSuite_DownloadManager *g_cloudRomDownloadManager = nullptr;

// ---- Threadsafe function for download callback ----
static napi_threadsafe_function g_tsfnDownloadStatus = nullptr;

// C API expects a pointer-to-function-pointer, so we store the callback address here
static OH_AudioSuite_DownloadCallback g_downloadCallbackPtr = nullptr;

// Data structure passed from C callback to JS thread
struct DownloadStatusCallbackData {
    uint32_t count;
    // Flattened arrays for each field
    std::vector<std::string> featureNames;
    std::vector<int32_t> downloadStatuses;
    std::vector<int64_t> sizes;
    std::vector<std::string> installPaths;
    std::vector<int32_t> progresses;
    std::vector<int32_t> errorCodes;
};

// The C callback — may be called from a non-JS thread
static void CloudRomDownloadCallbackImpl(OH_AudioSuite_DownloadStatusInfoArray *infoArray)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomDownloadCallbackImpl called, size=%{public}u",
                 infoArray ? infoArray->size : 0);
    if (g_tsfnDownloadStatus == nullptr || infoArray == nullptr) {
        return;
    }

    auto *data = new DownloadStatusCallbackData();
    data->count = infoArray->size;
    for (uint32_t i = 0; i < infoArray->size; i++) {
        OH_AudioSuite_DownloadStatusInfo *info = infoArray->downloadStatusInfo[i];
        if (info == nullptr) {
            data->featureNames.push_back("");
            data->downloadStatuses.push_back(0);
            data->sizes.push_back(0);
            data->installPaths.push_back("");
            data->progresses.push_back(0);
            data->errorCodes.push_back(0);
            continue;
        }
        data->featureNames.push_back(info->featureName);
        data->downloadStatuses.push_back(info->downloadStatus);
        data->sizes.push_back(info->size);
        data->installPaths.push_back(info->installPath);
        data->progresses.push_back(info->progress);
        data->errorCodes.push_back(info->errorCode);
    }

    if (napi_call_threadsafe_function(g_tsfnDownloadStatus, data, napi_tsfn_blocking) != napi_ok) {
        delete data;
    }
}

// Called on the JS thread — builds napi object and invokes JS callback
static void CloudRomDownloadStatusCallJs(napi_env env, napi_value jsCallback, void *context, void *data)
{
    (void)context;
    auto *cbData = static_cast<DownloadStatusCallbackData *>(data);
    if (cbData == nullptr || jsCallback == nullptr) {
        delete cbData;
        return;
    }

    napi_value resultArray;
    napi_create_array_with_length(env, cbData->count, &resultArray);
    for (uint32_t i = 0; i < cbData->count; i++) {
        napi_value item;
        napi_create_object(env, &item);

        napi_value featureName;
        napi_create_string_utf8(env, cbData->featureNames[i].c_str(), NAPI_AUTO_LENGTH, &featureName);
        napi_set_named_property(env, item, "featureName", featureName);

        napi_value downloadStatus;
        napi_create_int32(env, cbData->downloadStatuses[i], &downloadStatus);
        napi_set_named_property(env, item, "downloadStatus", downloadStatus);

        napi_value size;
        napi_create_int64(env, cbData->sizes[i], &size);
        napi_set_named_property(env, item, "size", size);

        napi_value installPath;
        napi_create_string_utf8(env, cbData->installPaths[i].c_str(), NAPI_AUTO_LENGTH, &installPath);
        napi_set_named_property(env, item, "installPath", installPath);

        napi_value progress;
        napi_create_int32(env, cbData->progresses[i], &progress);
        napi_set_named_property(env, item, "progress", progress);

        napi_value errorCode;
        napi_create_int32(env, cbData->errorCodes[i], &errorCode);
        napi_set_named_property(env, item, "errorCode", errorCode);

        napi_set_element(env, resultArray, i, item);
    }

    napi_call_function(env, nullptr, jsCallback, 1, &resultArray, nullptr);
    delete cbData;
}

// Helper: ensure download manager is initialized, return true on success
static bool EnsureDownloadManager()
{
    if (g_cloudRomDownloadManager != nullptr) {
        return true;
    }
    int32_t ret = OH_AudioSuite_GetDownloadManager(&g_cloudRomDownloadManager);
    if (ret != 0 || g_cloudRomDownloadManager == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "OH_AudioSuite_GetDownloadManager failed: %{public}d", ret);
        return false;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "OH_AudioSuite_GetDownloadManager success");
    return true;
}

// Helper: parse string array from napi (argv[index]) into vector<string>
static bool ParseStringArray(napi_env env, napi_value arrayVal, std::vector<std::string> &out)
{
    bool isArray = false;
    napi_is_array(env, arrayVal, &isArray);
    if (!isArray) {
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, arrayVal, &length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value elem;
        napi_get_element(env, arrayVal, i, &elem);
        size_t len = 0;
        napi_get_value_string_utf8(env, elem, nullptr, 0, &len);
        std::vector<char> buf(len + 1, '\0');
        napi_get_value_string_utf8(env, elem, buf.data(), len + 1, &len);
        out.push_back(std::string(buf.data(), len));
    }
    return true;
}

// Helper: create a napi object with int32 code field
static napi_value CreateCodeResult(napi_env env, int32_t code)
{
    napi_value obj;
    napi_create_object(env, &obj);
    napi_value codeVal;
    napi_create_int32(env, code, &codeVal);
    napi_set_named_property(env, obj, "code", codeVal);
    return obj;
}

// Helper: create a napi object with code + boolean array
static napi_value CreateBoolArrayResult(napi_env env, int32_t code, const std::vector<bool> &bools)
{
    napi_value obj;
    napi_create_object(env, &obj);
    napi_value codeVal;
    napi_create_int32(env, code, &codeVal);
    napi_set_named_property(env, obj, "code", codeVal);

    napi_value arr;
    napi_create_array_with_length(env, bools.size(), &arr);
    for (size_t i = 0; i < bools.size(); i++) {
        napi_value bVal;
        napi_get_boolean(env, bools[i], &bVal);
        napi_set_element(env, arr, i, bVal);
    }
    napi_set_named_property(env, obj, "installStatus", arr);
    return obj;
}

// Helper: set a string property on a napi object
static void SetStringProp(napi_env env, napi_value obj, const char *name, const char *value)
{
    napi_value val;
    napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &val);
    napi_set_named_property(env, obj, name, val);
}

// Helper: set an int32 property on a napi object
static void SetInt32Prop(napi_env env, napi_value obj, const char *name, int32_t value)
{
    napi_value val;
    napi_create_int32(env, value, &val);
    napi_set_named_property(env, obj, name, val);
}

// Helper: set an int64 property on a napi object
static void SetInt64Prop(napi_env env, napi_value obj, const char *name, int64_t value)
{
    napi_value val;
    napi_create_int64(env, value, &val);
    napi_set_named_property(env, obj, name, val);
}

// Helper: convert OH_AudioSuite_DownloadStatusInfo to napi object
static napi_value DownloadStatusInfoToNapi(napi_env env, const OH_AudioSuite_DownloadStatusInfo &info)
{
    napi_value item;
    napi_create_object(env, &item);
    SetStringProp(env, item, "featureName", info.featureName);
    SetInt32Prop(env, item, "downloadStatus", info.downloadStatus);
    SetInt64Prop(env, item, "size", info.size);
    SetStringProp(env, item, "installPath", info.installPath);
    SetInt32Prop(env, item, "progress", info.progress);
    SetInt32Prop(env, item, "errorCode", info.errorCode);
    return item;
}

// Helper: convert OH_AudioSuite_FeatureVersionInfo to napi object
static napi_value VersionInfoToNapi(napi_env env, const OH_AudioSuite_FeatureVersionInfo &info)
{
    napi_value item;
    napi_create_object(env, &item);
    SetStringProp(env, item, "featureName", info.featureName);
    SetInt32Prop(env, item, "downloadStatus", info.downloadStatus);
    SetStringProp(env, item, "version", info.version);
    SetInt64Prop(env, item, "size", info.size);
    SetInt32Prop(env, item, "errorCode", info.errorCode);
    return item;
}

// Helper: build const char* pointer array from string vector
static std::vector<const char *> ToConstCharPtrs(const std::vector<std::string> &strs)
{
    std::vector<const char *> ptrs(strs.size());
    for (size_t i = 0; i < strs.size(); i++) {
        ptrs[i] = strs[i].c_str();
    }
    return ptrs;
}

// Helper: common preamble for functions that take a string[] arg and need download manager.
// Returns true on success, fills featureNames. On failure, sets resultOut to the error napi_value.
static bool ParseFeatureArg(napi_env env, napi_callback_info info, std::vector<std::string> &featureNames,
                            napi_value &resultOut)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        resultOut = CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        return false;
    }
    if (!EnsureDownloadManager()) {
        resultOut = CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
        return false;
    }
    if (!ParseStringArray(env, argv[0], featureNames) || featureNames.empty()) {
        resultOut = CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        return false;
    }
    return true;
}

// ---- Napi wrapper implementations ----

napi_value CloudRomGetDownloadManager(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetDownloadManager start");
    int32_t code = 0;
    if (g_cloudRomDownloadManager == nullptr) {
        code = OH_AudioSuite_GetDownloadManager(&g_cloudRomDownloadManager);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetDownloadManager result: %{public}d", code);
    return CreateCodeResult(env, code);
}

napi_value CloudRomIsFeatureInstalled(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomIsFeatureInstalled start");
    std::vector<std::string> featureNames;
    napi_value errResult;
    if (!ParseFeatureArg(env, info, featureNames, errResult)) {
        return errResult;
    }

    uint32_t count = static_cast<uint32_t>(featureNames.size());
    auto namePtrs = ToConstCharPtrs(featureNames);
    bool *installStatus = new bool[count]();
    std::vector<bool> installStatusVec(count, false);

    int32_t code = OH_AudioSuite_IsFeatureInstalled(g_cloudRomDownloadManager, namePtrs.data(), count,
                                                    installStatus, count);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomIsFeatureInstalled result: %{public}d", code);
    for (uint32_t i = 0; i < count; i++) {
        installStatusVec[i] = installStatus[i];
    }
    delete[] installStatus;
    return CreateBoolArrayResult(env, code, installStatusVec);
}

napi_value CloudRomGetFeatureStatus(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetFeatureStatus start");
    std::vector<std::string> featureNames;
    napi_value errResult;
    if (!ParseFeatureArg(env, info, featureNames, errResult)) {
        return errResult;
    }

    uint32_t count = static_cast<uint32_t>(featureNames.size());
    auto namePtrs = ToConstCharPtrs(featureNames);
    std::vector<OH_AudioSuite_DownloadStatusInfo> statusInfos(count);

    int32_t code = OH_AudioSuite_GetFeatureStatus(g_cloudRomDownloadManager, namePtrs.data(), count,
                                                  statusInfos.data(), count);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetFeatureStatus result: %{public}d", code);

    napi_value obj;
    napi_create_object(env, &obj);
    SetInt32Prop(env, obj, "code", code);

    napi_value arr;
    napi_create_array_with_length(env, count, &arr);
    for (uint32_t i = 0; i < count; i++) {
        napi_set_element(env, arr, i, DownloadStatusInfoToNapi(env, statusInfos[i]));
    }
    napi_set_named_property(env, obj, "statusInfos", arr);
    return obj;
}

napi_value CloudRomGetDownloadStatus(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetDownloadStatus start");
    std::vector<std::string> featureNames;
    napi_value errResult;
    if (!ParseFeatureArg(env, info, featureNames, errResult)) {
        return errResult;
    }

    napi_value obj;
    napi_create_object(env, &obj);

    napi_value arr;
    napi_create_array_with_length(env, featureNames.size(), &arr);
    for (size_t i = 0; i < featureNames.size(); i++) {
        OH_AudioSuite_DownloadStatusInfo statusInfo = {};
        int32_t code = OH_AudioSuite_GetDownloadStatus(g_cloudRomDownloadManager, featureNames[i].c_str(),
                                                       &statusInfo);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "CloudRomGetDownloadStatus [%{public}s] result: %{public}d",
                     featureNames[i].c_str(), code);
        napi_value item = DownloadStatusInfoToNapi(env, statusInfo);
        SetInt32Prop(env, item, "code", code);
        napi_set_element(env, arr, i, item);
    }
    napi_set_named_property(env, obj, "results", arr);
    return obj;
}

napi_value CloudRomGetNewVersionInfo(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetNewVersionInfo start");
    std::vector<std::string> featureNames;
    napi_value errResult;
    if (!ParseFeatureArg(env, info, featureNames, errResult)) {
        return errResult;
    }

    uint32_t count = static_cast<uint32_t>(featureNames.size());
    auto namePtrs = ToConstCharPtrs(featureNames);
    std::vector<OH_AudioSuite_FeatureVersionInfo> versionInfos(count);

    int32_t code = OH_AudioSuite_GetNewVersionInfo(g_cloudRomDownloadManager, namePtrs.data(), count,
                                                   versionInfos.data(), count);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomGetNewVersionInfo result: %{public}d", code);

    napi_value obj;
    napi_create_object(env, &obj);
    SetInt32Prop(env, obj, "code", code);

    napi_value arr;
    napi_create_array_with_length(env, count, &arr);
    for (uint32_t i = 0; i < count; i++) {
        napi_set_element(env, arr, i, VersionInfoToNapi(env, versionInfos[i]));
    }
    napi_set_named_property(env, obj, "versionInfos", arr);
    return obj;
}

// Helper for single-feature operations (StartDownload, PauseDownload, CancelDownload,
// StartBackgroundDownload, UninstallCloudRom)
using SingleFeatureFunc = int32_t (*)(OH_AudioSuite_DownloadManager *, const char *);

static napi_value CallSingleFeatureOp(napi_env env, napi_callback_info info, SingleFeatureFunc func,
                                      const char *opName)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "%{public}s start", opName);
    std::vector<std::string> featureNames;
    napi_value errResult;
    if (!ParseFeatureArg(env, info, featureNames, errResult)) {
        return errResult;
    }

    napi_value obj;
    napi_create_object(env, &obj);

    napi_value arr;
    napi_create_array_with_length(env, featureNames.size(), &arr);
    for (size_t i = 0; i < featureNames.size(); i++) {
        int32_t code = func(g_cloudRomDownloadManager, featureNames[i].c_str());
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "%{public}s [%{public}s] result: %{public}d",
                     opName, featureNames[i].c_str(), code);
        napi_value item;
        napi_create_object(env, &item);
        SetStringProp(env, item, "featureName", featureNames[i].c_str());
        SetInt32Prop(env, item, "code", code);
        napi_set_element(env, arr, i, item);
    }
    napi_set_named_property(env, obj, "results", arr);
    return obj;
}

napi_value CloudRomStartDownload(napi_env env, napi_callback_info info)
{
    return CallSingleFeatureOp(env, info, OH_AudioSuite_StartDownload, "CloudRomStartDownload");
}

napi_value CloudRomPauseDownload(napi_env env, napi_callback_info info)
{
    return CallSingleFeatureOp(env, info, OH_AudioSuite_PauseDownload, "CloudRomPauseDownload");
}

napi_value CloudRomCancelDownload(napi_env env, napi_callback_info info)
{
    return CallSingleFeatureOp(env, info, OH_AudioSuite_CancelDownload, "CloudRomCancelDownload");
}

napi_value CloudRomStartBackgroundDownload(napi_env env, napi_callback_info info)
{
    return CallSingleFeatureOp(env, info, OH_AudioSuite_StartBackgroundDownload, "CloudRomStartBackgroundDownload");
}

napi_value CloudRomUninstallFeature(napi_env env, napi_callback_info info)
{
    return CallSingleFeatureOp(env, info, OH_AudioSuite_UninstallCloudRom, "CloudRomUninstallFeature");
}

napi_value CloudRomRegisterDownloadCallback(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomRegisterDownloadCallback start");
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        return CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    }
    if (!EnsureDownloadManager()) {
        return CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
    }

    // If already registered, unregister first
    if (g_tsfnDownloadStatus != nullptr) {
        napi_release_threadsafe_function(g_tsfnDownloadStatus, napi_tsfn_abort);
        g_tsfnDownloadStatus = nullptr;
    }

    napi_value callback = argv[0];
    napi_value resourceName;
    napi_create_string_utf8(env, "CloudRomDownloadCallback", NAPI_AUTO_LENGTH, &resourceName);

    int32_t result = napi_create_threadsafe_function(env, callback, nullptr, resourceName, 0, 1, nullptr, nullptr,
                                                     nullptr, CloudRomDownloadStatusCallJs, &g_tsfnDownloadStatus);
    if (result != napi_ok || g_tsfnDownloadStatus == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "create threadsafe function failed: %{public}d", result);
        return CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
    }

    g_downloadCallbackPtr = CloudRomDownloadCallbackImpl;
    int32_t code = OH_AudioSuite_RegisterDownloadCallback(g_cloudRomDownloadManager, &g_downloadCallbackPtr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "OH_AudioSuite_RegisterDownloadCallback result: %{public}d", code);
    if (code != 0) {
        napi_release_threadsafe_function(g_tsfnDownloadStatus, napi_tsfn_abort);
        g_tsfnDownloadStatus = nullptr;
    }
    return CreateCodeResult(env, code);
}

napi_value CloudRomUnregisterDownloadCallback(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "CloudRomUnregisterDownloadCallback start");
    if (!EnsureDownloadManager()) {
        return CreateCodeResult(env, AUDIOCOMMON_RESULT_ERROR_SYSTEM);
    }

    int32_t code = OH_AudioSuite_UnregisterDownloadCallback(g_cloudRomDownloadManager, &g_downloadCallbackPtr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "OH_AudioSuite_UnregisterDownloadCallback result: %{public}d",
                 code);

    if (g_tsfnDownloadStatus != nullptr) {
        napi_release_threadsafe_function(g_tsfnDownloadStatus, napi_tsfn_abort);
        g_tsfnDownloadStatus = nullptr;
    }
    return CreateCodeResult(env, code);
}
