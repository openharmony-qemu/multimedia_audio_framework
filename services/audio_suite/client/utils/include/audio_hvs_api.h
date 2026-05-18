/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_HVS_API_H
#define AUDIO_HVS_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define AUDIO_HVS_MEM_RESERVE            2
#define AUDIO_HVS_QUAT_NUM               4

/* Error codes */
#define AUDIO_HVS_OK                    (0)
#define AUDIO_HVS_ERR_UNKNOWN           (-1)
#define AUDIO_HVS_ERR_INVALID_PARAM     (-2)
#define AUDIO_HVS_ERR_INVALID_HANDLE    (-3)
#define AUDIO_HVS_ERR_MEM_FAIL          (-4)
#define AUDIO_HVS_ERR_MODEL_LOAD_FAIL   (-5)
#define AUDIO_HVS_ERR_STATUS            (-6)

/* Memory size structure returned by AudioHvsGetSize */
typedef struct {
    int32_t handleSize;
    int32_t scratchBufSize;
    int32_t reserve[AUDIO_HVS_MEM_RESERVE];
} AudioHvsMemSize;

typedef struct {
    float quaterninos[AUDIO_HVS_QUAT_NUM];
} AudioDynamicUserData;

/* Dynamic data passed per-Apply call */
typedef struct {
    int32_t *dataIn;
    int32_t *dataOut;
    int32_t samplingRate;
    int16_t dataAmbOrder;
    int32_t renderAmbOrder;
    int32_t stereoDownmix;
    int32_t useAFormatInput;
    int32_t hoaInputFormat;
    AudioDynamicUserData dynUserData;
} AudioHvsDynamicData;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_HVS_API_H */
