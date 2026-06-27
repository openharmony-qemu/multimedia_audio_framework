/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_effect_server_unit_test.h"
#include "audio_effect_server.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioEffectServer> AudioEffectServerUnitTest::audioEffectServer = nullptr;

void AudioEffectServerUnitTest::SetUpTestCase(void)
{
    // Setup for all test cases
    audioEffectServer = std::make_shared<AudioEffectServer>();
}

void AudioEffectServerUnitTest::TearDownTestCase(void)
{
    // Teardown for all test cases
    audioEffectServer = nullptr;
}

void AudioEffectServerUnitTest::SetUp(void)
{
    // Setup before each test case
}

void AudioEffectServerUnitTest::TearDown(void)
{
    // Teardown after each test case
}

/**
 * @tc.name  : Test AudioEffectServer constructor
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerConstructor_001
 * @tc.desc  : Test AudioEffectServer constructor.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerConstructor_001, TestSize.Level1)
{
    // Verify that audioEffectServer is properly created in SetUpTestCase
    // AudioEffectServer is already created in SetUpTestCase
    // We can verify it's not null and can call its methods
    std::vector<std::shared_ptr<AudioEffectLibEntry>> &entries = audioEffectServer->GetEffectEntries();
    // Initially, entries should be empty
    EXPECT_TRUE(entries.empty());
}

/**
 * @tc.name  : Test LoadAudioEffects with valid parameters
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerLoadAudioEffects_001
 * @tc.desc  : Test LoadAudioEffects with valid parameters.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerLoadAudioEffects_001, TestSize.Level1)
{
    // Test LoadAudioEffects with valid parameters
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    
    // Add a valid library
    Library library;
    library.name = "test_library";
    library.path = "libtest.so";
    libraries.push_back(library);
    
    // Add a valid effect
    Effect effect;
    effect.libraryName = "test_library";
    effect.name = "test_effect";
    effects.push_back(effect);
    
    bool result = audioEffectServer->LoadAudioEffects(libraries, effects, successEffectList);
    
    // Verify consistency: result should match successEffectList state
    if (result) {
        // If loading succeeded, successEffectList should not be empty
        EXPECT_FALSE(successEffectList.empty());
        // Verify that at least one effect was successfully loaded
        EXPECT_TRUE(successEffectList.size() > 0);
    } else {
        // If loading failed, successEffectList should be empty
        EXPECT_TRUE(successEffectList.empty());
    }
    // Consistency check: result should be true if and only if successEffectList is not empty
    EXPECT_EQ(result, !successEffectList.empty());
}

/**
 * @tc.name  : Test LoadAudioEffects with empty libraries
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerLoadAudioEffects_002
 * @tc.desc  : Test LoadAudioEffects with empty libraries.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerLoadAudioEffects_002, TestSize.Level1)
{
    // Test LoadAudioEffects with empty libraries
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    
    bool result = audioEffectServer->LoadAudioEffects(libraries, effects, successEffectList);
    
    // Verify that the function returns false for empty libraries
    EXPECT_FALSE(result);
    // Verify that successEffectList is empty
    EXPECT_TRUE(successEffectList.empty());
}

/**
 * @tc.name  : Test LoadAudioEffects with invalid library path
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerLoadAudioEffects_003
 * @tc.desc  : Test LoadAudioEffects with invalid library path.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerLoadAudioEffects_003, TestSize.Level1)
{
    // Test LoadAudioEffects with invalid library path
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    
    // Add a library with invalid path
    Library library;
    library.name = "invalid_library";
    library.path = "invalid_path.so";
    libraries.push_back(library);
    
    bool result = audioEffectServer->LoadAudioEffects(libraries, effects, successEffectList);
    
    // Verify that the function returns false for invalid library path
    EXPECT_FALSE(result);
    // Verify that successEffectList is empty
    EXPECT_TRUE(successEffectList.empty());
}

/**
 * @tc.name  : Test GetEffectEntries
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerGetEffectEntries_001
 * @tc.desc  : Test GetEffectEntries interface.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerGetEffectEntries_001, TestSize.Level1)
{
    // Test GetEffectEntries function
    std::vector<std::shared_ptr<AudioEffectLibEntry>> &entries = audioEffectServer->GetEffectEntries();
    
    // Verify that the function returns a reference to the internal vector
    // Initially, entries should be empty if no libraries are loaded
    EXPECT_TRUE(entries.empty());
}

/**
 * @tc.name  : Test LoadAudioEffects with null effect library
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerLoadAudioEffects_004
 * @tc.desc  : Test LoadAudioEffects with null effect library.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerLoadAudioEffects_004, TestSize.Level1)
{
    // Test LoadAudioEffects with null effect library
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    
    // Add an effect with null library name
    Effect effect;
    effect.libraryName = "";
    effect.name = "test_effect";
    effects.push_back(effect);
    
    bool result = audioEffectServer->LoadAudioEffects(libraries, effects, successEffectList);
    
    // Verify that the function returns false for null library name
    EXPECT_FALSE(result);
    // Verify that successEffectList is empty
    EXPECT_TRUE(successEffectList.empty());
}

/**
 * @tc.name  : Test GetEffectEntries after loading
 * @tc.type  : FUNC
 * @tc.number: AudioEffectServerGetEffectEntries_002
 * @tc.desc  : Test GetEffectEntries after loading effects.
 */
HWTEST_F(AudioEffectServerUnitTest, AudioEffectServerGetEffectEntries_002, TestSize.Level1)
{
    // Test GetEffectEntries after loading effects
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    
    // Add a valid library
    Library library;
    library.name = "test_library";
    library.path = "libtest.so";
    libraries.push_back(library);
    
    // Add a valid effect
    Effect effect;
    effect.libraryName = "test_library";
    effect.name = "test_effect";
    effects.push_back(effect);
    
    bool result = audioEffectServer->LoadAudioEffects(libraries, effects, successEffectList);
    
    // Get effect entries after loading
    std::vector<std::shared_ptr<AudioEffectLibEntry>> &entries = audioEffectServer->GetEffectEntries();
    
    // Verify entries state based on loading result
    if (result) {
        // Library loaded successfully
        // Verify that entries is a valid reference (implicitly true if no crash)
        // At least verify that we can access entries without crash
        // If loading succeeded, entries should contain the loaded library
        EXPECT_FALSE(entries.empty());  // 加载成功时，entries 应该不为空
    } else {
        // Library loading failed, entries should be empty
        EXPECT_TRUE(entries.empty());
    }
}

} // namespace AudioStandard
} // namespace OHOS
