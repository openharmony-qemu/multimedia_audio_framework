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

#include "hpae_offload_sinkoutput_node.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;
using ::testing::_;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int32_t OFFLOAD_FULL = -1;
constexpr int32_t OFFLOAD_WRITE_FAILED = -2;
constexpr size_t DATA_SIZE = 1024;
constexpr uint32_t OFFLOAD_SET_BUFFER_SIZE_NUM = 5;
class HpaeOffloadSinkOutputNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    void PrepareNodeInfo();

    std::shared_ptr<HpaeOffloadSinkOutputNode> offloadNode_;
    std::shared_ptr<MockAudioRenderSink> mockSink_;
};

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    size_t frameLen = 960;
    uint32_t nodeId = 1243;
    nodeInfo.nodeId = nodeId;
    nodeInfo.frameLen = frameLen;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
}

void HpaeOffloadSinkOutputNodeTest::SetUp()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    offloadNode_ = std::make_shared<HpaeOffloadSinkOutputNode>(nodeInfo);
    mockSink_ = std::make_shared<NiceMock<MockAudioRenderSink>>();
    offloadNode_->audioRendererSink_ = mockSink_;
    ::testing::DefaultValue<int32_t>::Set(0);
}

void HpaeOffloadSinkOutputNodeTest::TearDown()
{
    offloadNode_ = nullptr;
    mockSink_ = nullptr;
    ::testing::DefaultValue<int32_t>::Clear();
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS