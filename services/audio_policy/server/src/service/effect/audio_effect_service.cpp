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
#undef LOG_TAG
#define LOG_TAG "AudioEffectService"

#include "audio_effect_service.h"

#include <unordered_set>

#include "audio_device_type.h"
#include "audio_effect_map.h"

namespace OHOS {
namespace AudioStandard {
const std::set<std::string> STREAM_USAGE_SET = {
    "STREAM_USAGE_UNKNOWN",
    "STREAM_USAGE_MEDIA",
    "STREAM_USAGE_MUSIC",
    "STREAM_USAGE_VOICE_COMMUNICATION",
    "STREAM_USAGE_VOICE_ASSISTANT",
    "STREAM_USAGE_VOICE_CALL_ASSISTANT",
    "STREAM_USAGE_ALARM",
    "STREAM_USAGE_VOICE_MESSAGE",
    "STREAM_USAGE_NOTIFICATION_RINGTONE",
    "STREAM_USAGE_RINGTONE",
    "STREAM_USAGE_NOTIFICATION",
    "STREAM_USAGE_ACCESSIBILITY",
    "STREAM_USAGE_SYSTEM",
    "STREAM_USAGE_MOVIE",
    "STREAM_USAGE_GAME",
    "STREAM_USAGE_AUDIOBOOK",
    "STREAM_USAGE_NAVIGATION",
    "STREAM_USAGE_DTMF",
    "STREAM_USAGE_ENFORCED_TONE",
    "STREAM_USAGE_ULTRASONIC",
    "STREAM_USAGE_VIDEO_COMMUNICATION",
    "STREAM_USAGE_RANGING",
    "STREAM_USAGE_VOICE_MODEM_COMMUNICATION",
    "STREAM_USAGE_VOICE_RINGTONE"
};
AudioEffectService::AudioEffectService()
{
    AUDIO_INFO_LOG("AudioEffectService ctor");
}

AudioEffectService::~AudioEffectService()
{
}

void AudioEffectService::EffectServiceInit()
{
    AUDIO_INFO_LOG("In");
    // load XML
    std::unique_ptr<AudioEffectConfigParser> effectConfigParser = std::make_unique<AudioEffectConfigParser>();
    int32_t ret = effectConfigParser->LoadEffectConfig(oriEffectConfig_);
    CHECK_AND_RETURN_LOG(ret == 0, "AudioEffectService->effectConfigParser failed: %{public}d", ret);
    AUDIO_INFO_LOG("Out");
}

void AudioEffectService::GetAvailableEffects(std::vector<Effect> &availableEffects)
{
    availableEffects = availableEffects_;
}

void AudioEffectService::GetOriginalEffectConfig(OriginalEffectConfig &oriEffectConfig)
{
    oriEffectConfig = oriEffectConfig_;
}

void AudioEffectService::UpdateAvailableEffects(std::vector<Effect> &newAvailableEffects)
{
    availableEffects_ = newAvailableEffects;
}

int32_t AudioEffectService::QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    supportedEffectConfig = supportedEffectConfig_;
    return existDefault_;
}

void AudioEffectService::GetSupportedEffectConfig(SupportedEffectConfig &supportedEffectConfig)
{
    supportedEffectConfig = supportedEffectConfig_;
}

static void UpdateUnsupportedDevicePre(PreStreamScene &pp, Stream &stream, const std::string &mode,
                                       int32_t i, int32_t j)
{
    StreamEffectMode streamEffectMode;
    streamEffectMode.mode = mode;
    j = 0;
    for (auto &device: pp.device) {
        if (i == j) {
            for (auto &eachDevice: device) {
                streamEffectMode.devicePort.push_back(eachDevice);
            }
            break;
        }
        j += 1;
    }
    stream.streamEffectMode.push_back(streamEffectMode);
}

static void UpdateUnsupportedModePre(PreStreamScene &pp, Stream &stream, std::string &mode, int32_t i)
{
    int32_t isSupported = 0;
    if ((mode != "ENHANCE_NONE") &&
        (mode != "ENHANCE_DEFAULT")) {
        AUDIO_INFO_LOG("[supportedEnhanceConfig LOG10]:mode-> The %{public}s mode of %{public}s is unsupported, \
            and this mode is deleted!", mode.c_str(), stream.scene.c_str());
        isSupported = -1;
    }
    if (isSupported == 0) {
        int32_t j = 0;
        UpdateUnsupportedDevicePre(pp, stream, mode, i, j);
    }
}

static void UpdateUnsupportedDevicePost(PostStreamScene &ess, Stream &stream, const std::string &mode, int32_t i)
{
    StreamEffectMode streamEffectMode;
    streamEffectMode.mode = mode;
    int32_t j = 0;
    for (auto &device: ess.device) {
        if (i == j) {
            for (auto &a: device) {
                streamEffectMode.devicePort.push_back(a);
            }
            break;
        }
        j += 1;
    }
    stream.streamEffectMode.push_back(streamEffectMode);
}

static void UpdateUnsupportedModePost(PostStreamScene &ess, Stream &stream, std::string &mode, int32_t i)
{
    int32_t isSupported = 0;
    if ((mode != "EFFECT_NONE") &&
        (mode != "EFFECT_DEFAULT")) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG10]:mode-> The %{public}s mode of %{public}s is unsupported, \
            and this mode is deleted!", mode.c_str(), stream.scene.c_str());
        isSupported = -1;
    }
    if (isSupported == 0) {
        UpdateUnsupportedDevicePost(ess, stream, mode, i);
    }
}

static int32_t UpdateAvailableStreamPre(ProcessNew &preProcessNew, PreStreamScene &pp, ScenePriority priority)
{
    bool isDuplicate = false;
    bool isSupported = false;
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    for (auto &[scene, stream] : audioEnhanceSupportedSceneTypes) {
        if (pp.stream == stream) {
            isSupported = true;
            break;
        }
    }
    auto it = std::find_if(preProcessNew.stream.begin(), preProcessNew.stream.end(), [&](const Stream &x) {
        return ((x.scene == pp.stream) && (x.priority == priority));
    });
    if ((it == preProcessNew.stream.end()) && isSupported) {
        Stream stream;
        stream.priority = priority;
        stream.scene = pp.stream;
        int32_t i = 0;
        for (auto &mode: pp.mode) {
            UpdateUnsupportedModePre(pp, stream, mode, i);
        }
        preProcessNew.stream.push_back(stream);
    } else if (it != preProcessNew.stream.end()) {
        isDuplicate = true;
    }
    return isDuplicate;
}

static int32_t UpdateAvailableStreamPost(ProcessNew &postProcessNew, PostStreamScene &ess, ScenePriority priority)
{
    bool isDuplicate = false;
    bool isSupported = false;
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    for (auto &[scene, stream] : audioSupportedSceneTypes) {
        if (ess.stream == stream) {
            isSupported = true;
            break;
        }
    }
    auto it = std::find_if(postProcessNew.stream.begin(), postProcessNew.stream.end(), [&](const Stream &x) {
        return ((x.scene == ess.stream) && (x.priority == priority));
    });
    if ((it == postProcessNew.stream.end()) && isSupported) {
        Stream stream;
        stream.priority = priority;
        stream.scene = ess.stream;
        int32_t i = 0;
        for (auto &mode: ess.mode) {
            UpdateUnsupportedModePost(ess, stream, mode, i);
        }
        postProcessNew.stream.push_back(stream);
    } else if (it != postProcessNew.stream.end()) {
        isDuplicate = true;
    }
    return isDuplicate;
}

static int32_t UpdateAvailableSceneMapPost(SceneMappingItem &item, std::vector<SceneMappingItem> &postProcessSceneMap)
{
    bool isDuplicate = false;
    auto it = std::find_if(postProcessSceneMap.begin(), postProcessSceneMap.end(),
        [&item](const SceneMappingItem &x) {
        return x.name == item.name;
    });
    if ((it == postProcessSceneMap.end())) {
        postProcessSceneMap.push_back(item);
    } else {
        isDuplicate = true;
    }
    return isDuplicate;
}

bool AudioEffectService::VerifySceneMappingItem(const SceneMappingItem &item)
{
    return STREAM_USAGE_SET.find(item.name) != STREAM_USAGE_SET.end() &&
        std::find(postSceneTypeSet_.begin(), postSceneTypeSet_.end(), item.sceneType) != postSceneTypeSet_.end();
}

void AudioEffectService::UpdateEffectChains(std::vector<std::string> &availableLayout)
{
    int32_t count = 0;
    std::vector<int> deviceDelIdx;
    for (const auto &ec: supportedEffectConfig_.effectChains) {
        for (const auto &effectName: ec.apply) {
            auto it = std::find_if(availableEffects_.begin(), availableEffects_.end(),
                [&effectName](const Effect &effect) {
                return effect.name == effectName;
            });
            if (it == availableEffects_.end()) {
                deviceDelIdx.emplace_back(count);
                break;
            }
        }
        count += 1;
    }
    for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
        supportedEffectConfig_.effectChains.erase(supportedEffectConfig_.effectChains.begin() + *it);
    }
    if (supportedEffectConfig_.effectChains.empty()) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG1]:effectChains-> all effectChains are unavailable");
    }
    for (auto ec: supportedEffectConfig_.effectChains) {
        availableLayout.emplace_back(ec.name);
    }
}

void AudioEffectService::UpdateAvailableAEConfig(OriginalEffectConfig &aeConfig)
{
    int32_t ret = 0;
    supportedEffectConfig_.effectChains = aeConfig.effectChains;
    ProcessNew preProcessNew;
    for (PreStreamScene &pp: aeConfig.preProcess.defaultScenes) {
        ret += UpdateAvailableStreamPre(preProcessNew, pp, DEFAULT_SCENE);
    }
    for (PreStreamScene &pp: aeConfig.preProcess.priorScenes) {
        ret += UpdateAvailableStreamPre(preProcessNew, pp, PRIOR_SCENE);
    }
    for (PreStreamScene &pp: aeConfig.preProcess.normalScenes) {
        ret += UpdateAvailableStreamPre(preProcessNew, pp, NORMAL_SCENE);
    }

    ProcessNew postProcessNew;
    for (PostStreamScene &ess: aeConfig.postProcess.defaultScenes) {
        ret += UpdateAvailableStreamPost(postProcessNew, ess, DEFAULT_SCENE);
    }
    for (PostStreamScene &ess: aeConfig.postProcess.priorScenes) {
        ret += UpdateAvailableStreamPost(postProcessNew, ess, PRIOR_SCENE);
    }
    for (PostStreamScene &ess: aeConfig.postProcess.normalScenes) {
        ret += UpdateAvailableStreamPost(postProcessNew, ess, NORMAL_SCENE);
    }

    if (ret > 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG2]:stream-> duplicate streams has been deleted");
    }
    supportedEffectConfig_.preProcessNew = preProcessNew;
    supportedEffectConfig_.postProcessNew = postProcessNew;

    for (Stream &ss: supportedEffectConfig_.postProcessNew.stream) {
        postSceneTypeSet_.push_back(ss.scene);
    }
    AUDIO_INFO_LOG("postSceneTypeSet_ size is %{public}zu", supportedEffectConfig_.postProcessNew.stream.size());
    std::vector<SceneMappingItem> postSceneMap;
    for (SceneMappingItem &item: aeConfig.postProcess.sceneMap) {
        if (!VerifySceneMappingItem(item)) {
            AUDIO_WARNING_LOG("Invalid %{public}s-%{public}s pair has been ignored",
                item.name.c_str(), item.sceneType.c_str());
            continue;
        }
        if (UpdateAvailableSceneMapPost(item, postSceneMap)) {
            AUDIO_WARNING_LOG("The duplicate streamUsage-sceneType pair is deleted, \
                and the first configuration is retained!");
        }
    }
    supportedEffectConfig_.postProcessSceneMap = postSceneMap;
}

void AudioEffectService::UpdateDuplicateBypassMode(ProcessNew &processNew)
{
    int32_t flag = 0;
    std::vector<int32_t> deviceDelIdx;
    for (auto &stream: processNew.stream) {
        int32_t count = 0;
        deviceDelIdx.clear();
        for (const auto &streamEffectMode: stream.streamEffectMode) {
            if (streamEffectMode.mode == "EFFECT_NONE") {
                deviceDelIdx.push_back(count);
            }
            count += 1;
        }
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            stream.streamEffectMode[*it].devicePort = {};
            flag = -1;
        }
    }
    if (flag == -1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG3]:mode-> EFFECT_NONE can not configure by deveploer!");
    }
}

void AudioEffectService::UpdateDuplicateMode(ProcessNew &processNew)
{
    std::unordered_set<std::string> seen;
    std::vector<int32_t> toRemove;
    uint32_t i;
    for (auto &stream: processNew.stream) {
        seen.clear();
        toRemove.clear();
        for (i = 0; i < stream.streamEffectMode.size(); i++) {
            if (seen.count(stream.streamEffectMode[i].mode)) {
                toRemove.push_back(i);
            } else {
                seen.insert(stream.streamEffectMode[i].mode);
            }
        }
        for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG4]:mode-> The duplicate mode of %{public}s configuration \
                is deleted, and the first configuration is retained!", stream.scene.c_str());
            stream.streamEffectMode.erase(stream.streamEffectMode.begin() + *it);
        }
    }
}

static void UpdateDuplicateDeviceRecord(StreamEffectMode &streamEffectMode, Stream &stream)
{
    uint32_t i;
    std::unordered_set<std::string> seen;
    std::vector<int32_t> toRemove;
    seen.clear();
    toRemove.clear();
    for (i = 0; i < streamEffectMode.devicePort.size(); i++) {
        if (seen.count(streamEffectMode.devicePort[i].type)) {
            toRemove.push_back(i);
        } else {
            seen.insert(streamEffectMode.devicePort[i].type);
        }
    }
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG5]:device-> The duplicate device of %{public}s's %{public}s \
            mode configuration is deleted, and the first configuration is retained!",
            stream.scene.c_str(), streamEffectMode.mode.c_str());
        streamEffectMode.devicePort.erase(streamEffectMode.devicePort.begin() + *it);
    }
}

void AudioEffectService::UpdateDuplicateDevice(ProcessNew &processNew)
{
    for (auto &stream: processNew.stream) {
        for (auto &streamEffectMode: stream.streamEffectMode) {
            UpdateDuplicateDeviceRecord(streamEffectMode, stream);
        }
    }
}

void AudioEffectService::UpdateDuplicateScene(ProcessNew &processNew)
{
    // erase duplicate scene
    std::unordered_set<std::string> scenes;
    for (auto it = processNew.stream.begin(); it != processNew.stream.end();) {
        auto &stream = *it;
        auto its = scenes.find(stream.scene);
        if (its == scenes.end()) {
            scenes.insert(stream.scene);
        } else {
            if (stream.priority == NORMAL_SCENE) {
                it = processNew.stream.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void AudioEffectService::UpdateDuplicateDefaultScene(ProcessNew &processNew)
{
    // erase duplicate default scene
    bool flag = false;
    for (auto it = processNew.stream.begin(); it != processNew.stream.end();) {
        const auto &stream = *it;
        if (stream.priority == DEFAULT_SCENE) {
            if (flag) {
                it = processNew.stream.erase(it);
                continue;
            }
            flag = true;
        }
        ++it;
    }

    // add default scene if no default
    if (!flag) {
        for (auto it = processNew.stream.begin(); it != processNew.stream.end(); ++it) {
            auto &stream = *it;
            if (stream.priority == NORMAL_SCENE) {
                stream.priority = DEFAULT_SCENE;
                break;
            }
        }
    }
}

static int32_t UpdateUnavailableModes(std::vector<int32_t> &modeDelIdx, Stream &stream)
{
    int32_t ret = 0;
    for (auto it = modeDelIdx.rbegin(); it != modeDelIdx.rend(); ++it) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG7]:mode-> %{public}s's %{public}s mode is deleted!",
            stream.scene.c_str(), stream.streamEffectMode[*it].mode.c_str());
        if (stream.streamEffectMode[*it].mode == "PLAYBACK_DEAFULT") {
            ret = -1;
        }
        stream.streamEffectMode.erase(stream.streamEffectMode.begin() + *it);
        if (stream.streamEffectMode.empty()) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG8]:mode-> %{public}s's mode is only EFFECT_NONE!",
                stream.scene.c_str());
            StreamEffectMode streamEffectMode;
            streamEffectMode.mode = "EFFECT_NONE";
            stream.streamEffectMode.push_back(streamEffectMode);
        }
    }
    if (stream.streamEffectMode.empty()) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG8]:mode-> %{public}s's mode is only EFFECT_NONE!",
            stream.scene.c_str());
        StreamEffectMode streamEffectMode;
        streamEffectMode.mode = "EFFECT_NONE";
        stream.streamEffectMode.push_back(streamEffectMode);
    }
    return ret;
}

static void UpdateUnavailableEffectChainsRecord(std::vector<std::string> &availableLayout, Stream &stream,
    StreamEffectMode &streamEffectMode, std::vector<int32_t> &modeDelIdx, int32_t modeCount)
{
    std::vector<int32_t> deviceDelIdx;
    deviceDelIdx.clear();
    int32_t deviceCount = 0;
    if (streamEffectMode.devicePort.empty()) {
        modeDelIdx.push_back(modeCount);
    }
    for (auto &devicePort: streamEffectMode.devicePort) {
        auto index = std::find(availableLayout.begin(), availableLayout.end(), devicePort.chain);
        if (index == availableLayout.end()) {
            deviceDelIdx.push_back(deviceCount);
        }
        deviceCount += 1;
    }
    if (streamEffectMode.devicePort.size() != deviceDelIdx.size() && deviceDelIdx.size() != 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG6]:device-> The unavailable effectChain \
            of %{public}s's %{public}s mode are set to LAYOUT_BYPASS!",
            stream.scene.c_str(), streamEffectMode.mode.c_str());
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            streamEffectMode.devicePort[*it].chain = "LAYOUT_BYPASS";
        }
    } else {
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            streamEffectMode.devicePort.erase(streamEffectMode.devicePort.begin() + *it);
            if (streamEffectMode.devicePort.empty()) {
                modeDelIdx.push_back(modeCount);
            }
        }
    }
}

int32_t AudioEffectService::UpdateUnavailableEffectChains(std::vector<std::string> &availableLayout,
    ProcessNew &processNew)
{
    int32_t ret = 0;

    std::vector<int32_t> modeDelIdx;
    for (auto &stream: processNew.stream) {
        modeDelIdx.clear();
        int32_t modeCount = 0;
        for (auto &streamEffectMode: stream.streamEffectMode) {
            UpdateUnavailableEffectChainsRecord(availableLayout, stream, streamEffectMode, modeDelIdx, modeCount);
        }
        ret = UpdateUnavailableModes(modeDelIdx, stream);
    }
    return ret;
}

void AudioEffectService::UpdateSupportedEffectProperty(const Device &device,
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> &device2PropertySet)
{
    auto chainName = device.chain;
    auto effectChain = std::find_if(supportedEffectConfig_.effectChains.begin(),
        supportedEffectConfig_.effectChains.end(),
        [&chainName](const EffectChain& x) {
            return x.name == chainName;
        });
    if (effectChain == supportedEffectConfig_.effectChains.end()) {
        return;
    }
    for (const auto &effectName : effectChain->apply) {
        auto effectIter = std::find_if(availableEffects_.begin(), availableEffects_.end(),
            [&effectName](const Effect& effect) {
            return effect.name == effectName;
        });
        if (effectIter == availableEffects_.end()) {
            continue;
        }
        for (const auto &property : effectIter->effectProperty) {
            auto deviceIter = device2PropertySet.find(device.type);
            if (deviceIter == device2PropertySet.end()) {
                device2PropertySet[device.type].insert({effectIter->name, property});
            } else {
                deviceIter->second.insert({effectIter->name, property});
            }
            AUDIO_INFO_LOG("device %{public}s support effect [%{public}s, %{public}s]",
                device.type.c_str(), effectIter->name.c_str(), property.c_str());
        }
    }
}

void AudioEffectService::UpdateDuplicateProcessNew(std::vector<std::string> &availableLayout, ProcessNew &processNew)
{
    UpdateEffectChains(availableLayout);
    UpdateDuplicateBypassMode(processNew);
    UpdateDuplicateMode(processNew);
    UpdateDuplicateDevice(processNew);
    UpdateDuplicateDefaultScene(processNew);
    UpdateDuplicateScene(processNew);
    if (UpdateUnavailableEffectChains(availableLayout, supportedEffectConfig_.preProcessNew) != 0) {
        existDefault_ = -1;
    }
}

void AudioEffectService::BuildAvailableAEConfig()
{
    std::vector<std::string> availableLayout;
    existDefault_ = 1;
    if (oriEffectConfig_.effectChains.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG12]: effectChains is none!");
    }
    if (oriEffectConfig_.preProcess.defaultScenes.size() != 1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG13]: pre-defaultScene is not one!");
    }

    if (oriEffectConfig_.preProcess.normalScenes.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG14]: pre-normalScene is none!");
    }

    if (oriEffectConfig_.postProcess.defaultScenes.size() != 1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG15]: post-defaultScene is not one!");
    }
    if (oriEffectConfig_.postProcess.normalScenes.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG16]: post-normalScene is none!");
    }

    // update maxExtraSceneNum

    // Update duplicate defined modes, devices, and unsupported effect chain.
    UpdateAvailableAEConfig(oriEffectConfig_);

    UpdateDuplicateProcessNew(availableLayout, supportedEffectConfig_.preProcessNew);
    UpdateDuplicateProcessNew(availableLayout, supportedEffectConfig_.postProcessNew);

    for (auto &stream : supportedEffectConfig_.preProcessNew.stream) {
        for (auto &streamMode : stream.streamEffectMode) {
            for (auto &device : streamMode.devicePort) {
                UpdateSupportedEffectProperty(device, device2EnhancePropertySet_);
            }
        }
    }
    for (auto &stream : supportedEffectConfig_.postProcessNew.stream) {
        for (auto &streamMode : stream.streamEffectMode) {
            for (auto &device : streamMode.devicePort) {
                UpdateSupportedEffectProperty(device, device2EffectPropertySet_);
            }
        }
    }
}

void AudioEffectService::SetMasterSinkAvailable()
{
    isMasterSinkAvailable_ = true;
}

void AudioEffectService::SetEffectChainManagerAvailable()
{
    isEffectChainManagerAvailable_ = true;
}

bool AudioEffectService::CanLoadEffectSinks()
{
    return (isMasterSinkAvailable_ && isEffectChainManagerAvailable_);
}

template <typename T>
void AddKeyValueIntoMap(std::unordered_map<T, std::string> &map, std::string &key, std::string &value)
{
    if (map.count(key)) { // if the key already register in map
        return;
    }
    map[key] = value;
}

void AudioEffectService::ConstructEffectChainMode(StreamEffectMode &mode, std::string sceneType,
                                                  EffectChainManagerParam &effectChainMgrParam)
{
    std::unordered_map<std::string, std::string> &map = effectChainMgrParam.sceneTypeToChainNameMap;
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();

    std::string sceneMode = mode.mode;
    std::string key;
    std::string defaultChain;
    bool defaultFlag = false;
    for (auto &device : mode.devicePort) {
        if (device.type == "DEVICE_TYPE_DEFAULT") {
            defaultFlag = true;
            defaultChain = device.chain;
        } else {
            key = sceneType + "_&_" + sceneMode + "_&_" + device.type;
            AddKeyValueIntoMap(map, key, device.chain);
        }
        ConstructDefaultEffectProperty(device.chain, effectChainMgrParam.effectDefaultProperty);
    }
    if (defaultFlag) {
        for (const auto &deviceType : supportDeviceType) {
            key = sceneType + "_&_" + sceneMode + "_&_" + deviceType.second;
            AddKeyValueIntoMap(map, key, defaultChain);
        }
    }
}

void AudioEffectService::ConstructDefaultEffectProperty(const std::string &chainName,
    std::unordered_map<std::string, std::string> &effectDefaultProperty)
{
    auto effectChain = std::find_if(supportedEffectConfig_.effectChains.begin(),
        supportedEffectConfig_.effectChains.end(),
        [&chainName](const EffectChain& x) {
            return x.name == chainName;
        });
    if (effectChain == supportedEffectConfig_.effectChains.end()) {
        return;
    }
    for (const auto &effectName : effectChain->apply) {
        auto effectIter = std::find_if(availableEffects_.begin(), availableEffects_.end(),
            [&effectName](const Effect& effect) {
            return effect.name == effectName;
        });
        if (effectIter == availableEffects_.end()) {
            continue;
        }
        // if 0 property, no need to set default
        if (effectIter->effectProperty.size() > 0) {
            // first assign, and no need to assign twice
            if (!effectDefaultProperty.count(effectIter->name)) {
                // only first property is default set
                effectDefaultProperty[effectIter->name] = effectIter->effectProperty[0];
                AUDIO_INFO_LOG("effect %{public}s defaultProperty is %{public}s",
                    effectIter->name.c_str(), effectIter->effectProperty[0].c_str());
            }
        }
    }
}

void AudioEffectService::ConstructEffectChainManagerParam(EffectChainManagerParam &effectChainMgrParam)
{
    effectChainMgrParam.maxExtraNum = oriEffectConfig_.postProcess.maxExtSceneNum;
    std::string sceneType;

    for (auto &scene: supportedEffectConfig_.postProcessNew.stream) {
        sceneType = scene.scene;
        if (scene.priority == PRIOR_SCENE) {
            effectChainMgrParam.priorSceneList.push_back(sceneType);
        }
        if (scene.priority == DEFAULT_SCENE) {
            effectChainMgrParam.defaultSceneName = sceneType;
        }
        for (auto &mode: scene.streamEffectMode) {
            ConstructEffectChainMode(mode, sceneType, effectChainMgrParam);
        }
    }
    AUDIO_INFO_LOG("Constructed SceneTypeAndModeToEffectChainNameMap at policy, size is %{public}d",
        (int32_t)effectChainMgrParam.sceneTypeToChainNameMap.size());
}

void AudioEffectService::ConstructEnhanceChainManagerParam(EffectChainManagerParam &enhanceChainMgrParam)
{
    std::unordered_map<std::string, std::string> &map = enhanceChainMgrParam.sceneTypeToChainNameMap;
    std::unordered_map<std::string, std::string> &enhanceDefaultProperty = enhanceChainMgrParam.effectDefaultProperty;
    enhanceChainMgrParam.maxExtraNum = oriEffectConfig_.preProcess.maxExtSceneNum;

    std::string sceneType;
    std::string sceneMode;
    std::string key;

    for (auto &scene: supportedEffectConfig_.preProcessNew.stream) {
        sceneType = scene.scene;
        if (scene.priority == PRIOR_SCENE) {
            enhanceChainMgrParam.priorSceneList.push_back(sceneType);
        }
        if (scene.priority == DEFAULT_SCENE) {
            enhanceChainMgrParam.defaultSceneName = sceneType;
        }

        for (auto &mode: scene.streamEffectMode) {
            sceneMode = mode.mode;
            for (auto &device: mode.devicePort) {
                key = sceneType + "_&_" + sceneMode;
                AddKeyValueIntoMap(map, key, device.chain);
                ConstructDefaultEffectProperty(device.chain, enhanceDefaultProperty);
            }
        }
    }
    AUDIO_INFO_LOG("Constructed SceneTypeAndModeToEnhanceChainNameMap at policy, size is %{public}d",
        (int32_t)map.size());
}

int32_t AudioEffectService::AddSupportedPropertyByDeviceInner(const DeviceType& deviceType,
    std::set<std::pair<std::string, std::string>> &mergedSet,
    const std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> &device2PropertySet)
{
    const std::unordered_map<DeviceType, std::string> &supportDeviceType = GetSupportedDeviceType();
    auto deviceIter = supportDeviceType.find(deviceType);
    if (deviceIter == supportDeviceType.end()) {
        AUDIO_ERR_LOG("device not supported.");
        return -1;
    }
    auto deviceStr = deviceType == DEVICE_TYPE_INVALID ? "DEVICE_TYPE_DEFAULT" : deviceIter->second;
    auto propertySetIter = device2PropertySet.find(deviceStr);
    if (propertySetIter != device2PropertySet.end()) {
        mergedSet.insert(propertySetIter->second.begin(), propertySetIter->second.end());
    }
    return AUDIO_OK;
}

int32_t AudioEffectService::AddSupportedAudioEffectPropertyByDevice(const DeviceType &deviceType,
    std::set<std::pair<std::string, std::string>> &mergedSet)
{
    return AddSupportedPropertyByDeviceInner(deviceType, mergedSet, device2EffectPropertySet_);
}

int32_t AudioEffectService::AddSupportedAudioEnhancePropertyByDevice(const DeviceType &deviceType,
    std::set<std::pair<std::string, std::string>> &mergedSet)
{
    return AddSupportedPropertyByDeviceInner(deviceType, mergedSet, device2EnhancePropertySet_);
}
} // namespce AudioStandard
} // namespace OHOS
