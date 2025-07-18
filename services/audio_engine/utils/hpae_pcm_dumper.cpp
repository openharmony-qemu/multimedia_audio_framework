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

#include <unordered_map>
#include "hpae_pcm_dumper.h"
#include "audio_dump_pcm.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "securec.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaePcmDumper::HpaePcmDumper(const std::string &filename)
{
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, filename, &dumpFile_);
    filename_ = filename;
}

HpaePcmDumper::~HpaePcmDumper()
{
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

int32_t HpaePcmDumper::Dump(const int8_t *buffer, int32_t length)
{
    DumpFileUtil::WriteDumpFile(dumpFile_, (void *)(buffer), length);
    AudioCacheMgr::GetInstance().CacheData(filename_, (void *)(buffer), length);
    return SUCCESS;
}

bool HpaePcmDumper::CheckAndReopenHandle()
{
    if (dumpFile_ != nullptr) {
        return true;
    } else {
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, filename_, &dumpFile_);
        AUDIO_DEBUG_LOG("Reopen dump file: %{public}s", filename_.c_str());
    }
    return false;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
