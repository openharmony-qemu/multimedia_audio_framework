


#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include <algorithm>
#include <cinttypes>
#include "securec.h"
#include "audio_engine_log.h"
#include "down_mixer.h"


using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;

std::map<AudioChannelLayout> OUTPUT_LAYOUT_SET = {
    
}

class Down_mixer_test : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void Down_mixer_test::SetUp() {}

void Down_mixer_test::TearDown() {}

namespace {


constexpr AudioChannelInfo inChannelInfo;
constexpr AudioChannelInfo outChannelInfo;
constexpr uint32_t formatSize = 0;
constexpr bool mixLfe = false;
/**
 * @tc.name : Test SetParam API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetParam interface
*/
TEST_F(Down_mixer_test, SetParamRetFail)
{
     //DMIX_ERR_INVALID_ARG
     std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
     inChannelInfo.numChannels = MAX_CHANNELS + 1;
     inChannelInfo.channelLayout = MONO - 1;
     outChannelInfo.numChannels = MAX_CHANNELS + 1;
     outChannelInfo.channelLayout = MONO - 1;
     uint32_t ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
     EXPECT_EQ(ret, DMIX_ERR_INVALID_ARG);
     
}
TEST_F(Down_mixer_test, SetParamRetSuccessAndSetupDownMixTable)
{
    //DMIX_ERR_SUCCESS and SetupDownMixTable case: CH_LAYOUT_STEREO
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
    inChannelInfo.numChannels = 10;
    inChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    outChannelInfo.numChannels = 10;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    formatSize = 10;
    mixLfe = true;
    uint32_t ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);

    //case: CH_LAYOUT_5POINT1
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    outChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);

    //case: CH_LAYOUT_5POINT1POINT2
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1POINT2;
    outChannelInfo.channelLayout = CH_LAYOUT_5POINT1POINT2;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
    
    //case: CH_LAYOUT_5POINT1POINT4
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1POINT4;
    outChannelInfo.channelLayout = CH_LAYOUT_5POINT1POINT4;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);

    //case: CH_LAYOUT_7POINT1
    inChannelInfo.channelLayout = CH_LAYOUT_7POINT1;
    outChannelInfo.channelLayout = CH_LAYOUT_7POINT1;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);

    //case: CH_LAYOUT_7POINT1POINT2
    inChannelInfo.channelLayout = CH_LAYOUT_7POINT1POINT2;
    outChannelInfo.channelLayout = CH_LAYOUT_7POINT1POINT2;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);

    //case: CH_LAYOUT_7POINT1POINT4
    inChannelInfo.channelLayout = CH_LAYOUT_7POINT1POINT4;
    outChannelInfo.channelLayout = CH_LAYOUT_7POINT1POINT4;
    ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
}


constexpr uint32_t framesize = 0;   // <1920000
constexpr float in = 0.0f;
constexpr uint32_t inLen = 0;
constexpr float out = 0.0f;
constexpr uint32_t outLen = 0;
constexpr uint32_t ret = 0;
/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process
 * @tc.desc  : Test Process interface.
 */
//case 1: (初始化失败，入参正确的情况)
TEST_F(Down_mixer_test, ProcessRetFail)
{
    //DMIX_ERR_INVALID_ARG
    //入参
    framesize = 1900000;   
    in = 0.5;
    inLen = 10; 
    out = 0.2;
    outLen = 8;
    //构建初始化失败状态返回的申请失败
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
    inChannelInfo.numChannels = MAX_CHANNELS + 1;
    inChannelInfo.channelLayout = MONO - 1;
    outChannelInfo.numChannels = MAX_CHANNELS + 1;
    outChannelInfo.channelLayout = MONO - 1;
    uint32_t ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    ret = downMixer->Process(framesize, &in, inLen, &out, outLen);
    EXPECT_EQ(ret, DMIX_ERR_ALLOC_FAILED);
}
//case 2: (初始化成功，入参不合法出现的失败)
TEST_F(Down_mixer_test, ProcessRetFail_1)
{
    //入参
    framesize = 1;
    in = 0.5;
    inLen = 10; 
    out = 0.2;
    outLen = 8;
    //初始化正确用例
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
    inChannelInfo.numChannels = 10;
    inChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    outChannelInfo.numChannels = 10;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    formatSize = 10;
    mixLfe = true;
    uint32_t ret = downMixer->SetParam(inChannelInfo, outChannelInfo, formatSize, mixLfe);
    ret = downMixer->Process(framesize, &in, inLen, &out, outLen);
    EXPECT_EQ(ret, DMIX_ERR_ALLOC_FAILED);
}

TEST_F(Down_mixer_test, ProcessRetSuccess_0)
{
    framesize = 1;
    in = 0.5;
    inLen = 10; 
    out = 0.2;
    outLen = 8;
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
    ret = downMixer->Process(framesize, &in, inLen, &out, outLen);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
}

TEST_F(Down_mixer_test, ProcessRetSuccess_1)
{
    framesize = 1;
    in = 0.5;
    inLen = 10; 
    out = 0.2;
    outLen = 8;
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>();
    ret = downMixer->Process(framesize, &in, inLen, &out, outLen);
    EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
}

/**
 * @tc.name  : Test Reset API
 * @tc.type  : FUNC
 * @tc.number: Reset
 * @tc.desc  : Test Reset interface.
 */
TEST_F(Down_mixer_test, ResetFunc)
{
    std::shared_ptr<DownMixer> downMixer = std::make_shared<DownMixer>(); 
    downMixer->Reset();
}

}

