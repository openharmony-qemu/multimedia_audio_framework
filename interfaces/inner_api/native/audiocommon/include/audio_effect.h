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

#ifndef AUDIO_FRAMEWORK_AUDIO_EFFECT_H
#define AUDIO_FRAMEWORK_AUDIO_EFFECT_H

#include <cassert>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "audio_info.h"

#define AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR "AELI"
#define EFFECT_STRING_LEN_MAX 64

namespace OHOS {
namespace AudioStandard {
// audio effect manager info
constexpr int32_t AUDIO_EFFECT_COUNT_UPPER_LIMIT = 20;
constexpr uint32_t SEND_HDI_COMMAND_LEN = 20;
constexpr int32_t AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT = 7;
constexpr int32_t AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT = 64;
constexpr int32_t AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT = 20;

enum HdiSetParamCommandCode {
    HDI_INIT = 0,
    HDI_BYPASS = 1,
    HDI_HEAD_MODE = 2,
    HDI_ROOM_MODE = 3,
    HDI_BLUETOOTH_MODE = 4,
    HDI_DESTROY = 5,
    HDI_UPDATE_SPATIAL_DEVICE_TYPE = 6,
    HDI_VOLUME = 7,
    HDI_ROTATION = 8,
    HDI_EXTRA_SCENE_TYPE = 9,
    HDI_SPATIALIZATION_SCENE_TYPE = 10,
    HDI_STREAM_USAGE = 11,
    HDI_FOLD_STATE = 12,
    HDI_LID_STATE = 13,
    HDI_QUERY_CHANNELLAYOUT = 14,
    HDI_ABS_VOLUME_STATE = 15,
};

enum AudioSpatialDeviceType {
    EARPHONE_TYPE_NONE = 0,
    EARPHONE_TYPE_INEAR,
    EARPHONE_TYPE_HALF_INEAR,
    EARPHONE_TYPE_HEADPHONE,
    EARPHONE_TYPE_GLASSES,
    EARPHONE_TYPE_OTHERS,
};

enum FoldState : uint32_t {
    FOLD_STATE_EXPAND = 1,
    FOLD_STATE_CLOSE = 2,
    FOLD_STATE_MIDDLE = 3,
};

struct AudioSpatialDeviceState : public Parcelable {
    std::string address;
    bool isSpatializationSupported;
    bool isHeadTrackingSupported;
    AudioSpatialDeviceType spatialDeviceType;

    AudioSpatialDeviceState() = default;
    AudioSpatialDeviceState(const std::string &address, bool isSpatializationSupported,
        bool isHeadTrackingSupported, AudioSpatialDeviceType spatialDeviceType)
        : address(address), isSpatializationSupported(isSpatializationSupported),
        isHeadTrackingSupported(isHeadTrackingSupported), spatialDeviceType(spatialDeviceType)
    {
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(address) &&
            parcel.WriteBool(isSpatializationSupported) &&
            parcel.WriteBool(isHeadTrackingSupported) &&
            parcel.WriteInt32(spatialDeviceType);
    }

    static AudioSpatialDeviceState *Unmarshalling(Parcel &parcel)
    {
        auto deviceState = new(std::nothrow) AudioSpatialDeviceState();
        if (deviceState == nullptr) {
            return nullptr;
        }
        deviceState->address = parcel.ReadString();
        deviceState->isSpatializationSupported = parcel.ReadBool();
        deviceState->isHeadTrackingSupported = parcel.ReadBool();
        deviceState->spatialDeviceType = static_cast<AudioSpatialDeviceType>(parcel.ReadInt32());
        return deviceState;
    }
};

struct Library : public Parcelable {
    std::string name;
    std::string path;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(name) && parcel.WriteString(path);
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        name = parcel.ReadString();
        path = parcel.ReadString();
    }

    static Library *Unmarshalling(Parcel &parcel)
    {
        auto library = new(std::nothrow) Library();
        if (library == nullptr) {
            return nullptr;
        }
        library->UnmarshallingSelf(parcel);
        return library;
    }
};

struct Effect : public Parcelable {
    std::string name;
    std::string libraryName;
    std::vector<std::string> effectProperty;
    static constexpr int32_t MAX_EFFECT_PROPERTY_SIZE = 1000;

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteString(name);
        parcel.WriteString(libraryName);
        int32_t size = static_cast<int32_t>(effectProperty.size());
        parcel.WriteInt32(size);
        for (auto &property : effectProperty) {
            parcel.WriteString(property);
        }
        return true;
    }

    static Effect *Unmarshalling(Parcel &parcel)
    {
        auto effect = new(std::nothrow) Effect();
        if (effect == nullptr) {
            return nullptr;
        }

        effect->name = parcel.ReadString();
        effect->libraryName = parcel.ReadString();
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > MAX_EFFECT_PROPERTY_SIZE) {
            delete effect;
            return nullptr;
        }
        for (int32_t i = 0; i < size; ++i) {
            effect->effectProperty.push_back(parcel.ReadString());
        }
        return effect;
    }
};

struct EffectChain : public Parcelable {
    std::string name;
    std::vector<std::string> apply;
    std::string label = "";
    static constexpr int32_t AUDIO_EFFECT_COUNT_PER_CHAIN_UPPER_LIMIT = 16;

    EffectChain() = default;
    EffectChain(std::string sName, std::vector<std::string> applyVec, std::string sLabel)
        : name(sName), apply(applyVec), label(sLabel)
    {
    }

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteString(name);
        int32_t size = static_cast<int32_t>(apply.size());
        parcel.WriteInt32(size);
        for (auto &effect : apply) {
            parcel.WriteString(effect);
        }
        parcel.WriteString(label);
        return true;
    }

    static EffectChain *Unmarshalling(Parcel &parcel)
    {
        auto effectChain = new(std::nothrow) EffectChain();
        if (effectChain == nullptr) {
            return nullptr;
        }
        effectChain->name = parcel.ReadString();
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_COUNT_PER_CHAIN_UPPER_LIMIT) {
            delete effectChain;
            return nullptr;
        }
        for (int32_t i = 0; i < size; ++i) {
            effectChain->apply.push_back(parcel.ReadString());
        }
        effectChain->label = parcel.ReadString();
        return effectChain;
    }
};

struct Device {
    std::string type;
    std::string chain;
};

struct PreStreamScene {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
};

struct PostStreamScene {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
};

struct SceneMappingItem {
    std::string name;
    std::string sceneType;
};

struct PreProcessConfig {
    uint32_t maxExtSceneNum;
    std::vector<PreStreamScene> defaultScenes;
    std::vector<PreStreamScene> priorScenes;
    std::vector<PreStreamScene> normalScenes;
};

struct PostProcessConfig {
    uint32_t maxExtSceneNum;
    std::vector<PostStreamScene> defaultScenes;
    std::vector<PostStreamScene> priorScenes;
    std::vector<PostStreamScene> normalScenes;
    std::vector<SceneMappingItem> sceneMap;
};

struct OriginalEffectConfig {
    std::string version;
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<EffectChain> effectChains;
    PreProcessConfig preProcess;
    PostProcessConfig postProcess;
};

struct EffectChainManagerParam : public Parcelable {
    uint32_t maxExtraNum = 0;
    std::string defaultSceneName;
    std::vector<std::string> priorSceneList;
    std::unordered_map<std::string, std::string> sceneTypeToChainNameMap;
    std::unordered_map<std::string, std::string> effectDefaultProperty;

    EffectChainManagerParam() = default;
    EffectChainManagerParam(uint32_t maxExtraNum, std::string defaultSceneName, std::vector<std::string> priorSceneList,
        std::unordered_map<std::string, std::string> sceneTypeToChainNameMap,
        std::unordered_map<std::string, std::string> effectDefaultProperty)
        : maxExtraNum(maxExtraNum), defaultSceneName(defaultSceneName), priorSceneList(priorSceneList),
        sceneTypeToChainNameMap(sceneTypeToChainNameMap), effectDefaultProperty(effectDefaultProperty)
    {
    }

    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteUint32(maxExtraNum);
        parcel.WriteString(defaultSceneName);
        int32_t size = static_cast<int32_t>(priorSceneList.size());
        parcel.WriteInt32(size);
        for (auto &scene : priorSceneList) {
            parcel.WriteString(scene);
        }
        size = static_cast<int32_t>(sceneTypeToChainNameMap.size());
        parcel.WriteInt32(size);
        for (const auto &[scene, chain] : sceneTypeToChainNameMap) {
            parcel.WriteString(scene);
            parcel.WriteString(chain);
        }
        size = static_cast<int32_t>(effectDefaultProperty.size());
        parcel.WriteInt32(size);
        for (const auto &[effect, property] : effectDefaultProperty) {
            parcel.WriteString(effect);
            parcel.WriteString(property);
        }
        return true;
    }

    static EffectChainManagerParam *Unmarshalling(Parcel &parcel)
    {
        auto param = new(std::nothrow) EffectChainManagerParam();
        if (param == nullptr) {
            return nullptr;
        }
        param->maxExtraNum = parcel.ReadUint32();
        param->defaultSceneName = parcel.ReadString();
        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT) {
            delete param;
            return nullptr;
        }
        for (int32_t i = 0; i < size; ++i) {
            param->priorSceneList.push_back(parcel.ReadString());
        }
        size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT) {
            delete param;
            return nullptr;
        }
        for (int32_t i = 0; i < size; ++i) {
            std::string scene = parcel.ReadString();
            std::string chain = parcel.ReadString();
            param->sceneTypeToChainNameMap[scene] = chain;
        }
        size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT) {
            delete param;
            return nullptr;
        }
        for (int32_t i = 0; i < size; ++i) {
            std::string effect = parcel.ReadString();
            std::string property = parcel.ReadString();
            param->effectDefaultProperty[effect] = property;
        }
        return param;
    }
};

struct StreamEffectMode {
    std::string mode;
    std::vector<Device> devicePort;
};

enum ScenePriority {
    DEFAULT_SCENE = 0,
    PRIOR_SCENE = 1,
    NORMAL_SCENE = 2
};

struct Stream {
    ScenePriority priority;
    std::string scene;
    std::vector<StreamEffectMode> streamEffectMode;
};

struct ProcessNew {
    std::vector<Stream> stream;
};

struct SupportedEffectConfig : public Parcelable {
    std::vector<EffectChain> effectChains;
    ProcessNew preProcessNew;
    ProcessNew postProcessNew;
    std::vector<SceneMappingItem> postProcessSceneMap;
    static constexpr uint32_t POST_PROCESS_SCENE_MAP_MAX_SIZE = 1000;
    static constexpr uint32_t STREAM_MAX_SIZE = 1000;
    static constexpr uint32_t STREAM_EFFECT_MODE_MAX_SIZE = 1000;

    bool MarshallingStream(Parcel &parcel, const Stream &stream) const
    {
        parcel.WriteInt32(stream.priority);
        parcel.WriteString(stream.scene);
        uint32_t count = static_cast<uint32_t>(stream.streamEffectMode.size());
        parcel.WriteUint32(count);
        for (const auto &item : stream.streamEffectMode) {
            parcel.WriteString(item.mode);
            uint32_t deviceCount = static_cast<uint32_t>(item.devicePort.size());
            parcel.WriteUint32(deviceCount);
            for (const auto &device : item.devicePort) {
                parcel.WriteString(device.type);
                parcel.WriteString(device.chain);
            }
        }
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        uint32_t countPre = static_cast<uint32_t>(preProcessNew.stream.size());
        parcel.WriteUint32(countPre);
        for (const auto &item : preProcessNew.stream) {
            MarshallingStream(parcel, item);
        }

        uint32_t countPost = static_cast<uint32_t>(postProcessNew.stream.size());
        parcel.WriteUint32(countPost);
        for (const auto &item : postProcessNew.stream) {
            MarshallingStream(parcel, item);
        }

        uint32_t countPostMap = static_cast<uint32_t>(postProcessSceneMap.size());
        parcel.WriteUint32(countPostMap);
        for (const auto &item : postProcessSceneMap) {
            parcel.WriteString(item.name);
            parcel.WriteString(item.sceneType);
        }
        return true;
    }

    static bool UnmarshallingStream(Parcel &parcel, Stream &stream)
    {
        stream.priority = static_cast<ScenePriority>(parcel.ReadInt32());
        stream.scene = parcel.ReadString();
        uint32_t count = parcel.ReadUint32();
        if (count > STREAM_MAX_SIZE) {
            return false;
        }
        for (uint32_t i = 0; i < count; ++i) {
            StreamEffectMode mode;
            mode.mode = parcel.ReadString();
            uint32_t deviceCount = parcel.ReadUint32();
            if (deviceCount > STREAM_EFFECT_MODE_MAX_SIZE) {
                return false;
            }
            for (uint32_t j = 0; j < deviceCount; ++j) {
                Device device;
                device.type = parcel.ReadString();
                device.chain = parcel.ReadString();
                mode.devicePort.push_back(device);
            }
            stream.streamEffectMode.push_back(mode);
        }
        return true;
    }

    static SupportedEffectConfig *Unmarshalling(Parcel &parcel)
    {
        auto config = new(std::nothrow) SupportedEffectConfig();
        if (config == nullptr) {
            return nullptr;
        }
        uint32_t countPre = parcel.ReadUint32();
        if (countPre > AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            delete config;
            return nullptr;
        }
        for (uint32_t i = 0; i < countPre; ++i) {
            Stream stream = {};
            if (!UnmarshallingStream(parcel, stream)) {
                delete config;
                return nullptr;
            }
            config->preProcessNew.stream.push_back(stream);
        }

        uint32_t countPost = parcel.ReadUint32();
        if (countPost > AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            delete config;
            return nullptr;
        }
        for (uint32_t i = 0; i < countPost; ++i) {
            Stream stream = {};
            if (!UnmarshallingStream(parcel, stream)) {
                delete config;
                return nullptr;
            }
            config->postProcessNew.stream.push_back(stream);
        }
        uint32_t countPostMap = parcel.ReadUint32();
        if (countPostMap > POST_PROCESS_SCENE_MAP_MAX_SIZE) {
            delete config;
            return nullptr;
        }
        for (uint32_t i = 0; i < countPostMap; ++i) {
            SceneMappingItem item;
            item.name = parcel.ReadString();
            item.sceneType = parcel.ReadString();
            config->postProcessSceneMap.push_back(item);
        }
        return config;
    }
};

/**
* Enumerates the audio scene effect type.
*/
enum AudioEffectScene {
    SCENE_INITIAL = -1,
    SCENE_OTHERS = 0,
    SCENE_MUSIC = 1,
    SCENE_MOVIE = 2,
    SCENE_GAME = 3,
    SCENE_SPEECH = 4,
    SCENE_RING = 5,
    SCENE_VOIP_DOWN = 6,
    SCENE_COLLABORATIVE = 7,
};

/**
* Enumerates the audio enhance scene effect type.
*/
enum AudioEnhanceScene {
    SCENE_VOIP_UP = 0,
    SCENE_RECORD = 1,
    SCENE_PRE_ENHANCE = 2,
    SCENE_ASR = 4,
    SCENE_VOICE_MESSAGE = 5,
    SCENE_RECOGNITION = 6,
    SCENE_NONE = 7,
};

/**
* Enumerates the audio scene effct mode.
*/
enum AudioEffectMode {
    EFFECT_NONE = 0,
    EFFECT_DEFAULT = 1
};

/**
* Enumerates the audio enhance scene effct mode.
*/
enum AudioEnhanceMode {
    ENHANCE_NONE = 0,
    ENHANCE_DEFAULT = 1
};

struct AudioSceneEffectInfo {
    std::vector<AudioEffectMode> mode;
};

enum EffectFlag { RENDER_EFFECT_FLAG = 0, CAPTURE_EFFECT_FLAG = 1};

struct AudioEffectPropertyV3 {
    std::string name;
    std::string category;
    EffectFlag flag;
    friend bool operator==(const AudioEffectPropertyV3 &lhs, const AudioEffectPropertyV3 &rhs)
    {
        return (lhs.category == rhs.category && lhs.name == rhs.name && lhs.flag == rhs.flag);
    };
    friend bool operator<(const AudioEffectPropertyV3 &lhs, const AudioEffectPropertyV3 &rhs)
    {
        return ((lhs.name == rhs.name) || (lhs.name == rhs.name && lhs.category < rhs.category)
            || (lhs.name == rhs.name && lhs.category == rhs.category && lhs.flag < rhs.flag));
    };
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteString(name)&&
            parcel.WriteString(category)&&
            parcel.WriteInt32(flag);
    };
    void UnmarshallingSelf(Parcel &parcel)
    {
        name = parcel.ReadString();
        category = parcel.ReadString();
        flag = static_cast<EffectFlag>(parcel.ReadInt32());
    };
};

struct AudioEffectPropertyArrayV3 : public Parcelable {
    std::vector<AudioEffectPropertyV3> property;

    bool Marshalling(Parcel &parcel) const override
    {
        int32_t size = static_cast<int32_t>(property.size());
        parcel.WriteInt32(size);
        for (const auto &item : property) {
            if (!item.Marshalling(parcel)) {
                return false;
            }
        }
        return true;
    }

    static AudioEffectPropertyArrayV3 *Unmarshalling(Parcel &parcel)
    {
        auto propertyArray = new(std::nothrow) AudioEffectPropertyArrayV3();
        if (propertyArray == nullptr) {
            return nullptr;
        }

        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            delete propertyArray;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            AudioEffectPropertyV3 property;
            property.UnmarshallingSelf(parcel);
            propertyArray->property.push_back(property);
        }
        return propertyArray;
    }
};

struct AudioEnhanceProperty {
    std::string enhanceClass;
    std::string enhanceProp;
    friend bool operator==(const AudioEnhanceProperty &lhs, const AudioEnhanceProperty &rhs)
    {
        return lhs.enhanceClass == rhs.enhanceClass && lhs.enhanceProp == rhs.enhanceProp;
    }
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteString(enhanceClass)&&
            parcel.WriteString(enhanceProp);
    }
    void UnmarshallingSelf(Parcel &parcel)
    {
        enhanceClass = parcel.ReadString();
        enhanceProp = parcel.ReadString();
    }
};

struct AudioEnhancePropertyArray : public Parcelable {
    std::vector<AudioEnhanceProperty> property;

    bool Marshalling(Parcel &parcel) const override
    {
        int32_t size = static_cast<int32_t>(property.size());
        parcel.WriteInt32(size);
        for (const auto &item : property) {
            if (!item.Marshalling(parcel)) {
                return false;
            }
        }
        return true;
    }

    static AudioEnhancePropertyArray *Unmarshalling(Parcel &parcel)
    {
        auto propertyArray = new(std::nothrow) AudioEnhancePropertyArray();
        if (propertyArray == nullptr) {
            return nullptr;
        }

        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            delete propertyArray;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            AudioEnhanceProperty property;
            property.UnmarshallingSelf(parcel);
            propertyArray->property.push_back(property);
        }
        return propertyArray;
    }
};

struct AudioEffectProperty {
    std::string effectClass;
    std::string effectProp;
    friend bool operator==(const AudioEffectProperty &lhs, const AudioEffectProperty &rhs)
    {
        return lhs.effectClass == rhs.effectClass && lhs.effectProp == rhs.effectProp;
    }
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteString(effectClass)&&
            parcel.WriteString(effectProp);
    }
    void UnmarshallingSelf(Parcel &parcel)
    {
        effectClass = parcel.ReadString();
        effectProp = parcel.ReadString();
    }
};

struct AudioEffectPropertyArray : public Parcelable {
    std::vector<AudioEffectProperty> property;

    bool Marshalling(Parcel &parcel) const override
    {
        int32_t size = static_cast<int32_t>(property.size());
        parcel.WriteInt32(size);
        for (const auto &item : property) {
            if (!item.Marshalling(parcel)) {
                return false;
            }
        }
        return true;
    }

    static AudioEffectPropertyArray *Unmarshalling(Parcel &parcel)
    {
        auto propertyArray = new(std::nothrow) AudioEffectPropertyArray();
        if (propertyArray == nullptr) {
            return nullptr;
        }

        int32_t size = parcel.ReadInt32();
        if (size < 0 || size > AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            delete propertyArray;
            return nullptr;
        }
        for (int32_t i = 0; i < size; i++) {
            AudioEffectProperty property;
            property.UnmarshallingSelf(parcel);
            propertyArray->property.push_back(property);
        }
        return propertyArray;
    }
};

enum AudioEffectChainSetParamIndex {
    COMMAND_CODE_INDEX = 0,
    SCENE_TYPE_INDEX = 1,
    EFFECT_MODE_INDEX = 2,
    ROTATION_INDEX = 3,
    VOLUME_INDEX = 4,
    EXTRA_SCENE_TYPE_INDEX = 5,
    SPATIAL_DEVICE_TYPE_INDEX = 6,
    SPATIALIZATION_SCENE_TYPE_INDEX = 7,
    SPATIALIZATION_ENABLED_INDEX = 8,
    STREAM_USAGE_INDEX = 9,
    FOLD_STATE_INDEX = 10,
    LID_STATE_INDEX = 11,
    LOUDNESS_GAIN_INDEX = 12,
    ABS_VOLUME_STATE = 13,
    MAX_PARAM_INDEX,
};

enum AudioEffectCommandCode {
    EFFECT_CMD_INIT = 0,
    EFFECT_CMD_SET_CONFIG = 1,
    EFFECT_CMD_ENABLE = 2,
    EFFECT_CMD_DISABLE = 3,
    EFFECT_CMD_SET_PARAM = 4,
    EFFECT_CMD_GET_PARAM = 5,
    EFFECT_CMD_GET_CONFIG = 6,
    EFFECT_CMD_SET_IMU = 7,
    EFFECT_CMD_SET_PROPERTY = 8
};

enum AudioEffectParamSetCode {
    EFFECT_SET_BYPASS = 1,
    EFFECT_SET_PARAM = 2,
};

enum AudioDataFormat {
    DATA_FORMAT_S16 = SAMPLE_S16LE,
    DATA_FORMAT_S24 = SAMPLE_S24LE,
    DATA_FORMAT_S32 = SAMPLE_S32LE,
    DATA_FORMAT_F32 = SAMPLE_F32LE,
};

struct AudioEffectParam {
    int32_t status;
    uint32_t paramSize;
    uint32_t valueSize;
    int32_t data[];
};

struct AudioBuffer {
    size_t frameLength;
    union {
        void*     raw;
        float*    f32;
        int32_t*  s32;
        int16_t*  s16;
        uint8_t*  u8;
    };
    void *metaData = nullptr;
};

struct AudioBufferConfig {
    uint32_t samplingRate;
    uint32_t channels;
    uint8_t format;
    uint64_t channelLayout;
    AudioEncodingType encoding;
};

struct AudioEffectConfig {
    AudioBufferConfig inputCfg;
    AudioBufferConfig outputCfg;
};

struct AudioEffectTransInfo {
    uint32_t size;
    void *data;
};

struct AudioEffectDescriptor {
    std::string libraryName;
    std::string effectName;
};

typedef struct AudioEffectInterface **AudioEffectHandle;

struct AudioEffectInterface {
    int32_t (*process) (AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer);
    int32_t (*command) (AudioEffectHandle self, uint32_t cmdCode,
        AudioEffectTransInfo *cmdInfo, AudioEffectTransInfo *replyInfo);
};

struct AlgoSupportConfig {
    bool isSupport;
    bool isRealTime;
    uint32_t frameLen;
    uint32_t inSampleRate;
    uint32_t inChannels;
    uint32_t inFormat;
    uint32_t outSampleRate;
    uint32_t outChannels;
    uint32_t outFormat;
};

struct AudioEffectLibrary {
    uint32_t version;
    const char *name;
    const char *implementor;
    bool (*checkEffect) (const AudioEffectDescriptor descriptor);
    int32_t (*createEffect) (const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);
    int32_t (*releaseEffect) (AudioEffectHandle handle);
    void (*supportEffect) (AlgoSupportConfig *config);
};

struct AudioEffectLibEntry {
    AudioEffectLibrary *audioEffectLibHandle;
    std::string libraryName;
    std::vector<std::string> effectName;
};

struct AudioSpatializationState : public Parcelable {
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;

    AudioSpatializationState() = default;
    AudioSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
    {
        this->spatializationEnabled = spatializationEnabled;
        this->headTrackingEnabled = headTrackingEnabled;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteBool(spatializationEnabled)
            && parcel.WriteBool(headTrackingEnabled);
    }

    static AudioSpatializationState *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioSpatializationState();
        if (info == nullptr) {
            return nullptr;
        }
        info->spatializationEnabled = parcel.ReadBool();
        info->headTrackingEnabled = parcel.ReadBool();
        return info;
    }
};

struct ConverterConfig : public Parcelable {
    std::string version;
    Library library;
    uint64_t outChannelLayout = 0;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteString(version) &&
            library.Marshalling(parcel) &&
            parcel.WriteUint64(outChannelLayout);
    }

    static ConverterConfig *Unmarshalling(Parcel &parcel)
    {
        auto config = new(std::nothrow) ConverterConfig();
        if (config == nullptr) {
            return nullptr;
        }
        config->version = parcel.ReadString();
        config->library.UnmarshallingSelf(parcel);
        config->outChannelLayout = parcel.ReadUint64();
        return config;
    }
};

enum AudioSpatializationSceneType {
    SPATIALIZATION_SCENE_TYPE_DEFAULT = 0,
    SPATIALIZATION_SCENE_TYPE_MUSIC = 1,
    SPATIALIZATION_SCENE_TYPE_MOVIE = 2,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK = 3,
    SPATIALIZATION_SCENE_TYPE_MAX = SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
};

struct AudioRendererInfoForSpatialization {
    RendererState rendererState;
    std::string deviceMacAddress;
    StreamUsage streamUsage;
};

struct AudioEnhanceParam {
    uint32_t muteInfo;
    uint32_t volumeInfo;
    uint32_t foldState;
    uint32_t powerState;
    const char *preDevice;
    const char *postDevice;
    const char *sceneType;
    const char *preDeviceName;
};

struct AlgoConfig {
    uint32_t frameLength;
    uint32_t sampleRate;
    uint32_t dataFormat;
    uint32_t micNum;
    uint32_t ecNum;
    uint32_t micRefNum;
    uint32_t outNum;
};

enum ProcessClusterOperation {
    NO_NEED_TO_CREATE_PROCESSCLUSTER,
    CREATE_NEW_PROCESSCLUSTER,
    CREATE_DEFAULT_PROCESSCLUSTER,
    USE_DEFAULT_PROCESSCLUSTER,
    USE_NONE_PROCESSCLUSTER,
    CREATE_EXTRA_PROCESSCLUSTER
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FRAMEWORK_AUDIO_EFFECT_H