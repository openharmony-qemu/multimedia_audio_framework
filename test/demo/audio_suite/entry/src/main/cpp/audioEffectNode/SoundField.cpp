/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "SoundField.h"

OH_SoundFieldType getSoundFieldTypeByNum(int mode)
{
    switch (mode) {
        case OH_SoundFieldType::SOUND_FIELD_FRONT_FACING:
            return OH_SoundFieldType::SOUND_FIELD_FRONT_FACING;
        case OH_SoundFieldType::SOUND_FIELD_GRAND:
            return OH_SoundFieldType::SOUND_FIELD_GRAND;
        case OH_SoundFieldType::SOUND_FIELD_NEAR:
            return OH_SoundFieldType::SOUND_FIELD_NEAR;
        case OH_SoundFieldType::SOUND_FIELD_WIDE:
            return OH_SoundFieldType::SOUND_FIELD_WIDE;
        default:
            return OH_SoundFieldType::SOUND_FIELD_FRONT_FACING;
    }
}