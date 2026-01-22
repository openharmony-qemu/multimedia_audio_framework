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
#ifndef AUDIO_SECURE_HASH_H
#define AUDIO_SECURE_HASH_H
#include <cstddef>
#include <cstdint>

#define SHA256_DIGEST_LENGTH 32

namespace OHOS {
namespace AudioStandard {
class AudioSecureHash {
public:
    static unsigned char *AudioSecureHashAlgo(const unsigned char *d, size_t n, unsigned char *md);
private:
    AudioSecureHash() = delete;
};
}
}
#endif