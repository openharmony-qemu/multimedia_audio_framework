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
#include "audio_secure_hash.h"
#include <string>
#include <vector>
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
static constexpr size_t DIGEST_SIZE = SHA256_DIGEST_LENGTH;
class AudioSecureHashUnitTest : public testing::Test {
public:

    static void SetUpTestCase(void) {}

    static void TearDownTestCase(void) {}

    void SetUp(void) {}

    void TearDown(void) {}
};

static std::string BytesToHex(const unsigned char* bytes, size_t length) {
    static const char hex_digits[] = "0123456789abcdef";
    std::string hex;
    hex.reserve(length * 2);
    for (size_t i = 0; i < length; ++i) {
        hex.push_back(hex_digits[(bytes[i] >> 4) & 0xF]);
        hex.push_back(hex_digits[bytes[i] & 0xF]);
    }
    return hex;
}

/**
 * @tc.name: AudioSecureHashBasicTest001
 * @tc.desc: Test basic SHA256 hash functionality with empty input
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashBasicTest001, TestSize.Level4)
{
    // Test empty input
    const char* emptyInput = "";
    unsigned char hash[DIGEST_SIZE] = {0};
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(emptyInput), 0, hash);
    
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, hash);
    
    // Expected SHA256 hash of empty string
    const char* expectedHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string calculatedHash = BytesToHex(hash, DIGEST_SIZE);
    EXPECT_STREQ(calculatedHash.c_str(), expectedHash);
}

/**
 * @tc.name: AudioSecureHashNullPointerTest001
 * @tc.desc: Test null pointer handling - null data with non-zero length should return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashNullPointerTest001, TestSize.Level4)
{
    unsigned char hash[DIGEST_SIZE];
    // Test case: data is nullptr but length > 0 (should return nullptr)
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(nullptr, 10, hash);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AudioSecureHashNullPointerTest002
 * @tc.desc: Test null pointer handling - null data with zero length should succeed
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashNullPointerTest002, TestSize.Level4)
{
    unsigned char hash[DIGEST_SIZE] = {0};
    // Test case: data is nullptr and length is 0 (should succeed)
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(nullptr, 0, hash);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result, hash);
}

/**
 * @tc.name: AudioSecureHashStaticBufferTest001
 * @tc.desc: Test static buffer usage when output buffer is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashStaticBufferTest001, TestSize.Level4)
{
    const char* testData = "test";
    // First call with nullptr output buffer
    unsigned char* result1 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(testData), 4, nullptr);
    EXPECT_NE(result1, nullptr);
    
    // Second call with nullptr output buffer should return same static buffer
    unsigned char* result2 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(testData), 4, nullptr);
    EXPECT_NE(result2, nullptr);
    
    // Both results should be identical
    EXPECT_EQ(memcmp(result1, result2, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashNISTVectorsTest001
 * @tc.desc: Test NIST standard test vectors for SHA256
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashNISTVectorsTest001, TestSize.Level4)
{
    struct TestVector {
        const char* input;
        const char* expectedHash;
    };
    
    TestVector testVectors[] = {
        {"abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
        {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
         "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"},
    };
    
    for (size_t i = 0; i < sizeof(testVectors) / sizeof(testVectors[0]); i++) {
        size_t inputLen = strlen(testVectors[i].input);
        unsigned char hash[DIGEST_SIZE];
        
        unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
            reinterpret_cast<const unsigned char*>(testVectors[i].input),
            inputLen,
            hash);
        
        EXPECT_NE(result, nullptr);
        
        std::string calculatedHash = BytesToHex(hash, DIGEST_SIZE);
        EXPECT_STREQ(calculatedHash.c_str(), testVectors[i].expectedHash)
            << "Failed for test vector " << i << ": " << testVectors[i].input;
    }
}

/**
 * @tc.name: AudioSecureHashBoundaryTest001
 * @tc.desc: Test boundary conditions around block size (56-57 bytes)
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashBoundaryTest001, TestSize.Level4)
{
    // Test case 1: Exactly 56 bytes (single block padding)
    std::string data56(56, 'A');
    unsigned char hash56[DIGEST_SIZE];
    unsigned char* result56 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data56.c_str()),
        56,
        hash56);
    EXPECT_NE(result56, nullptr);
    
    // Test case 2: Exactly 57 bytes (requires extra block for padding)
    std::string data57(57, 'A');
    unsigned char hash57[DIGEST_SIZE];
    unsigned char* result57 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data57.c_str()),
        57,
        hash57);
    EXPECT_NE(result57, nullptr);
    
    // 56-byte and 57-byte inputs should produce different hashes
    EXPECT_NE(memcmp(hash56, hash57, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashBoundaryTest002
 * @tc.desc: Test boundary conditions around block size (63-64-65 bytes)
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashBoundaryTest002, TestSize.Level4)
{
    // Test case 1: Exactly 63 bytes
    std::string data63(63, 'B');
    unsigned char hash63[DIGEST_SIZE];
    unsigned char* result63 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data63.c_str()),
        63,
        hash63);
    EXPECT_NE(result63, nullptr);
    
    // Test case 2: Exactly 64 bytes (one full block)
    std::string data64(64, 'B');
    unsigned char hash64[DIGEST_SIZE];
    unsigned char* result64 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data64.c_str()),
        64,
        hash64);
    EXPECT_NE(result64, nullptr);
    
    // Test case 3: Exactly 65 bytes (one block + one byte)
    std::string data65(65, 'B');
    unsigned char hash65[DIGEST_SIZE];
    unsigned char* result65 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data65.c_str()),
        65,
        hash65);
    EXPECT_NE(result65, nullptr);
    
    // All three should produce different hashes
    EXPECT_NE(memcmp(hash63, hash64, DIGEST_SIZE), 0);
    EXPECT_NE(memcmp(hash64, hash65, DIGEST_SIZE), 0);
    EXPECT_NE(memcmp(hash63, hash65, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashPartialBufferTest001
 * @tc.desc: Test partial buffer filling scenario
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashPartialBufferTest001, TestSize.Level4)
{
    // Create data that will partially fill buffer multiple times
    // First chunk: 30 bytes
    std::string chunk1(30, 'X');
    // Second chunk: 20 bytes (30 + 20 = 50 < 64, stays in buffer)
    std::string chunk2(20, 'Y');
    // Third chunk: 20 bytes (50 + 20 = 70 > 64, processes block)
    std::string chunk3(20, 'Z');
    
    std::string combinedData = chunk1 + chunk2 + chunk3;
    unsigned char hash[DIGEST_SIZE];
    
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(combinedData.c_str()),
        combinedData.length(),
        hash);
    
    EXPECT_NE(result, nullptr);
    EXPECT_NE(hash[0], 0); // Verify hash is non-zero
}

/**
 * @tc.name: AudioSecureHashMultipleBlocksTest001
 * @tc.desc: Test processing of multiple full blocks
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashMultipleBlocksTest001, TestSize.Level4)
{
    // Create data that is exactly 3 blocks (192 bytes)
    const size_t BLOCK_SIZE = 64;
    const size_t BLOCK_COUNT = 3;
    std::string data(BLOCK_SIZE * BLOCK_COUNT, 'M');
    
    unsigned char hash[DIGEST_SIZE];
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        hash);
    
    EXPECT_NE(result, nullptr);
    
    // Verify hash consistency
    unsigned char hash2[DIGEST_SIZE];
    unsigned char* result2 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        hash2);
    
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(memcmp(hash, hash2, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashLengthEncodingTest001
 * @tc.desc: Test message length encoding for different sizes
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashLengthEncodingTest001, TestSize.Level4)
{
    // Test various message lengths to ensure length encoding works correctly
    const size_t testLengths[] = {0, 1, 255, 256, 1000, 10000};
    
    for (size_t length : testLengths) {
        // Create test data of specified length
        std::vector<unsigned char> data(length);
        for (size_t i = 0; i < length; i++) {
            data[i] = static_cast<unsigned char>(i & 0xFF);
        }
        
        unsigned char hash[DIGEST_SIZE];
        unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
            data.data(), length, hash);
        
        EXPECT_NE(result, nullptr) << "Failed for length: " << length;
        
        // Verify same input produces same output
        unsigned char hash2[DIGEST_SIZE];
        unsigned char* result2 = AudioSecureHash::AudioSecureHashAlgo(
            data.data(), length, hash2);
        
        EXPECT_NE(result2, nullptr);
        EXPECT_EQ(memcmp(hash, hash2, DIGEST_SIZE), 0);
    }
}

/**
 * @tc.name: AudioSecureHashConsistencyTest001
 * @tc.desc: Test hash consistency for same input data
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashConsistencyTest001, TestSize.Level4)
{
    const char* testData = "Consistency test data for SHA256 hash algorithm";
    size_t dataLength = strlen(testData);
    
    // Calculate hash multiple times
    unsigned char hash1[DIGEST_SIZE];
    unsigned char hash2[DIGEST_SIZE];
    unsigned char hash3[DIGEST_SIZE];
    
    unsigned char* result1 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(testData), dataLength, hash1);
    unsigned char* result2 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(testData), dataLength, hash2);
    unsigned char* result3 = AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(testData), dataLength, hash3);
    
    EXPECT_NE(result1, nullptr);
    EXPECT_NE(result2, nullptr);
    EXPECT_NE(result3, nullptr);
    
    // All three hashes should be identical
    EXPECT_EQ(memcmp(hash1, hash2, DIGEST_SIZE), 0);
    EXPECT_EQ(memcmp(hash2, hash3, DIGEST_SIZE), 0);
    EXPECT_EQ(memcmp(hash1, hash3, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashDifferentInputTest001
 * @tc.desc: Test that different inputs produce different hashes
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashDifferentInputTest001, TestSize.Level4)
{
    const char* data1 = "First test message";
    const char* data2 = "Second test message";
    const char* data3 = "First test message "; // Extra space at end
    
    unsigned char hash1[DIGEST_SIZE];
    unsigned char hash2[DIGEST_SIZE];
    unsigned char hash3[DIGEST_SIZE];
    
    AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data1), strlen(data1), hash1);
    AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data2), strlen(data2), hash2);
    AudioSecureHash::AudioSecureHashAlgo(
        reinterpret_cast<const unsigned char*>(data3), strlen(data3), hash3);
    
    // All three should be different
    EXPECT_NE(memcmp(hash1, hash2, DIGEST_SIZE), 0);
    EXPECT_NE(memcmp(hash1, hash3, DIGEST_SIZE), 0);
    EXPECT_NE(memcmp(hash2, hash3, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashBinaryDataTest001
 * @tc.desc: Test SHA256 with binary data containing null bytes
 * @tc.type: FUNC
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashBinaryDataTest001, TestSize.Level4)
{
    // Create binary data with null bytes and special characters
    unsigned char binaryData[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    
    unsigned char hash[DIGEST_SIZE];
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
        binaryData, sizeof(binaryData), hash);
    
    EXPECT_NE(result, nullptr);
    
    // Calculate hash again to verify consistency
    unsigned char hash2[DIGEST_SIZE];
    unsigned char* result2 = AudioSecureHash::AudioSecureHashAlgo(
        binaryData, sizeof(binaryData), hash2);
    
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(memcmp(hash, hash2, DIGEST_SIZE), 0);
}

/**
 * @tc.name: AudioSecureHashPerformanceTest001
 * @tc.desc: Performance test with large data (1MB)
 * @tc.type: PERFORMANCE
 */
HWTEST_F(AudioSecureHashUnitTest, AudioSecureHashPerformanceTest001, TestSize.Level1)
{
    const size_t DATA_SIZE = 1024 * 1024; // 1MB
    std::vector<unsigned char> largeData(DATA_SIZE);
    
    // Fill with pseudo-random data
    for (size_t i = 0; i < DATA_SIZE; i++) {
        largeData[i] = static_cast<unsigned char>((i * 1234567) & 0xFF);
    }
    
    unsigned char hash[DIGEST_SIZE];
    
    // Time the operation
    auto startTime = std::chrono::high_resolution_clock::now();
    
    unsigned char* result = AudioSecureHash::AudioSecureHashAlgo(
        largeData.data(), DATA_SIZE, hash);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    
    EXPECT_NE(result, nullptr);
    
    // Verify that processing completed within reasonable time
    // (Adjust timeout based on platform capabilities)
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
}
}
}