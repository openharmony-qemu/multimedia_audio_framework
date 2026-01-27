/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Timeline.h"
#include "audioEffectNode/Input.h"
#include <hilog/log.h>
#include "./utils/Constant.h"

static const int GLOBAL_RESMGR = 0xFF00;
static const char *TAG = "[AudioEditTestApp_Timeline_cpp]";

Timeline& Timeline::GetInstance()
{
    static Timeline instance;
    return instance;
}

void Timeline::Clear()
{
    for (auto it = audioTrackMap.begin(); it != audioTrackMap.end(); ++it) {
        it->second.assets.clear();
    }
    audioTrackMap.clear();
    currentTime = 0;
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "clear success");
}

bool Timeline::AddAudioTrack(AudioTrack& track)
{
    auto it = audioTrackMap.find(track.trackId);
    if (it != audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "addAudioTrack trackId: %{public}s is repeated", track.trackId.c_str());
        return true;
    }
    audioTrackMap.emplace(track.trackId, track);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "addAudioTrack trackId: %{public}s success", track.trackId.c_str());
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                 "audioTrackMap size: %{public}d success", audioTrackMap.size());
    return true;
}

bool Timeline::DeleteAudioTrack(const std::string &trackId)
{
    auto it = audioTrackMap.find(trackId);
    if (it != audioTrackMap.end()) {
        audioTrackMap.erase(it);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "deleteAudioTrack trackId: %{public}s success", trackId.c_str());
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "deleteAudioTrack trackId: %{public}s not exist", trackId.c_str());
    return true;
}

bool Timeline::DeleteAllAudioTrack()
{
    for (auto& [trackId, audioTrack] : audioTrackMap) {
        audioTrack.assets.clear();  // 清空每个 AudioTrack 中的 assets
    }
    audioTrackMap.clear();  // 清空整个 map
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG, "deleteAllAudioTrack success");
    return true;
}

bool Timeline::UpdateAudioTrack(AudioTrack& track)
{
    auto it = audioTrackMap.find(track.trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "updateAudioTrack trackId: %{public}s not exist", track.trackId.c_str());
        return false;
    }
    it->second = track;
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "updateAudioTrack trackId: %{public}s success", track.trackId.c_str());
    return true;
}

AudioTrack* Timeline::GetAudioTrack(const std::string& trackId)
{
    auto it = audioTrackMap.find(trackId);
    if (it != audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "getAudioTrack trackId: %{public}s success", trackId.c_str());
        return &it->second;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "getAudioTrack trackId: %{public}s not exist", trackId.c_str());
    return nullptr;
}

bool Timeline::SetAudioTrack(const std::string& trackId, AudioTrack& track)
{
    auto it = audioTrackMap.find(trackId);
    if (it != audioTrackMap.end()) {
        it->second = track;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "setAudioTrack trackId: %{public}s success", trackId.c_str());
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "setAudioTrack trackId: %{public}s not exist", trackId.c_str());
    return false;
}

bool Timeline::SetAudioTrackSilent(const std::vector<std::string>& trackIds, const std::vector<bool>& isSilents)
{
    if (trackIds.size() != isSilents.size()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "setAudioTrackSilent trackIds.size(): %{public}zu is not equal isSilents.size(): %{public}zu",
                     trackIds.size(), isSilents.size());
        return false;
    }
    for (size_t i = 0; i < trackIds.size(); i++) {
        std::string trackId = trackIds[i];
        bool isSilent = isSilents[i];
        auto it = audioTrackMap.find(trackId);
        if (it != audioTrackMap.end()) {
            it->second.isSilent = isSilent;
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                         "setAudioTrackSilent trackId: %{public}s success", trackId.c_str());
            continue;
        }
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "setAudioTrackSilent trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    return true;
}

// 获取音频的时长(ms)
long GetIndexMultiple(int channels, int bitsPerSample)
{
    int bits = UINT_1;
    switch (bitsPerSample) {
        case UINT_0:
            bits = UINT_1;
            break;
        case UINT_1:
            bits = UINT_2;
            break;
        case UINT_2:
            bits = UINT_3;
            break;
        case UINT_3:
            bits = UINT_4;
            break;
        case UINT_4:
            bits = UINT_4;
            break;
        default:
            bits = UINT_1;
    }
    return  bits * channels;
}

bool Timeline::CheckIndex(long &startIndex, long &endIndex, std::string &oldKey)
{
    auto item = g_writeDataBufferMap.find(oldKey);
    if (item == g_writeDataBufferMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "addAudioAsset g_writeDataBufferMap failed, oldKey is not exist");
        return true;
    }
    if (startIndex < 0 || startIndex > endIndex) {
        OH_LOG_Print(
            LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
            "addAudioAsset invalid index range: start=%{public}ld, end=%{public}ld, oldPcmdataSize: %{public}d",
            startIndex, endIndex, item->second.size());
        return true;
    }
    return false;
}

bool Timeline::AddAudioAsset(const std::string &trackId, AudioAsset &asset, long oldStartTime,
                             std::vector<long> &indexs, bool isCopyMultiple)
{
    long startIndex = indexs[ARG_0];
    long endIndex = indexs[ARG_1];
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "addAudioAsset trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(oldStartTime);
    if (assetIt == assetMap.end()) {
        return false;
    }
    std::string oldKey = trackId;
    if (oldStartTime != 0) {
        oldKey = trackId.c_str() + std::to_string(oldStartTime);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "addAudioAsset key: %{public}s", oldKey.c_str());
    if (CheckIndex(startIndex, endIndex, oldKey)) { return false; }
    auto item = g_writeDataBufferMap.find(oldKey);
    int indexMultiple = GetIndexMultiple(assetIt->second.channels, assetIt->second.bitsPerSample);
    if (indexMultiple != 0 && startIndex % indexMultiple != 0) {
        startIndex = (startIndex / indexMultiple + 1) * indexMultiple;
    }
    // Extract data within a specified range
    if (endIndex >= item->second.size() && !isCopyMultiple) {
        endIndex = item->second.size();
    }
    indexs[ARG_0] = startIndex;
    indexs[ARG_1] = endIndex;
    long endTime = asset.startTime + 1000 * (endIndex - startIndex) / assetIt->second.sampleRate /
                                      assetIt->second.channels / GetBit(assetIt->second.bitsPerSample);
    AudioAsset newAsset{
        startTime : asset.startTime,
        endTime : endTime,
        pcmBufferLength : endIndex - startIndex,
        sampleRate : assetIt->second.sampleRate,
        channels : assetIt->second.channels,
        bitsPerSample : assetIt->second.bitsPerSample,
    };
    assetMap.emplace(asset.startTime, newAsset);
    // Recalculate the maximum end time (to ensure absolute accuracy)
    long maxEnd = 0;
    for (const auto &[_, assetVal] : assetMap) {
        if (assetVal.endTime > maxEnd) {
            maxEnd = assetVal.endTime;
        }
    }
    it->second.maxEndTime = maxEnd;
    return true;
}

bool Timeline::UpdateAudioAsset(const std::string& trackId, AudioAsset& asset, long startIndex, long endIndex)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "updateAudioAsset trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(asset.startTime);
    if (assetIt == assetMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "updateAudioAsset startTime: %{public}ld not exist", asset.startTime);
        return false;
    }
    long endTime = asset.startTime + 1000 * (endIndex - startIndex) / assetIt->second.sampleRate /
                                         assetIt->second.channels / GetBit(assetIt->second.bitsPerSample);
    
    AudioAsset newAsset {
        startTime: asset.startTime,
        endTime: endTime,
        pcmBufferLength: endIndex - startIndex,
        sampleRate: assetIt->second.sampleRate,
        channels: assetIt->second.channels,
        bitsPerSample: assetIt->second.bitsPerSample,
    };
    for (auto iter = it->second.assets.begin(); iter != it->second.assets.end();) {
        if (iter->second.startTime == asset.startTime) {
            iter = it->second.assets.erase(iter); // 删除当前元素并更新迭代器
        } else {
            ++iter; // 移动到下一个元素
        }
    }
    assetMap.emplace(asset.startTime, newAsset);
    // 重新计算最大结束时间（确保绝对正确）
    long maxEnd = 0;
    for (const auto& [_, assetVal] : assetMap) {
        if (assetVal.endTime > maxEnd) maxEnd = assetVal.endTime;
    }
    it->second.maxEndTime = maxEnd;
    auto track = audioTrackMap.find(trackId);
    for (const auto &pair : track->second.assets) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "Timeline::updateAudioAsset startTime: %{public}d, endTime: %{public}d, maxEndTime: %{public}ld",
                     pair.second.startTime, pair.second.endTime, it->second.maxEndTime);
    }
    return true;
}

bool Timeline::DeleteAudioAsset(const std::string& trackId, const long startTime)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "deleteAudioAsset trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        assetMap.erase(assetIt);
      // 重新计算最大结束时间（确保绝对正确）
        long maxEnd = 0;
        for (const auto& [_, assetVal] : assetMap) {
            if (assetVal.endTime > maxEnd) maxEnd = assetVal.endTime;
        }
        it->second.maxEndTime = maxEnd;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "deleteAudioAsset trackId: %{public}s startTime: %{public}ld success",
                     trackId.c_str(), startTime);
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "deleteAudioAsset startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return true;
}

AudioAsset* Timeline::GetAudioAsset(const std::string& trackId, const long startTime)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "getAudioAsset trackId: %{public}s not exist", trackId.c_str());
        return nullptr;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "getAudioAsset trackId: %{public}s startTime: %{public}ld success",
                     trackId.c_str(), startTime);
        return &assetIt->second;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "getAudioAsset startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return nullptr;
}

bool Timeline::SetAudioAsset(const std::string& trackId, const long startTime, AudioAsset& asset)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "setAudioAsset trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        assetIt->second = asset;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "setAudioAsset trackId: %{public}s startTime: %{public}ld success",
                     trackId.c_str(), startTime);
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "setAudioAsset startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return false;
}

bool Timeline::SetAudioAssetStartTime(const std::string& trackId, const long originStartTime, const long newStartTime)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "setAudioAssetStartTime trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(originStartTime);
    if (assetIt != assetMap.end()) {
        AudioAsset asset = assetIt->second;
        asset.startTime = newStartTime;
        asset.endTime += (newStartTime - originStartTime);
        assetMap.erase(assetIt);
        assetMap.emplace(newStartTime, asset);
         // 重新计算最大结束时间（确保绝对正确）
        long maxEnd = 0;
        for (const auto& [_, assetVal] : assetMap) {
            if (assetVal.endTime > maxEnd) maxEnd = assetVal.endTime;
        }
        it->second.maxEndTime = maxEnd;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "setAudioAssetStartTime trackId: %{public}s "
                     "originStartTime: %{public}ld newStartTime: %{public}ld success",
                     trackId.c_str(), originStartTime, newStartTime);
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "setAudioAssetStartTime originStartTime: %{public}ld not exist in track %{public}s",
                 originStartTime, trackId.c_str());
    return false;
}
bool Timeline::SetAudioAssetPcmBufferLength(const std::string& trackId,
                                            const long startTime, const long pcmBufferLength)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "setAudioAssetPcmBufferLength trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        assetIt->second.pcmBufferLength = pcmBufferLength;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG, "setAudioAssetPcmBufferLength trackId: %{public}s "
                     "startTime: %{public}ld pcmBufferLength: %{public}ld success",
                     trackId.c_str(), startTime, pcmBufferLength);
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "setAudioAssetPcmBufferLength startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return false;
}

std::vector<std::string> Timeline::GetAudioAssetEffectNodeIdList(const std::string& trackId,
                                                                 const long startTime)
{
    std::vector<std::string> result;
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "getAudioAssetEffectNodeIdList trackId: %{public}s not exist", trackId.c_str());
        return result;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        result = assetIt->second.effectNodeIdList;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "getAudioAssetEffectNodeIdList trackId: %{public}s startTime: %{public}ld success",
                     trackId.c_str(), startTime);
        return result;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "getAudioAssetEffectNodeIdList startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return result;
}
bool Timeline::AddAudioAssetEffectNode(const std::string& trackId, const long startTime,
                                       const std::string& effectNodeId)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "addAudioAssetEffectNode trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        assetIt->second.effectNodeIdList.push_back(effectNodeId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "addAudioAssetEffectNode trackId: %{public}s startTime: %{public}ld "
                     "effectNodeId: %{public}s success", trackId.c_str(), startTime, effectNodeId.c_str());
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "addAudioAssetEffectNode startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return false;
}
bool Timeline::DeleteAudioAssetEffectNode(const std::string& trackId, const long startTime,
                                          const std::string& effectNodeId)
{
    auto it = audioTrackMap.find(trackId);
    if (it == audioTrackMap.end()) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                     "deleteAudioAssetEffectNode trackId: %{public}s not exist", trackId.c_str());
        return false;
    }
    auto& assetMap = it->second.assets;
    auto assetIt = assetMap.find(startTime);
    if (assetIt != assetMap.end()) {
        auto& nodeList = assetIt->second.effectNodeIdList;
        nodeList.erase(
            std::remove(nodeList.begin(), nodeList.end(), effectNodeId),
            nodeList.end()
        );
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, TAG,
                     "deleteAudioAssetEffectNode trackId: %{public}s startTime: %{public}ld "
                     "effectNodeId: %{public}s success", trackId.c_str(), startTime, effectNodeId.c_str());
        return true;
    }
    OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, TAG,
                 "deleteAudioAssetEffectNode startTime: %{public}ld not exist in track %{public}s",
                 startTime, trackId.c_str());
    return false;
}

void Timeline::ResetCurrent(const long currentTimeTemp)
{
    for (auto& pair : audioTrackMap) {
        auto& track = pair.second;
        // 给 currentTime 重新赋值，这里假设我们将其设置为 maxEndTime
        track.currentTime = currentTimeTemp;
    }
}