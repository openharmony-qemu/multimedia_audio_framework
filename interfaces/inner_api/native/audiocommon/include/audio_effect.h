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
constexpr int32_t AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT = 200;
constexpr int32_t AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT = 1;
constexpr int32_t AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT = 1;
constexpr int32_t AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT = 1;
constexpr int32_t AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT = 64; // max conf for sceneType + effectMode + deviceType
constexpr int32_t AUDIO_EFFECT_CHAIN_COUNT_UPPER_LIMIT = 32; // max num of effectChain
constexpr int32_t AUDIO_EFFECT_COUNT_PER_CHAIN_UPPER_LIMIT = 16; // max num of effect per effectChain
constexpr int32_t AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT = 7; // max num of effect prior scene
constexpr int32_t AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT = 20; // max num of property

constexpr int32_t HDI_EFFECT_NUM = 2;
constexpr int32_t HDI_SET_PATAM = 6;

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
};

enum AudioSpatialDeviceType {
    EARPHONE_TYPE_NONE = 0,
    EARPHONE_TYPE_INEAR,
    EARPHONE_TYPE_HALF_INEAR,
    EARPHONE_TYPE_HEADPHONE,
    EARPHONE_TYPE_GLASSES,
    EARPHONE_TYPE_OTHERS,
};

struct AudioSpatialDeviceState {
    std::string address;
    bool isSpatializationSupported;
    bool isHeadTrackingSupported;
    AudioSpatialDeviceType spatialDeviceType;
};

struct Library {
    std::string name;
    std::string path;
};

struct Effect {
    std::string name;
    std::string libraryName;
    std::vector<std::string> effectProperty;
};

struct EffectChain {
    std::string name;
    std::vector<std::string> apply;
    std::string label = "";
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

struct EffectChainManagerParam {
    uint32_t maxExtraNum = 0;
    std::string defaultSceneName;
    std::vector<std::string> priorSceneList;
    std::unordered_map<std::string, std::string> sceneTypeToChainNameMap;
    std::unordered_map<std::string, std::string> effectDefaultProperty;
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

struct SupportedEffectConfig {
    std::vector<EffectChain> effectChains;
    ProcessNew preProcessNew;
    ProcessNew postProcessNew;
    std::vector<SceneMappingItem> postProcessSceneMap;
};


/**
* Enumerates the audio scene effect type.
*/
enum AudioEffectScene {
    SCENE_OTHERS = 0,
    SCENE_MUSIC = 1,
    SCENE_MOVIE = 2,
    SCENE_GAME = 3,
    SCENE_SPEECH = 4,
    SCENE_RING = 5,
    SCENE_VOIP_DOWN = 6,
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

const std::unordered_map<AudioEffectScene, std::string> AUDIO_SUPPORTED_SCENE_TYPES {
    {SCENE_OTHERS, "SCENE_OTHERS"},
    {SCENE_MUSIC, "SCENE_MUSIC"},
    {SCENE_MOVIE, "SCENE_MOVIE"},
    {SCENE_GAME, "SCENE_GAME"},
    {SCENE_SPEECH, "SCENE_SPEECH"},
    {SCENE_RING, "SCENE_RING"},
    {SCENE_VOIP_DOWN, "SCENE_VOIP_DOWN"},
};

const std::unordered_map<AudioEnhanceScene, std::string> AUDIO_ENHANCE_SUPPORTED_SCENE_TYPES {
    {SCENE_VOIP_UP, "SCENE_VOIP_UP"},
    {SCENE_RECORD, "SCENE_RECORD"},
    {SCENE_ASR, "SCENE_ASR"},
    {SCENE_PRE_ENHANCE, "SCENE_PRE_ENHANCE"},
    {SCENE_VOICE_MESSAGE, "SCENE_VOICE_MESSAGE"},
};

const std::unordered_map<AudioEffectMode, std::string> AUDIO_SUPPORTED_SCENE_MODES {
    {EFFECT_NONE, "EFFECT_NONE"},
    {EFFECT_DEFAULT, "EFFECT_DEFAULT"},
};

const std::unordered_map<DeviceType, std::string> SUPPORTED_DEVICE_TYPE {
    {DEVICE_TYPE_NONE, "DEVICE_TYPE_NONE"},
    {DEVICE_TYPE_INVALID, "DEVICE_TYPE_INVALID"},
    {DEVICE_TYPE_EARPIECE, "DEVICE_TYPE_EARPIECE"},
    {DEVICE_TYPE_SPEAKER, "DEVICE_TYPE_SPEAKER"},
    {DEVICE_TYPE_WIRED_HEADSET, "DEVICE_TYPE_WIRED_HEADSET"},
    {DEVICE_TYPE_WIRED_HEADPHONES, "DEVICE_TYPE_WIRED_HEADPHONES"},
    {DEVICE_TYPE_BLUETOOTH_SCO, "DEVICE_TYPE_BLUETOOTH_SCO"},
    {DEVICE_TYPE_BLUETOOTH_A2DP, "DEVICE_TYPE_BLUETOOTH_A2DP"},
    {DEVICE_TYPE_BLUETOOTH_A2DP_IN, "DEVICE_TYPE_BLUETOOTH_A2DP_IN"},
    {DEVICE_TYPE_MIC, "DEVICE_TYPE_MIC"},
    {DEVICE_TYPE_WAKEUP, "DEVICE_TYPE_WAKEUP"},
    {DEVICE_TYPE_USB_HEADSET, "DEVICE_TYPE_USB_HEADSET"},
    {DEVICE_TYPE_USB_ARM_HEADSET, "DEVICE_TYPE_USB_ARM_HEADSET"},
    {DEVICE_TYPE_DP, "DEVICE_TYPE_DP"},
    {DEVICE_TYPE_FILE_SINK, "DEVICE_TYPE_FILE_SINK"},
    {DEVICE_TYPE_FILE_SOURCE, "DEVICE_TYPE_FILE_SOURCE"},
    {DEVICE_TYPE_EXTERN_CABLE, "DEVICE_TYPE_EXTERN_CABLE"},
    {DEVICE_TYPE_DEFAULT, "DEVICE_TYPE_DEFAULT"},
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
    void Unmarshalling(Parcel &parcel)
    {
        name = parcel.ReadString();
        category = parcel.ReadString();
        flag = static_cast<EffectFlag>(parcel.ReadInt32());
    };
};

struct AudioEffectPropertyArrayV3 {
    std::vector<AudioEffectPropertyV3> property;
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
    void Unmarshalling(Parcel &parcel)
    {
        enhanceClass = parcel.ReadString();
        enhanceProp = parcel.ReadString();
    }
};

struct AudioEnhancePropertyArray {
    std::vector<AudioEnhanceProperty> property;
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
    void Unmarshalling(Parcel &parcel)
    {
        effectClass = parcel.ReadString();
        effectProp = parcel.ReadString();
    }
};

struct AudioEffectPropertyArray {
    std::vector<AudioEffectProperty> property;
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

struct AudioEffectLibrary {
    uint32_t version;
    const char *name;
    const char *implementor;
    bool (*checkEffect) (const AudioEffectDescriptor descriptor);
    int32_t (*createEffect) (const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);
    int32_t (*releaseEffect) (AudioEffectHandle handle);
};

struct AudioEffectLibEntry {
    AudioEffectLibrary *audioEffectLibHandle;
    std::string libraryName;
    std::vector<std::string> effectName;
};

struct AudioSpatializationState {
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
};

struct ConverterConfig {
    std::string version;
    Library library;
    uint64_t outChannelLayout = 0;
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
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FRAMEWORK_AUDIO_EFFECT_H