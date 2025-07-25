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
    int64_t createTimeStamp_ = 0;
    int64_t startTimeStamp_ = 0;
    AudioRendererInfo rendererInfo_ = {};
    AudioCapturerInfo capturerInfo_ = {};
    AppInfo appInfo_ = {};
    uint32_t sessionId_ = 0;
    int32_t callerUid_ = -1;
    int32_t callerPid_ = -1;
    AudioStreamStatus streamStatus_ = STREAM_STATUS_NEW;
    AudioStreamAction streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDeviceDescs_ = {};
    mutable std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDescs_ = {};
    std::string bundleName_ = "";

    AudioStreamDescriptor();
    virtual ~AudioStreamDescriptor();

    bool Marshalling(Parcel &parcel) const override;
    static AudioStreamDescriptor *Unmarshalling(Parcel &parcel);
    bool WriteDeviceDescVectorToParcel(
        Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs) const;
    void UnmarshallingDeviceDescVector(Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);

    void SetBunduleName(std::string &bundleName);

    // log and dump
    void Dump(std::string &dumpString);
    std::string GetNewDevicesTypeString();
    std::string GetNewDevicesInfo();
    std::string GetDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> desc);

private:
    bool IsRenderer()
    {
        return audioMode_ == AUDIO_MODE_PLAYBACK;
    }
    void DumpCommonAttrs(std::string &dumpString);
    void DumpRendererStreamAttrs(std::string &dumpString);
    void DumpCapturerStreamAttrs(std::string &dumpString);
    void DumpDeviceAttrs(std::string &dumpString);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_DESCRIPTOR_H
