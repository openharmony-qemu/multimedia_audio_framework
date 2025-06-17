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

#include "simd_utils.h"
#include <algorithm>
#include <limits>
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
#if USE_ARM_NEON == 1
constexpr int ALIGIN_FLOAT_SIZE = 4;
#endif
void SimdPointByPointAdd(size_t length, const float* inputLeft, const float* inputRight, float* output)
{
    CHECK_AND_RETURN_LOG(inputLeft, "inputLeft is nullptr");
    CHECK_AND_RETURN_LOG(inputRight, "inputRight is nullptr");
    CHECK_AND_RETURN_LOG(output, "output is nullptr");
#if USE_ARM_NEON == 1
    if (length < ALIGIN_FLOAT_SIZE) {
        for (size_t i = 0; i < length; i++) {
            output[i] = inputLeft[i] + inputRight[i];
        }
    } else {
        size_t procLen = length >> 2;
        float32x4_t left32x4;
        float32x4_t right32x4;
        float32x4_t out32x4;
        for (size_t i = 0; i < procLen; i++) {
            left32x4 = vld1q_f32(inputLeft + i * ALIGIN_FLOAT_SIZE);
            right32x4 = vld1q_f32(inputRight + i * ALIGIN_FLOAT_SIZE);
            out32x4 = vaddq_f32(left32x4, right32x4);
            vst1q_f32(output + i * ALIGIN_FLOAT_SIZE, out32x4);
        }
        size_t odd = length - procLen * ALIGIN_FLOAT_SIZE;
        if (odd) {
            for (size_t j = length - odd; j < length; j++) {
                output[j] = inputLeft[j] + inputRight[j];
            }
        }
    }
#else
    for (size_t i = 0; i < length; i++) {
        output[i] = inputLeft[i] + inputRight[i];
    }
#endif
}

void SimdPointByPointSub(size_t length, const float* inputLeft, const float* inputRight, float* output)
{
    CHECK_AND_RETURN_LOG(inputLeft, "inputLeft is nullptr");
    CHECK_AND_RETURN_LOG(inputRight, "inputRight is nullptr");
    CHECK_AND_RETURN_LOG(output, "output is nullptr");
#if USE_ARM_NEON == 1
    if (length < ALIGIN_FLOAT_SIZE) {
        for (size_t i = 0; i < length; i++) {
            output[i] = inputLeft[i] - inputRight[i];
        }
    } else {
        size_t procLen = length >> 2;
        float32x4_t left32x4;
        float32x4_t right32x4;
        float32x4_t out32x4;
        for (size_t i = 0; i < procLen; i++) {
            left32x4 = vld1q_f32(inputLeft + i * ALIGIN_FLOAT_SIZE);
            right32x4 = vld1q_f32(inputRight + i * ALIGIN_FLOAT_SIZE);
            out32x4 = vsubq_f32(left32x4, right32x4);
            vst1q_f32(output  + i * ALIGIN_FLOAT_SIZE, out32x4);
        }
        size_t odd = length - procLen * ALIGIN_FLOAT_SIZE;
        if (odd) {
            for (size_t j = length - odd; j < length; j++) {
                output[j] = inputLeft[j] - inputRight[j];
            }
        }
    }
#else
    for (size_t i = 0; i < length; i++) {
        output[i] = inputLeft[i] - inputRight[i];
    }
#endif
}

void SimdPointByPointMul(size_t length, const float* inputLeft, const float* inputRight, float* output)
{
    CHECK_AND_RETURN_LOG(inputLeft, "inputLeft is nullptr");
    CHECK_AND_RETURN_LOG(inputRight, "inputRight is nullptr");
    CHECK_AND_RETURN_LOG(output, "output is nullptr");
#if USE_ARM_NEON == 1
    if (length < ALIGIN_FLOAT_SIZE) {
        for (size_t i = 0; i < length; i++) {
            output[i] = inputLeft[i] * inputRight[i];
        }
    } else {
        size_t procLen = length >> 2;
        float32x4_t left32x4;
        float32x4_t right32x4;
        float32x4_t out32x4;
        for (size_t i = 0; i < procLen; i++) {
            left32x4 = vld1q_f32(inputLeft + i * ALIGIN_FLOAT_SIZE);
            right32x4 = vld1q_f32(inputRight + i * ALIGIN_FLOAT_SIZE);
            out32x4 = vmulq_f32(left32x4, right32x4);
            vst1q_f32(output  + i * ALIGIN_FLOAT_SIZE, out32x4);
        }
        size_t odd = length - procLen * ALIGIN_FLOAT_SIZE;
        if (odd) {
            for (size_t j = length - odd; j < length; j++) {
                output[j] = inputLeft[j] * inputRight[j];
            }
        }
    }
#else
    for (size_t i = 0; i < length; i++) {
        output[i] = inputLeft[i] * inputRight[i];
    }
#endif
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
