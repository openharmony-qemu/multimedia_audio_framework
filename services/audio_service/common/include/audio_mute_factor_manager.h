#ifndef AUDIO_MUTE_FACTOR_MANAGER_H
#define AUDIO_MUTE_FACTOR_MANAGER_H

namespace OHOS {
namespace AudioStandard {

class AudioMuteFactorManager {
public:
    static AudioMuteFactorManager& GetInstance();

	AudioMuteFactorManager(const AudioMuteFactorManager&) = delete;
	AudioMuteFactorManager& operator=(const AudioMuteFactorManager&) = delete;

	bool GetMdmMuteStatus() const;

	void SetMdmMuteStatus(bool mdmMute);

private:
    AudioMuteFactorManager();
	~AudioMuteFactorManager();

    bool isMdmMute_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_MUTE_FACTOR_MANAGER_H