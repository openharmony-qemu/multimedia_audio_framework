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
#define LOG_TAG "WakeupAudioCaptureSource"
#endif

#include "source/wakeup_audio_capture_source.h"
#include "common/hdi_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
WakeupBuffer::WakeupBuffer(IAudioCaptureSource *source)
    : source_(source)
{
    buffer_ = std::make_unique<char[]>(MAX_BUFFER_SIZE);
}

int32_t WakeupBuffer::Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &curWritePos)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (curWritePos < headNum_) {
        curWritePos = headNum_;
    }

    if (curWritePos >= (headNum_ + size_)) {
        if (requestBytes > MAX_BUFFER_SIZE) {
            requestBytes = MAX_BUFFER_SIZE;
        }
        int32_t res = source_->CaptureFrame(frame, requestBytes, replyBytes);
        Offer(frame, replyBytes);
        return res;
    }
    if (requestBytes > size_) { // size != 0
        replyBytes = size_;
    } else {
        replyBytes = requestBytes;
    }

    uint64_t tail = (head_ + size_) % MAX_BUFFER_SIZE;
    if (tail > head_) {
        MemcpyAndCheck(frame, replyBytes, buffer_.get() + head_, replyBytes);
        headNum_ += replyBytes;
        size_ -= replyBytes;
        head_ = (head_ + replyBytes) % MAX_BUFFER_SIZE;
    } else {
        uint64_t copySize = std::min((MAX_BUFFER_SIZE - head_), replyBytes);
        if (copySize != 0) {
            MemcpyAndCheck(frame, replyBytes, buffer_.get() + head_, copySize);
            headNum_ += copySize;
            size_ -= copySize;
            head_ = (head_ + copySize) % MAX_BUFFER_SIZE;
        }

        uint64_t remainCopySize = replyBytes - copySize;
        if (remainCopySize != 0) {
            MemcpyAndCheck(frame + copySize, remainCopySize, buffer_.get(), remainCopySize);
            headNum_ += remainCopySize;
            size_ -= remainCopySize;
            head_ = (head_ + remainCopySize) % MAX_BUFFER_SIZE;
        }
    }

    return SUCCESS;
}

void WakeupBuffer::Offer(const char *frame, const uint64_t bufferBytes)
{
    if ((size_ + bufferBytes) > MAX_BUFFER_SIZE) { // head_ need shift
        uint64_t shift = (size_ + bufferBytes) - MAX_BUFFER_SIZE; // 1 to MAX_BUFFER_SIZE
        headNum_ += shift;
        if (size_ > shift) {
            size_ -= shift;
            head_ = ((head_ + shift) % MAX_BUFFER_SIZE);
        } else {
            size_ = 0;
            head_ = 0;
        }
    }

    uint64_t tail = (head_ + size_) % MAX_BUFFER_SIZE;
    if (tail < head_) {
        MemcpyAndCheck((buffer_.get() + tail), bufferBytes, frame, bufferBytes);
    } else {
        uint64_t copySize = std::min(MAX_BUFFER_SIZE - tail, bufferBytes);
        MemcpyAndCheck((buffer_.get() + tail), MAX_BUFFER_SIZE - tail, frame, copySize);

        if (copySize < bufferBytes) {
            MemcpyAndCheck((buffer_.get()), bufferBytes - copySize, frame + copySize, bufferBytes - copySize);
        }
    }
    size_ += bufferBytes;
}

WakeupAudioCaptureSource::WakeupAudioCaptureSource(const uint32_t captureId)
    : audioCaptureSource_(captureId)
{
}

int32_t WakeupAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (sourceInited_) {
        return SUCCESS;
    }
    curWritePos_ = 0;

    int32_t ret = SUCCESS;
    if (sourceInitCount_ == 0) {
        if (wakeupBuffer_ == nullptr) {
            wakeupBuffer_ = std::make_unique<WakeupBuffer>(&audioCaptureSource_);
        }
        ret = audioCaptureSource_.Init(attr);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    }

    sourceInited_ = true;
    ++sourceInitCount_;
    return SUCCESS;
}

void WakeupAudioCaptureSource::DeInit(void)
{
    AudioXCollie audioXCollie("WakeupAudioCaptureSource::DeInit", TIMEOUT_SECONDS_5,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG);

    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (!sourceInited_) {
        return;
    }
    sourceInited_ = false;
    --sourceInitCount_;
    if (sourceInitCount_ == 0) {
        wakeupBuffer_.reset();
        audioCaptureSource_.DeInit();
    }
}

bool WakeupAudioCaptureSource::IsInited(void)
{
    return sourceInited_;
}

int32_t WakeupAudioCaptureSource::Start(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (started_) {
        return SUCCESS;
    }

    int32_t ret = SUCCESS;
    if (startCount_ == 0) {
        ret = audioCaptureSource_.Start();
    }
    if (ret == SUCCESS) {
        started_ = true;
        ++startCount_;
    }
    return SUCCESS;
}

int32_t WakeupAudioCaptureSource::Stop(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (!started_) {
        return SUCCESS;
    }

    int32_t ret = SUCCESS;
    if (startCount_ == 1) {
        ret = audioCaptureSource_.Stop();
    }
    if (ret == SUCCESS) {
        started_ = false;
        --startCount_;
    }
    return SUCCESS;
}

int32_t WakeupAudioCaptureSource::Resume(void)
{
    return audioCaptureSource_.Resume();
}

int32_t WakeupAudioCaptureSource::Pause(void)
{
    return audioCaptureSource_.Pause();
}

int32_t WakeupAudioCaptureSource::Flush(void)
{
    return audioCaptureSource_.Flush();
}

int32_t WakeupAudioCaptureSource::Reset(void)
{
    return audioCaptureSource_.Reset();
}

int32_t WakeupAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t res = wakeupBuffer_->Poll(frame, requestBytes, replyBytes, curWritePos_);
    curWritePos_ += replyBytes;
    return res;
}

int32_t WakeupAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

std::string WakeupAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t WakeupAudioCaptureSource::SetVolume(float left, float right)
{
    return audioCaptureSource_.SetVolume(left, right);
}

int32_t WakeupAudioCaptureSource::GetVolume(float &left, float &right)
{
    return audioCaptureSource_.GetVolume(left, right);
}

int32_t WakeupAudioCaptureSource::SetMute(bool isMute)
{
    return audioCaptureSource_.SetMute(isMute);
}

int32_t WakeupAudioCaptureSource::GetMute(bool &isMute)
{
    return audioCaptureSource_.GetMute(isMute);
}

uint64_t WakeupAudioCaptureSource::GetTransactionId(void)
{
    return audioCaptureSource_.GetTransactionId();
}

int32_t WakeupAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    return audioCaptureSource_.GetPresentationPosition(frames, timeSec, timeNanoSec);
}

float WakeupAudioCaptureSource::GetMaxAmplitude(void)
{
    return audioCaptureSource_.GetMaxAmplitude();
}

int32_t WakeupAudioCaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    return audioCaptureSource_.SetAudioScene(audioScene);
}

int32_t WakeupAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    return audioCaptureSource_.UpdateActiveDevice(inputDevice);
}

void WakeupAudioCaptureSource::RegistCallback(uint32_t type, IAudioSourceCallback *callback)
{
    return audioCaptureSource_.RegistCallback(type, callback);
}

void WakeupAudioCaptureSource::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback)
{
    return audioCaptureSource_.RegistCallback(type, callback);
}

int32_t WakeupAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    return audioCaptureSource_.UpdateAppsUid(appsUid, size);
}

int32_t WakeupAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return audioCaptureSource_.UpdateAppsUid(appsUid);
}

void WakeupAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: WakeupSource\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

void WakeupAudioCaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

} // namespace AudioStandard
} // namespace OHOS
