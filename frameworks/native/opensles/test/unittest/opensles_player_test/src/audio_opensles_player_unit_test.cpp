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
#define LOG_TAG "AudioOpenslesPlayerUnitTest"
#endif

#include "audio_opensles_player_unit_test.h"

#include "common.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "pcm2wav.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const char *TEST_FILE_PATH = "/data/test_44100_2.wav";
    const int64_t PERFORMANCE_TEST_TIME = 10;
    const int64_t USEC_TIMES = 1000000000;
    const int64_t EXPECT_TIME = 1000000000;
    FILE *wavFile_;
    wav_hdr wavHeader_;
    SLObjectItf engineObject_;
    SLObjectItf outputMixObject_;
    SLPlayItf playItf_;
    SLVolumeItf volumeItf_;
    SLOHBufferQueueItf bufferQueueItf_;
    SLObjectItf pcmPlayerObject_;
    SLEngineItf engineEngine_;
} // namespace

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    FILE *wavFile = (FILE *)pContext;
    if (!feof(wavFile)) {
        SLuint8 *buffer = nullptr;
        SLuint32 bufferSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, &bufferSize);
        if (buffer != nullptr) {
            fread(buffer, 1, size, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        }
    }
    return;
}

void AudioOpenslesPlayerUnitTest::SetUpTestCase(void) { }

void AudioOpenslesPlayerUnitTest::TearDownTestCase(void) { }

void AudioOpenslesPlayerUnitTest::SetUp(void) { }

void AudioOpenslesPlayerUnitTest::TearDown(void) { }

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_001, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_002, TestSize.Level0)
{
    SLresult result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_003, TestSize.Level0)
{
    SLresult result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_001, TestSize.Level0)
{
    SLresult result = (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_002, TestSize.Level0)
{
    SLresult result = (*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Realize_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->Realize(nullptr, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Resume_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->Resume(outputMixObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetState_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->GetState(nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetState_002, TestSize.Level1)
{
    SLuint32 state;
    SLresult result = (*outputMixObject_)->GetState(outputMixObject_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->RegisterCallback(outputMixObject_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_AbortAsyncOperation_001, TestSize.Level1)
{
    (*engineObject_)->AbortAsyncOperation(outputMixObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_001, TestSize.Level0)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_001: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_001, TestSize.Level1)
{
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_002, TestSize.Level1)
{
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::STEREO,
        SL_SAMPLINGRATE_11_025,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_003, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_003: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_12,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_004, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_004: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::CHANNEL_4,
        SL_SAMPLINGRATE_16,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_005, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_005: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_22_05,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_006, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_006: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_24,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_007, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_007: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_32,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_008, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_008: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_44_1,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_009, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_009: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_48,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_010, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_010: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_64,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_011, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayerAdapter_011: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_96,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_014, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayStateAdapter_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = AudioPlayerAdapter::GetInstance()->GetPlayStateAdapter(-1, &state);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetCaptureStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetCaptureStateAdapter_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(-1, &state);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_EnqueueAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->EnqueueAdapter(-1, nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_ClearAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->ClearAdapter(-1);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->GetStateAdapter(-1, nullptr);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetBufferAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->GetBufferAdapter(-1, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_002, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->Realize(pcmPlayerObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_003, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_PLAY, &playItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_004, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_004: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        SL_SAMPLINGRATE_44_1,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, nullptr,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetInterface_003, TestSize.Level1)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(nullptr, SL_IID_PLAY, &playItf_);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateLEDDevice_001, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateLEDDevice_001: CreateLEDDevice Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLresult result = (*engineEngine_)->CreateLEDDevice(engineEngine_, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateVibraDevice_001, TestSize.Level1)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateVibraDevice_001: CreateVibraDevice Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLresult result = (*engineEngine_)->CreateVibraDevice(engineEngine_, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateMidiPlayer_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateMidiPlayer(engineEngine_, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateListener_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateListener(engineEngine_, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Create3DGroup_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->Create3DGroup(engineEngine_, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateMetadataExtractor_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateMetadataExtractor(engineEngine_, nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateExtensionObject_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateExtensionObject(engineEngine_, nullptr, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QueryNumSupportedInterfaces_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QueryNumSupportedInterfaces(engineEngine_, 0, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QueryNumSupportedExtensions_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QueryNumSupportedExtensions(engineEngine_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QuerySupportedInterfaces_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QuerySupportedInterfaces(engineEngine_, 0, 0, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QuerySupportedExtension_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QuerySupportedExtension(engineEngine_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_IsExtensionSupported_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->IsExtensionSupported(engineEngine_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoumeItf_001, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_SEEK, &volumeItf_);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoume_001, TestSize.Level0)
{
    SLmillibel level = 0;
    SLresult result = (*volumeItf_)->GetVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoume_002, TestSize.Level0)
{
    SLmillibel level = 0;
    SLresult result = (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetVoume_001, TestSize.Level0)
{
    SLresult result = (*volumeItf_)->SetVolumeLevel(volumeItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetVoume_002, TestSize.Level0)
{
    SLmillibel level = 0;
    (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    SLresult result = (*volumeItf_)->SetVolumeLevel(volumeItf_, level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetBufferQueue_001, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_002, TestSize.Level0)
{
    SLresult result = (*bufferQueueItf_)->RegisterCallback(bufferQueueItf_, BufferQueueCallback, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*bufferQueueItf_)->Clear(bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_001, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_002, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, -1);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetDuration_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetDuration(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_003, TestSize.Level1)
{
    SLresult result = (*playItf_)->RegisterCallback(playItf_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetCallbackEventsMask(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetCallbackEventsMask(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetMarkerPosition(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetMarkerPosition(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPositionUpdatePeriod(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetPositionUpdatePeriod(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_ClearMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->ClearMarkerPosition(playItf_);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetPosition(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_003, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PAUSED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_004, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_005, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(nullptr, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_002, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PLAYING;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_003, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_STOPPED;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_004, TestSize.Level1)
{
    SLuint32 state = -1;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_005, TestSize.Level1)
{
    SLuint32 state = -1;
    SLresult result = (*playItf_)->GetPlayState(nullptr, &state);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_006, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, -1);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_001, TestSize.Level0)
{
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_002, TestSize.Level0)
{
    (*engineObject_)->Destroy(engineObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_003, TestSize.Level0)
{
    (*outputMixObject_)->Destroy(outputMixObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateEngine_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateEngine_002, TestSize.Level1)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        slCreateEngine(nullptr, 0, nullptr, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_DestoryEngine_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        engineObject_ = {};
        slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->Destroy(engineObject_);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_Realize_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    engineObject_ = {};
    slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetInterface_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateOutputMix_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateAudioPlayer_001, TestSize.Level0)
{
    wavFile_ = fopen(TEST_FILE_PATH, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_001: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource, &slSink, 0, nullptr, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
        (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetVolumeLevel_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    SLmillibel level = 0;
    (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->GetVolumeLevel(volumeItf_, &level);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetVolumeLevel_002, TestSize.Level1)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->GetVolumeLevel(volumeItf_, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetMaxVolumeLevel_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    SLmillibel level = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetMaxVolumeLevel_002, TestSize.Level1)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, nullptr);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_SetVolumeLevel_001, TestSize.Level0)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    SLmillibel level = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->SetVolumeLevel(volumeItf_, level);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_SetVolumeLevel_002, TestSize.Level1)
{
    struct timespec tv1 = {0};
    struct timespec tv2 = {0};
    int64_t totalTime = 0;
    SLmillibel level = 0;
    for (int32_t i = 0; i < PERFORMANCE_TEST_TIME; i++) {
        clock_gettime(CLOCK_REALTIME, &tv1);
        (*volumeItf_)->SetVolumeLevel(nullptr, level);
        clock_gettime(CLOCK_REALTIME, &tv2);
        totalTime += tv2.tv_sec * USEC_TIMES + tv2.tv_nsec - (tv1.tv_sec * USEC_TIMES + tv1.tv_nsec);
    }
    EXPECT_TRUE(totalTime <= EXPECT_TIME * PERFORMANCE_TEST_TIME);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetMute_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->SetMute(volumeItf_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetMute_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->GetMute(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_EnableStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->EnableStereoPosition(volumeItf_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_IsEnabledStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->IsEnabledStereoPosition(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->SetStereoPosition(volumeItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->GetStereoPosition(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPriority_001, TestSize.Level1)
{
    SLresult result = (*engineObject_)->GetPriority(engineObject_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPriority_001, TestSize.Level1)
{
    SLresult result = (*engineObject_)->SetPriority(engineObject_, 0, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("Audio_Opensles_CreateAudioPlayer_005 start");
    slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);

    (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    (*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_8,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Realize(pcmPlayerObject_, SL_BOOLEAN_FALSE);
    (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_PLAY, &playItf_);
    (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("Audio_Opensles_SetPlayState_007 start");
    SLmillibel level = 0; // Min Volume
    SLmillibel temp = 2; // Half of MaxVolumeLevel
    SLresult result = (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    level = (SLmillibel) (level / temp);
    result = (*volumeItf_)->SetVolumeLevel(volumeItf_, level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*volumeItf_)->GetVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);

    SLuint32 state = SL_PLAYSTATE_PLAYING;
    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);

    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PAUSED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    usleep(30000); // 30ms sleep time
    result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}
HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("Audio_Opensles_CreateAudioPlayer_006 start");
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_192,
        SL_PCMSAMPLEFORMAT_FIXED_32,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);

    pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::MONO,
        SL_SAMPLINGRATE_88_2,
        SL_PCMSAMPLEFORMAT_FIXED_24,
        0,
        0,
        0
    };
    result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("Audio_Opensles_CreateAudioPlayer_007 start");
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::CHANNEL_3,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_28,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result != SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);

    pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::STEREO,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_20,
        0,
        0,
        0
    };
    result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result != SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);

    pcmFormat = {
        SL_DATAFORMAT_PCM,
        AudioChannel::CHANNEL_4,
        SL_SAMPLINGRATE_88_2,
        -1,
        0,
        0,
        0
    };
    result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result != SL_RESULT_SUCCESS);
}
HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_004, TestSize.Level0)
{
    AUDIO_INFO_LOG("Audio_Opensles_Destroy_004 start");
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
    (*engineObject_)->Destroy(engineObject_);
    (*outputMixObject_)->Destroy(outputMixObject_);
    EXPECT_TRUE(true);
}
} // namespace AudioStandard
} // namespace OHOS
