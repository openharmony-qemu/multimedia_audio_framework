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

#ifndef AUDIO_DEVICES_CLIENT_MANAGER_H
#define AUDIO_DEVICES_CLIENT_MANAGER_H

#include <cstdlib>
#include <vector>
#include "audio_device_descriptor.h"
#include "audio_system_manager_ext.h"
#include "audio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class AudioDevicesClientManager {
public:
    static AudioDevicesClientManager &GetInstance();

     /**
     * @brief Select output device.
     *
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectOutputDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Select input device.
     *
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectInputDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief get selected device info.
     *
     * @param uid identifier.
     * @param pid identifier.
     * @param streamType audio stream type.
     * @return Returns device info.
     * @since 9
     */
    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const;

    /**
     * @brief Select the audio input device according to the filter conditions.
     *
     * @param audioRendererFilter filter conditions.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Exclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Unexclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @param audioDeviceDescriptors Output device object.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;

    /**
     * @brief Unexclude the audio output device according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 16
     */
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const;

    /**
     * @brief Get the list of excluded audio output devices according to the DeviceUsage.
     *
     * @param audioDevUsage AudioDeviceUsage.
     * @return Returns the device list is obtained.
     * @since 16
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage) const;

    /**
     * @brief Get the list of audio devices.
     *
     * @param deviceFlag Flag of device type.
     * @param GetAudioParameter Key of audio parameters to be obtained.
     * @return Returns the device list is obtained.
     * @since 9
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    /**
     * @brief Get the list of audio devices (inner).
     *
     * @param deviceFlag Flag of device type.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    /**
     * @brief Get the audio output device according to the filter conditions.
     *
     * @param AudioRendererFilter filter conditions.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(sptr<AudioRendererFilter> audioRendererFilter);

    /**
     * @brief Get the audio input device according to the filter conditions.
     *
     * @param AudioCapturerFilter filter conditions.
     * @return Returns the device list is obtained.
     * @since 12
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter);

     /**
     * @brief Set device active.
     *
     * @param deviceType device type.
     * @param flag Device activation status.
     * @param clientPid pid of caller.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 9
     */
    int32_t SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientUid = -1) const;

    /**
     * @brief get device active.
     *
     * @param deviceType device type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 9
     */
    bool IsDeviceActive(DeviceType deviceType) const;

    /**
     * @brief get active output device.
     *
     * @return Returns device type.
     * @since 9
     */
    DeviceType GetActiveOutputDevice();

    /**
     * @brief get active input device.
     *
     * @return Returns device type.
     * @since 9
     */
    DeviceType GetActiveInputDevice();

    /**
     * @brief Registers the deviceChange callback listener.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetDeviceChangeCallback(const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>
        &callback);

    /**
     * @brief Unregisters the deviceChange callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetDeviceChangeCallback(DeviceFlag flag = DeviceFlag::ALL_DEVICES_FLAG,
        std::shared_ptr<AudioManagerDeviceChangeCallback> callback = nullptr);

    /**
     * @brief Set device address.
     *
     * @param deviceType device type.
     * @param flag Device activation status.
     * @param address Device address
     * @param clientPid pid of caller.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 11
     */
    int32_t SetCallDeviceActive(DeviceType deviceType, bool flag, std::string address,
        const int32_t clientUid = -1) const;

    /**
     * @brief Get active output deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 8
     */
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetActiveOutputDeviceDescriptors();

    /**
     * @brief Get preferred input device deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 10
     */
    int32_t GetPreferredInputDeviceDescriptors();
private:
    AudioDevicesClientManager() = default;
    ~AudioDevicesClientManager() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICES_CLIENT_MANAGER_H
