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

#ifndef HPAE_NODE_COMMON_H
#define HPAE_NODE_COMMON_H
#include "hpae_define.h"
#include "audio_effect.h"
#include "audio_module_info.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
bool CheckHpaeNodeInfoIsSame(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &curNodeInfo);
HpaeProcessorType TransStreamTypeToSceneType(AudioStreamType streamType);
HpaeProcessorType TransEffectSceneToSceneType(AudioEffectScene effectScene);
HpaeProcessorType TransSourceTypeToSceneType(SourceType sourceType);
bool CheckSceneTypeNeedEc(HpaeProcessorType processorType);
bool CheckSceneTypeNeedMicRef(HpaeProcessorType processorType);
std::string TransNodeInfoToStringKey(HpaeNodeInfo& nodeInfo);
AudioEnhanceScene TransProcessType2EnhanceScene(const HpaeProcessorType &processorType);
std::string TransProcessorTypeToSceneType(HpaeProcessorType processorType);
uint64_t ConvertDatalenToUs(size_t bufferSize, const HpaeNodeInfo &nodeInfo);
size_t ConvertUsToFrameCount(uint64_t usTime, const HpaeNodeInfo &nodeInfo);
std::string ConvertSessionState2Str(HpaeSessionState state);
std::string ConvertStreamManagerState2Str(StreamManagerState state);
void AdjustMchSinkInfo(const AudioModuleInfo &audioModuleInfo, HpaeSinkInfo &sinkInfo);
int32_t TransModuleInfoToHpaeSinkInfo(const AudioModuleInfo &audioModuleInfo, HpaeSinkInfo &sinkInfo);
bool CheckSourceInfoIsDifferent(const HpaeSourceInfo &info, const HpaeSourceInfo &oldInfo);
int32_t TransModuleInfoToHpaeSourceInfo(const AudioModuleInfo &audioModuleInfo, HpaeSourceInfo &sourceInfo);
AudioSampleFormat TransFormatFromStringToEnum(std::string format);
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif