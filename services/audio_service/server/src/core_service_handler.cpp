/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "CoreServiceHandler"
#endif

#include "core_service_handler.h"

#include "audio_errors.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {

}

CoreServiceHandler& CoreServiceHandler::GetInstance()
{
    static CoreServiceHandler CoreServiceHandler;
    return CoreServiceHandler;
}

CoreServiceHandler::CoreServiceHandler()
{
    AUDIO_INFO_LOG("Ctor");
}

CoreServiceHandler::~CoreServiceHandler()
{
    iCoreServiceProvider_ = nullptr;
    AUDIO_INFO_LOG("Dtor");
}

int32_t CoreServiceHandler::ConfigCoreServiceProvider(const sptr<ICoreServiceProviderIpc> coreServiceProvider)
{
    CHECK_AND_RETURN_RET_LOG(coreServiceProvider != nullptr, ERR_INVALID_PARAM, "Failed with null provider!");
    if (iCoreServiceProvider_ == nullptr) {
        iCoreServiceProvider_ = coreServiceProvider;
        return SUCCESS;
    }
    AUDIO_ERR_LOG("Provider is already configed!");
    return ERR_INVALID_OPERATION;
}

int32_t CoreServiceHandler::UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
    SessionOperationMsg opMsg)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    return iCoreServiceProvider_->UpdateSessionOperation(sessionId, operation, opMsg);
}

int32_t CoreServiceHandler::ReloadCaptureSession(uint32_t sessionId, SessionOperation operation)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    return iCoreServiceProvider_->ReloadCaptureSession(sessionId, operation);
}

int32_t CoreServiceHandler::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning, bool skipForce)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    return iCoreServiceProvider_->SetDefaultOutputDevice(defaultOutputDevice, sessionID, streamUsage, isRunning,
        skipForce);
}

std::string CoreServiceHandler::GetAdapterNameBySessionId(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, "", "iCoreServiceProvider_ is nullptr!");
    std::string ret{};
    iCoreServiceProvider_->GetAdapterNameBySessionId(sessionId, ret);
    return ret;
}

int32_t CoreServiceHandler::GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
    AudioStreamInfo &streamInfo, bool isReloadProcess)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    return iCoreServiceProvider_->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, streamInfo, isReloadProcess);
}

uint32_t CoreServiceHandler::GenerateSessionId()
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    uint32_t ret{};
    iCoreServiceProvider_->GenerateSessionId(ret);
    return ret;
}

int32_t CoreServiceHandler::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    int32_t ret = ERROR;
    iCoreServiceProvider_->SetWakeUpAudioCapturerFromAudioServer(config, ret);
    return ret;
}

uint32_t CoreServiceHandler::GetPaIndexByPortName(const std::string &portName)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    uint32_t ret = OPEN_PORT_FAILURE;
    iCoreServiceProvider_->GetPaIndexByPortName(portName, ret);
    return ret;
}

int32_t CoreServiceHandler::SetRendererTarget(uint32_t target, uint32_t lastTarget, uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "Injector::iCoreServiceProvider_ is nullptr!");
    int32_t ret = ERROR;
    iCoreServiceProvider_->SetRendererTarget(target, lastTarget, sessionId, ret);
    return ret;
}

int32_t CoreServiceHandler::StartInjection(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "Injector::iCoreServiceProvider_ is nullptr!");
    int32_t ret = ERROR;
    iCoreServiceProvider_->StartInjection(sessionId, ret);
    return ret;
}

void CoreServiceHandler::RemoveIdForInjector(uint32_t sessionId)
{
    CHECK_AND_RETURN_LOG(iCoreServiceProvider_ != nullptr, "Injector::iCoreServiceProvider_ is nullptr!");
    iCoreServiceProvider_->RemoveIdForInjector(sessionId);
}

void CoreServiceHandler::ReleaseCaptureInjector()
{
    CHECK_AND_RETURN_LOG(iCoreServiceProvider_ != nullptr, "iCoreServiceProvider_ is nullptr!");
    iCoreServiceProvider_->ReleaseCaptureInjector();
}

void CoreServiceHandler::RebuildCaptureInjector(uint32_t sessionId)
{
    CHECK_AND_RETURN_LOG(iCoreServiceProvider_ != nullptr, "Injector::iCoreServiceProvider_ is nullptr!");
    iCoreServiceProvider_->RebuildCaptureInjector(sessionId);
}

int32_t CoreServiceHandler::A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize,
                                                         uint32_t &timeStamp)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr!");
    int32_t ret = ERROR;
    iCoreServiceProvider_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp, ret);
    return ret;
}

void CoreServiceHandler::OnCheckActiveMusicTime(const std::string &reason)
{
    CHECK_AND_RETURN_LOG(iCoreServiceProvider_ != nullptr, "iCoreServiceProvider_ is nullptr!");
    iCoreServiceProvider_->OnCheckActiveMusicTime(reason);
}

int32_t CoreServiceHandler::CaptureConcurrentCheck(const uint32_t sessionID)
{
    CHECK_AND_RETURN_RET_LOG(iCoreServiceProvider_ != nullptr, ERROR, "iCoreServiceProvider_ is nullptr");
    return iCoreServiceProvider_->CaptureConcurrentCheck(sessionID);
}
} // namespace AudioStandard
} // namespace OHOS
