/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef AUDIO_MANAGER_FUZZER_H
#define AUDIO_MANAGER_FUZZER_H

#define FUZZ_PROJECT_NAME "audiomanager_fuzzer"

namespace OHOS {
namespace AudioStandard {
class AudioRendererStateCallbackFuzz : public AudioRendererStateChangeCallback {
public:
    explicit AudioRendererStateCallbackFuzz() = default;
    virtual ~AudioRendererStateCallbackFuzz() = default;
    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
};

class AudioCapturerStateCallbackFuzz : public AudioCapturerStateChangeCallback {
public:
    explicit AudioCapturerStateCallbackFuzz() = default;
    virtual ~AudioCapturerStateCallbackFuzz() = default;
    void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
};

class AudioPreferredOutputDeviceChangeCallbackFuzz : public AudioPreferredOutputDeviceChangeCallback {
public:
    explicit AudioPreferredOutputDeviceChangeCallbackFuzz() = default;
    virtual ~AudioPreferredOutputDeviceChangeCallbackFuzz() = default;
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) {};
};

class AudioPreferredInputDeviceChangeCallbackFuzz : public AudioPreferredInputDeviceChangeCallback {
public:
    explicit AudioPreferredInputDeviceChangeCallbackFuzz() = default;
    virtual ~AudioPreferredInputDeviceChangeCallbackFuzz() = default;
    virtual void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) {};
};

class AudioFocusInfoChangeCallbackFuzz : public AudioFocusInfoChangeCallback {
public:
    explicit AudioFocusInfoChangeCallbackFuzz() = default;
    virtual ~AudioFocusInfoChangeCallbackFuzz() = default;
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) {};
};

class AudioRingerModeCallbackFuzz : public AudioRingerModeCallback {
public:
    explicit AudioRingerModeCallbackFuzz() = default;
    virtual ~AudioRingerModeCallbackFuzz() = default;
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) {};
};
} // namespace AudioStandard
} // namesapce OHOS

#endif // AUDIO_MANAGER_FUZZER_H
