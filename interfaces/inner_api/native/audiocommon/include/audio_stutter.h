/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_STUTTER_H
#define AUDIO_STUTTER_H

#include <parcel.h>
#include <audio_stream_info.h>

namespace OHOS {
namespace AudioStandard {

enum DataTransferStateChangeType {
    AUDIO_STREAM_START,     // stream start
    AUDIO_STREAM_STOP,      // stream stop
    AUDIO_STREAM_PAUSE,     // stream pause
    DATA_TRANS_STOP,        // data transfer stop
    DATA_TRANS_RESUME,      // data transfer resume
};

enum BadDataTransferType {
    NO_DATA_TRANS,      // monitor none data transfer
    SILENCE_DATA_TRANS, // monitor silence data transfer
    MAX_DATATRANS_TYPE
};

struct AudioRendererDataTransferStateChangeInfo {
    int32_t clientPid;                              // client pid
    int32_t clientUID;                              // client uid
    int32_t sessionId;                              // session id
    StreamUsage streamUsage;                        // stream type
    DataTransferStateChangeType stateChangeType;
    bool isBackground;
    int32_t badDataRatio[MAX_DATATRANS_TYPE];

    AudioRendererDataTransferStateChangeInfo() = default;
    ~AudioRendererDataTransferStateChangeInfo() = default;
    bool Marshalling(Parcel &parcel) const
    {
        bool ret =  parcel.WriteInt32(clientPid) && parcel.WriteInt32(clientUID) &&
            parcel.WriteInt32(sessionId) && parcel.WriteInt32(static_cast<int32_t>(streamUsage)) &&
            parcel.WriteUint32(static_cast<int32_t>(stateChangeType)) &&
            parcel.WriteBool(isBackground);

        for (uint32_t i = 0; i < MAX_DATATRANS_TYPE; i++) {
            ret = ret && parcel.WriteInt32(badDataRatio[i]);
        }
        
        return ret;
    }
    void Unmarshalling(Parcel &parcel)
    {
        clientPid = parcel.ReadInt32();
        clientUID = parcel.ReadInt32();
        sessionId = parcel.ReadInt32();
        streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        stateChangeType = static_cast<DataTransferStateChangeType>(parcel.ReadInt32());
        isBackground = parcel.ReadBool();

        for (uint32_t i = 0; i < MAX_DATATRANS_TYPE; i++) {
            badDataRatio[i] = parcel.ReadInt32();
        }
    }
};

struct DataTransferMonitorParam {
    int32_t clientUID;
    int32_t badDataTransferTypeBitMap;
    int64_t timeInterval;
    int32_t badFramesRatio;

    DataTransferMonitorParam() = default;
    ~DataTransferMonitorParam() = default;
    bool operator==(const DataTransferMonitorParam& param) const
    {
        return clientUID == param.clientUID && badDataTransferTypeBitMap == param.badDataTransferTypeBitMap &&
        timeInterval == param.timeInterval && badFramesRatio == param.badFramesRatio;
    }
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(clientUID)
            && parcel.WriteInt32(badDataTransferTypeBitMap)
            && parcel.WriteInt32(timeInterval)
            && parcel.WriteInt32(badFramesRatio);
    }
    void Unmarshalling(Parcel &parcel)
    {
        clientUID = parcel.ReadInt32();
        badDataTransferTypeBitMap = parcel.ReadInt32();
        timeInterval = parcel.ReadInt32();
        badFramesRatio = parcel.ReadInt32();
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STUTTER_H
