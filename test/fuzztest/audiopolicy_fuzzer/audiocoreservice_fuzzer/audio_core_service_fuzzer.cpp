/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "audio_select_interface_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
typedef void (*TestFuncs)();
shared_ptr<AudioCoreService> audioCoreService;
class RemoteObjectFuzzTestStub : public IRemoteObject {
public:
    RemoteObjectFuzzTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectFuzzTestStub");
};

class AudioClientTrackerFuzzTest : public AudioClientTracker {
    public:
        virtual ~AudioClientTrackerFuzzTest() = default;
        virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
        virtual void SetLowPowerVolumeImpl(float volume) {};
        virtual void GetLowPowerVolumeImpl(float &volume) {};
        virtual void GetSingleStreamVolumeImpl(float &volume) {};
        virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
        virtual void UnsetOffloadModeImpl() {};
    };

const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};

const vector<AudioStreamDeviceChangeReason> g_testReasons = {
    AudioStreamDeviceChangeReason::UNKNOWN,
    AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
    AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
    AudioStreamDeviceChangeReason::OVERRODE,
};

const vector<StreamSetState> g_testStreamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

void AudioCoreServiceDeInitFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->Init();
    audioCoreService->DeInit();
}

void AudioCoreServiceDumpPipeManagerFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->Init();
    std::string dumpString = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->DumpPipeManager(dumpString);
}

void AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->CheckAndSetCurrentOutputDevice(desc, sessionId);
}

void AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->CheckAndSetCurrentInputDevice(desc, uid);
}

void AudioCoreServiceSetCallDeviceActiveFuzzTest(FuzzedDataProvider& fdp)
{
    InternalDeviceType deviceType = ConsumeEnum<InternalDeviceType>(fdp);
    bool active = fdp.ConsumeBool();
    std::string address = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->Init();
    audioCoreService->SetCallDeviceActive(deviceType, active, address, uid);
}

void AudioCoreServiceGetExcludedDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    audioCoreService->Init();
    audioCoreService->GetExcludedDevices(audioDevUsage);
}

void AudioCoreServiceExcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioCoreService->Init();
    audioCoreService->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceUnexcludeOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceUsage audioDevUsage = ConsumeEnum<AudioDeviceUsage>(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = ConsumeEnum<DeviceType>(fdp);
    audioDevDesc->networkId_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioDeviceDescriptors.push_back(audioDevDesc);
    AudioRecoveryDevice::GetInstance().audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioCoreService->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

void AudioCoreServiceOnReceiveUpdateDeviceNameEventFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string deviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->Init();
    audioCoreService->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

void AudioCoreServiceNotifyRemoteRenderStateFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string condition = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->Init();
    audioCoreService->NotifyRemoteRenderState(networkId, condition, value);
}

void AudioCoreServiceOnCapturerSessionRemovedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegral<uint64_t>();
    audioCoreService->Init();
    audioCoreService->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreServiceTriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum =
        ConsumeEnum<AudioStreamDeviceChangeReasonExt::ExtEnum>(fdp);
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->Init();
    audioCoreService->TriggerFetchDevice(reason);
}

void AudioCoreServiceOnCapturerSessionAddedFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionID = fdp.ConsumeIntegral<uint64_t>();
    SessionInfo sessionInfo;
    sessionInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    sessionInfo.rate = fdp.ConsumeIntegralInRange<uint32_t>(0, 1);
    sessionInfo.channels = fdp.ConsumeIntegralInRange<uint32_t>(0, 1);
    AudioStreamInfo streamInfo;
    audioCoreService->Init();
    audioCoreService->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreServiceSetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    sptr<IRemoteObject> object =new RemoteObjectFuzzTestStub();
    audioCoreService->Init();
    audioCoreService->SetAudioDeviceAnahsCallback(object);
}

void AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    audioCoreService->Init();
    audioCoreService->UnsetAudioDeviceAnahsCallback();
}

void AudioCoreServiceOnUpdateAnahsSupportFuzzTest(FuzzedDataProvider& fdp)
{
    std::string anahsShowType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->Init();
    audioCoreService->OnUpdateAnahsSupport(anahsShowType);
}

void AudioCoreServiceIsNoRunningStreamFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs;
    audioCoreService->IsNoRunningStream(outputStreamDescs);
}

void AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    pid_t uid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    audioCoreService->BluetoothServiceCrashedCallback(pid, uid);
}

void AudioCoreServiceLoadSplitModuleFuzzTest(FuzzedDataProvider& fdp)
{
    CHECK_AND_RETURN(audioCoreService != nullptr);
    std::string splitArgs = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->LoadSplitModule("", networkId);
    audioCoreService->LoadSplitModule(splitArgs, networkId);
}

void AudioCoreServiceSetPreferredInputDeviceIfValidFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->SetPreferredInputDeviceIfValid(streamDesc);
}

void AudioSelectInterfaceServiceSetInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioSelectInterfaceService::GetInstance().SetInputDevice(deviceType, sessionId, uid);
}

void AudioSelectInterfaceServiceSetPreferredInputDeviceIfValidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    streamDesc->appInfo_.appUid = fdp.ConsumeIntegral<int32_t>();
    streamDesc->capturerInfo_.sourceType = ConsumeEnum<SourceType>(fdp);
    streamDesc->preferredInputDevice.deviceType_ = ConsumeEnum<DeviceType>(fdp);
    streamDesc->preferredInputDevice.deviceRole_ = INPUT_DEVICE;
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(streamDesc);
}

void AudioCoreServiceParsePreferredInputDeviceHistoryFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->ParsePreferredInputDeviceHistory(streamDesc);
}

void AudioCoreServiceGetFlagForMmapStreamFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->GetFlagForMmapStream(streamDesc);
}

void AudioCoreServiceGetPaIndexByPortNameFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string portName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->GetPaIndexByPortName(portName);
}

void AudioCoreServiceNotifyDistributedOutputChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->NotifyDistributedOutputChange(desc);
}

void AudioCoreServiceSelectInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioCapturerFilter->uid = fdp.ConsumeIntegral<int32_t>();
    audioCapturerFilter->capturerInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    audioCapturerFilter->capturerInfo.capturerFlags = fdp.ConsumeIntegral<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    audioCoreService->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreServiceGetPreferBluetoothAndNearlinkRecordByUidFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->GetPreferBluetoothAndNearlinkRecordByUid(uid);
}

void AudioCoreServiceCloseWakeUpAudioCapturerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->CloseWakeUpAudioCapturer();
}

void AudioCoreServiceUnregisterBluetoothListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->UnregisterBluetoothListener();
}

void AudioCoreServiceConfigDistributedRoutingRoleFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    CastType type = ConsumeEnum<CastType>(fdp);
    audioCoreService->ConfigDistributedRoutingRole(descriptor, type);
}

void AudioCoreServiceHandleA2dpSuspendWhenLoadFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->HandleA2dpSuspendWhenLoad();
}

void AudioCoreServiceHandleA2dpRestoreFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->HandleA2dpRestore();
}

void AudioCoreServiceCaptureConcurrentCheckFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->CaptureConcurrentCheck(sessionId);
}

void AudioCoreServiceUpdateStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string pipeName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    DeviceStreamInfo streamInfo;
    std::list<DeviceStreamInfo> deviceStreamInfo;
    deviceStreamInfo.push_back(streamInfo);
    std::list<std::string> supportDevices;
    supportDevices.push_back(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    audioCoreService->UpdateStreamPropInfo(adapterName, pipeName, deviceStreamInfo, supportDevices);
}

void AudioCoreServiceGetStreamPropInfoSizeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string adapterName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string pipeName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->GetStreamPropInfoSize(adapterName, pipeName);
}

void AudioCoreServiceIsA2dpOffloadStreamFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint sessionId = fdp.ConsumeIntegral<uint>();
    audioCoreService->IsA2dpOffloadStream(sessionId);
}

void AudioCoreServiceSetRendererTargetFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    RenderTarget target = ConsumeEnum<RenderTarget>(fdp);
    RenderTarget lastTarget = ConsumeEnum<RenderTarget>(fdp);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->SetRendererTarget(target, lastTarget, sessionId);
}

void AudioCoreServiceStartInjectionFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->StartInjection(streamId);
}

void AudioCoreServiceRemoveIdForInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->RemoveIdForInjector(streamId);
}

void AudioCoreServiceReleaseCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    audioCoreService->ReleaseCaptureInjector();
}

void AudioCoreServiceRebuildCaptureInjectorFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->RebuildCaptureInjector(streamId);
}

void AudioCoreServiceA2dpOffloadGetRenderPositionFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    uint32_t delayValue = fdp.ConsumeIntegral<uint32_t>();
    uint64_t sendDataSize = fdp.ConsumeIntegral<uint64_t>();
    uint32_t timeStamp = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

void AudioCoreServiceInVideoCommFastBlockListFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->InVideoCommFastBlockList(bundleName);
}

void AudioCoreServiceSetQueryBundleNameListCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    audioCoreService->SetQueryBundleNameListCallback(remoteObject);
}

void AudioCoreServiceOnCheckActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();
    std::string reason = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->OnCheckActiveMusicTime(reason);
    audioCoreService->IsDistributeServiceOnline();
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->HandleDeviceConfigChanged(selectedAudioDevice);
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    bool enable = fdp.ConsumeBool();
    audioCoreService->NotifyRemoteRouteStateChange(networkId, deviceType, enable);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->FetchAndActivateOutputDevice(deviceDesc, streamDesc);
}

void AudioCoreServiceCheckInterphoneFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->Init();

    StreamUsage streamUsage = ConsumeEnum<StreamUsage>(fdp);
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = ConsumeEnum<DeviceType>(fdp);
    desc->deviceRole_ = ConsumeEnum<DeviceRole>(fdp);
    desc->networkId_ = fdp.ConsumeRandomLengthString(THRESHOLD);
    desc->deviceCategory_ = ConsumeEnum<DeviceCategory>(fdp);

    bool hasPairDevice = fdp.ConsumeBool();
    if (hasPairDevice) {
        desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>();
        desc->pairDeviceDescriptor_->deviceType_ = ConsumeEnum<DeviceType>(fdp);
        desc->pairDeviceDescriptor_->deviceRole_ = ConsumeEnum<DeviceRole>(fdp);
    }

    AudioPolicyUtils::CheckInterphone(streamUsage, desc);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioCoreServiceDeInitFuzzTest,
    AudioCoreServiceDumpPipeManagerFuzzTest,
    AudioCoreServiceCheckAndSetCurrentOutputDeviceFuzzTest,
    AudioCoreServiceCheckAndSetCurrentInputDeviceFuzzTest,
    AudioCoreServiceSetCallDeviceActiveFuzzTest,
    AudioCoreServiceGetExcludedDevicesFuzzTest,
    AudioCoreServiceExcludeOutputDevicesFuzzTest,
    AudioCoreServiceUnexcludeOutputDevicesFuzzTest,
    AudioCoreServiceOnReceiveUpdateDeviceNameEventFuzzTest,
    AudioCoreServiceNotifyRemoteRenderStateFuzzTest,
    AudioCoreServiceOnCapturerSessionAddedFuzzTest,
    AudioCoreServiceOnCapturerSessionRemovedFuzzTest,
    AudioCoreServiceTriggerFetchDeviceFuzzTest,
    AudioCoreServiceSetAudioDeviceAnahsCallbackFuzzTest,
    AudioCoreServiceUnsetAudioDeviceAnahsCallbackFuzzTest,
    AudioCoreServiceOnUpdateAnahsSupportFuzzTest,
    AudioCoreServiceIsNoRunningStreamFuzzTest,
    AudioCoreServiceBluetoothServiceCrashedCallbackFuzzTest,
    AudioCoreServiceSetPreferredInputDeviceIfValidFuzzTest,
    AudioSelectInterfaceServiceSetInputDeviceFuzzTest,
    AudioSelectInterfaceServiceSetPreferredInputDeviceIfValidFuzzTest,
    AudioCoreServiceParsePreferredInputDeviceHistoryFuzzTest,
    AudioCoreServiceGetFlagForMmapStreamFuzzTest,
    AudioCoreServiceGetPaIndexByPortNameFuzzTest,
    AudioCoreServiceNotifyDistributedOutputChangeFuzzTest,
    AudioCoreServiceSelectInputDeviceFuzzTest,
    AudioCoreServiceGetPreferBluetoothAndNearlinkRecordByUidFuzzTest,
    AudioCoreServiceCloseWakeUpAudioCapturerFuzzTest,
    AudioCoreServiceUnregisterBluetoothListenerFuzzTest,
    AudioCoreServiceConfigDistributedRoutingRoleFuzzTest,
    AudioCoreServiceHandleA2dpSuspendWhenLoadFuzzTest,
    AudioCoreServiceHandleA2dpRestoreFuzzTest,
    AudioCoreServiceCaptureConcurrentCheckFuzzTest,
    AudioCoreServiceUpdateStreamPropInfoFuzzTest,
    AudioCoreServiceGetStreamPropInfoSizeFuzzTest,
    AudioCoreServiceIsA2dpOffloadStreamFuzzTest,
    AudioCoreServiceSetRendererTargetFuzzTest,
    AudioCoreServiceStartInjectionFuzzTest,
    AudioCoreServiceRemoveIdForInjectorFuzzTest,
    AudioCoreServiceReleaseCaptureInjectorFuzzTest,
    AudioCoreServiceRebuildCaptureInjectorFuzzTest,
    AudioCoreServiceA2dpOffloadGetRenderPositionFuzzTest,
    AudioCoreServiceInVideoCommFastBlockListFuzzTest,
    AudioCoreServiceSetQueryBundleNameListCallbackFuzzTest,
    AudioCoreServiceOnCheckActiveMusicTimeFuzzTest, AudioCoreServiceCheckInterphoneFuzzTest,
    });
    func(fdp);
}
void Init()
{
    audioCoreService = std::make_shared<AudioCoreService>();
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
