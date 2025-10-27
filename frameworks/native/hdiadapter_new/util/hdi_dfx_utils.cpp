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
#define LOG_TAG "HdiDfxUtils"
#endif

#include "util/hdi_dfx_utils.h"

namespace OHOS {
namespace AudioStandard {
void HdiDfxUtils::PrintVolumeInfo(char *data, uint64_t &len, const IAudioSourceAttr &attr, std::string logUtilsTag,
    int64_t &volumeDataCount)
{
    BufferDesc buffer = { reinterpret_cast<uint8_t*>(data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr.format), static_cast<AudioChannel>(attr.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag, volumeDataCount);
}

void HdiDfxUtils::DumpData(char *data, uint64_t &len, FILE *dumpFile, std::string dumpFileName)
{
    CHECK_AND_RETURN(AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION);
    DumpFileUtil::WriteDumpFile(dumpFile, static_cast<void *>(data), len);
    AudioCacheMgr::GetInstance().CacheData(dumpFileName, static_cast<void *>(data), len);
}
} // namespace AudioStandard
} // namespace OHOS
