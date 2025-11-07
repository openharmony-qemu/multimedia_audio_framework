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
#ifndef HPAE_MOCKS_H
#define HPAE_MOCKS_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "hpae_msg_channel.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "i_renderer_stream.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class MockSendMsgCallback : public ISendMsgCallback {
public:
    MockSendMsgCallback() = default;
    virtual ~MockSendMsgCallback() = default;
    MOCK_METHOD(void, Invoke, (HpaeMsgCode cmdID, const std::any& args), (override));
    MOCK_METHOD(void, InvokeSync, (HpaeMsgCode cmdID, const std::any& args), (override));
};

class MockAudioRenderSink : public IAudioRenderSink {
public:
    MockAudioRenderSink() = default;
    virtual ~MockAudioRenderSink() = default;
    MOCK_METHOD(int32_t, Init, (const IAudioSinkAttr &attr), (override));
    MOCK_METHOD(void, DeInit, (), (override));
    MOCK_METHOD(bool, IsInited, (), (override));

    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Pause, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, Reset, (), (override));
    MOCK_METHOD(int32_t, RenderFrame, (char &data, uint64_t len, uint64_t &writeLen), (override));
    MOCK_METHOD(int64_t, GetVolumeDataCount, (), (override));

    MOCK_METHOD(int32_t, SuspendRenderSink, (), (override));
    MOCK_METHOD(int32_t, RestoreRenderSink, (), (override));

    MOCK_METHOD(void, SetAudioParameter,
        (const AudioParamKey key, const std::string &condition, const std::string &value), (override));
    MOCK_METHOD(std::string, GetAudioParameter,
        (const AudioParamKey key, const std::string &condition), (override));

    MOCK_METHOD(int32_t, SetVolume, (float left, float right), (override));
    MOCK_METHOD(int32_t, GetVolume, (float &left, float &right), (override));

    MOCK_METHOD(int32_t, GetLatency, (uint32_t &latency), (override));
    MOCK_METHOD(int32_t, GetTransactionId, (uint64_t &transactionId), (override));
    MOCK_METHOD(int32_t, GetPresentationPosition,
        (uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec), (override));
    MOCK_METHOD(float, GetMaxAmplitude, (), (override));
    MOCK_METHOD(void, SetAudioMonoState, (bool audioMono), (override));
    MOCK_METHOD(void, SetAudioBalanceValue, (float audioBalance), (override));

    MOCK_METHOD(int32_t, SetSinkMuteForSwitchDevice, (bool mute), (override));
    MOCK_METHOD(int32_t, SetDeviceConnectedFlag, (bool flag), (override));
    MOCK_METHOD(void, SetSpeed, (float speed), (override));

    MOCK_METHOD(int32_t, SetAudioScene, (AudioScene audioScene, bool scoExcludeFlag), (override));
    MOCK_METHOD(int32_t, GetAudioScene, (), (override));

    MOCK_METHOD(int32_t, UpdateActiveDevice, (std::vector<DeviceType> &outputDevices), (override));

    MOCK_METHOD(void, RegistCallback, (uint32_t type, IAudioSinkCallback *callback), (override));
    MOCK_METHOD(void, RegistCallback, (uint32_t type, std::shared_ptr<IAudioSinkCallback> callback), (override));
    MOCK_METHOD(void, ResetActiveDeviceForDisconnect, (DeviceType device), (override));

    MOCK_METHOD(int32_t, SetPaPower, (int32_t flag), (override));
    MOCK_METHOD(int32_t, SetPriPaPower, (), (override));

    MOCK_METHOD(int32_t, UpdateAppsUid, (const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size), (override));
    MOCK_METHOD(int32_t, UpdateAppsUid, (const std::vector<int32_t> &appsUid), (override));

    MOCK_METHOD(int32_t, SetRenderEmpty, (int32_t durationUs), (override));
    MOCK_METHOD(void, SetAddress, (const std::string &address), (override));
    MOCK_METHOD(void, SetInvalidState, (), (override));
    MOCK_METHOD(bool, IsSinkInited, (), (override));  // 注意与已存在的 IsInited 区分

    MOCK_METHOD(int32_t, GetMmapBufferInfo,
        (int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
         uint32_t &byteSizePerFrame, uint32_t &syncInfoSize), (override));
    MOCK_METHOD(int32_t, GetMmapHandlePosition,
        (uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec), (override));

    MOCK_METHOD(int32_t, Drain, (AudioDrainType type), (override));
    MOCK_METHOD(void, RegistOffloadHdiCallback,
        (std::function<void(const RenderCallbackType type)> callback), (override));
    MOCK_METHOD(int32_t, RegistDirectHdiCallback,
        (std::function<void(const RenderCallbackType type)> callback), (override));
    MOCK_METHOD(int32_t, SetBufferSize, (uint32_t sizeMs), (override));
    MOCK_METHOD(int32_t, SetOffloadRenderCallbackType, (RenderCallbackType type), (override));
    MOCK_METHOD(int32_t, LockOffloadRunningLock, (), (override));
    MOCK_METHOD(int32_t, UnLockOffloadRunningLock, (), (override));

    MOCK_METHOD(int32_t, SplitRenderFrame,
        (char &data, uint64_t len, uint64_t &writeLen, SplitStreamType splitStreamType), (override));
    MOCK_METHOD(int32_t, UpdatePrimaryConnectionState, (uint32_t operation), (override));
    MOCK_METHOD(void, SetDmDeviceType, (uint16_t dmDeviceType, DeviceType deviceType), (override));

    MOCK_METHOD(void, DumpInfo, (std::string &dumpString), (override));
};

// Mock INodeCallback
class MockNodeCallback : public INodeCallback {
public:
    MockNodeCallback() = default;
    virtual ~MockNodeCallback() = default;
    MOCK_METHOD(void, OnNodeStatusUpdate, (uint32_t, IOperation), (override));
    MOCK_METHOD(void, OnFadeDone, (uint32_t), (override));
    MOCK_METHOD(void, OnRequestLatency, (uint32_t, uint64_t &), (override));
    MOCK_METHOD(void, OnRewindAndFlush, (uint64_t, uint64_t), (override));
    MOCK_METHOD(void, OnNotifyQueue, (), (override));
    MOCK_METHOD(void, OnDisConnectProcessCluster, (HpaeProcessorType), (override));
    MOCK_METHOD(void, OnNotifyDfxNodeInfo, (bool, uint32_t, uint32_t), (override));
    MOCK_METHOD(void, OnNotifyDfxNodeInfoChanged, (uint32_t, const HpaeDfxNodeInfo &), (override));
};

// Mock IStreamCallback
class MockStreamCallback : public IStreamCallback {
public:
    MockStreamCallback() = default;
    virtual ~MockStreamCallback() = default;
    MOCK_METHOD(int32_t, OnStreamData, (AudioCallBackStreamInfo&), (override));
    MOCK_METHOD(bool, OnQueryUnderrun, (), (override));
};

// Mock IAudioCaptureSource
class MockAudioCaptureSource : public IAudioCaptureSource {
public:
    MockAudioCaptureSource() = default;
    virtual ~MockAudioCaptureSource() = default;
    MOCK_METHOD(int32_t, Init, (const IAudioSourceAttr &attr), (override));
    MOCK_METHOD(void, DeInit, (), (override));
    MOCK_METHOD(bool, IsInited, (), (override));
    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Stop, (), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));
    MOCK_METHOD(int32_t, Pause, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, Reset, (), (override));
    MOCK_METHOD(int32_t, CaptureFrame, (char *frame, uint64_t requestBytes, uint64_t &replyBytes), (override));
    MOCK_METHOD(int32_t, CaptureFrameWithEc, (FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
        uint64_t &replyBytesEc), (override));
    MOCK_METHOD(std::string, GetAudioParameter, (const AudioParamKey key, const std::string &condition), (override));
    MOCK_METHOD(void, SetAudioParameter, (const AudioParamKey key, const std::string &condition,
        const std::string &value), (override));
    MOCK_METHOD(int32_t, SetVolume, (float left, float right), (override));
    MOCK_METHOD(int32_t, GetVolume, (float &left, float &right), (override));
    MOCK_METHOD(int32_t, SetMute, (bool isMute), (override));
    MOCK_METHOD(int32_t, GetMute, (bool &isMute), (override));
    MOCK_METHOD(uint64_t, GetTransactionId, (), (override));
    MOCK_METHOD(int32_t, GetPresentationPosition, (uint64_t &frames, int64_t &timeSec,
        int64_t &timeNanoSec), (override));
    MOCK_METHOD(float, GetMaxAmplitude, (), (override));
    MOCK_METHOD(int32_t, SetAudioScene, (AudioScene audioScene, bool scoExcludeFlag), (override));
    MOCK_METHOD(int32_t, UpdateActiveDevice, (DeviceType inputDevice), (override));
    MOCK_METHOD(int32_t, UpdateAppsUid, (const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size), (override));
    MOCK_METHOD(int32_t, UpdateAppsUid, (const std::vector<int32_t> &appsUid), (override));
    MOCK_METHOD(int32_t, GetArmUsbDeviceStatus, (), (override));
};
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_MOCKS_H