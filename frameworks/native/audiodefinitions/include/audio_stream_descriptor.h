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

#ifndef AUDIO_STREAM_DESCRIPTOR_H
#define AUDIO_STREAM_DESCRIPTOR_H

#include <memory>
#include <mutex>
#include "parcel.h"
#include "audio_device_descriptor.h"
#include "audio_stream_enum.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
enum AudioStreamAction : uint32_t {
    AUDIO_STREAM_ACTION_DEFAULT = 0,
    AUDIO_STREAM_ACTION_NEW,
    AUDIO_STREAM_ACTION_MOVE,
    AUDIO_STREAM_ACTION_RECREATE,
};

class AudioStreamDescriptor : public Parcelable {
public:
    AudioStreamInfo streamInfo_;
    AudioMode audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioFlag audioFlag_ = AUDIO_FLAG_NONE;
    uint32_t routeFlag_ = AUDIO_FLAG_NONE;
    uint32_t oldRouteFlag_ = AUDIO_FLAG_NONE;
    int64_t createTimeStamp_ = 0;
    int64_t startTimeStamp_ = 0;
    AudioRendererInfo rendererInfo_ = {};
    AudioCapturerInfo capturerInfo_ = {};
    AppInfo appInfo_ = {};
    uint32_t sessionId_ = 0;
    int32_t callerUid_ = -1;
    int32_t callerPid_ = -1;
    uint32_t rendererTarget_ = 0;
    AudioStreamStatus streamStatus_ = STREAM_STATUS_NEW;
    AudioStreamAction streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    AudioDeviceDescriptor preferredInputDevice = {};
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDeviceDescs_ = {};
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDescs_ = {};
    // for dup device
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDupDeviceDescs_ = {};
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDupDeviceDescs_ = {};
    std::string bundleName_ = "";
    int32_t oldOriginalFlag_ = AUDIO_FLAG_NORMAL;

    AudioStreamDescriptor() = default;
    AudioStreamDescriptor(AudioStreamInfo streamInfo, AudioRendererInfo rendererInfo, AppInfo appInfo);
    AudioStreamDescriptor(AudioStreamInfo streamInfo, AudioCapturerInfo rendererInfo, AppInfo appInfo);
    virtual ~AudioStreamDescriptor() = default;

    // Need to delete later
    void CopyToStruct(AudioStreamDescriptor &streamDesc);

    bool Marshalling(Parcel &parcel) const override;
    static AudioStreamDescriptor *Unmarshalling(Parcel &parcel);

    // log and dump
    void Dump(std::string &dumpString);
    std::string GetNewDevicesTypeString();
    std::string GetNewDupDevicesTypeString();
    std::string GetNewDevicesInfo();
    std::string GetDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> desc);

    // Common info funcs above
    bool IsPlayback() const
    {
        return (audioMode_ == AUDIO_MODE_PLAYBACK);
    }

    bool IsRecording() const
    {
        return (audioMode_ == AUDIO_MODE_RECORD);
    }

    uint32_t GetSessionId() const
    {
        return sessionId_;
    }

    bool IsRunning() const
    {
        return (streamStatus_ == STREAM_STATUS_STARTED);
    }

    StreamUsage GetRenderUsage() const
    {
        return rendererInfo_.streamUsage;
    }

    AudioPrivacyType GetRenderPrivacyType() const
    {
        return rendererInfo_.privacyType;
    }

    void SetStatus(AudioStreamStatus status)
    {
        streamStatus_ = status;
    }

    AudioStreamStatus GetStatus() const
    {
        return streamStatus_;
    }

    void SetAction(AudioStreamAction action)
    {
        streamAction_ = action;
    }

    void SetBunduleName(std::string &bundleName);

    std::string GetBundleName()
    {
        return bundleName_;
    }

    AudioStreamAction GetAction() const
    {
        return streamAction_;
    }

    AudioFlag GetAudioFlag() const
    {
        return audioFlag_;
    }

    void SetAudioFlag(AudioFlag flag)
    {
        audioFlag_ = flag;
    }

    bool IsUseMoveToConcedeType() const
    {
        return (audioFlag_ == AUDIO_OUTPUT_FLAG_LOWPOWER) ||
            (audioFlag_ == AUDIO_OUTPUT_FLAG_MULTICHANNEL);
    }

    bool IsMediaScene() const
    {
        return (rendererInfo_.streamUsage == STREAM_USAGE_MUSIC) ||
            (rendererInfo_.streamUsage == STREAM_USAGE_MOVIE) ||
            (rendererInfo_.streamUsage == STREAM_USAGE_GAME) ||
            (rendererInfo_.streamUsage == STREAM_USAGE_AUDIOBOOK);
    }

    // Route funcs above
    uint32_t GetRoute() const
    {
        return routeFlag_;
    }

    void SetRoute(uint32_t flag)
    {
        routeFlag_ = flag;
    }

    void SetOldRoute(uint32_t route)
    {
        oldRouteFlag_ = route;
    }

    uint32_t GetOldRoute() const
    {
        return oldRouteFlag_;
    }

    bool IsRouteNormal() const
    {
        if (IsPlayback()) {
            return (routeFlag_ == AUDIO_OUTPUT_FLAG_NORMAL);
        } else {
            return (routeFlag_ == AUDIO_INPUT_FLAG_NORMAL);
        }
    }

    bool IsRouteOffload() const
    {
        return (routeFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER);
    }

    bool IsSamePidUid(int32_t uid, int32_t pid) const
    {
        return callerPid_ == pid && callerUid_ == uid;
    }

    bool IsNoRunningOffload() const
    {
        return IsRouteOffload() && !IsRunning();
    }

    void ResetToNormalRoute(bool updateRoute);

    bool IsOldRouteOffload() const
    {
        return (oldRouteFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER);
    }

    bool IsRenderStreamNeedRecreate() const
    {
        return ((routeFlag_ & AUDIO_OUTPUT_FLAG_DIRECT) || (routeFlag_ & AUDIO_OUTPUT_FLAG_FAST));
    }

    bool IsSelectFlagOffload() const
    {
        return (audioFlag_ & AUDIO_OUTPUT_FLAG_LOWPOWER);
    }

    bool IsSelectFlagHd() const
    {
        return (audioFlag_ & AUDIO_OUTPUT_FLAG_HD);
    }

    void SetOriginalFlagForcedNormal()
    {
        rendererInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    }

    void ResetOriginalFlag()
    {
        rendererInfo_.originalFlag = rendererInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL ? oldOriginalFlag_:
            rendererInfo_.originalFlag;
    }

    // Device funcs above
    DeviceType GetMainNewDeviceType()
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (newDeviceDescs_.size() < 1 || newDeviceDescs_[0] == nullptr) {
            return DEVICE_TYPE_NONE;
        }
        return newDeviceDescs_[0]->getType();
    }

    bool IsA2dpOffloadStream()
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (newDeviceDescs_.size() < 1 || newDeviceDescs_[0] == nullptr) {
            return false;
        }
        return newDeviceDescs_[0]->IsA2dpOffload();
    }

    void AddNewDevice(std::shared_ptr<AudioDeviceDescriptor> device)
    {
        if (device == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(lock_);
        newDeviceDescs_.push_back(device);
    }

    void UpdateNewDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto &device : devices) {
            if (device == nullptr) {
                return;
            }
        }
        newDeviceDescs_ = devices;
    }

    void UpdateNewDeviceWithoutCheck(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
    {
        std::lock_guard<std::mutex> lock(lock_);
        newDeviceDescs_ = devices;
    }

    void UpdateOldDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices)
    {
        std::lock_guard<std::mutex> lock(lock_);
        oldDeviceDescs_ = devices;
    }

    bool IsDeviceRemote()
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto &device : newDeviceDescs_) {
            if (device != nullptr && device->IsRemote()) {
                return true;
            }
        }
        return false;
    }

    int32_t GetRealUid() const;

private:
    bool WriteDeviceDescVectorToParcel(
        Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs) const;
    void UnmarshallingDeviceDescVector(Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);

    void DumpCommonAttrs(std::string &dumpString);
    void DumpRendererStreamAttrs(std::string &dumpString);
    void DumpCapturerStreamAttrs(std::string &dumpString);
    void DumpDeviceAttrs(std::string &dumpString);

    std::mutex lock_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_DESCRIPTOR_H
