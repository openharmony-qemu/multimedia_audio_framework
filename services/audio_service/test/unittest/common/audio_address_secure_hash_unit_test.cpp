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

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include "audio_address_secure_hash.h"

using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace {
constexpr int32_t NUMBER_FOUR = 0;

// Convert a 32-byte digest into its lowercase hex string representation.
std::string ToHex(const unsigned char *md)
{
    static const char *hexChars = "0123456789abcdef";
    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH * 2); // 2 hex chars per byte
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        result.push_back(hexChars[(md[i] >> NUMBER_FOUR) & 0x0F]);
        result.push_back(hexChars[md[i] & 0x0F]);
    }
    return result;
}

const unsigned char *AsBytes(const std::string &s)
{
    return reinterpret_cast<const unsigned char *>(s.c_str());
}
} // namespace

class AudioAddressSecureHashTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

// NIST FIPS 180-4 / RFC 6234 known-answer vectors.
static const std::string SHA256_OF_EMPTY =
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
static const std::string SHA256_OF_ABC =
    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
// 56-byte message: forces the two-block padding branch (len > PAD_LENGTH).
static const std::string SHA256_OF_LONG =
    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
static const std::string LONG_MSG = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

/**
 * @tc.name: AudioAddressSecureHash_Empty_001
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_Empty_001
 * @tc.desc: SHA-256 of an empty message matches the NIST known-answer vector.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_Empty_001, TestSize.Level0)
{
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(""), 0, md);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ToHex(md), SHA256_OF_EMPTY);
    EXPECT_EQ(ToHex(ret), SHA256_OF_EMPTY);
    EXPECT_EQ(ret, md); // when md != nullptr, the returned pointer equals md
}

/**
 * @tc.name: AudioAddressSecureHash_Abc_002
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_Abc_002
 * @tc.desc: SHA-256 of "abc" matches the NIST known-answer vector.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_Abc_002, TestSize.Level0)
{
    std::string input = "abc";
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(input), input.size(), md);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ToHex(md), SHA256_OF_ABC);
}

/**
 * @tc.name: AudioAddressSecureHash_Long_003
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_Long_003
 * @tc.desc: SHA-256 of a 56-byte message (two-block padding path) matches the NIST vector.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_Long_003, TestSize.Level0)
{
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(LONG_MSG), LONG_MSG.size(), md);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ToHex(md), SHA256_OF_LONG);
}

/**
 * @tc.name: AudioAddressSecureHash_DefaultBuffer_004
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_DefaultBuffer_004
 * @tc.desc: When md is nullptr, the internal thread-local buffer is used and the result is still correct.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_DefaultBuffer_004, TestSize.Level0)
{
    std::string input = "abc";
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(input), input.size(), nullptr);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ToHex(ret), SHA256_OF_ABC);
}

/**
 * @tc.name: AudioAddressSecureHash_IllegalParam_005
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_IllegalParam_005
 * @tc.desc: A null data pointer with non-zero length is rejected (returns nullptr).
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_IllegalParam_005, TestSize.Level0)
{
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(nullptr, 5, md);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AudioAddressSecureHash_NullDataZeroLen_006
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_NullDataZeroLen_006
 * @tc.desc: A null data pointer with zero length is valid and yields the empty-message digest.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_NullDataZeroLen_006, TestSize.Level0)
{
    unsigned char md[SHA256_DIGEST_LENGTH] = {0};
    unsigned char *ret = AudioAddressSecureHash::AudioSecureHashAlgo(nullptr, 0, md);
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ToHex(md), SHA256_OF_EMPTY);
}

/**
 * @tc.name: AudioAddressSecureHash_Determinism_007
 * @tc.type: FUNC
 * @tc.number: AudioAddressSecureHash_Determinism_007
 * @tc.desc: Repeated hashing of the same input is deterministic, and different inputs differ.
 */
HWTEST_F(AudioAddressSecureHashTest, AudioAddressSecureHash_Determinism_007, TestSize.Level0)
{
    std::string input = "AA:BB:CC:DD:EE:FF";
    unsigned char md1[SHA256_DIGEST_LENGTH] = {0};
    unsigned char md2[SHA256_DIGEST_LENGTH] = {0};
    ASSERT_NE(AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(input), input.size(), md1), nullptr);
    ASSERT_NE(AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes(input), input.size(), md2), nullptr);
    EXPECT_EQ(memcmp(md1, md2, SHA256_DIGEST_LENGTH), 0);
    EXPECT_EQ(ToHex(md1).size(), 64U); // 32 bytes -> 64 hex chars

    unsigned char mdOther[SHA256_DIGEST_LENGTH] = {0};
    ASSERT_NE(AudioAddressSecureHash::AudioSecureHashAlgo(AsBytes("abc"), 3, mdOther), nullptr);
    EXPECT_NE(memcmp(md1, mdOther, SHA256_DIGEST_LENGTH), 0);
}
