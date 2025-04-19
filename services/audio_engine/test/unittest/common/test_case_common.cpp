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
#define LOG_TAG "TestCaseCommon"
#endif

#include "test_case_common.h"
#include "hpae_info.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

int32_t WriteFixedDataCb::OnStreamData(AudioCallBackStreamInfo& callBackStremInfo)
{
    size_t sampleSize = GetSizeFromFormat(format_);
    CHECK_AND_RETURN_RET_LOG(sampleSize != 0, SUCCESS, "sampleSize is zero, invalid format");
    for (size_t i = 0; i < callBackStremInfo.requestDataLen / sampleSize; i++) {
        switch (format_) {
            case AudioSampleFormat::SAMPLE_U8: {
                *(callBackStremInfo.inputData + i) = writeNum_;
                break;
            }
            case SAMPLE_S16LE: {
                *((int16_t*)callBackStremInfo.inputData + i) = writeNum_;
                break;
            }
            case SAMPLE_S24LE: {
                uint8_t *p = (uint8_t *)(callBackStremInfo.inputData + OFFSET_BIT_24 * i);
                p[BIT_DEPTH_TWO] = (uint8_t) (writeNum_ >> BIT_16);
                p[1] = (uint8_t) (writeNum_ >> BIT_8);
                p[0] = (uint8_t) writeNum_;
                break;
            }
            case SAMPLE_S32LE: {
                *((int32_t*)callBackStremInfo.inputData + i) = writeNum_;
                break;
            }
            case SAMPLE_F32LE: {
                *((float*)callBackStremInfo.inputData + i) = writeNum_;
                break;
            }
            default:
                break;
        }
    }
    writeNum_++;
    return 0;
}

int32_t WriteFixedValueCb::OnStreamData(AudioCallBackStreamInfo& callBackStremInfo)
{
    size_t sampleSize = GetSizeFromFormat(format_);
    CHECK_AND_RETURN_RET_LOG(sampleSize != 0, SUCCESS, "sampleSize is zero, invalid format");
    for (size_t i = 0; i < callBackStremInfo.requestDataLen / sampleSize; i++) {
        switch (format_) {
            case AudioSampleFormat::SAMPLE_U8: {
                *(callBackStremInfo.inputData + i) = fixValue_;
                break;
            }
            case SAMPLE_S16LE: {
                *((int16_t*)callBackStremInfo.inputData + i) = fixValue_;
                break;
            }
            case SAMPLE_S24LE: {
                uint8_t *p = (uint8_t *)(callBackStremInfo.inputData + OFFSET_BIT_24 * i);
                p[BIT_DEPTH_TWO] = (uint8_t) (fixValue_ >> BIT_16);
                p[1] = (uint8_t) (fixValue_ >> BIT_8);
                p[0] = (uint8_t) fixValue_;
                break;
            }
            case SAMPLE_S32LE: {
                *((int32_t*)callBackStremInfo.inputData + i) = fixValue_;
                break;
            }
            case SAMPLE_F32LE: {
                *((float*)callBackStremInfo.inputData + i) = fixValue_;
                break;
            }
            default:
                break;
        }
    }
    return 0;
}

int32_t WriteIncDataCb::OnStreamData(AudioCallBackStreamInfo& callBackStremInfo)
{
    for (size_t i = 0; i < callBackStremInfo.requestDataLen / GetSizeFromFormat(format_); i++) {
        switch (format_) {
            case AudioSampleFormat::SAMPLE_U8: {
                *(callBackStremInfo.inputData + i) = i;
                break;
            }
            case SAMPLE_S16LE: {
                *((int16_t*)callBackStremInfo.inputData + i) = i;
                break;
            }
            case SAMPLE_S24LE: {
                uint8_t *p = (uint8_t *)(callBackStremInfo.inputData + OFFSET_BIT_24 * i);
                p[BIT_DEPTH_TWO] = (uint8_t) (i >> BIT_16);
                p[1] = (uint8_t) (i >> BIT_8);
                p[0] = (uint8_t) i;
                break;
            }
            case SAMPLE_S32LE: {
                *((int32_t*)callBackStremInfo.inputData + i) = i;
                break;
            }
            case SAMPLE_F32LE: {
                *((float*)callBackStremInfo.inputData + i) = i;
                break;
            }
            default:
                break;
        }
    }
    writeNum_++;
    return 0;
}

void StatusChangeCb::OnStatusUpdate(IOperation operation)
{
    switch (operation) {
        case OPERATION_STARTED:
            status_ = I_STATUS_STARTED;
            break;
        case OPERATION_PAUSED:
            status_ = I_STATUS_PAUSED;
            break;
        case OPERATION_STOPPED:
            status_ = I_STATUS_STOPPED;
            break;
        default:
            status_ = I_STATUS_INVALID;
    }
}

IStatus StatusChangeCb::GetStatus()
{
    return status_;
}

ReadDataCb::ReadDataCb(const std::string &fileName)
{
    testFile_ = fopen(fileName.c_str(), "ab");
    if (testFile_ == nullptr) {
        AUDIO_ERR_LOG("Open file failed");
    }
}

ReadDataCb::~ReadDataCb()
{
    if (testFile_) {
        fclose(testFile_);
        testFile_ = nullptr;
    }
}

int32_t ReadDataCb::OnReadData(size_t length)
{
    AUDIO_WARNING_LOG("ProAudio do not support!");
    return SUCCESS;
}

int32_t ReadDataCb::OnReadData(std::vector<char>& outputData, size_t requestDataLen)
{
    CHECK_AND_RETURN_RET_LOG(testFile_ != nullptr, ERROR, "testFile_ is nullptr");
    fwrite(outputData.data(), 1, requestDataLen, testFile_);
    return SUCCESS;
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS