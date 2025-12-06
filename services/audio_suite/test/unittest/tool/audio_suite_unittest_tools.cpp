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

#include "audio_suite_unittest_tools.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

bool CreateOutputPcmFile(const std::string &filename)
{
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }

    std::ofstream ofs;
    ofs.open(filename, std::ios::out | std::ios::trunc);
    CHECK_AND_RETURN_RET_LOG(ofs.is_open(), false, "Failed to open output file: %{public}s", filename.c_str());
    ofs.close();
    return true;
}

bool WritePcmFile(const std::string &filename, const uint8_t *data, size_t dataSize)
{
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    if (!file.write(reinterpret_cast<const char *>(data), dataSize)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool IsFilesEqual(const std::string &filename1, const std::string &filename2)
{
    std::ifstream file1(filename1, std::ios::binary);
    std::ifstream file2(filename2, std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        return false;
    }

    file1.seekg(0, std::ios::end);
    file2.seekg(0, std::ios::end);
    if (file1.tellg() != file2.tellg()) {
        return false;
    }

    file1.seekg(0, std::ios::beg);
    file2.seekg(0, std::ios::beg);

    char buffer1[4096];
    char buffer2[4096];
    size_t bytesRead;

    do {
        file1.read(buffer1, sizeof(buffer1));
        file2.read(buffer2, sizeof(buffer2));
        bytesRead = file1.gcount();
        if (bytesRead != file2.gcount()) {
            return false;
        }
        if (std::memcmp(buffer1, buffer2, bytesRead) != 0) {
            return false;
        }
    } while (bytesRead > 0);

    return true;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS