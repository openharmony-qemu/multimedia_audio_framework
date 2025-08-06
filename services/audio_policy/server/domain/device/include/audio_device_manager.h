/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_DEVICE_MANAGER_H
#define ST_AUDIO_DEVICE_MANAGER_H

#include <list>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "audio_device_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr int32_t NEED_TO_FETCH = 1;

typedef function<bool(const std::shared_ptr<AudioDeviceDescriptor> &desc)> IsPresentFunc;
class AudioDeviceManager {
public:
    static AudioDeviceManager& GetAudioDeviceManager()
    {
        static AudioDeviceManager audioDeviceManager;
        return audioDeviceManager;
    }

    void AddNewDevice(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);
    void RemoveNewDevice(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);
    void OnXmlParsingCompleted(const unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> &xmlData);
    int32_t GetDeviceUsageFromType(const DeviceType devType) const;
    void ParseDeviceXml();
    AudioStreamDeviceChangeReasonExt UpdateDevicesListInfo(
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, const DeviceInfoUpdateCommand updateCommand);

    vector<shared_ptr<AudioDeviceDescriptor>> GetRemoteRenderDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetRemoteCaptureDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCommRenderPrivacyDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCommRenderPublicDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCommRenderBTCarDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCommCapturePrivacyDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCommCapturePublicDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetMediaRenderPrivacyDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetMediaRenderPublicDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetMediaCapturePrivacyDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetMediaCapturePublicDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCapturePrivacyDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetCapturePublicDevices();
    vector<shared_ptr<AudioDeviceDescriptor>> GetRecongnitionCapturePrivacyDevices();
    shared_ptr<AudioDeviceDescriptor> GetCommRenderDefaultDevice(StreamUsage streamUsage);
    shared_ptr<AudioDeviceDescriptor> GetRenderDefaultDevice();
    shared_ptr<AudioDeviceDescriptor> GetCaptureDefaultDevice();
    shared_ptr<AudioDeviceDescriptor> FindConnectedDeviceById(const int32_t deviceId);
    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> GetDevicePrivacyMaps();
    vector<shared_ptr<AudioDeviceDescriptor>> GetAvailableDevicesByUsage(AudioDeviceUsage usage);
    void GetAvailableDevicesWithUsage(const AudioDeviceUsage usage,
        const list<DevicePrivacyInfo> &deviceInfos, const std::shared_ptr<AudioDeviceDescriptor> &dev,
        std::vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    vector<shared_ptr<AudioDeviceDescriptor>> GetAvailableBluetoothDevice(DeviceType devType,
        const std::string &macAddress);
    bool GetScoState();
    void UpdateEarpieceStatus(const bool hasEarPiece);
    vector<shared_ptr<AudioDeviceDescriptor>> GetDevicesByFilter(DeviceType devType, DeviceRole devRole,
        const string &macAddress, const string &networkId, ConnectState connectState);
    DeviceUsage GetDeviceUsage(const AudioDeviceDescriptor &desc);
    std::string GetConnDevicesStr();
    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);
    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
    bool IsDeviceConnected(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptors);
    bool IsConnectedDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);
    bool IsVirtualConnectedDevice(const std::shared_ptr<AudioDeviceDescriptor> &selectedDesc);
    int32_t UpdateDeviceDescDeviceId(std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const StreamUsage streamUsage, bool isRunning);
    int32_t UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID);
    int32_t UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID);
    int32_t RemoveSelectedDefaultOutputDevice(const uint32_t sessionID);
    shared_ptr<AudioDeviceDescriptor> GetSelectedMediaRenderDevice();
    shared_ptr<AudioDeviceDescriptor> GetSelectedCallRenderDevice();
    int32_t SetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType, VolumeBehavior volumeBehavior);
    VolumeBehavior GetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType);
    int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const SourceType sourceType, bool isRunning);
    int32_t RemoveSelectedInputDevice(const uint32_t sessionID);
    shared_ptr<AudioDeviceDescriptor> GetSelectedCaptureDevice(const uint32_t sessionID);
    void Dump(std::string &dumpString);
    void UpdateVirtualDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc, bool isConnected);
    void GetAllConnectedDeviceByType(std::string networkId, DeviceType deviceType,
        std::string macAddress, DeviceRole deviceRole, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);
    bool IsSessionSetDefaultDevice(uint32_t sessionId);
    bool ExistsByType(DeviceType devType) const;
    bool ExistsByTypeAndAddress(DeviceType devType, const string &address) const;
    bool ExistSameRemoteDeviceByMacAddress(std::shared_ptr<AudioDeviceDescriptor> desc);
    shared_ptr<AudioDeviceDescriptor> GetActiveScoDevice(std::string scoMac, DeviceRole role);

private:
    AudioDeviceManager();
    ~AudioDeviceManager() {};
    bool DeviceAttrMatch(const shared_ptr<AudioDeviceDescriptor> &devDesc, AudioDevicePrivacyType &privacyType,
        DeviceRole &devRole, DeviceUsage &devUsage);

    void FillArrayWhenDeviceAttrMatch(const shared_ptr<AudioDeviceDescriptor> &devDesc,
        AudioDevicePrivacyType privacyType, DeviceRole devRole, DeviceUsage devUsage, string logName,
        vector<shared_ptr<AudioDeviceDescriptor>> &descArray);

    void RemoveMatchDeviceInArray(const AudioDeviceDescriptor &devDesc, string logName,
        vector<shared_ptr<AudioDeviceDescriptor>> &descArray);

    void MakePairedDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void MakePairedDefaultDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc, DeviceRole devRole);
    void MakePairedDefaultDeviceImpl(const shared_ptr<AudioDeviceDescriptor> &devDesc,
        const shared_ptr<AudioDeviceDescriptor> &connectedDesc);
    void UpdateConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc, bool isConnected);
    void AddConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void RemoveConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddRemoteRenderDev(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddRemoteCaptureDev(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddDefaultDevices(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);

    bool IsVirtualDevicesExist(const shared_ptr<AudioDeviceDescriptor> &devDesc);

    void UpdateDeviceInfo(shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    void AddCommunicationDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddMediaDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddCaptureDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void HandleScoWithDefaultCategory(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    bool IsExistedDevice(const std::shared_ptr<AudioDeviceDescriptor> &device,
        const vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void AddAvailableDevicesByUsage(const AudioDeviceUsage usage,
        const DevicePrivacyInfo &deviceInfo, const std::shared_ptr<AudioDeviceDescriptor> &dev,
        std::vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void GetDefaultAvailableDevicesByUsage(AudioDeviceUsage usage,
        vector<shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void GetRemoteAvailableDevicesByUsage(AudioDeviceUsage usage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void ReorderAudioDevices(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors,
        const std::string &remoteInfoNetworkId, DeviceType remoteInfoDeviceType);
    bool UpdateExistDeviceDescriptor(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

    void AddBtToOtherList(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void RemoveBtFromOtherList(const AudioDeviceDescriptor &devDesc);
    void RemoveRemoteDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveCommunicationDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveMediaDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveCaptureDevices(const AudioDeviceDescriptor &devDesc);
    bool UpdateConnectState(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    bool UpdateDeviceCategory(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    bool UpdateEnableState(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    bool UpdateExceptionFlag(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    AudioStreamDeviceChangeReasonExt UpdateDeviceUsage(const shared_ptr<AudioDeviceDescriptor> &deviceDesc);

    void RemoveVirtualConnectedDevice(const shared_ptr<AudioDeviceDescriptor> &devDesc);

    list<DevicePrivacyInfo> privacyDeviceList_;
    list<DevicePrivacyInfo> publicDeviceList_;

    vector<shared_ptr<AudioDeviceDescriptor>> remoteRenderDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> remoteCaptureDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> commRenderPrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> commRenderPublicDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> commCapturePrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> commCapturePublicDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> mediaRenderPrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> mediaRenderPublicDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> mediaCapturePrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> mediaCapturePublicDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> capturePrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> capturePublicDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> connectedDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> reconCapturePrivacyDevices_;
    vector<shared_ptr<AudioDeviceDescriptor>> virtualDevices_;
    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};
    std::shared_ptr<AudioDeviceDescriptor> earpiece_ = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> speaker_ = nullptr;
    std::shared_ptr<AudioDeviceDescriptor> defalutMic_ = nullptr;
    bool hasEarpiece_ = false;
    unordered_map<uint32_t, std::pair<DeviceType, StreamUsage>> selectedDefaultOutputDeviceInfo_;
    vector<std::pair<uint32_t, DeviceType>> mediaDefaultOutputDevices_;
    vector<std::pair<uint32_t, DeviceType>> callDefaultOutputDevices_;
    DeviceType selectedMediaDefaultOutputDevice_ = DEVICE_TYPE_DEFAULT;
    DeviceType selectedCallDefaultOutputDevice_ = DEVICE_TYPE_DEFAULT;
    std::mutex selectDefaultOutputDeviceMutex_;
    std::mutex currentActiveDevicesMutex_;
    unordered_map<uint32_t, std::pair<DeviceType, SourceType>> selectedInputDeviceInfo_;
    std::mutex selectInputDeviceMutex_;
    std::string remoteInfoNetworkId_ = "";
    DeviceType remoteInfoDeviceType_ = DEVICE_TYPE_DEFAULT;
    std::mutex virtualDevicesMutex_;
    std::mutex descArrayMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H
