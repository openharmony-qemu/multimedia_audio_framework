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

#ifndef SIMD_UTILS_H
#define SIMD_UTILS_H

#include <cstddef>
#include <cstdint>

#if !defined(DISABLE_SIMD) && \
    (defined(__aarch64__) || (defined(__arm__) && defined(__ARM_NEON__)))
// enable arm Simd
#include <arm_neon.h>
#define USE_ARM_NEON 1
#else
// disable SIMD.
#define USE_ARM_NEON 0
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

void SimdPointByPointAdd(size_t length, const float* inputLeft, const float* inputRight, float* output);
void SimdPointByPointSub(size_t length, const float* inputLeft, const float* inputRight, float* output);
void SimdPointByPointMul(size_t length, const float* inputLeft, const float* inputRight, float* output);
}}}

#endif