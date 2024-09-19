/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ADAPTER_MANAGER_H
#define ST_AUDIO_ADAPTER_MANAGER_H

#include <list>
#include <unordered_map>
#include <cinttypes>

#include "audio_service_adapter.h"
#include "distributed_kv_data_manager.h"
#include "iaudio_policy_interface.h"
#include "types.h"
#include "audio_policy_log.h"
#include "audio_policy_server_handler.h"
#include "audio_volume_config.h"
#include "volume_data_maintainer.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::DistributedKv;

class AudioOsAccountInfo;

class AudioAdapterManager : public IAudioPolicyInterface {
public:
    static constexpr std::string_view HDI_SINK = "libmodule-hdi-sink.z.so";
    static constexpr std::string_view HDI_SOURCE = "libmodule-hdi-source.z.so";
    static constexpr std::string_view PIPE_SINK = "libmodule-pipe-sink.z.so";
    static constexpr std::string_view PIPE_SOURCE = "libmodule-pipe-source.z.so";
    static constexpr std::string_view CLUSTER_SINK = "libmodule-cluster-sink.z.so";
    static constexpr std::string_view EFFECT_SINK = "libmodule-effect-sink.z.so";
    static constexpr std::string_view INNER_CAPTURER_SINK = "libmodule-inner-capturer-sink.z.so";
    static constexpr std::string_view RECEIVER_SINK = "libmodule-receiver-sink.z.so";
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_COUNT = 5;
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_DELAY_TIME = 200000;
    static constexpr float MIN_VOLUME = 0.0f;
    static constexpr uint32_t NUMBER_TWO = 2;
    bool Init();
    void Deinit(void);
    void InitKVStore();
    bool ConnectServiceAdapter();

    static IAudioPolicyInterface& GetInstance()
    {
        static AudioAdapterManager audioAdapterManager;
        return audioAdapterManager;
    }

    virtual ~AudioAdapterManager() {}

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType);

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType);

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel);

    int32_t GetSystemVolumeLevel(AudioStreamType streamType);

    int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType);

    float GetSystemVolumeDb(AudioStreamType streamType);

    int32_t SetStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage = STREAM_USAGE_UNKNOWN);

    int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute);

    bool GetStreamMute(AudioStreamType streamType);

    std::vector<SinkInfo> GetAllSinks();

    std::vector<SinkInput> GetAllSinkInputs();

    std::vector<SourceOutput> GetAllSourceOutputs();

    AudioIOHandle OpenAudioPort(const AudioModuleInfo &audioModuleInfo);

    int32_t CloseAudioPort(AudioIOHandle ioHandle);

    int32_t SelectDevice(DeviceRole deviceRole, InternalDeviceType deviceType, std::string name);

    int32_t SetDeviceActive(InternalDeviceType deviceType, std::string name, bool active,
        DeviceFlag flag = ALL_DEVICES_FLAG);

    void SetVolumeForSwitchDevice(InternalDeviceType deviceType);

    int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName);

    int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName);

    int32_t SetRingerMode(AudioRingerMode ringerMode);

    AudioRingerMode GetRingerMode(void) const;

    int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback);

    int32_t SuspendAudioDevice(std::string &name, bool isSuspend);

    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false);

    float CalculateVolumeDb(int32_t volumeLevel);

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    float GetMinStreamVolume(void) const;

    float GetMaxStreamVolume(void) const;

    int32_t UpdateSwapDeviceStatus();

    bool IsVolumeUnadjustable();

    float CalculateVolumeDbNonlinear(AudioStreamType streamType, DeviceType deviceType, int32_t volumeLevel);

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    DeviceVolumeType GetDeviceCategory(DeviceType deviceType);

    void SetActiveDevice(DeviceType deviceType);

    DeviceType GetActiveDevice();

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType);

    bool IsUseNonlinearAlgo() { return useNonlinearAlgo_; }

    void SetAbsVolumeScene(bool isAbsVolumeScene);

    bool IsAbsVolumeScene() const;

    void SetAbsVolumeMute(bool mute);

    bool IsAbsVolumeMute() const;

    std::string GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const;

    void ResetRemoteCastDeviceVolume();

    int32_t GetStreamVolume(AudioStreamType streamType);

    void NotifyAccountsChanged(const int &id);

    void SafeVolumeDump(std::string &dumpString);

    int32_t DoRestoreData();
    SafeStatus GetCurrentDeviceSafeStatus(DeviceType deviceType);

    int64_t GetCurentDeviceSafeTime(DeviceType deviceType);

    int32_t SetDeviceSafeStatus(DeviceType deviceType, SafeStatus status);

    int32_t SetDeviceSafeTime(DeviceType deviceType, int64_t time);

    int32_t GetSafeVolumeLevel() const;

    int32_t GetSafeVolumeTimeout() const;

    int32_t GetCurActivateCount(void) const;

    void HandleKvData(bool isFirstBoot);

    int32_t SetPersistMicMuteState(const bool isMute);

    int32_t GetPersistMicMuteState(bool &isMute) const;

    void HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel);

    void HandleStreamMuteStatus(AudioStreamType streamType, bool mute, StreamUsage streamUsage = STREAM_USAGE_UNKNOWN);

    void HandleRingerMode(AudioRingerMode ringerMode);
private:
    friend class PolicyCallbackImpl;

    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t DEFAULT_VOLUME_LEVEL = 7;
    static constexpr int32_t CONST_FACTOR = 100;
    static constexpr int32_t DEFAULT_SAFE_VOLUME_TIMEOUT = 1140;
    static constexpr int32_t CONVERT_FROM_MS_TO_SECONDS = 1000;
    static constexpr float MIN_STREAM_VOLUME = 0.0f;
    static constexpr float MAX_STREAM_VOLUME = 1.0f;

    struct UserData {
        AudioAdapterManager *thiz;
        AudioStreamType streamType;
        float volume;
        bool mute;
        bool isCorked;
        uint32_t idx;
    };

    AudioAdapterManager()
        : ringerMode_(RINGER_MODE_NORMAL),
          audioPolicyKvStore_(nullptr),
          audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
          volumeDataMaintainer_(VolumeDataMaintainer::GetVolumeDataMaintainer())
    {
        InitVolumeMapIndex();
    }

    AudioStreamType GetStreamIDByType(std::string streamType);
    AudioStreamType GetStreamForVolumeMap(AudioStreamType streamType);
    int32_t ReInitKVStore();
    bool InitAudioPolicyKvStore(bool& isFirstBoot);
    void InitVolumeMap(bool isFirstBoot);
    bool LoadVolumeMap(void);
    std::string GetVolumeKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    void InitRingerMode(bool isFirstBoot);
    void InitMuteStatusMap(bool isFirstBoot);
    bool LoadMuteStatusMap(void);
    std::string GetMuteKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    void InitSystemSoundUriMap();
    void InitVolumeMapIndex();
    void UpdateVolumeMapIndex();
    void GetVolumePoints(AudioVolumeType streamType, DeviceVolumeType deviceType,
        std::vector<VolumePoint> &volumePoints);
    uint32_t GetPositionInVolumePoints(std::vector<VolumePoint> &volumePoints, int32_t idx);
    void SaveRingtoneVolumeToLocal(AudioVolumeType volumeType, int32_t volumeLevel);
    int32_t SetVolumeDb(AudioStreamType streamType);
    int32_t SetVolumeDbForVolumeTypeGroup(const std::vector<AudioStreamType> &volumeTypeGroup, float volumeDb);
    bool GetStreamMuteInternal(AudioStreamType streamType);
    int32_t SetRingerModeInternal(AudioRingerMode ringerMode);
    int32_t SetStreamMuteInternal(AudioStreamType streamType, bool mute, StreamUsage streamUsage);
    void InitKVStoreInternal(void);
    void DeleteAudioPolicyKvStore();
    void TransferMuteStatus(void);
    void CloneMuteStatusMap(void);
    void CloneVolumeMap(void);
    void CloneSystemSoundUrl(void);
    void InitSafeStatus(bool isFirstBoot);
    void InitSafeTime(bool isFirstBoot);
    void ConvertSafeTime(void);
    void UpdateSafeVolume();
    void CheckAndDealMuteStatus(const DeviceType &deviceType, const AudioStreamType &streamType);
    void SetVolumeCallbackAfterClone();
    template<typename T>
    std::vector<uint8_t> TransferTypeToByteArray(const T &t)
    {
        return std::vector<uint8_t>(reinterpret_cast<uint8_t *>(const_cast<T *>(&t)),
            reinterpret_cast<uint8_t *>(const_cast<T *>(&t)) + sizeof(T));
    }

    template<typename T>
    T TransferByteArrayToType(const std::vector<uint8_t> &data)
    {
        if (data.size() != sizeof(T) || data.size() == 0) {
            constexpr int tSize = sizeof(T);
            uint8_t tContent[tSize] = { 0 };
            return *reinterpret_cast<T *>(tContent);
        }
        return *reinterpret_cast<T *>(const_cast<uint8_t *>(&data[0]));
    }

    std::unique_ptr<AudioServiceAdapter> audioServiceAdapter_;
    std::unordered_map<AudioStreamType, int> minVolumeIndexMap_;
    std::unordered_map<AudioStreamType, int> maxVolumeIndexMap_;
    std::mutex systemSoundMutex_;
    std::unordered_map<std::string, std::string> systemSoundUriMap_;
    StreamVolumeInfoMap streamVolumeInfos_;
    DeviceType currentActiveDevice_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioRingerMode ringerMode_;
    int32_t safeVolume_ = 0;
    SafeStatus safeStatus_ = SAFE_ACTIVE;
    SafeStatus safeStatusBt_ = SAFE_ACTIVE;
    int64_t safeActiveTime_ = 0;
    int64_t safeActiveBtTime_ = 0;
    int32_t safeVolumeTimeout_ = DEFAULT_SAFE_VOLUME_TIMEOUT;
    bool isWiredBoot_ = true;
    bool isBtBoot_ = true;
    bool isBtFirstSetVolume_ = true;
    int32_t curActiveCount_ = 0;

    std::shared_ptr<SingleKvStore> audioPolicyKvStore_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    AudioStreamRemovedCallback *sessionCallback_ = nullptr;
    VolumeDataMaintainer &volumeDataMaintainer_;
    bool isVolumeUnadjustable_ = false;
    bool testModeOn_ {false};
    float getSystemVolumeInDb_ = 0.0f;
    bool useNonlinearAlgo_ = false;
    bool isAbsVolumeScene_ = false;
    bool isAbsVolumeMute_ = false;
    bool isNeedCopyVolumeData_ = false;
    bool isNeedCopyMuteData_ = false;
    bool isNeedCopyRingerModeData_ = false;
    bool isNeedCopySystemUrlData_ = false;
    bool isLoaded_ = false;
    bool isAllCopyDone_ = false;
    bool isNeedConvertSafeTime_ = false;
};

class PolicyCallbackImpl : public AudioServiceAdapterCallback {
public:
    explicit PolicyCallbackImpl(AudioAdapterManager *audioAdapterManager)
    {
        audioAdapterManager_ = audioAdapterManager;
    }

    ~PolicyCallbackImpl()
    {
        AUDIO_WARNING_LOG("Destructor PolicyCallbackImpl");
    }

    virtual std::pair<float, int32_t> OnGetVolumeDbCb(AudioStreamType streamType)
    {
        AudioStreamType streamForVolumeMap = audioAdapterManager_->GetStreamForVolumeMap(streamType);
        int32_t volumeLevel = audioAdapterManager_->GetStreamVolume(streamForVolumeMap);

        bool isAbsVolumeScene = audioAdapterManager_->IsAbsVolumeScene();
        DeviceType activeDevice = audioAdapterManager_->GetActiveDevice();
        if (streamForVolumeMap == STREAM_MUSIC && activeDevice == DEVICE_TYPE_BLUETOOTH_A2DP && isAbsVolumeScene) {
            int32_t vol = audioAdapterManager_->IsAbsVolumeMute() ? 0.0f : 1.0f;
            return {vol, volumeLevel};
        }

        bool muteStatus = audioAdapterManager_->GetStreamMute(streamForVolumeMap);
        if (muteStatus) {
            return {0.0f, 0};
        }

        float volumeDb = 1.0f;
        if (audioAdapterManager_->IsUseNonlinearAlgo()) {
            volumeDb = audioAdapterManager_->CalculateVolumeDbNonlinear(streamForVolumeMap,
                audioAdapterManager_->GetActiveDevice(), volumeLevel);
        } else {
            volumeDb = audioAdapterManager_->CalculateVolumeDb(volumeLevel);
        }
        return {volumeDb, volumeLevel};
    }

    void OnAudioStreamRemoved(const uint64_t sessionID)
    {
        AUDIO_DEBUG_LOG("PolicyCallbackImpl OnAudioStreamRemoved: Session ID %{public}" PRIu64"", sessionID);
        if (audioAdapterManager_->sessionCallback_ == nullptr) {
            AUDIO_DEBUG_LOG("PolicyCallbackImpl audioAdapterManager_->sessionCallback_ == nullptr"
                "not firing OnAudioStreamRemoved");
        } else {
            audioAdapterManager_->sessionCallback_->OnAudioStreamRemoved(sessionID);
        }
    }

private:
    AudioAdapterManager *audioAdapterManager_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_PULSEAUDIO_ADAPTER_MANAGER_H
