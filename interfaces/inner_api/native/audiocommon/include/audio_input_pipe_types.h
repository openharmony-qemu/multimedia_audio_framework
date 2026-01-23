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

#ifndef AUDIO_INPUT_PIPE_TYPES_H
#define AUDIO_INPUT_PIPE_TYPES_H

#include <vector>
#include <map>

#include "parcel.h"

#include "audio_info.h"

#ifndef AUDIO_CHECK_MACRO
#define AUDIO_CHECK_AND_RETURN_RET(cond, ret, ...)     \
    do {                                               \
        if (!(cond)) {                                 \
            return ret;                                \
        }                                              \
    } while (0)

#define AUDIO_CHECK_AND_RETURN(cond, ...)              \
    do {                                               \
        if (!(cond)) {                                 \
            return;                                    \
        }                                              \
    } while (0)
#endif

namespace OHOS {
namespace AudioStandard {

class AudioInputPipeInfo : public Parcelable {
public:
    AudioInputPipeInfo() = default;
    AudioInputPipeInfo(uint32_t id, HdiAdapterType adapter, uint32_t routeFlag)
        : id_(id), adapter_(adapter), routeFlag_(routeFlag)
    {};
    AudioInputPipeInfo(const AudioInputPipeInfo &pipeInfo) = default;
    virtual ~AudioInputPipeInfo() = default;

    uint32_t GetId()
    {
        return id_;
    }

    HdiAdapterType GetAdapter()
    {
        return adapter_;
    }

    uint32_t GetRouteFlag()
    {
        return routeFlag_;
    }

    AudioPipeStatus GetStatus()
    {
        return status_;
    }

    std::vector<DeviceType> GetDevices()
    {
        return devices_;
    }

    std::map<uint32_t, CapturerStreamInfo> GetStreams()
    {
        return streams_;
    }

    void SetStatus(AudioPipeStatus status)
    {
        status_ = status;
    }

    void SetDevice(DeviceType device)
    {
        devices_ = { device };
    }

    void SetDevices(const std::vector<DeviceType> &devices)
    {
        devices_ = devices;
    }

    void AddStream(uint32_t streamId, SourceType source, CapturerState state, std::string bundleName = "")
    {
        CapturerStreamInfo info;
        info.streamId_ = streamId;
        info.source_ = source;
        info.state_ = state;
        info.bundleName_ = bundleName;
        streams_[streamId] = info;
    }

    void RemoveStream(uint32_t streamId)
    {
        streams_.erase(streamId);
    }

    void RemoveAllStreams()
    {
        streams_.clear();
    }

    void UpdateStream(uint32_t streamId, CapturerState state)
    {
        if (streams_.find(streamId) != streams_.end()) {
            streams_[streamId].state_ = state;
        }
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteUint32(id_) &&
            parcel.WriteUint32(adapter_) &&
            parcel.WriteUint32(routeFlag_) &&
            parcel.WriteInt32(status_) &&
            WriteDeviceTypeVector(parcel, devices_) &&
            WriteCapturerStreamInfoMap(parcel, streams_);
    }

    static AudioInputPipeInfo *Unmarshalling(Parcel &parcel)
    {
        AudioInputPipeInfo *out = new (std::nothrow) AudioInputPipeInfo();
        AUDIO_CHECK_AND_RETURN_RET(out != nullptr, nullptr);

        out->id_ = parcel.ReadUint32();
        out->adapter_ = static_cast<HdiAdapterType>(parcel.ReadUint32());
        out->routeFlag_ = parcel.ReadUint32();
        out->status_ = static_cast<AudioPipeStatus>(parcel.ReadInt32());
        out->ReadDeviceTypeVector(parcel);
        out->ReadCapturerStreamInfoMap(parcel);
        return out;
    }

private:
    bool WriteDeviceTypeVector(Parcel &parcel, const std::vector<DeviceType> &devices) const
    {
        size_t vSize = devices.size();
        AUDIO_CHECK_AND_RETURN_RET(vSize <= MAX_COMMON_IPC_ARRAY_SIZE, false);
        bool ret = parcel.WriteUint32(vSize);
        AUDIO_CHECK_AND_RETURN_RET(ret, false);

        for (auto &device : devices) {
            ret = parcel.WriteInt32(static_cast<int32_t>(device));
            AUDIO_CHECK_AND_RETURN_RET(ret, false);
        }
        return true;
    }

    bool WriteCapturerStreamInfoMap(Parcel &parcel, const std::map<uint32_t, CapturerStreamInfo> &streams) const
    {
        size_t vSize = streams.size();
        AUDIO_CHECK_AND_RETURN_RET(vSize <= MAX_COMMON_IPC_ARRAY_SIZE, false);
        bool ret = parcel.WriteUint32(vSize);
        AUDIO_CHECK_AND_RETURN_RET(ret, false);

        for (auto &stream : streams) {
            ret = parcel.WriteUint32(stream.second.streamId_) &&
                parcel.WriteInt32(stream.second.state_) &&
                parcel.WriteInt32(stream.second.source_) &&
                parcel.WriteString(stream.second.bundleName_);
            AUDIO_CHECK_AND_RETURN_RET(ret, false);
        }
        return true;
    }

    void ReadDeviceTypeVector(Parcel &parcel)
    {
        size_t vSize = parcel.ReadUint32();
        AUDIO_CHECK_AND_RETURN(vSize <= MAX_COMMON_IPC_ARRAY_SIZE);
        devices_.clear();
        for (size_t i = 0; i < vSize; i++) {
            devices_.push_back(static_cast<DeviceType>(parcel.ReadInt32()));
        }
    }

    void ReadCapturerStreamInfoMap(Parcel &parcel)
    {
        size_t vSize = parcel.ReadUint32();
        AUDIO_CHECK_AND_RETURN(vSize <= MAX_COMMON_IPC_ARRAY_SIZE);
        for (size_t i = 0; i < vSize; i++) {
            CapturerStreamInfo stream;
            stream.streamId_ = parcel.ReadUint32();
            stream.state_ = static_cast<CapturerState>(parcel.ReadInt32());
            stream.source_ = static_cast<SourceType>(parcel.ReadInt32());
            stream.bundleName_ = parcel.ReadString();
            streams_[stream.streamId_] = stream;
        }
    }

private:
    uint32_t id_ = PIPE_ID_INVALID;

    HdiAdapterType adapter_ = HDI_ADAPTER_TYPE_UNKNOWN;

    uint32_t routeFlag_ = 0;

    AudioPipeStatus status_ = PIPE_STATUS_CLOSE;

    std::vector<DeviceType> devices_ = { DEVICE_TYPE_NONE };

    std::map<uint32_t, CapturerStreamInfo> streams_ = {};
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INPUT_PIPE_TYPES_H
