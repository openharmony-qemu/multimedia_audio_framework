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
#ifndef ST_AUDIO_DEVICE_MANAGER_H
#define ST_AUDIO_DEVICE_MANAGER_H

#include <list>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr int32_t NEED_TO_FETCH = 1;

typedef function<bool(const std::unique_ptr<AudioDeviceDescriptor> &desc)> IsPresentFunc;
std::string GetEncryptAddr(const std::string &addr);
class AudioDeviceManager {
public:
    static AudioDeviceManager& GetAudioDeviceManager()
    {
        static AudioDeviceManager audioDeviceManager;
        return audioDeviceManager;
    }

    void AddNewDevice(const sptr<AudioDeviceDescriptor> &devDesc);
    void RemoveNewDevice(const sptr<AudioDeviceDescriptor> &devDesc);
    void OnXmlParsingCompleted(const unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> &xmlData);
    int32_t GetDeviceUsageFromType(const DeviceType devType) const;
    void ParseDeviceXml();
    void UpdateDevicesListInfo(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const DeviceInfoUpdateCommand updateCommand);

    vector<unique_ptr<AudioDeviceDescriptor>> GetRemoteRenderDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetRemoteCaptureDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCommRenderPrivacyDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCommRenderPublicDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCommRenderBTCarDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCommCapturePrivacyDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCommCapturePublicDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetMediaRenderPrivacyDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetMediaRenderPublicDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetMediaCapturePrivacyDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetMediaCapturePublicDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCapturePrivacyDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetCapturePublicDevices();
    vector<unique_ptr<AudioDeviceDescriptor>> GetRecongnitionCapturePrivacyDevices();
    unique_ptr<AudioDeviceDescriptor> GetCommRenderDefaultDevice(StreamUsage streamUsage);
    unique_ptr<AudioDeviceDescriptor> GetRenderDefaultDevice();
    unique_ptr<AudioDeviceDescriptor> GetCaptureDefaultDevice();
    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> GetDevicePrivacyMaps();
    vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableDevicesByUsage(AudioDeviceUsage usage);
    void GetAvailableDevicesWithUsage(const AudioDeviceUsage usage,
        const list<DevicePrivacyInfo> &deviceInfos, const sptr<AudioDeviceDescriptor> &dev,
        std::vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableBluetoothDevice(DeviceType devType,
        const std::string &macAddress);
    bool GetScoState();
    vector<shared_ptr<AudioDeviceDescriptor>> GetDevicesByFilter(DeviceType devType, DeviceRole devRole,
        const string &macAddress, const string &networkId, ConnectState connectState);
    void UpdateEarpieceStatus(const bool hasEarPiece);
    DeviceUsage GetDeviceUsage(const AudioDeviceDescriptor &desc);
    std::string GetConnDevicesStr();
    std::string GetConnDevicesStr(const vector<shared_ptr<AudioDeviceDescriptor>> &descs);
    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);
    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
    bool IsDeviceConnected(sptr<AudioDeviceDescriptor> &audioDeviceDescriptors);
    bool IsVirtualConnectedDevice(const sptr<AudioDeviceDescriptor> &selectedDesc);
    int32_t UpdateDeviceDescDeviceId(sptr<AudioDeviceDescriptor> &deviceDescriptor);
    int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const StreamUsage streamUsage, bool isRunning);
    int32_t UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID);
    int32_t UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID);
    int32_t RemoveSelectedDefaultOutputDevice(const uint32_t sessionID);
    unique_ptr<AudioDeviceDescriptor> GetSelectedMediaRenderDevice();
    unique_ptr<AudioDeviceDescriptor> GetSelectedCallRenderDevice();
    void SaveRemoteInfo(const std::string &networkId, DeviceType deviceType);

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
    void MakePairedDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc, DeviceRole devRole);
    void MakePairedDefaultDeviceDescriptor(const shared_ptr<AudioDeviceDescriptor> &devDesc, DeviceRole devRole);
    void MakePairedDefaultDeviceImpl(const shared_ptr<AudioDeviceDescriptor> &devDesc,
        const shared_ptr<AudioDeviceDescriptor> &connectedDesc);
    void UpdateConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc, bool isConnected);
    void AddConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void RemoveConnectedDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddRemoteRenderDev(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddRemoteCaptureDev(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddDefaultDevices(const sptr<AudioDeviceDescriptor> &devDesc);

    void UpdateDeviceInfo(shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    void AddCommunicationDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddMediaDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void AddCaptureDevices(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void HandleScoWithDefaultCategory(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    bool IsExistedDevice(const sptr<AudioDeviceDescriptor> &device,
        const vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void AddAvailableDevicesByUsage(const AudioDeviceUsage usage,
        const DevicePrivacyInfo &deviceInfo, const sptr<AudioDeviceDescriptor> &dev,
        std::vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void GetDefaultAvailableDevicesByUsage(AudioDeviceUsage usage,
        vector<unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    bool UpdateExistDeviceDescriptor(const sptr<AudioDeviceDescriptor> &deviceDescriptor);
    void GetRemoteAvailableDevicesByUsage(AudioDeviceUsage usage,
        std::vector<std::unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    void ReorderAudioDevices(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors,
        const std::string &remoteInfoNetworkId, DeviceType remoteInfoDeviceType);

    void AddBtToOtherList(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    void RemoveBtFromOtherList(const AudioDeviceDescriptor &devDesc);
    void RemoveRemoteDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveCommunicationDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveMediaDevices(const AudioDeviceDescriptor &devDesc);
    void RemoveCaptureDevices(const AudioDeviceDescriptor &devDesc);
    bool UpdateConnectState(const shared_ptr<AudioDeviceDescriptor> &devDesc);
    bool UpdateDeviceCategory(const sptr<AudioDeviceDescriptor> &deviceDescriptor);
    bool UpdateEnableState(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    bool UpdateExceptionFlag(const shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);

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
    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};
    sptr<AudioDeviceDescriptor> earpiece_ = nullptr;
    sptr<AudioDeviceDescriptor> speaker_ = nullptr;
    sptr<AudioDeviceDescriptor> defalutMic_ = nullptr;
    bool hasEarpiece_ = false;
    unordered_map<uint32_t, std::pair<DeviceType, StreamUsage>> selectedDefaultOutputDeviceInfo_;
    vector<std::pair<uint32_t, DeviceType>> mediaDefaultOutputDevices_;
    vector<std::pair<uint32_t, DeviceType>> callDefaultOutputDevices_;
    DeviceType selectedMediaDefaultOutputDevice_ = DEVICE_TYPE_DEFAULT;
    DeviceType selectedCallDefaultOutputDevice_ = DEVICE_TYPE_DEFAULT;
    std::mutex selectDefaultOutputDeviceMutex_;
    std::mutex currentActiveDevicesMutex_;
    std::string remoteInfoNetworkId_ = "";
    DeviceType remoteInfoDeviceType_ = DEVICE_TYPE_DEFAULT;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H
