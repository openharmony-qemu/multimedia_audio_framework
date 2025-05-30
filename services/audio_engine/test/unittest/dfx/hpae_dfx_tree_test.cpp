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
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "hpae_dfx_tree.h"
#include <locale>

using namespace OHOS {
using namespace AudioStandard {
using namespace HPAE {

class HpaeDfxTreeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeDfxTreeTest::SetUp()
{
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());
}

void HpaeDfxTreeTest::TearDown()
{}

TEST_F(HpaeDfxTreeTest, constructHpaeDfxTreeTest)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeNodeInfo info;
    uint32_t nodeId = 123;
    uint32_t sessionId = 12345;
    size_t frameLen = 960;
    uint32_t preNodeId = 0;
    int32_t testNum = 10;
    for (int32_t i = 0; i < testNum; i++) {
        preNodeId = info.nodeId;
        info.nodeId = nodeId + i;
        info.sessionId = sessionId + i;
        info.nodeName = "testNode1";
        info.frameLen = frameLen;
        info.channels = STEREO;
        info.samplingRate = SAMPLE_RATE_48000;
        info.format = SAMPLE_F32LE;
        info.sceneType = HPAE_SCENE_DEFAULT;
        EXPECT_EQ(hpaeDfxTree.Insert(preNodeId, info), true);
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> result = hpaeDfxTree.LevelOrderTraversal();
    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    std::cout << outStr.c_str() << std::endl;
    int32_t index = 0;
    for (int32_t i = 0; i < result.size(); i++) {
        for (int32_t j = 0; j < result[i].size(); j++) {
            EXPECT_EQ(result[i][j].nodeId, index + nodeId);
            EXPECT_EQ(result[i][j].sessionId, index + sessionId);
            EXPECT_EQ(result[i][j].frameLen, frameLen);
            EXPECT_EQ(result[i][j].samplingRate, SAMPLE_RATE_48000);
            EXPECT_EQ(result[i][j].channels, STEREO);
            EXPECT_EQ(result[i][j].format, SAMPLE_F32LE);
            index++;
        }
    }
}

TEST_F(HpaeDfxTreeTest, RemoveDfxTreeTest)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeNodeInfo info;
    uint32_t nodeId = 123;
    uint32_t sessionId = 12345;
    size_t frameLen = 960;
    uint32_t preNodeId = 0;
    int32_t testNum = 10;
    for (int32_t i = 0; i < testNum; i++) {
        preNodeId = info.nodeId;
        info.nodeId = nodeId + i;
        info.sessionId = sessionId + i;
        info.nodeName = "testNode2";
        info.frameLen = frameLen;
        info.channels = MONO;
        info.samplingRate = SAMPLE_RATE_16000;
        info.format = SAMPLE_F32LE;
        info.sceneType = HPAE_SCENE_MUSIC;
        EXPECT_EQ(hpaeDfxTree.Insert(preNodeId, info), true);
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), testNum);
    uint32_t removeNodeIndex = 3;
    EXPECT_EQ(hpaeDfxTree.Remove(nodeId + removeNodeIndex), true);
    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    std::cout << outStr.c_str() << std::endl;
    result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), removeNodeIndex);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
