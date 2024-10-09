/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "HeadTracker"
#endif

#include "audio_head_tracker.h"
#include "audio_effect_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
#ifdef SENSOR_ENABLE
HeadPostureData HeadTracker::headPostureData_ = {1, 1.0, 0.0, 0.0, 0.0};
std::mutex HeadTracker::headTrackerMutex_;

const uint32_t ORDER_ONE = 1;
const uint32_t HP_DATA_PRINT_COUNT = 60; // Print once per second

void HeadTracker::HeadPostureDataProcCb(SensorEvent *event)
{
    std::lock_guard<std::mutex> lock(headTrackerMutex_);

    if (event == nullptr) {
        AUDIO_ERR_LOG("Audio HeadTracker Sensor event is nullptr!");
        return;
    }

    if (event[0].data == nullptr) {
        AUDIO_ERR_LOG("Audio HeadTracker Sensor event[0].data is nullptr!");
        return;
    }

    if (event[0].dataLen < sizeof(HeadPostureData)) {
        AUDIO_ERR_LOG("Event dataLen less than head posture data size, event.dataLen:%{public}u", event[0].dataLen);
        return;
    }
    HeadPostureData *headPostureDataTmp = reinterpret_cast<HeadPostureData *>(event[0].data);
    headPostureData_.order = headPostureDataTmp->order;
    headPostureData_.w = headPostureDataTmp->w;
    headPostureData_.x = headPostureDataTmp->x;
    headPostureData_.y = headPostureDataTmp->y;
    headPostureData_.z = headPostureDataTmp->z;
    if (headPostureData_.order % HP_DATA_PRINT_COUNT == ORDER_ONE) {
        AUDIO_DEBUG_LOG("Head posture data of order %{public}d received", headPostureData_.order);
    }
}

HeadTracker::HeadTracker()
{
    AUDIO_INFO_LOG("HeadTracker created!");
}

HeadTracker::~HeadTracker()
{
    AUDIO_INFO_LOG("HeadTracker destroyed!");
}

int32_t HeadTracker::SensorInit()
{
    sensorUser_.callback = HeadPostureDataProcCb;
    return SubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_);
}

int32_t HeadTracker::SensorSetConfig(int32_t spatializerEngineState)
{
    int32_t ret;
    switch (spatializerEngineState) {
        case NONE_SPATIALIZER_ENGINE:
            AUDIO_ERR_LOG("system has no spatializer engine!");
            ret = ERROR;
            break;
        case ARM_SPATIALIZER_ENGINE:
            AUDIO_INFO_LOG("system uses arm spatializer engine!");
            ret = SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_,
                sensorSamplingInterval_, sensorSamplingInterval_);
            break;
        case DSP_SPATIALIZER_ENGINE:
            AUDIO_INFO_LOG("system uses dsp spatializer engine!");
            ret = SetBatch(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_,
                sensorSamplingInterval_, sensorSamplingInterval_ * 2); // 2 * sampling for DSP
            break;
        default:
            AUDIO_ERR_LOG("spatializerEngineState error!");
            ret = ERROR;
            break;
    }
    return ret;
}

int32_t HeadTracker::SensorActive()
{
    return ActivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_);
}

int32_t HeadTracker::SensorDeactive()
{
    return DeactivateSensor(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_);
}

int32_t HeadTracker::SensorUnsubscribe()
{
    return UnsubscribeSensor(SENSOR_TYPE_ID_HEADPOSTURE, &sensorUser_);
}

HeadPostureData HeadTracker::GetHeadPostureData()
{
    std::lock_guard<std::mutex> lock(headTrackerMutex_);
    return headPostureData_;
}

void HeadTracker::SetHeadPostureData(HeadPostureData headPostureData)
{
    std::lock_guard<std::mutex> lock(headTrackerMutex_);
    headPostureData_ = headPostureData;
}
#endif
}  // namespace AudioStandard
}  // namespace OHOS