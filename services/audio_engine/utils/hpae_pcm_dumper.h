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
#ifndef HPAE_PCM_DUMPER_H
#define HPAE_PCM_DUMPER_H
#include <string>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaePcmDumper {
public:
    explicit HpaePcmDumper(const std::string &filename);
    ~HpaePcmDumper();
    int32_t Dump(const int8_t *buffer, int32_t length);
    bool CheckAndReopenHandle();
private:
    FILE *dumpFile_ = nullptr;
    std::string filename_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif