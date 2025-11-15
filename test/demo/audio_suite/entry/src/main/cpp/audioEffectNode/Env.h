/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef TEST1_ENV_H
#define TEST1_ENV_H

#include <cstdint>
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"

struct EnvEffectParams {
    std::string inputIdStr;
    std::string uuidStr;
    unsigned int mode = 0;
    std::string selectedNodeId;
};

class Env {
};

void getEnvEnumByNumber(int num, OH_EnvironmentType &type);

#endif //TEST1_ENV_H