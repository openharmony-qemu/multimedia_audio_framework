/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_TIMELINE_H
#define AUDIOEDITTESTAPP_TIMELINE_H
#include <map>
#include <string>
#include <vector>

struct AudioAsset {
    long startTime = 0;
    long endTime = 0;
    long pcmBufferLength = 0;
    int sampleRate = 0;
    int channels = 0;
    int bitsPerSample = 0;
    std::vector<std::string> effectNodeIdList = {};
    long currentPcmBufferIndex = 0;
};

struct AudioTrack {
    std::string trackId;
    bool isSilent = false;
    std::multimap<long, AudioAsset> assets;
    long maxEndTime = 0;
    long currentTime = 0;
};

class Timeline {
public:
    ~Timeline() = default;
    static Timeline& GetInstance();
    void Clear();
    // AudioTrack
    bool AddAudioTrack(AudioTrack& track);
    bool DeleteAudioTrack(const std::string& trackId);
    bool DeleteAllAudioTrack();
    bool UpdateAudioTrack(AudioTrack& track);
    AudioTrack* GetAudioTrack(const std::string& trackId);
    bool SetAudioTrack(const std::string& trackId, const AudioTrack& track);
    bool SetAudioTrackSilent(const std::vector<std::string>& trackIds, const std::vector<bool>& isSilents);
    // AudioAsset
    bool CheckIndex(const long &startIndex, const long &endIndex, const std::string &oldKey) const;
    bool AddAudioAsset(const std::string &trackId, const AudioAsset &asset, const long oldStartTime, std::vector<long> &indexs,
                       const bool isCopyMultiple);
    bool UpdateAudioAsset(const std::string& trackId, AudioAsset& asset, const long startIndex, const long endIndex);
    bool DeleteAudioAsset(const std::string& trackId, const long startTime);
    AudioAsset* GetAudioAsset(const std::string& trackId, const long startTime);
    bool SetAudioAsset(const std::string& trackId, const long startTime, AudioAsset& asset);
    bool SetAudioAssetStartTime(const std::string& trackId, const long originStartTime, const long newStartTime);
    bool SetAudioAssetPcmBufferLength(const std::string& trackId, const long startTime, const long pcmBufferLength);
    std::vector<std::string> GetAudioAssetEffectNodeIdList(const std::string& trackId, const long startTime);
    bool AddAudioAssetEffectNode(const std::string& trackId, const long startTime,
                                 const std::string& effectNodeId);
    bool DeleteAudioAssetEffectNode(const std::string& trackId, const long startTime,
                                    const std::string& effectNodeId);
    // AudioAsset Operation
    bool CopyAudioAsset(const std::string& trackId, const std::string& srcAssetId, const std::string& copyAssetId);
    bool SplitAudioAsset(const std::string& trackId, const std::string& srcAssetId, long splitTime,
                         const std::string& splitLeftAssetId, const std::string& splitRightAssetId);
    // PcmBuffer Operation
    bool GetPcmBuffer(void* pcmBuffer, const std::string& trackId, const long startTime);
    
    void ResetCurrent(const long currentTimeTemp);
private:
    Timeline() = default;
private:
    long currentTime = 0;
    std::map<std::string, AudioTrack> audioTrackMap;
};

#endif //AUDIOEDITTESTAPP_TIMELINE_H
