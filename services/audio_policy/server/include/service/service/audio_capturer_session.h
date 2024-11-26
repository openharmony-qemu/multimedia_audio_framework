
#ifndef ST_AUDIO_POLICY_CAPTURER_SESSION_H
#define ST_AUDIO_POLICY_CAPTURER_SESSION_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_adapter_info.h"
#include "audio_utils.h"
#include "audio_errors.h"


namespace OHOS {
namespace AudioStandard {

class AudioPolicyCapturerSession {
public:
    static AudioPolicyCapturerSession& GetInstance()
    {
        static AudioPolicyCapturerSession instance;
        return instance;
    }
    void SetConfigParserFlag();
    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
    void OnCapturerSessionRemoved(uint64_t sessionID);

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);
    int32_t CloseWakeUpAudioCapturer();
private:
    AudioPolicyCapturerSession() {}
    ~AudioPolicyCapturerSession() {}

    void HandleRemainingSource();
    int32_t FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, StreamPropInfo &targetInfo,
        SourceType &targetSourceType);
    void BluetoothScoDisconectForRecongnition();
    void HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo = {});

    void LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo);
    void UnloadInnerCapturerSink(std::string moduleName);

    bool ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
        AudioModuleInfo &audioModuleInfo);
    bool FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
        AudioModuleInfo &audioModuleInfo);
private:
    std::atomic<bool> isPolicyConfigParsered_ = false;
    std::unordered_map<uint32_t, SessionInfo> sessionWithNormalSourceType_;
    std::unordered_set<uint32_t> sessionIdisRemovedSet_;
    // sourceType is SOURCE_TYPE_PLAYBACK_CAPTURE, SOURCE_TYPE_WAKEUP or SOURCE_TYPE_VIRTUAL_CAPTURE
    std::unordered_map<uint32_t, SessionInfo> sessionWithSpecialSourceType_;
};

}
}

#endif