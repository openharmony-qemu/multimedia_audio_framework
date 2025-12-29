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
 
#ifndef AUDIO_HMS_SPACE_RENDER_API_H
#define AUDIO_HMS_SPACE_RENDER_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define SPACE_RENDER_OK (0)
#define SPACE_RENDER_UNKNOWN (-1)
#define SPACE_RENDER_INV_MODE_PARAM (-2)
#define SPACE_RENDER_INV_CART_POINT_PARAM (-3)
#define SPACE_RENDER_INV_ROTATION_TIME_PARAM (-4)
#define SPACE_RENDER_INV_ROTATION_DIRECTION_PARAM (-5)
#define SPACE_RENDER_INV_EXPAND_ANGLE_PARAM (-6)
#define SPACE_RENDER_INV_EXPAND_RADIUS_PARAM (-7)
#define SPACE_RENDER_INV_HANDLE (-11)
#define SPACE_RENDER_INV_PARAM (-12)
#define SPACE_RENDER_ERR_STATUS (-13)
#define SPACE_RENDER_MEM_FAIL (-14)
#define SPACE_RENDER_MODEL_LOAD_FAIL (-15)

static const float SPACE_RENDER_MIN_CART_POINT_DISTANCE = -5.0f;
static const float SPACE_RENDER_MAX_CART_POINT_DISTANCE = 5.0f;
static const float SPACE_RENDER_MIN_ROTATION_TIME = 2.0f;
static const float SPACE_RENDER_MAX_ROTATION_TIME = 40.0f;
static const int SPACE_RENDER_MIN_EXPAND_ANGLE = 1;
static const int SPACE_RENDER_MAX_EXPAND_ANGLE = 360;
static const float SPACE_RENDER_MIN_EXPAND_RADIUS = 1.0f;
static const float SPACE_RENDER_MAX_EXPAND_RADIUS = 5.0f;

typedef struct {
    bool isSupport;
    bool isRealTime;
    uint32_t frameLen;
    uint32_t inSampleRate;
    uint32_t inChannels;
    uint32_t inFormat;
    uint32_t outSampleRate;
    uint32_t outChannels;
    uint32_t outFormat;;
} SpaceRenderSpeces;

typedef enum {
    SPACE_RENDER_MODE_STATIC = 1,
    SPACE_RENDER_MODE_ROTATION = 2,
    SPACE_RENDER_MODE_EXPAND = 3,
} SpaceRenderMode;

typedef enum {
    SPACE_RENDER_ROTATION_MODE_CCW = 0,
    SPACE_RENDER_ROTATION_MODE_CW = 1,
} SpaceRenderRotationMode;

typedef struct {
    SpaceRenderMode mode;
    float cartPoint[3];
    float rotationTime;
    SpaceRenderRotationMode rotationDirection;
    float expandRadius;
    int expandAngle;
} SpaceRenderParam;

SpaceRenderSpeces SpaceRenderGetSpeces(void);
int SpaceRenderGetSize(const SpaceRenderParam *params);
int SpaceRenderGetLateSamples(const char *phandle);
int SpaceRenderInit(char *phandle, const SpaceRenderParam *params);
int SpaceRenderApply(char *phandle, const short *pcmIn, const int inSampleCnt, short *pcmOut);
int SpaceRenderRelease(char *phandle);

#ifdef __cplusplus
}
#endif

#endif