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
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_stream_checker.h"
#include "audio_utils.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t COUNT = 4;
const int32_t DEFAULT_BAD_DATA_TRANSFER_BIT_MAP = 3;
const int32_t DEFAULT_TIME_INTERVAL = 2000000000;
const int32_t DEFAULT_BAD_FRAME_RATIO = 50;
const int32_t DEFAULT_SUMFRAME_COUNT = 100;
const int32_t DEFAULT_ABNORMAL_FRAME_NUM = 40;
const float DEFAULT_FLOAT_BAD_FRAME_RATIO = 0.5f;

typedef void (*TestFuncs)();

void AudioStreamCheckerInitCheckerFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t callbackId = g_fuzzUtils.GetData<int32_t>();
    checker->InitChecker(para, pid, callbackId);
    checker->InitChecker(para, pid, callbackId);
}

void AudioStreamCheckerRecordFrameFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    checker->RecordMuteFrame();
    checker->RecordNodataFrame();
}

void AudioStreamCheckerGetAppUidFuzzTest()
{
    AudioProcessConfig cfg;
    cfg.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    int32_t uid = checker->GetAppUid();
}

void AudioStreamCheckerDeleteCheckerParaFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    int32_t pid1 = g_fuzzUtils.GetData<int32_t>();
    int32_t pid2 = g_fuzzUtils.GetData<int32_t>();
    int32_t callbackId1 = g_fuzzUtils.GetData<int32_t>();
    int32_t callbackId2 = g_fuzzUtils.GetData<int32_t>();
    checker->InitChecker(para, pid1, callbackId2);
    checker->InitChecker(para, pid2, callbackId2);
    checker->DeleteCheckerPara(pid1, callbackId1);
    checker->DeleteCheckerPara(pid1, callbackId2);
}

void AudioStreamCheckerMonitorCheckFrameFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = 0;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    if (g_fuzzUtils.GetData<bool>()) {
        checker->RecordMuteFrame();
        checker->RecordNormalFrame();
    }
    if (g_fuzzUtils.GetData<bool>()) {
        for (int i = 0; i < COUNT; i++) {
            checker->RecordNormalFrame();
        }
    }
    checker->MonitorCheckFrame();
}

void AudioStreamCheckerMonitorOnAllCallbackFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    checker->MonitorOnAllCallback(g_fuzzUtils.GetData<DataTransferStateChangeType>(), g_fuzzUtils.GetData<bool>());
}

void AudioStreamCheckerOnRemoteAppDiedFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    checker->InitChecker(para, pid, g_fuzzUtils.GetData<int32_t>());
    checker->OnRemoteAppDied(pid);
}

void AudioStreamCheckerMonitorCheckFrameSubFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    CheckerParam checkerPara;
    checkerPara.hasInitCheck = g_fuzzUtils.GetData<bool>();
    checkerPara.isMonitorMuteFrame = g_fuzzUtils.GetData<bool>();
    checkerPara.isMonitorNoDataFrame = g_fuzzUtils.GetData<bool>();
    if (g_fuzzUtils.GetData<bool>()) {
        checkerPara.para.timeInterval = DEFAULT_TIME_INTERVAL;
    }
    checkerPara.lastUpdateTime = ClockTime::GetCurNano();
    checker->MonitorCheckFrameSub(checkerPara);
}

void AudioStreamCheckerMonitorCheckFrameActionFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    CheckerParam checkerPara;
    checkerPara.lastStatus = g_fuzzUtils.GetData<DataTransferStateChangeType>();
    checkerPara.sumFrameCount = DEFAULT_SUMFRAME_COUNT;
    int64_t abnormalFrameNum = DEFAULT_ABNORMAL_FRAME_NUM;
    float badFrameRatio = DEFAULT_FLOAT_BAD_FRAME_RATIO;
    checker->MonitorCheckFrameAction(checkerPara, abnormalFrameNum, badFrameRatio);
}

void AudioStreamCheckerMonitorOnCallbackFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    CheckerParam checkerPara;
    checkerPara.sumFrameCount = DEFAULT_SUMFRAME_COUNT;
    checkerPara.hasInitCheck = g_fuzzUtils.GetData<bool>();
    checker->MonitorOnCallback(g_fuzzUtils.GetData<DataTransferStateChangeType>(),
        g_fuzzUtils.GetData<bool>(), checkerPara);
}

void AudioStreamCheckerRecordStandbyTimeFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    checker->RecordStandbyTime(g_fuzzUtils.GetData<bool>());
}

void AudioStreamCheckerUpdateAppStateFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->InitChecker(para, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    checker->UpdateAppState(g_fuzzUtils.GetData<bool>());
}

void AudioStreamCheckerCleanRecordDataFuzzTest()
{
    AudioProcessConfig cfg;
    DataTransferMonitorParam para;
    para.badDataTransferTypeBitMap = DEFAULT_BAD_DATA_TRANSFER_BIT_MAP;
    para.timeInterval = DEFAULT_TIME_INTERVAL;
    para.badFramesRatio = DEFAULT_BAD_FRAME_RATIO;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    CheckerParam tmpPara;
    CheckerParam &checkerPara = tmpPara;
    checker->CleanRecordData(checkerPara);
}

void AudioStreamCheckerCalculateFrameAfterStandbyFuzzTest()
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    CheckerParam para;
    int64_t abnormalFrameNum = 0;

    para.standbyStartTime = 0;
    para.standbyStopTime = g_fuzzUtils.GetData<int64_t>();
    para.lastUpdateTime = 0;
    para.isMonitorNoDataFrame = g_fuzzUtils.GetData<bool>();
    checker->streamConfig_.rendererInfo.rendererFlags = 0;
    checker->CalculateFrameAfterStandby(para, abnormalFrameNum);
}

void AudioStreamCheckerCheckVolumeFuzzTest()
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->curVolume_ = g_fuzzUtils.GetData<float>();
    checker->preVolume_ = g_fuzzUtils.GetData<float>();
    checker->CheckVolume();
}

void AudioStreamCheckerSetVolumeFuzzTest()
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    float volume = g_fuzzUtils.GetData<float>();
    checker->SetVolume(volume);
}

void AudioStreamCheckerGetVolumeFuzzTest()
{
    AudioProcessConfig cfg;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    CHECK_AND_RETURN(checker != nullptr);
    checker->GetVolume();
}

vector<TestFuncs> g_testFuncs = {
    AudioStreamCheckerInitCheckerFuzzTest,
    AudioStreamCheckerRecordFrameFuzzTest,
    AudioStreamCheckerGetAppUidFuzzTest,
    AudioStreamCheckerDeleteCheckerParaFuzzTest,
    AudioStreamCheckerMonitorCheckFrameFuzzTest,
    AudioStreamCheckerMonitorOnAllCallbackFuzzTest,
    AudioStreamCheckerOnRemoteAppDiedFuzzTest,
    AudioStreamCheckerMonitorCheckFrameSubFuzzTest,
    AudioStreamCheckerMonitorCheckFrameActionFuzzTest,
    AudioStreamCheckerMonitorOnCallbackFuzzTest,
    AudioStreamCheckerRecordStandbyTimeFuzzTest,
    AudioStreamCheckerUpdateAppStateFuzzTest,
    AudioStreamCheckerCleanRecordDataFuzzTest,
    AudioStreamCheckerCalculateFrameAfterStandbyFuzzTest,
    AudioStreamCheckerCheckVolumeFuzzTest,
    AudioStreamCheckerSetVolumeFuzzTest,
    AudioStreamCheckerGetVolumeFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
