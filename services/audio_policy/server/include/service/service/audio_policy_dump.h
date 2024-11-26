#ifndef ST_AUDIO_POLICY_DUMP_H
#define ST_AUDIO_POLICY_DUMP_H

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
#include "audio_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyDump {
public:
    static AudioPolicyDump& GetInstance()
    {
        static AudioPolicyDump instance;
        return instance;
    }
    void DevicesInfoDump(std::string &dumpString);
    void AudioModeDump(std::string &dumpString);
    void StreamVolumesDump(std::string &dumpString);
    void AudioPolicyParserDump(std::string &dumpString);
    void AudioStreamDump(std::string &dumpString);
    void XmlParsedDataMapDump(std::string &dumpString);
    void EffectManagerInfoDump(std::string &dumpString);
    void MicrophoneMuteInfoDump(std::string &dumpString);
private:
    std::vector<sptr<AudioDeviceDescriptor>> GetDumpDeviceInfo(std::string &dumpString,
        DeviceFlag deviceFlag);
    std::vector<sptr<AudioDeviceDescriptor>> GetDumpDevices(DeviceFlag deviceFlag);
    void GetMicrophoneDescriptorsDump(std::string &dumpString);
    void GetOffloadStatusDump(std::string &dumpString);

    void GetCallStatusDump(std::string &dumpString);
    void GetRingerModeDump(std::string &dumpString);

    void GetVolumeConfigDump(std::string &dumpString);
    void DeviceVolumeInfosDump(std::string &dumpString, DeviceVolumeInfoMap &deviceVolumeInfos);
    void GetGroupInfoDump(std::string &dumpString);

    void GetCapturerStreamDump(std::string &dumpString);

    void StreamEffectSceneInfoDump(std::string &dumpString, const ProcessNew &processNew, const std::string processType);
private:
    AudioPolicyDump() {}
    ~AudioPolicyDump() {}
private:
    DeviceType priorityOutputDevice_ = DEVICE_TYPE_INVALID;
    DeviceType priorityInputDevice_ = DEVICE_TYPE_INVALID;
    ConnectType conneceType_ = CONNECT_TYPE_LOCAL;
};

}
}

#endif