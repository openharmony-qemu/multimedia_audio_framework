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

#include <cinttypes>
#include <ostream>
#include <iostream>
#include <thread>
#include <stdint.h>
#include <time.h>

#include "audio_service_log.h"
#include "audio_utils.h"
#include "fast/fast_audio_renderer_sink.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

class AudioHdiDeviceTest {
public:
    void RenderFrameFromFile()
    {
        if (hdiRenderSink_ == nullptr) {
            AUDIO_ERR_LOG("RenderFrameFromFile hdiRenderSink_ null");
            return;
        }

        int fd = 0;
        uint32_t totalSizeInframe = 0;
        uint32_t spanSizeInframe = 0;
        uint32_t byteSizePerFrame = 0;

        hdiRenderSink_->GetMmapBufferInfo(fd, totalSizeInframe, spanSizeInframe, byteSizePerFrame);
        if (byteSizePerFrame == 0) {
            AUDIO_ERR_LOG("RenderFrameFromFile():byteSizePerFrame is zero");
            return;
        }
        if (spanSizeInframe > SIZE_MAX / byteSizePerFrame) {
            AUDIO_ERR_LOG("RenderFrameFromFile():data overflow");
            return;
        }
        size_t tempBufferSize = spanSizeInframe * byteSizePerFrame;
        char *buffer = (char *)malloc(tempBufferSize);
        if (buffer == nullptr) {
            AUDIO_ERR_LOG("AudioHdiDeviceTest: failed to malloc");
            cout << "failed to get buffer" << endl;
            return;
        }

        uint64_t frameCount = 0;
        int64_t timeSec = 0;
        int64_t timeNanoSec = 0;

        int64_t periodNanoSec = 5000000; // 5ms
        int64_t fwkSyncTime = ClockTime::GetCurNano();

        uint64_t written = 0;
        int32_t ret = 0;
        uint64_t writeCount = 0;
        while (!stopThread && !feof(wavFile)) {
            Trace trace1("read_write");
            if (writeCount == 0) {
                hdiRenderSink_->GetMmapHandlePosition(frameCount, timeSec, timeNanoSec);
                int64_t temp = timeNanoSec + timeSec * AUDIO_NS_PER_SECOND;
                fwkSyncTime = temp;
            }
            writeCount++;

            fread(buffer, 1, tempBufferSize, wavFile);
            ret = hdiRenderSink_->RenderFrame(*buffer, tempBufferSize, written);

            int64_t writeTime = fwkSyncTime + writeCount * periodNanoSec + deltaTime;
            trace1.End();
            ClockTime::AbsoluteSleep(writeTime);
        }
        free(buffer);
    }

    bool InitHdiRender()
    {
        hdiRenderSink_ = FastAudioRendererSink::GetInstance();
        IAudioSinkAttr attr = {};
        attr.adapterName = "primary";
        attr.sampleRate = 48000; // 48000hz
        attr.channel = 2; // two channel
        attr.format = HdiAdapterFormat::SAMPLE_S16;

        hdiRenderSink_->Init(attr);

        return true;
    }

    void StartHdiRender(int32_t time)
    {
        if (hdiRenderSink_ == nullptr) {
            AUDIO_ERR_LOG("StartHdiRender hdiRenderSink_ null");
            return;
        }

        int32_t ret = hdiRenderSink_->Start();
        AUDIO_INFO_LOG("AudioHdiDeviceTest Start, ret %{public}d", ret);
        float vol = 0.12; // for test
        ret = hdiRenderSink_->SetVolume(vol, vol); // volume
        AUDIO_INFO_LOG("AudioHdiDeviceTest set volume to 0.5, ret %{public}d", ret);

        timeThread_ = make_unique<thread>(&AudioHdiDeviceTest::RenderFrameFromFile, this);

        sleep(time);

        cout << "stop running" << endl;
        stopThread = true;
        timeThread_->join();
        hdiRenderSink_->Stop();
        hdiRenderSink_->DeInit();
    }

    bool TestPlayback(int argc, char *argv[])
    {
        AUDIO_INFO_LOG("TestPlayback in");

        wav_hdr wavHeader;
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        int32_t time = strtol(argv[2], nullptr, 10);
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            AUDIO_ERR_LOG("Invalid path");
            return false;
        }
        AUDIO_INFO_LOG("AudioHdiDeviceTest: path = %{public}s", path);
        wavFile = fopen(path, "rb");
        if (wavFile == nullptr) {
            AUDIO_INFO_LOG("AudioHdiDeviceTest: Unable to open wave file");
            return false;
        }
        size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
        AUDIO_INFO_LOG("AudioHdiDeviceTest: Header Read in bytes %{public}zu", bytesRead);

        InitHdiRender();
        StartHdiRender(time);

        return true;
    }
private:
    IMmapAudioRendererSink *hdiRenderSink_ = nullptr;
    unique_ptr<thread> timeThread_ = nullptr;
    int64_t deltaTime = 4000000; // 4ms
    bool stopThread = false;
    FILE* wavFile = nullptr;
};

// usage: audio_hdi_device_test /data/data/xxx.pcm 5
int main(int argc, char *argv[])
{
    AUDIO_INFO_LOG("AudioHdiDeviceTest: Render test in");

    if (argv == nullptr) {
        AUDIO_ERR_LOG("AudioHdiDeviceTest: argv is null");
        return 0;
    }

    int32_t argsCountThree_ = 3;
    if (argc != argsCountThree_) {
        AUDIO_ERR_LOG("AudioHdiDeviceTest: incorrect argc. Enter 3 args");
        cout << "AudioHdiDeviceTest: incorrect argc" << endl;
        return 0;
    }

    AUDIO_INFO_LOG("AudioHdiDeviceTest: argc=%{public}d", argc);
    AUDIO_INFO_LOG("file path argv[1]=%{public}s", argv[1]);

    AudioHdiDeviceTest testObj;
    bool ret = testObj.TestPlayback(argc, argv);

    return ret;
}
