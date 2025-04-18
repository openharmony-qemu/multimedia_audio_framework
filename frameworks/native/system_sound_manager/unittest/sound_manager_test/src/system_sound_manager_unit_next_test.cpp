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
#include "system_sound_manager_unit_next_test.h"

using namespace OHOS::AbilityRuntime;
using namespace testing::ext;

namespace OHOS {
namespace Media {
const int SUCCESS = 0;
const int RESULT_SIZE = -26;
const int32_t TONE_CATEGORY = -13;
const int32_t SYSPARA_SIZE = 128;
void SystemSoundManagerUnitNextTest::SetUpTestCase(void) {}
void SystemSoundManagerUnitNextTest::TearDownTestCase(void) {}
void SystemSoundManagerUnitNextTest::SetUp(void) {}
void SystemSoundManagerUnitNextTest::TearDown(void) {}

const int STORAGE_MANAGER_MANAGER_ID = 5003;
static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(int32_t systemAbilityId)
{
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, RINGTONE_URI);
}

class MockContext : public AbilityRuntime::Context {
    public:
        MockContext() = default;
        ~MockContext() override = default;

        std::string GetBundleName() const override { return "mock_bundle_name"; }
        std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override { return nullptr; }
        std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override { return nullptr; }
        std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override { return nullptr; }
        std::string GetBundleCodePath() const override { return ""; }
        std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override { return nullptr; }
        std::string GetBundleCodeDir() override { return ""; }
        std::string GetCacheDir() override { return ""; }
        std::string GetTempDir() override { return ""; }
        std::string GetFilesDir() override { return ""; }
        std::string GetResourceDir() override { return ""; }
        bool IsUpdatingConfigurations() override { return false; }
        bool PrintDrawnCompleted() override { return false; }
        std::string GetDatabaseDir() override { return ""; }
        int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
            override { return 0; }
        std::string GetPreferencesDir() override { return ""; }
        int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir)
            override { return 0; }
        std::string GetGroupDir(std::string groupId) override { return ""; }
        std::string GetDistributedFilesDir() override { return ""; }
        std::string GetCloudFileDir() override { return ""; }
        sptr<IRemoteObject> GetToken() override { return nullptr; }
        void SetToken(const sptr<IRemoteObject> &token) override {}
        void SwitchArea(int mode) override {}
        std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override { return nullptr; }
        std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName)
            override { return nullptr; }
        std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(const std::string &bundleName,
            const std::string &moduleName) override { return nullptr; }
        int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName, const std::string &moduleName,
            std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override { return 0; }
        int32_t CreateHspModuleResourceManager(const std::string &bundleName, const std::string &moduleName,
            std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override { return 0; }
        int GetArea() override { return 0; }
        std::string GetProcessName() override { return ""; }
        std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override { return nullptr; }
        std::string GetBaseDir() const override { return ""; }
        Global::Resource::DeviceType GetDeviceType() const override
            { return Global::Resource::DeviceType::DEVICE_PHONE; }
        std::shared_ptr<Context> CreateAreaModeContext(int areaMode) override { return nullptr; }
        std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId) override { return nullptr; }
        bool IsContext(size_t contextTypeId) override { return false; }
};

/**
 * @tc.name  : IsRingtoneTypeValid
 * @tc.number: IsRingtoneTypeValid_001
 * @tc.desc  : Test IsRingtoneTypeValid when an invalid RingtoneType is passed
 */
HWTEST(SystemSoundManagerUnitNextTest, IsRingtoneTypeValid_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    RingtoneType invalidType = static_cast<RingtoneType>(RINGTONE_TYPE_SIM_CARD_0 + 2);
    bool result = systemSoundManagerImpl_->IsRingtoneTypeValid(invalidType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : IsSystemToneTypeValid
 * @tc.number: IsSystemToneTypeValid_001
 * @tc.desc  : Test IsSystemToneTypeValid when an invalid SystemToneType is passed
 */
HWTEST(SystemSoundManagerUnitNextTest, IsSystemToneTypeValid_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    SystemToneType invalidType = static_cast<SystemToneType>(SYSTEM_TONE_TYPE_NOTIFICATION + 1);
    bool result = systemSoundManagerImpl_->IsSystemToneTypeValid(invalidType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : GetDefaultRingtoneUri
 * @tc.number: GetDefaultRingtoneUri_001
 * @tc.desc  : Test GetDefaultRingtoneUri when a valid RingtoneType is passed
 */
HWTEST(SystemSoundManagerUnitNextTest, GetDefaultRingtoneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::unordered_map<RingtoneType, std::string> defaultRingtoneUriMap = {
        {RINGTONE_TYPE_SIM_CARD_0, "uri_sim_card_0"},
        {RINGTONE_TYPE_SIM_CARD_1, "uri_sim_card_1"}
    };
    systemSoundManagerImpl_->defaultRingtoneUriMap_ = defaultRingtoneUriMap;

    RingtoneType validType = RINGTONE_TYPE_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetDefaultRingtoneUri(validType);
    EXPECT_EQ(result, "uri_sim_card_0");
}

/**
 * @tc.name  : ReadDefaultToneHaptics
 * @tc.number: ReadDefaultToneHaptics_001
 * @tc.desc  : Test ReadDefaultToneHaptics when GetParameter returns empty string
 */
HWTEST(SystemSoundManagerUnitNextTest, ReadDefaultToneHaptics_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    systemSoundManagerImpl_->defaultToneHapticsUriMap_.clear();
    const char* paramName = "invalid_param";
    char paramValue[SYSPARA_SIZE] = {0};

    std::fill_n(paramValue, SYSPARA_SIZE, 0);
    ToneHapticsType toneHapticsType = CALL_SIM_CARD_0;
    systemSoundManagerImpl_->ReadDefaultToneHaptics(paramName, toneHapticsType);
    EXPECT_TRUE(systemSoundManagerImpl_->defaultToneHapticsUriMap_.empty());
}

/**
 * @tc.name  : RemoveSourceTypeForRingTone
 * @tc.number: RemoveSourceTypeForRingTone_001
 * @tc.desc  : Test RemoveSourceTypeForRingTone when entering RINGTONE_TYPE_SIM_CARD_1 branch
 */
HWTEST(SystemSoundManagerUnitNextTest, RemoveSourceTypeForRingTone_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(dataShareHelper, nullptr);

    RingtoneType ringtoneType = RINGTONE_TYPE_SIM_CARD_1;
    SourceType sourceType = SOURCE_TYPE_CUSTOMISED;
    int32_t result = systemSoundManagerImpl_->RemoveSourceTypeForRingTone(dataShareHelper, ringtoneType, sourceType);
    EXPECT_EQ(result, RESULT_SIZE);
}

/**
 * @tc.name  : RemoveSourceTypeForRingTone
 * @tc.number: RemoveSourceTypeForRingTon_002
 * @tc.desc  : Test RemoveSourceTypeForRingTone when entering default branch
 */
HWTEST(SystemSoundManagerUnitNextTest, RemoveSourceTypeForRingTone_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(dataShareHelper, nullptr);
    RingtoneType invalidType = static_cast<RingtoneType>(RINGTONE_TYPE_SIM_CARD_1 + 1);

    SourceType sourceType = SOURCE_TYPE_CUSTOMISED;
    int32_t result = systemSoundManagerImpl_->RemoveSourceTypeForRingTone(dataShareHelper, invalidType, sourceType);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : GetRingtoneUriByType_NotInitialized
 * @tc.number: GetRingtoneUriByType_001
 * @tc.desc  : Test GetRingtoneUriByType when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUriByType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(dataShareHelper, nullptr);
    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.isProxy = false;
    databaseTool.dataShareHelper = dataShareHelper;

    std::string type = "ringtone_type";
    std::string result = systemSoundManagerImpl_->GetRingtoneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetRingtoneUriByType_DataShareHelperNull
 * @tc.number: GetRingtoneUriByType_002
 * @tc.desc  : Test GetRingtoneUriByType when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUriByType_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = false;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "ringtone_type";
    std::string result = systemSoundManagerImpl_->GetRingtoneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetRingtoneUriByType_IsProxy
 * @tc.number: GetRingtoneUriByType_003
 * @tc.desc  : Test GetRingtoneUriByType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUriByType_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    std::string type = "ringtone_type";
    std::string result = systemSoundManagerImpl_->GetRingtoneUriByType(databaseTool, type);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetPresetRingToneUriByType_NotInitialized
 * @tc.number: GetPresetRingToneUriByType_001
 * @tc.desc  : Test GetPresetRingToneUriByType when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetRingToneUriByType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.isProxy = false;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "preset_ringtone_type";
    std::string result = systemSoundManagerImpl_->GetPresetRingToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetRingToneUriByType_DataShareHelperNull
 * @tc.number: GetPresetRingToneUriByType_002
 * @tc.desc  : Test GetPresetRingToneUriByType when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetRingToneUriByType_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    EXPECT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = false;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "preset_ringtone_type";
    std::string result = systemSoundManagerImpl_->GetPresetRingToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetRingToneUriByType_IsProxy
 * @tc.number: GetPresetRingToneUriByType_003
 * @tc.desc  : Test GetPresetRingToneUriByType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetRingToneUriByType_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    std::string type = "preset_ringtone_type";
    std::string result = systemSoundManagerImpl_->GetPresetRingToneUriByType(databaseTool, type);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetRingtoneUri_NotInitialized
 * @tc.number: GetRingtoneUri_001
 * @tc.desc  : Test GetRingtoneUri when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    RingtoneType ringtoneType = RINGTONE_TYPE_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetRingtoneUri(databaseTool, ringtoneType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetRingtoneUri_DataShareHelperNull
 * @tc.number: GetRingtoneUri_002
 * @tc.desc  : Test GetRingtoneUri when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUri_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;

    RingtoneType ringtoneType = RINGTONE_TYPE_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetRingtoneUri(databaseTool, ringtoneType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetRingtoneUri_DefaultCase
 * @tc.number: GetRingtoneUri_003
 * @tc.desc  : Test GetRingtoneUri when entering the default case of switch
 */
HWTEST(SystemSoundManagerUnitNextTest, GetRingtoneUri_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    RingtoneType ringtoneType = static_cast<RingtoneType>(RINGTONE_TYPE_SIM_CARD_1 + 1);
    std::string result = systemSoundManagerImpl_->GetRingtoneUri(databaseTool, ringtoneType);
    EXPECT_EQ(result, NO_RING_SOUND);
}

/**
 * @tc.name  : SetSystemToneUri_NoSystemSound
 * @tc.number: SetSystemToneUri_001
 * @tc.desc  : Test SetSystemToneUri when uri is NO_SYSTEM_SOUND
 */
HWTEST(SystemSoundManagerUnitNextTest, SetSystemToneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<MockContext>();
    ASSERT_NE(context, nullptr);

    std::string uri = NO_SYSTEM_SOUND;
    SystemToneType systemToneType = SYSTEM_TONE_TYPE_NOTIFICATION;
    int32_t result = systemSoundManagerImpl_->SetSystemToneUri(context, uri, systemToneType);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : GetSpecificRingTonePlayer_NotEmptyUri
 * @tc.number: GetSpecificRingTonePlayer_001
 * @tc.desc  : Test GetSpecificRingTonePlayer when ringtoneUri is not empty
 */
HWTEST(SystemSoundManagerUnitNextTest, GetSpecificRingTonePlayer_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<MockContext>();
    ASSERT_NE(context, nullptr);
    RingtoneType ringtoneType = RINGTONE_TYPE_SIM_CARD_0;
    std::string ringtoneUri = "valid_uri";
    std::shared_ptr<RingtonePlayer> result = systemSoundManagerImpl_->GetSpecificRingTonePlayer(context,
        ringtoneType, ringtoneUri);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : SetRingtoneUri_NoRingSound
 * @tc.number: SetRingtoneUri_001
 * @tc.desc  : Test SetRingtoneUri when uri is NO_RING_SOUND
 */
HWTEST(SystemSoundManagerUnitNextTest, SetRingtoneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<MockContext>();
    ASSERT_NE(context, nullptr);

    std::string uri = NO_RING_SOUND;
    RingtoneType ringtoneType = RINGTONE_TYPE_SIM_CARD_0;
    int32_t result = systemSoundManagerImpl_->SetRingtoneUri(context, uri, ringtoneType);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : GetShotToneUriByType_NotInitialized
 * @tc.number: GetShotToneUriByType_001
 * @tc.desc  : Test GetShotToneUriByType when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetShotToneUriByType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    std::string type = "shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetShotToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetShotToneUriByType_DataShareHelperNull
 * @tc.number: GetShotToneUriByType_002
 * @tc.desc  : Test GetShotToneUriByType when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetShotToneUriByType_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetShotToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetShotToneUriByType_IsProxy
 * @tc.number: GetShotToneUriByType_003
 * @tc.desc  : Test GetShotToneUriByType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetShotToneUriByType_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    std::string type = "shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetShotToneUriByType(databaseTool, type);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetPresetShotToneUriByType_NotInitialized
 * @tc.number: GetPresetShotToneUriByType_001
 * @tc.desc  : Test GetPresetShotToneUriByType when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetShotToneUriByType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "preset_shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetPresetShotToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetShotToneUriByType_DataShareHelperNull
 * @tc.number: GetPresetShotToneUriByType_002
 * @tc.desc  : Test GetPresetShotToneUriByType when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetShotToneUriByType_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;

    std::string type = "preset_shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetPresetShotToneUriByType(databaseTool, type);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetShotToneUriByType_IsProxy
 * @tc.number: GetPresetShotToneUriByType_003
 * @tc.desc  : Test GetPresetShotToneUriByType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetShotToneUriByType_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);

    std::string type = "preset_shot_tone_type";
    std::string result = systemSoundManagerImpl_->GetPresetShotToneUriByType(databaseTool, type);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetNotificationToneUriByType_NotInitialized
 * @tc.number: GetNotificationToneUriByType_001
 * @tc.desc  : Test GetNotificationToneUriByType when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetNotificationToneUriByType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = nullptr;
    std::string result = systemSoundManagerImpl_->GetNotificationToneUriByType(databaseTool);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetNotificationToneUriByType_DataShareHelperNull
 * @tc.number: GetNotificationToneUriByType_002
 * @tc.desc  : Test GetNotificationToneUriByType when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetNotificationToneUriByType_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;
    std::string result = systemSoundManagerImpl_->GetNotificationToneUriByType(databaseTool);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetNotificationToneUriByType_IsProxy
 * @tc.number: GetNotificationToneUriByType_003
 * @tc.desc  : Test GetNotificationToneUriByType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetNotificationToneUriByType_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);
    std::string result = systemSoundManagerImpl_->GetNotificationToneUriByType(databaseTool);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetPresetNotificationToneUri_NotInitialized
 * @tc.number: GetPresetNotificationToneUri_001
 * @tc.desc  : Test GetPresetNotificationToneUri when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetNotificationToneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = nullptr;
    std::string result = systemSoundManagerImpl_->GetPresetNotificationToneUri(databaseTool);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetNotificationToneUri_DataShareHelperNull
 * @tc.number: GetPresetNotificationToneUri_002
 * @tc.desc  : Test GetPresetNotificationToneUri when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetNotificationToneUri_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;
    std::string result = systemSoundManagerImpl_->GetPresetNotificationToneUri(databaseTool);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetPresetNotificationToneUri_IsProxy
 * @tc.number: GetPresetNotificationToneUri_003
 * @tc.desc  : Test GetPresetNotificationToneUri when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetPresetNotificationToneUri_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);
    std::string result = systemSoundManagerImpl_->GetPresetNotificationToneUri(databaseTool);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetSystemToneUri_NotInitialized
 * @tc.number: GetSystemToneUri_001
 * @tc.desc  : Test GetSystemToneUri when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetSystemToneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = nullptr;
    SystemToneType systemToneType = SYSTEM_TONE_TYPE_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetSystemToneUri(databaseTool, systemToneType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetSystemToneUri_DataShareHelperNull
 * @tc.number: GetSystemToneUri_002
 * @tc.desc  : Test GetSystemToneUri when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetSystemToneUri_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;
    SystemToneType systemToneType = SYSTEM_TONE_TYPE_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetSystemToneUri(databaseTool, systemToneType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : AddCustomizedToneByExternalUri_ExternalUriWithFdHead
 * @tc.number: AddCustomizedToneByExternalUri_001
 * @tc.desc  : Test AddCustomizedToneByExternalUri when externalUri contains fdHead
 */
HWTEST(SystemSoundManagerUnitNextTest, AddCustomizedToneByExternalUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<MockContext>();
    ASSERT_NE(context, nullptr);

    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file", "test_uri", ToneCustomizedType::CUSTOMISED, 0);
    toneAttrs->SetTitle("test_tone");
    toneAttrs->SetCategory(0);
    std::string externalUri = "fd://123";
    std::string result = systemSoundManagerImpl_->AddCustomizedToneByExternalUri(context, toneAttrs, externalUri);
    EXPECT_NE(result, "vs");
}

/**
 * @tc.name  : GetCustomizedTone_WithFileType
 * @tc.number: GetCustomizedTone_001
 * @tc.desc  : Test GetCustomizedTone when file name contains a file type in RINGTONETYPE
 */
HWTEST(SystemSoundManagerUnitNextTest, GetCustomizedTone_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file.mp3", "test_uri", ToneCustomizedType::CUSTOMISED, 0);

    systemSoundManagerImpl_->displayName_ = "";
    systemSoundManagerImpl_->mimeType_ = "";
    systemSoundManagerImpl_->GetCustomizedTone(toneAttrs);
    EXPECT_EQ(systemSoundManagerImpl_->mimeType_, "mp3");
    EXPECT_EQ(systemSoundManagerImpl_->displayName_, "test_file.mp3");
}

/**
 * @tc.name  : GetCustomizedTone_WithoutFileType
 * @tc.number: GetCustomizedTone_002
 * @tc.desc  : Test GetCustomizedTone when file name does not contain any file type in RINGTONETYPE
 */
HWTEST(SystemSoundManagerUnitNextTest, GetCustomizedTone_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file", "test_uri", ToneCustomizedType::CUSTOMISED, 0);

    systemSoundManagerImpl_->displayName_ = "";
    systemSoundManagerImpl_->mimeType_ = "";
    systemSoundManagerImpl_->GetCustomizedTone(toneAttrs);
    EXPECT_EQ(systemSoundManagerImpl_->mimeType_, "ogg");
    EXPECT_EQ(systemSoundManagerImpl_->displayName_, "test_file.ogg");
}

/**
 * @tc.name  : AddCustomizedTone_ContactsCategory
 * @tc.number: AddCustomizedTone_001
 * @tc.desc  : Test AddCustomizedTone when entering TONE_CATEGORY_CONTACTS case
 */
HWTEST(SystemSoundManagerUnitNextTest, AddCustomizedTone_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(dataShareHelper, nullptr);

    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file", "test_uri", ToneCustomizedType::CUSTOMISED, TONE_CATEGORY_CONTACTS);
    int32_t result = systemSoundManagerImpl_->AddCustomizedTone(dataShareHelper, toneAttrs);
    EXPECT_EQ(result, TONE_CATEGORY);
}

/**
 * @tc.name  : AddCustomizedTone_DefaultCategory
 * @tc.number: AddCustomizedTone_002
 * @tc.desc  : Test AddCustomizedTone when entering default case
 */
HWTEST(SystemSoundManagerUnitNextTest, AddCustomizedTone_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(dataShareHelper, nullptr);

    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file", "test_uri", ToneCustomizedType::CUSTOMISED, TONE_CATEGORY_INVALID);
    int32_t result = systemSoundManagerImpl_->AddCustomizedTone(dataShareHelper, toneAttrs);
    EXPECT_EQ(result, TONE_CATEGORY);
}

/**
 * @tc.name  : AddCustomizedToneByFdAndOffset_LseekError
 * @tc.number: AddCustomizedToneByFdAndOffset_001
 * @tc.desc  : Test AddCustomizedToneByFdAndOffset when lseekResult is -1
 */
HWTEST(SystemSoundManagerUnitNextTest, AddCustomizedToneByFdAndOffset_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<MockContext>();
    ASSERT_NE(context, nullptr);
    std::shared_ptr<ToneAttrs> toneAttrs = std::make_shared<ToneAttrs>(
        "test_tone", "test_file", "test_uri", ToneCustomizedType::CUSTOMISED, TONE_CATEGORY_RINGTONE);

    int32_t fd = 123;
    int32_t offset = -1;
    int32_t length = 1024;
    std::string result = systemSoundManagerImpl_->AddCustomizedToneByFdAndOffset(context,
        toneAttrs, fd, offset, length);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : ConvertToSystemToneType_Default
 * @tc.number: ConvertToSystemToneType_001
 * @tc.desc  : Test ConvertToSystemToneType when entering default branch
 */
HWTEST(SystemSoundManagerUnitNextTest, ConvertToSystemToneType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    ToneHapticsType toneHapticsType = ToneHapticsType::CALL_SIM_CARD_0;
    SystemToneType systemToneType;
    bool result = systemSoundManagerImpl_->ConvertToSystemToneType(toneHapticsType, systemToneType);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : IntToToneHapticsMode_None
 * @tc.number: IntToToneHapticsMode_001
 * @tc.desc  : Test IntToToneHapticsMode when value is NONE
 */
HWTEST(SystemSoundManagerUnitNextTest, IntToToneHapticsMode_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    int32_t value = NONE;
    ToneHapticsMode result = systemSoundManagerImpl_->IntToToneHapticsMode(value);
    EXPECT_EQ(result, NONE);
}

/**
 * @tc.name  : IntToToneHapticsMode_Sync
 * @tc.number: IntToToneHapticsMode_002
 * @tc.desc  : Test IntToToneHapticsMode when value is SYNC
 */
HWTEST(SystemSoundManagerUnitNextTest, IntToToneHapticsMode_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    int32_t value = SYNC;
    ToneHapticsMode result = systemSoundManagerImpl_->IntToToneHapticsMode(value);
    EXPECT_EQ(result, SYNC);
}

/**
 * @tc.name  : IntToToneHapticsMode_Default
 * @tc.number: IntToToneHapticsMode_003
 * @tc.desc  : Test IntToToneHapticsMode when entering default branch
 */
HWTEST(SystemSoundManagerUnitNextTest, IntToToneHapticsMode_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    int32_t value = 999;
    ToneHapticsMode result = systemSoundManagerImpl_->IntToToneHapticsMode(value);
    EXPECT_EQ(result, NONE);
}

/**
 * @tc.name  : GetCurrentToneUri_NotInitialized
 * @tc.number: GetCurrentToneUri_001
 * @tc.desc  : Test GetCurrentToneUri when databaseTool is not initialized
 */
HWTEST(SystemSoundManagerUnitNextTest, GetCurrentToneUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = false;
    databaseTool.dataShareHelper = nullptr;
    ToneHapticsType toneHapticsType = ToneHapticsType::CALL_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetCurrentToneUri(databaseTool, toneHapticsType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetCurrentToneUri_DataShareHelperNull
 * @tc.number: GetCurrentToneUri_002
 * @tc.desc  : Test GetCurrentToneUri when dataShareHelper is null
 */
HWTEST(SystemSoundManagerUnitNextTest, GetCurrentToneUri_002, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = nullptr;
    ToneHapticsType toneHapticsType = ToneHapticsType::CALL_SIM_CARD_0;
    std::string result = systemSoundManagerImpl_->GetCurrentToneUri(databaseTool, toneHapticsType);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name  : GetCurrentToneUri_InvalidType
 * @tc.number: GetCurrentToneUri_003
 * @tc.desc  : Test GetCurrentToneUri when entering the else branch
 */
HWTEST(SystemSoundManagerUnitNextTest, GetCurrentToneUri_003, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);
    ToneHapticsType toneHapticsType = ToneHapticsType::NOTIFICATION;
    std::string result = systemSoundManagerImpl_->GetCurrentToneUri(databaseTool, toneHapticsType);
    EXPECT_EQ(result,  "no_system_sound");
}

/**
 * @tc.name  : GetSimcardSettingAssetByToneHapticsType_IsProxy
 * @tc.number: GetSimcardSettingAssetByToneHapticsType_001
 * @tc.desc  : Test GetSimcardSettingAssetByToneHapticsType when isProxy is true
 */
HWTEST(SystemSoundManagerUnitNextTest, GetSimcardSettingAssetByToneHapticsType_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);

    DatabaseTool databaseTool;
    databaseTool.isInitialized = true;
    databaseTool.isProxy = true;
    databaseTool.dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    ASSERT_NE(databaseTool.dataShareHelper, nullptr);
    ToneHapticsType toneHapticsType = ToneHapticsType::CALL_SIM_CARD_0;
    std::unique_ptr<SimcardSettingAsset> result =
        systemSoundManagerImpl_->GetSimcardSettingAssetByToneHapticsType(databaseTool, toneHapticsType);
    ASSERT_NE(result, nullptr);
}

/**
 * @tc.name  : GetFirstNonSyncedHapticsUri_EnterIf
 * @tc.number: GetFirstNonSyncedHapticsUri_001
 * @tc.desc  : Test GetFirstNonSyncedHapticsUri when entering the if branch
 */
HWTEST(SystemSoundManagerUnitNextTest, GetFirstNonSyncedHapticsUri_001, TestSize.Level0)
{
    auto systemSoundManager_ = SystemSoundManagerFactory::CreateSystemSoundManager();
    std::shared_ptr<SystemSoundManagerImpl> systemSoundManagerImpl_ =
        std::static_pointer_cast<SystemSoundManagerImpl>(systemSoundManager_);
    ASSERT_NE(systemSoundManagerImpl_, nullptr);
    std::string result = systemSoundManagerImpl_->GetFirstNonSyncedHapticsUri();
    EXPECT_EQ(result, "");
}
} // namespace Media
} // namespace OHOS
