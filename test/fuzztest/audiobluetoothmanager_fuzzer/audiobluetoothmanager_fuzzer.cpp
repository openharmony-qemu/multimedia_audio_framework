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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";

void AudioBluetoothManagerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioPolicyServer> AudioPolicyServerPtr =
        std::make_shared<AudioPolicyServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    DeviceType devType = *reinterpret_cast<const DeviceType *>(rawData);
    bool isConnected = *reinterpret_cast<const bool *>(rawData);
    std::string macAddress(reinterpret_cast<const char*>(rawData), size);
    std::string deviceName(reinterpret_cast<const char*>(rawData), size);

    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
    audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);

    AudioPolicyServerPtr->audioPolicyService_
        .OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, audioStreamInfo);

    AudioPolicyServerPtr->audioPolicyService_
        .OnDeviceConfigurationChanged(devType, macAddress, deviceName, audioStreamInfo);
}

void FetchOutputDeviceForTrackInternalFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.Unmarshalling(data);
    std::shared_ptr<AudioPolicyServer> AudioPolicyServerPtr =
        std::make_shared<AudioPolicyServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioPolicyServerPtr->audioPolicyService_.FetchOutputDeviceForTrack(streamChangeInfo,
        AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioBluetoothManagerFuzzTest(data, size);
    OHOS::AudioStandard::FetchOutputDeviceForTrackInternalFuzzTest(data, size);
    return 0;
}
