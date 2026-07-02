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

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <sstream>
#include <condition_variable>
#include "audio_suite_sa_types.h"
#include "icloud_rom_download_callback.h"
#include "iupdate_engine_callback.h"
#include "cloud_rom_download_callback_impl.h"
#include "syspara/parameters.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// 常量定义
static const int32_t CLOUD_ROM_NETTYPE = 2 | 4;  // WIFI
static const int32_t CALLBACK_REGISTER_TIMEOUT_MS = 1000;  // 回调注册超时时间 1 秒

// 下载管理器
class DownloadManager {
public:
    DownloadManager();
    ~DownloadManager();

    // 初始化下载管理器
    int32_t Init();

    // 释放下载管理器
    int32_t Release();

    // 开始下载
    int32_t StartDownload(const std::string& version);

    // 取消下载
    int32_t CancelDownload();

    // 开始后台下载
    int32_t StartBackgroundDownload(const std::string& version);

    // 获取下载状态
    int32_t GetDownloadStatus(DownloadStatus& status, const std::string& version);

    // 注册下载状态变化回调
    void RegisterDownloadCallback(std::function<void(const DownloadStatus &)> callback);

    int32_t UninstallCloudRom(const std::string& version);

    // 设置 UpdateEngine 回调
    void SetUpdateEngineCallback(const sptr<IRemoteObject>& updateEngineCallback);

    // 等待回调注册完成（带超时）
    bool WaitForCallbackRegistration(int timeoutMs = CALLBACK_REGISTER_TIMEOUT_MS);

    // 处理云 ROM 回调（供 CloudRomDownloadCallbackImpl 调用）
    ErrCode OnCloudRomCallback(const CloudRomCallback &cloudRomData);

    // 注册云 ROM 回调
    bool RegisterCallback();

private:
    // 执行下载
    void DoDownload();

    // 更新下载状态
    void UpdateDownloadState(DownloadState state, int32_t errorCode = 0);

    // 更新下载状态（内部使用，调用方已持有锁）
    void UpdateDownloadStateInternal(DownloadState state, int32_t errorCode = 0);

    // 检查是否需要停止下载（条件不满足）
    bool ShouldStopDownload();

    // 通知下载状态变化
    void NotifyDownloadStatusChanged(const DownloadStatus &status);

    // 获取远程版本信息
    bool GetRemoteInfo(const std::string &version, bool useCache = true);

    // 下载并安装
    bool DownloadAndInstall(const std::string &version);

    // 注册云 ROM 下载回调到 audio_policy_ext
    bool RegisterCloudRomDownloadCallback();

    bool IsDownloaded(const std::string &modelName, bool useCache);

    // 新增：通过回调获取远程信息
    bool GetRemoteInfoViaCallback(const std::string &version, bool useCache);

    // 新增：通过回调检查是否已下载
    bool IsDownloadedViaCallback(const std::string &modelName, bool useCache);

    // 新增：通过回调下载并安装
    bool DownloadAndInstallViaCallback(const std::string &version);

    // 新增：通过回调取消下载
    void PauseDownloadCloudRomViaCallback(const std::string &version);

    // 新增：通过回调卸载云 ROM
    int32_t UninstallCloudRomViaCallback(const std::string& version);

    // GetDownloadStatus 辅助函数
    sptr<IUpdateEngineCallback> GetUpdateEngineCallbackProxy();
    int32_t GetFeatureStatusInfo(const sptr<IUpdateEngineCallback>& proxy,
        const std::string& version, StatusInfo& statusInfo);
    int32_t GetFeatureNewVersionInfo(const sptr<IUpdateEngineCallback>& proxy,
        const std::string& version, CloudRomNewVersionInfo& versionInfo);
    void FillDownloadStatus(DownloadStatus& status, const std::string& version,
        const StatusInfo& statusInfo, const CloudRomNewVersionInfo& versionInfo);

    // OnCloudRomCallback 辅助函数
    void ProcessCloudRomStatus(const StatusInfo &info);
    void HandleInstallSuccess(const StatusInfo &info);
    void HandleDownloadFail(const StatusInfo &info);
    void NotifyDownloadFailedAfterRetry();
    void HandleDownloading(const StatusInfo &info);

    // GetRemoteInfoViaCallback 辅助函数
    bool GetRemoteInfoFromProxy(const sptr<IUpdateEngineCallback>& proxy,
        const std::string& version);

    std::mutex mutex_;

    // 下载状态
    std::atomic<DownloadState> downloadState_;
    std::atomic<uint32_t> downloadProgress_;
    std::string currentVersion_;
    std::string romPath_;
    int32_t errorCode_;
    std::string errMessage_;

    // 下载状态标志
    std::atomic<bool> isDownloading_;
    std::atomic<int> downloadRetryCount_;
    std::atomic<bool> cancelRequested_;

    // UpdateEngine 相关
    std::string productVersion_;

    // 回调函数
    std::function<void(const DownloadStatus&)> downloadCallback_;

    // UpdateEngine 回调接口（用于调用 audio_policy_ext）
    sptr<IRemoteObject> updateEngineCallback_;
    std::mutex updateEngineCallbackMutex_;

    // 回调注册同步
    std::atomic<bool> isCallbackRegistered_;
    std::condition_variable callbackRegisteredCond_;
    std::mutex callbackMutex_;

    // 云 ROM 下载回调实现
    sptr<CloudRomDownloadCallbackImpl> cloudRomDownloadCallback_;

    // 云 ROM 回调处理互斥锁（防止并发回调）
    std::mutex cloudRomCallbackMutex_;

    // 是否为后台下载标志
    std::atomic<bool> isBackgroundDownload_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // DOWNLOAD_MANAGER_H