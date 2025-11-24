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

#ifndef AUDIO_SUITE_TEMPO_PITCH_API_H
#define AUDIO_SUITE_TEMPO_PITCH_API_H

#define FFT_LENGTH 1024
#define FFT_FRAME_LEN 1024
#define PV_MAX_BUFFER 200000
#ifdef __cplusplus
extern "C" {
#endif

struct PVStruct;
typedef struct PVStruct *PVParam;

struct PVStruct {
    short inBuffer1[FFT_FRAME_LEN];
    short inBuffer2[FFT_FRAME_LEN];
    int outBuffer[FFT_FRAME_LEN];
    int OutHistData[FFT_FRAME_LEN];
 
    short InHistData[FFT_FRAME_LEN];
    int signOfPhaReset; // 补0后相位归置标志
    int signOfMagReset; // 补0后幅度因子重置标志
    int lengthOfMagReset;
    int countSign_test;
    float countSign2_test;
 
    int readPosition;
    int writePosition;
    int readPosition_out;
    int writePosition_out;
    int signOfcount;                      // 超存个数
    int overBuffer;                       // 超存标志
    int count_i;                          // 计算次数
    int numCount;                         // 传入次数
    short InBufferData[PV_MAX_BUFFER];    // 输入缓存区
 
    float fftPart1[FFT_LENGTH + 4];
    float fftPart2[FFT_LENGTH + 4];
    float fftNewOut[FFT_LENGTH + 4];
    float dphi[FFT_LENGTH + 4];
    int fftOutput[FFT_LENGTH + 4];
 
    float speed;
    int numChannels;
    int numInputSamples;
    int sampleRate;
 
    float syn_hopsize; // mag计算因子
    float phase[FFT_LENGTH + 4];
};

typedef struct {
    bool currentDeviceSupport;
    bool realTimeSupport;
    int sampleRate;
    int channel; // only support 1 ch
    int dataFormat; // 0:S24LE,1:S16LE,2:S32LE,3:F32LE
} AudioPVSpec;

extern PVParam PVCreate(int sampleRate);
extern void PVDestroypvHandle(PVParam pvHandle);
extern int PVSetSpeed(PVParam pvHandle, float speed);
extern int PVChangeSpeed(PVParam pvHandle, const short *dataIn, short *dataOut, int inCount, int outCount);
extern AudioPVSpec PVGetSpec(void);

#ifdef __cplusplus
}
#endif

#endif