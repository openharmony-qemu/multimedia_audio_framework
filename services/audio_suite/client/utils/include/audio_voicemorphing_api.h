/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_VOICEMORPHING_API_H
#define AUDIO_VOICEMORPHING_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_VMP_EOK 0
#define AUDIO_VOICEMPH_EOK 0

typedef struct {
    int scratchSize;
    int stateSize;
} AudioVoiceMorphingMemSize;

typedef struct {
    int *dataIn;
    int *dataOut;
    int dataSize;
    int enableFlag;
    int dataFormat;
    int inCh;
    int outCh;
} AudioVoiceMorphingData;

typedef struct {
    bool currentDeviceSupport;
    bool realTimeSupport;
    int frameLen;
    int sampleRate;
    int channel;
    int dataFormat;
} AudioVoiceMorhpingSpec;

typedef enum {
    AUDIO_VOICE_MORPH_CLEAR = 0,
    AUDIO_VOICE_MORPH_THEATRE,
    AUDIO_VOICE_MORPH_CD,
    AUDIO_VOICE_MORPH_RECORDING_STUDIO,
    AUDIO_VOICE_MPH_SEASONED,
    AUDIO_VOICE_MPH_CUTE,
    AUDIO_VOICE_MPH_FEMALE,
    AUDIO_VOICE_MPH_MALE,
    AUDIO_VOICE_MPH_MONSTER,
    AUDIO_VOICE_MPH_TRILL,
    AUDIO_VOICE_MPH_CYBERPUNK,
    AUDIO_VOICE_MPH_WAR,
    AUDIO_VOICE_MPH_MIX,
    AUDIO_VOICE_MPH_SYNTH
} AudioVoiceMorphingType;

extern int AudioVoiceMorphingGetsize(AudioVoiceMorphingMemSize *memSize);
extern int AudioVoiceMorphingInit(char *handle, char *scratchBuf);
extern int AudioVoiceMorphingSetParam(char *handle, AudioVoiceMorphingType type);
extern int AudioVoiceMorphingApply(AudioVoiceMorphingData *data, char *handle, char *scratchBuf);

typedef struct {
    int stateSize;
    int scratchSize;
} AudioVoiceMphMemSize;

typedef struct {
    float *dataIn;
    float *dataOut;
    int inCh;
    int outCh;
} AudioVoiceMphData;

typedef struct {
    bool currentDeviceSupport;
    bool realTimeSupport;
    int frameLen;
    int sampleRate;
    int channel;
    int dataFormat;
} AudioVoiceMphTradSpec;

typedef enum {
    AUDIO_VOICE_MPH_TRAD_SEASONED = 0,
    AUDIO_VOICE_MPH_TRAD_CUTE,
    AUDIO_VOICE_MPH_TRAD_FEMALE,
    AUDIO_VOICE_MPH_TRAD_MALE,
    AUDIO_VOICE_MPH_TRAD_MONSTER,
    AUDIO_VOICE_MPH_TRAD_ROBOTS,
    AUDIO_VOICE_MPH_TRAD_CARTOON
} AudioVoiceMphTradType;

typedef enum {
    VMP_TRAD_MALE = 0,
    VMP_TRAD_FEMALE
}SpeakerSex;

extern int AudioVoiceMphGetsize(AudioVoiceMphMemSize *memSize);
extern int AudioVoiceMphInit(char *handle, char *scratchBuf);
extern int AudioVoiceMphSetPara(char *handle, SpeakerSex gender, AudioVoiceMphTradType type);
extern int AudioVoiceMphApply(char *handle, char *scratchBuf, AudioVoiceMphData *data);

#ifdef __cplusplus
}
#endif

#endif