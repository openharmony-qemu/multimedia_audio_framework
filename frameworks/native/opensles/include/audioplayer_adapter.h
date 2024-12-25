/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIOPLAYER_ADAPTER_H
#define AUDIOPLAYER_ADAPTER_H

#include <OpenSLES.h>
#include <OpenSLES_Platform.h>
#include <iostream>
#include <map>
#include <audio_renderer.h>
#include <audio_system_manager.h>
#include <readorwritecallback_adapter.h>

namespace OHOS {
namespace AudioStandard {
class AudioPlayerAdapter {
public:
    static AudioPlayerAdapter* GetInstance();
    std::shared_ptr<AudioRenderer> GetAudioRenderById(SLuint32 id);
    void EraseAudioRenderById(SLuint32 id);
    SLresult CreateAudioPlayerAdapter
        (SLuint32 id, SLDataSource *dataSource, SLDataSink *dataSink, AudioStreamType streamType);
    SLresult SetPlayStateAdapter(SLuint32 id, SLuint32 state);
    SLresult GetPlayStateAdapter(SLuint32 id, SLuint32 *state);
    SLresult SetVolumeLevelAdapter(SLuint32 id, SLmillibel level);
    SLresult GetVolumeLevelAdapter(SLuint32 id, SLmillibel *level);
    SLresult GetMaxVolumeLevelAdapter(SLuint32 id, SLmillibel *level);
    SLresult EnqueueAdapter(SLuint32 id, const void *buffer, SLuint32 size);
    SLresult ClearAdapter(SLuint32 id);
    SLresult GetStateAdapter(SLuint32 id, SLOHBufferQueueState *state);
    SLresult GetBufferAdapter(SLuint32 id, SLuint8 **buffer, SLuint32 *size);
    SLresult RegisterCallbackAdapter(SLOHBufferQueueItf itf, SlOHBufferQueueCallback callback, void *pContext);
    
private:
    AudioPlayerAdapter();
    ~AudioPlayerAdapter();
    const float MAGNIFICATION = 2000;
    std::map<SLuint32, std::shared_ptr<AudioRenderer>> renderMap_;
    std::shared_ptr<ReadOrWriteCallbackAdapter> callbackPtr_;
    std::map<SLuint32, std::shared_ptr<ReadOrWriteCallbackAdapter>> callbackMap_;

    void ConvertPcmFormat(SLDataFormat_PCM *slFormat, AudioRendererParams *rendererParams);
    AudioSampleFormat SlToOhosSampelFormat(SLDataFormat_PCM *pcmFormat);
    AudioSamplingRate SlToOhosSamplingRate(SLDataFormat_PCM *pcmFormat);
    AudioChannel SlToOhosChannel(SLDataFormat_PCM *pcmFormat);
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_SINK_H
