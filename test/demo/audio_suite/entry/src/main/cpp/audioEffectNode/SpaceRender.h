/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_SPACERENDER_H
#define AUDIOEDITTESTAPP_SPACERENDER_H
#include "NodeManager.h"
#include "napi/native_api.h"
#include <ohaudio/native_audio_suite_base.h>
#include <string>

class SpaceRender {
};

struct DynamicRenderParams {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
    int surroundTime = 2;
    int surroundDirection = 0;
    OH_AudioSuite_SurroundDirection surroundDirectionType =
        OH_AudioSuite_SurroundDirection::SPACE_RENDER_CW;
    std::string effectNodeId;
    std::string inputId;
    std::string selectedNodeId;
};

napi_value StartFixedPositionEffect(napi_env env, napi_callback_info info);

napi_value ResetFixedPositionEffect(napi_env env, napi_callback_info info);

napi_value StartDynamicRenderEffect(napi_env env, napi_callback_info info);

void ParseDynamicRenderParams(napi_env env, napi_value* argv, DynamicRenderParams& params);

napi_value ResetDynamicRenderEffect(napi_env env, napi_callback_info info);

napi_value StartExpandEffect(napi_env env, napi_callback_info info);

napi_value ResetExpandEffect(napi_env env, napi_callback_info info);

napi_value GetFixedPositionParams(napi_env env, napi_callback_info info);

napi_value GetDynamicRenderParams(napi_env env, napi_callback_info info);

napi_value GetExpandParams(napi_env env, napi_callback_info info);

bool AddNodeToPipeline(napi_env env, std::string inputId, std::string effectNodeId, std::string selectedNodeId,
    napi_value ret);

OH_AudioSuite_Result CreateRenderNodeAndSetPosition(OH_AudioSuite_SpaceRenderPositionParams &positionPara,
                                                    std::string effectNodeId, Node &node);

OH_AudioSuite_Result CreateRenderNodeAndSetRenderPara(OH_AudioSuite_SpaceRenderRotationParams &rotationPara,
                                                      std::string effectNodeId, Node &node);

OH_AudioSuite_Result CreateRenderNodeAndSetExtension(OH_AudioSuite_SpaceRenderExtensionParams &extensionPara,
                                                     std::string effectNodeId, Node &node);

#endif //AUDIOEDITTESTAPP_SPACERENDER_H