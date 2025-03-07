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

#ifndef LOG_TAG
#define LOG_TAG "HdiAdapterFactory"
#endif

#include "manager/hdi_adapter_factory.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "util/id_handler.h"
#include "sink/audio_render_sink.h"
#include "sink/bluetooth_audio_render_sink.h"
#include "sink/fast_audio_render_sink.h"
#include "sink/file_audio_render_sink.h"
#include "sink/multichannel_audio_render_sink.h"
#include "sink/offload_audio_render_sink.h"
#include "sink/remote_audio_render_sink.h"
#include "sink/remote_fast_audio_render_sink.h"
#include "source/audio_capture_source.h"
#include "source/bluetooth_audio_capture_source.h"
#include "source/wakeup_audio_capture_source.h"
#include "source/fast_audio_capture_source.h"
#include "source/file_audio_capture_source.h"
#include "source/remote_audio_capture_source.h"
#include "source/remote_fast_audio_capture_source.h"
#include "adapter/local_device_manager.h"
#include "adapter/bluetooth_device_manager.h"
#include "adapter/remote_device_manager.h"

namespace OHOS {
namespace AudioStandard {
HdiAdapterFactory &HdiAdapterFactory::GetInstance(void)
{
    static HdiAdapterFactory instance;
    return instance;
}

std::shared_ptr<IAudioRenderSink> HdiAdapterFactory::CreateRenderSink(uint32_t renderId)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    CHECK_AND_RETURN_RET(idHandler.CheckId(renderId, HDI_ID_BASE_RENDER), nullptr);
    uint32_t type = idHandler.ParseType(renderId);
    std::string info = idHandler.ParseInfo(renderId);

    std::shared_ptr<IAudioRenderSink> sink = nullptr;
    switch (type) {
        case HDI_ID_TYPE_PRIMARY:
            sink = CreatePrimaryRenderSink(renderId, info);
            break;
        case HDI_ID_TYPE_BLUETOOTH:
            sink = CreateBluetoothRenderSink(info);
            break;
        case HDI_ID_TYPE_FAST:
            sink = std::make_shared<FastAudioRenderSink>();
            break;
        case HDI_ID_TYPE_FILE:
            sink = std::make_shared<FileAudioRenderSink>();
            break;
        case HDI_ID_TYPE_MULTICHANNEL:
            sink = std::make_shared<MultichannelAudioRenderSink>();
            break;
        case HDI_ID_TYPE_OFFLOAD:
            sink = std::make_shared<OffloadAudioRenderSink>();
            break;
        case HDI_ID_TYPE_REMOTE:
            sink = CreateRemoteRenderSink(info);
            break;
        case HDI_ID_TYPE_REMOTE_FAST:
            sink = CreateRemoteFastRenderSink(info);
            break;
        default:
            AUDIO_ERR_LOG("invalid type");
            break;
    }
    return sink;
}

std::shared_ptr<IAudioCaptureSource> HdiAdapterFactory::CreateCaptureSource(uint32_t captureId)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    CHECK_AND_RETURN_RET(idHandler.CheckId(captureId, HDI_ID_BASE_CAPTURE), nullptr);
    uint32_t type = idHandler.ParseType(captureId);
    std::string info = idHandler.ParseInfo(captureId);

    std::shared_ptr<IAudioCaptureSource> source = nullptr;
    switch (type) {
        case HDI_ID_TYPE_PRIMARY:
            source = CreatePrimaryCaptureSource(captureId, info);
            break;
        case HDI_ID_TYPE_BLUETOOTH:
            source = std::make_shared<BluetoothAudioCaptureSource>(captureId);
            break;
        case HDI_ID_TYPE_WAKEUP:
            source = std::make_shared<WakeupAudioCaptureSource>(captureId);
            break;
        case HDI_ID_TYPE_FAST:
            source = std::make_shared<FastAudioCaptureSource>();
            break;
        case HDI_ID_TYPE_FILE:
            source = std::make_shared<FileAudioCaptureSource>();
            break;
        case HDI_ID_TYPE_REMOTE:
            source = CreateRemoteCaptureSource(info);
            break;
        case HDI_ID_TYPE_REMOTE_FAST:
            source = CreateRemoteFastCaptureSource(info);
            break;
        default:
            AUDIO_ERR_LOG("invalid type");
            break;
    }
    return source;
}

std::shared_ptr<IDeviceManager> HdiAdapterFactory::CreateDeviceManager(uint32_t type)
{
    std::shared_ptr<IDeviceManager> deviceManager = nullptr;
    switch (type) {
        case HDI_DEVICE_MANAGER_TYPE_LOCAL:
            deviceManager = std::make_shared<LocalDeviceManager>();
            break;
        case HDI_DEVICE_MANAGER_TYPE_BLUETOOTH:
            deviceManager = std::make_shared<BluetoothDeviceManager>();
            break;
        case HDI_DEVICE_MANAGER_TYPE_REMOTE:
            deviceManager = std::make_shared<RemoteDeviceManager>();
            break;
        default:
            AUDIO_ERR_LOG("invalid type");
            break;
    }
    return deviceManager;
}

std::shared_ptr<IAudioRenderSink> HdiAdapterFactory::CreatePrimaryRenderSink(const uint32_t renderId,
    const std::string &info)
{
    if (info == HDI_ID_INFO_DIRECT || info == HDI_ID_INFO_VOIP || info == HDI_ID_INFO_DP ||
        info == HDI_ID_INFO_USB) {
        return std::make_shared<AudioRenderSink>(renderId, info);
    }
    return std::make_shared<AudioRenderSink>(renderId);
}

std::shared_ptr<IAudioRenderSink> HdiAdapterFactory::CreateBluetoothRenderSink(const std::string &info)
{
    if (info == HDI_ID_INFO_MMAP) {
        return std::make_shared<BluetoothAudioRenderSink>(true);
    }
    return std::make_shared<BluetoothAudioRenderSink>();
}

std::shared_ptr<IAudioRenderSink> HdiAdapterFactory::CreateRemoteRenderSink(const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!info.empty(), nullptr, "deviceNetworkId is nullptr");
    return std::make_shared<RemoteAudioRenderSink>(info);
}

std::shared_ptr<IAudioRenderSink> HdiAdapterFactory::CreateRemoteFastRenderSink(const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!info.empty(), nullptr, "deviceNetworkId is nullptr");
    return std::make_shared<RemoteFastAudioRenderSink>(info);
}

std::shared_ptr<IAudioCaptureSource> HdiAdapterFactory::CreatePrimaryCaptureSource(const uint32_t captureId,
    const std::string &info)
{
    if (info == HDI_ID_INFO_USB) {
        return std::make_shared<AudioCaptureSource>(captureId, info);
    }
    return std::make_shared<AudioCaptureSource>(captureId);
}

std::shared_ptr<IAudioCaptureSource> HdiAdapterFactory::CreateRemoteCaptureSource(const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!info.empty(), nullptr, "deviceNetworkId is nullptr");
    return std::make_shared<RemoteAudioCaptureSource>(info);
}

std::shared_ptr<IAudioCaptureSource> HdiAdapterFactory::CreateRemoteFastCaptureSource(const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!info.empty(), nullptr, "deviceNetworkId is nullptr");
    return std::make_shared<RemoteFastAudioCaptureSource>(info);
}

} // namespace AudioStandard
} // namespace OHOS
