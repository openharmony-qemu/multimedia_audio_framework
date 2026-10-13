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

#ifndef CLOUD_ROM_DOWNLOAD_NAPI_H
#define CLOUD_ROM_DOWNLOAD_NAPI_H

#include "napi/native_api.h"

// Cloud ROM download napi function declarations (C API wrappers)

napi_value CloudRomGetDownloadManager(napi_env env, napi_callback_info info);

napi_value CloudRomIsFeatureInstalled(napi_env env, napi_callback_info info);

napi_value CloudRomGetFeatureStatus(napi_env env, napi_callback_info info);

napi_value CloudRomGetDownloadStatus(napi_env env, napi_callback_info info);

napi_value CloudRomGetNewVersionInfo(napi_env env, napi_callback_info info);

napi_value CloudRomStartDownload(napi_env env, napi_callback_info info);

napi_value CloudRomPauseDownload(napi_env env, napi_callback_info info);

napi_value CloudRomCancelDownload(napi_env env, napi_callback_info info);

napi_value CloudRomStartBackgroundDownload(napi_env env, napi_callback_info info);

napi_value CloudRomUninstallFeature(napi_env env, napi_callback_info info);

napi_value CloudRomRegisterDownloadCallback(napi_env env, napi_callback_info info);

napi_value CloudRomUnregisterDownloadCallback(napi_env env, napi_callback_info info);

#endif // CLOUD_ROM_DOWNLOAD_NAPI_H
