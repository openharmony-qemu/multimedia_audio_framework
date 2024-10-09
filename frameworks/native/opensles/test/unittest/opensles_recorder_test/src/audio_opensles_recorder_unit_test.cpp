/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioOpenslesRecorderUnitTest"
#endif

#include "audio_opensles_recorder_unit_test.h"

#include "common.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const char *TEST_FILE_PATH = "/data/test_capture.pcm";
    FILE *wavFile_;
    SLObjectItf engineObject_;
    SLRecordItf captureItf_;
    SLOHBufferQueueItf bufferQueueItf_;
    SLObjectItf pcmCapturerObject_;
    SLEngineItf engineEngine_;
} // namespace

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    FILE *wavFile = (FILE *)pContext;
    if (wavFile != nullptr) {
        SLuint8 *buffer = nullptr;
        SLuint32 bufferSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, &bufferSize);
        if (buffer != nullptr) {
            fwrite(buffer, 1, bufferSize, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        }
    }

    return;
}

void AudioOpenslesRecorderUnitTest::SetUpTestCase(void) { }

void AudioOpenslesRecorderUnitTest::TearDownTestCase(void) { }

void AudioOpenslesRecorderUnitTest::SetUp(void) { }

void AudioOpenslesRecorderUnitTest::TearDown(void) { }

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateEngine_001, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateEngine_002, TestSize.Level0)
{
    SLresult result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateEngine_003, TestSize.Level0)
{
    SLresult result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateAudioRecorder_001, TestSize.Level0)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("AudioCaptureTest: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::MONO,
        OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_44100,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };

    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };

    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
                                                            &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateAudioRecorder_002, TestSize.Level0)
{
    SLresult result = (*pcmCapturerObject_)->Realize(pcmCapturerObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_CreateAudioRecorder_003, TestSize.Level0)
{
    SLresult result = (*pcmCapturerObject_)->GetInterface(pcmCapturerObject_, SL_IID_RECORD, &captureItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetBufferQueue_001, TestSize.Level0)
{
    SLresult result = (*pcmCapturerObject_)->GetInterface(pcmCapturerObject_, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_RegisterCallback_001, TestSize.Level0)
{
    SLresult result = (*bufferQueueItf_)->RegisterCallback(bufferQueueItf_, BufferQueueCallback, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*bufferQueueItf_)->Clear(bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_RegisterCallback_002, TestSize.Level1)
{
    SLresult result = (*bufferQueueItf_)->RegisterCallback(nullptr, BufferQueueCallback, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetRecordState_001, TestSize.Level0)
{
    SLresult result = (*captureItf_)->SetRecordState(captureItf_, SL_RECORDSTATE_RECORDING);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_001, TestSize.Level0)
{
    if (wavFile_ != nullptr) {
        SLuint8* buffer = nullptr;
        SLuint32 bufferSize = 0;
        SLresult result = (*bufferQueueItf_)->GetBuffer(bufferQueueItf_, &buffer, &bufferSize);
        EXPECT_TRUE(result == SL_RESULT_SUCCESS);
        if (buffer != nullptr) {
            fwrite(buffer, 1, bufferSize, wavFile_);
            result = (*bufferQueueItf_)->Enqueue(bufferQueueItf_, buffer, bufferSize);
            EXPECT_TRUE(result == SL_RESULT_SUCCESS);
        }
    }
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetState_001, TestSize.Level0)
{
    SLOHBufferQueueState state;
    SLresult result = (*bufferQueueItf_)->GetState(bufferQueueItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetState_002, TestSize.Level1)
{
    SLOHBufferQueueState state;
    SLresult result = (*bufferQueueItf_)->GetState(nullptr, &state);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetBuffer_001, TestSize.Level1)
{
    SLuint32 bufferSize = 0;
    SLresult result = (*bufferQueueItf_)->GetBuffer(nullptr, nullptr, &bufferSize);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetRecordState_002, TestSize.Level0)
{
    SLresult result = (*captureItf_)->SetRecordState(captureItf_, SL_RECORDSTATE_PAUSED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetRecordState_003, TestSize.Level0)
{
    SLresult result = (*captureItf_)->SetRecordState(captureItf_, SL_RECORDSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetRecordState_004, TestSize.Level1)
{
    SLresult result = (*captureItf_)->SetRecordState(nullptr, SL_RECORDSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetRecordState_001, TestSize.Level0)
{
    SLuint32 state;
    SLresult result = (*captureItf_)->GetRecordState(captureItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetRecordState_002, TestSize.Level1)
{
    SLuint32 state;
    SLresult result = (*captureItf_)->GetRecordState(nullptr, &state);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetDurationLimit_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->SetDurationLimit(nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetPosition_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->GetPosition(nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_RegisterCallback_003, TestSize.Level1)
{
    SLresult result = (*captureItf_)->RegisterCallback(captureItf_, nullptr, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->SetCallbackEventsMask(nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->GetCallbackEventsMask(nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->SetMarkerPosition(nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_ClearMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->ClearMarkerPosition(nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->GetMarkerPosition(nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->SetPositionUpdatePeriod(nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_GetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*captureItf_)->GetPositionUpdatePeriod(nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_clear_001, TestSize.Level0)
{
    SLresult result = (*bufferQueueItf_)->Clear(bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_clear_002, TestSize.Level1)
{
    SLresult result = (*bufferQueueItf_)->Clear(nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_Destroy_001, TestSize.Level0)
{
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_Destroy_002, TestSize.Level0)
{
    (*engineObject_)->Destroy(engineObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesRecorderUnitTest, Prf_Audio_Opensles_Capture_CreateEngine_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t performanceTestTimes = 10;
    int64_t usecTimes = 1000000000;
    int64_t totalTime = 0;
    for (int32_t i = 0; i < performanceTestTimes; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * usecTimes + tv2.tv_nsec - (tv1.tv_sec * usecTimes + tv1.tv_nsec);
    }
    int64_t expectTime = 1000000000;
    EXPECT_TRUE(totalTime <= expectTime * performanceTestTimes);
}

HWTEST(AudioOpenslesRecorderUnitTest, Prf_Audio_Opensles_Capture_DestoryEngine_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t performanceTestTimes = 10;
    int64_t usecTimes = 1000000000;
    int64_t totalTime = 0;
    for (int32_t i = 0; i < performanceTestTimes; i++) {
        engineObject_ = {};
        slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->Destroy(engineObject_);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * usecTimes + tv2.tv_nsec - (tv1.tv_sec * usecTimes + tv1.tv_nsec);
    }
    int64_t expectTime = 1000000000;
    EXPECT_TRUE(totalTime <= expectTime * performanceTestTimes);
}

HWTEST(AudioOpenslesRecorderUnitTest, Prf_Audio_Opensles_Capture_Realize_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t performanceTestTimes = 10;
    int64_t usecTimes = 1000000000;
    int64_t totalTime = 0;
    engineObject_ = {};
    slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    for (int32_t i = 0; i < performanceTestTimes; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * usecTimes + tv2.tv_nsec - (tv1.tv_sec * usecTimes + tv1.tv_nsec);
    }
    int64_t expectTime = 1000000000;
    EXPECT_TRUE(totalTime <= expectTime * performanceTestTimes);
}

HWTEST(AudioOpenslesRecorderUnitTest, Prf_Audio_Opensles_Capture_GetInterface_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t performanceTestTimes = 10;
    int64_t usecTimes = 1000000000;
    int64_t totalTime = 0;
    for (int32_t i = 0; i < performanceTestTimes; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * usecTimes + tv2.tv_nsec - (tv1.tv_sec * usecTimes + tv1.tv_nsec);
    }
    int64_t expectTime = 1000000000;
    EXPECT_TRUE(totalTime <= expectTime * performanceTestTimes);
}

HWTEST(AudioOpenslesRecorderUnitTest, Prf_Audio_Opensles_CreateAudioRecorder_001, TestSize.Level0)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("AudioCaptureTest: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::MONO,
        OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_44100,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };

    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };

    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t performanceTestTimes = 10;
    int64_t usecTimes = 1000000000;
    int64_t totalTime = 0;
    for (int32_t i = 0; i < performanceTestTimes; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
                                              &audioSink, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * usecTimes + tv2.tv_nsec - (tv1.tv_sec * usecTimes + tv1.tv_nsec);
        (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
    }
    int64_t expectTime = 1000000000;
    EXPECT_TRUE(totalTime <= expectTime * performanceTestTimes);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_001, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_001: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::MONO,
        OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_8000,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };

    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_002, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_002: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::STEREO,
        SL_SAMPLINGRATE_11_025,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_003, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_003: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        3,
        SL_SAMPLINGRATE_12,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_004, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_004: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_24,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_005, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_005: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::STEREO,
        SL_SAMPLINGRATE_22_05,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };

    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_006, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_006: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::STEREO,
        SL_SAMPLINGRATE_24,
        SL_PCMSAMPLEFORMAT_FIXED_32,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_007, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_007: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::MONO,
        SL_SAMPLINGRATE_32,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_008, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_008: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::STEREO,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_009, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_009: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_48,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_010, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_010: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_64,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_011, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_011: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_96,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}

HWTEST(AudioOpenslesRecorderUnitTest, Audio_Opensles_Capture_SlToOhosChannel_012, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("SlToOhosChannel_012: Unable to open record file.");
    }

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_88_2,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };
    SLresult result = (*engineEngine_)->CreateAudioRecorder(engineEngine_, &pcmCapturerObject_, &audioSource,
        &audioSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmCapturerObject_)->Destroy(pcmCapturerObject_);
}
} // namespace AudioStandard
} // namespace OHOS
