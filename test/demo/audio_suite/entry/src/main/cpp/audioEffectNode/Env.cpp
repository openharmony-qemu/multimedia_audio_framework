/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Env.h"

void getEnvEnumByNumber(int num, OH_EnvironmentType &type)
{
    switch (num) {
        case ENVIRONMENT_TYPE_BROADCAST:
            type = ENVIRONMENT_TYPE_BROADCAST;
            break;
        case ENVIRONMENT_TYPE_EARPIECE:
            type = ENVIRONMENT_TYPE_EARPIECE;
            break;
        case ENVIRONMENT_TYPE_UNDERWATER:
            type = ENVIRONMENT_TYPE_UNDERWATER;
            break;
        case ENVIRONMENT_TYPE_GRAMOPHONE:
            type = ENVIRONMENT_TYPE_GRAMOPHONE;
            break;
        default:
            type = ENVIRONMENT_TYPE_BROADCAST;
            break;
    }
}