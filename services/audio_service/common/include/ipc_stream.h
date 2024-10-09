/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef IPC_STREAM_H
#define IPC_STREAM_H

#include <memory>

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "audio_info.h"
#include "audio_process_config.h"
#include "i_stream_listener.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class IpcStream : public IRemoteBroker {
public:
    virtual ~IpcStream() = default;

    virtual int32_t RegisterStreamListener(sptr<IRemoteObject> object) = 0;

    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) = 0;

    virtual int32_t UpdatePosition() = 0;

    virtual int32_t GetAudioSessionID(uint32_t &sessionId) = 0;

    virtual int32_t Start() = 0;

    virtual int32_t Pause() = 0;

    virtual int32_t Stop() = 0;

    virtual int32_t Release() = 0;

    virtual int32_t Flush() = 0;

    virtual int32_t Drain(bool stopFlag = false) = 0;

    virtual int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) = 0;

    virtual int32_t GetAudioTime(uint64_t &framePos, uint64_t &timestamp) = 0;

    virtual int32_t GetAudioPosition(uint64_t &framePos, uint64_t &timestamp) = 0;

    virtual int32_t GetLatency(uint64_t &latency) = 0;

    virtual int32_t SetRate(int32_t rate) = 0; // SetRenderRate

    virtual int32_t GetRate(int32_t &rate) = 0; // SetRenderRate

    virtual int32_t SetLowPowerVolume(float volume) = 0; // renderer only

    virtual int32_t GetLowPowerVolume(float &volume) = 0; // renderer only

    virtual int32_t SetAudioEffectMode(int32_t effectMode) = 0; // renderer only

    virtual int32_t GetAudioEffectMode(int32_t &effectMode) = 0; // renderer only

    virtual int32_t SetPrivacyType(int32_t privacyType) = 0; // renderer only

    virtual int32_t GetPrivacyType(int32_t &privacyType) = 0; // renderer only

    virtual int32_t SetOffloadMode(int32_t state, bool isAppBack) = 0; // renderer only

    virtual int32_t UnsetOffloadMode() = 0; // renderer only

    virtual int32_t GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
        uint64_t &cacheTimeDsp, uint64_t &cacheTimePa) = 0; // renderer only

    virtual int32_t OffloadSetVolume(float volume) = 0; // renderer only

    virtual int32_t UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled) = 0; // rendererOnly

    virtual int32_t GetStreamManagerType() = 0;

    virtual int32_t SetSilentModeAndMixWithOthers(bool on) = 0;

    virtual int32_t SetClientVolume() = 0;

    virtual int32_t RegisterThreadPriority(uint32_t tid, const std::string &bundleName) = 0;
    // IPC code.
    enum IpcStreamMsg : uint32_t {
        ON_REGISTER_STREAM_LISTENER = 0,
        ON_RESOLVE_BUFFER,
        ON_UPDATE_POSITION,
        ON_GET_AUDIO_SESSIONID,
        ON_START,
        ON_PAUSE,
        ON_STOP,
        ON_RELEASE,
        ON_FLUSH,
        ON_DRAIN,
        ON_UPDATA_PLAYBACK_CAPTURER_CONFIG,
        OH_GET_AUDIO_TIME,
        OH_GET_AUDIO_POSITION,
        ON_GET_LATENCY,
        ON_SET_RATE,
        ON_GET_RATE,
        ON_SET_LOWPOWER_VOLUME,
        ON_GET_LOWPOWER_VOLUME,
        ON_SET_EFFECT_MODE,
        ON_GET_EFFECT_MODE,
        ON_SET_PRIVACY_TYPE,
        ON_GET_PRIVACY_TYPE,
        ON_SET_OFFLOAD_MODE,
        ON_UNSET_OFFLOAD_MODE,
        ON_GET_OFFLOAD_APPROXIMATELY_CACHE_TIME,
        ON_SET_OFFLOAD_VOLUME,
        ON_UPDATE_SPATIALIZATION_STATE,
        ON_GET_STREAM_MANAGER_TYPE,
        ON_SET_SILENT_MODE_AND_MIX_WITH_OTHERS,
        ON_SET_CLIENT_VOLUME,
        ON_REGISTER_THREAD_PRIORITY,
        IPC_STREAM_MAX_MSG
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IpcStream");
};

class IpcStreamListener : public IRemoteBroker, public IStreamListener {
public:
    virtual ~IpcStreamListener() = default;

    // IPC code.
    enum IpcStreamListenerMsg : uint32_t {
        ON_OPERATION_HANDLED = 0,
        IPC_STREAM_LISTENER_MAX_MSG
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"IpcStreamListener");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_STREAM_H
