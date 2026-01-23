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

#ifndef I_AUDIO_CAPTURE_SOURCE_H
#define I_AUDIO_CAPTURE_SOURCE_H

#include <iostream>
#include <string>
#include "audio_info.h"
#include "audio_errors.h"
#include "audio_engine_callback_types.h"
#include "common/hdi_adapter_info.h"
#include "common/hdi_adapter_type.h"
#include "util/callback_wrapper.h"
#include "i_audio_source_callback.h"

#define SUCCESS_RET { return SUCCESS; }
#define NOT_SUPPORT_RET { return ERR_NOT_SUPPORTED; }

namespace OHOS {
namespace AudioStandard {

class IAudioCaptureSource {
public:
    virtual ~IAudioCaptureSource() = default;

    virtual int32_t Init(const IAudioSourceAttr &attr) = 0;
    virtual void DeInit(void) = 0;
    virtual bool IsInited(void) = 0;

    virtual int32_t Start(void) = 0;
    virtual int32_t Stop(void) = 0;
    virtual int32_t Resume(void) = 0;
    virtual int32_t Pause(void) = 0;
    virtual int32_t Flush(void) = 0;
    virtual int32_t Reset(void) = 0;
    virtual int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) NOT_SUPPORT_RET
    virtual int32_t CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
        uint64_t &replyBytesEc) NOT_SUPPORT_RET

    virtual void SetAudioParameter(const AudioParamKey key,
        const std::string &condition, const std::string &value) {};
    virtual std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) { return ""; }

    virtual int32_t SetVolume(float left, float right) = 0;
    virtual int32_t GetVolume(float &left, float &right) = 0;
    virtual int32_t SetMute(bool isMute) = 0;
    virtual int32_t GetMute(bool &isMute) = 0;

    virtual uint64_t GetTransactionId(void) = 0;
    virtual int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) = 0;
    virtual float GetMaxAmplitude(void) = 0;

    virtual int32_t SetAudioScene(AudioScene audioScene, bool scoExcludeFlag = false) NOT_SUPPORT_RET

    virtual int32_t UpdateActiveDevice(DeviceType inputDevice) NOT_SUPPORT_RET
    virtual int32_t UpdateSourceType(SourceType sourceType) SUCCESS_RET

    virtual int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) = 0;
    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) = 0;

    virtual void SetAddress(const std::string &address) {}
    virtual void SetInvalidState(void) {}

    virtual void DumpInfo(std::string &dumpString) {}

    virtual void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) {}
    virtual bool IsCaptureInvalid(void) NOT_SUPPORT_RET
    virtual int32_t GetArmUsbDeviceStatus() {return 0;}

    // mmap extend function
    virtual int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame, uint32_t &syncInfoSize) NOT_SUPPORT_RET
    virtual int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) NOT_SUPPORT_RET

    // Implement by self (begin)
    virtual void RegistCallback(uint32_t type, IAudioSourceCallback *callback);
    virtual void RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback);

    virtual void NotifyStreamChangeToSource(StreamChangeType change,
        uint32_t streamId, SourceType source, CapturerState state, uint32_t appUid = INVALID_UID);
    virtual std::shared_ptr<AudioInputPipeInfo> GetInputPipeInfo();
    // Implement by self (end)

protected:
    // Funcs to handle pipe info
    virtual void InitPipeInfo(uint32_t id, HdiAdapterType adapter, uint32_t routeFlag,
        std::vector<DeviceType> devices = { DEVICE_TYPE_NONE });
    virtual void ChangePipeStatus(AudioPipeStatus state);
    virtual void ChangePipeDevice(const std::vector<DeviceType> &devices);
    virtual void ChangePipeStream(StreamChangeType change,
        uint32_t streamId, SourceType source, CapturerState state, uint32_t appUid = INVALID_UID);
    virtual void DeinitPipeInfo();

    // Common variables
    SourceCallbackWrapper callback_ = {};

private:
    // For source info notify
    std::shared_ptr<AudioInputPipeInfo> pipeInfo_ = nullptr;
    std::mutex pipeLock_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_CAPTURE_SOURCE_H
