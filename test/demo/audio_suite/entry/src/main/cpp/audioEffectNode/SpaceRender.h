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

napi_value startFixedPositionEffect(napi_env env, napi_callback_info info);

napi_value resetFixedPositionEffect(napi_env env, napi_callback_info info);

napi_value startDynamicRenderEffect(napi_env env, napi_callback_info info);

void parseDynamicRenderParams(napi_env env, napi_value* argv, DynamicRenderParams& params);

napi_value resetDynamicRenderEffect(napi_env env, napi_callback_info info);

napi_value startExpandEffect(napi_env env, napi_callback_info info);

napi_value resetExpandEffect(napi_env env, napi_callback_info info);

napi_value getFixedPositionParams(napi_env env, napi_callback_info info);

napi_value getDynamicRenderParams(napi_env env, napi_callback_info info);

napi_value getExpandParams(napi_env env, napi_callback_info info);

bool AddNodeToPipeline(napi_env env, std::string inputId, std::string effectNodeId, std::string selectedNodeId,
    napi_value ret);

OH_AudioSuite_Result createRenderNodeAndSetPosition(OH_AudioSuite_SpaceRenderPositionParams &positionPara,
                                                    std::string effectNodeId, Node &node);

OH_AudioSuite_Result createRenderNodeAndSetRenderPara(OH_AudioSuite_SpaceRenderRotationParams &rotationPara,
                                                      std::string effectNodeId, Node &node);

OH_AudioSuite_Result createRenderNodeAndSetExtension(OH_AudioSuite_SpaceRenderExtensionParams &extensionPara,
                                                     std::string effectNodeId, Node &node);

#endif //AUDIOEDITTESTAPP_SPACERENDER_H