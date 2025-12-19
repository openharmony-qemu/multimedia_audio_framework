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

#include "sink/i_audio_render_sink.h"

#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

void IAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
}

void IAudioRenderSink::RegistCallback(uint32_t type, std::shared_ptr<IAudioSinkCallback> callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
}

void IAudioRenderSink::NotifyStreamChangeToSink(StreamChangeType change,
    uint32_t streamId, StreamUsage usage, RendererState state)
{
    ChangePipeStream(change, streamId, usage, state);
}

std::shared_ptr<AudioOutputPipeInfo> IAudioRenderSink::GetOutputPipeInfo()
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    CHECK_AND_RETURN_RET(pipeInfo_ != nullptr, nullptr);
    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    return copyPipe;
}

void IAudioRenderSink::InitPipeInfo(uint32_t id, HdiAdapterType adapter, uint32_t routeFlag,
    std::vector<DeviceType> devices)
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    pipeInfo_ = std::make_shared<AudioOutputPipeInfo>(id, adapter, routeFlag);
    pipeInfo_->SetStatus(PIPE_STATUS_OPEN);
    pipeInfo_->SetDevices(devices);

    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    callback_.OnOutputPipeChange(PIPE_CHANGE_TYPE_PIPE_STATUS, copyPipe);
}

void IAudioRenderSink::ChangePipeStatus(AudioPipeStatus state)
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    CHECK_AND_RETURN_LOG(pipeInfo_ != nullptr, "pipe info not inited");
    pipeInfo_->SetStatus(state);

    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    callback_.OnOutputPipeChange(PIPE_CHANGE_TYPE_PIPE_STATUS, copyPipe);
}

void IAudioRenderSink::ChangePipeDevice(const std::vector<DeviceType> &devices)
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    CHECK_AND_RETURN_LOG(pipeInfo_ != nullptr, "pipe info not inited");
    pipeInfo_->SetDevices(devices);

    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    callback_.OnOutputPipeChange(PIPE_CHANGE_TYPE_PIPE_DEVICE, copyPipe);
}

void IAudioRenderSink::ChangePipeStream(StreamChangeType change,
    uint32_t streamId, StreamUsage usage, RendererState state)
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    CHECK_AND_RETURN_LOG(pipeInfo_ != nullptr, "pipe info not inited");

    switch (change) {
        case STREAM_CHANGE_TYPE_ADD:
            pipeInfo_->AddStream(streamId, usage, state);
            break;
        case STREAM_CHANGE_TYPE_REMOVE:
            pipeInfo_->RemoveStream(streamId);
            break;
        case STREAM_CHANGE_TYPE_REMOVE_ALL:
            pipeInfo_->RemoveAllStreams();
            break;
        case STREAM_CHANGE_TYPE_STATE_CHANGE:
            pipeInfo_->UpdateStream(streamId, state);
            break;
        default:
            return;
    }

    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    callback_.OnOutputPipeChange(PIPE_CHANGE_TYPE_PIPE_STREAM, copyPipe);
}

void IAudioRenderSink::DeinitPipeInfo()
{
    std::lock_guard<std::mutex> lock(pipeLock_);
    CHECK_AND_RETURN_LOG(pipeInfo_ != nullptr, "pipe info not inited");
    pipeInfo_->RemoveAllStreams();
    pipeInfo_->SetStatus(PIPE_STATUS_CLOSE);

    auto copyPipe = std::make_shared<AudioOutputPipeInfo>(*pipeInfo_);
    callback_.OnOutputPipeChange(PIPE_CHANGE_TYPE_PIPE_STATUS, copyPipe);

    // clear pipe for get func
    pipeInfo_ = nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
