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
#include <memory>
#include "audio_suite_eq_algo_interface_impl.h"
#include "audio_suite_nr_algo_interface_impl.h"
#include "audio_suite_aiss_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
namespace {
HWTEST(AudioSuiteAlgoInterface, CreateAlgoInterfaceTest, TestSize.Level0) {
    NodeCapability nc;
    auto eqAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        AlgoType::AUDIO_NODE_TYPE_EQUALIZER, nc);
    EXPECT_NE(eqAlgo, nullptr);
    eqAlgo.reset();

    auto anrAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        AlgoType::AUDIO_NODE_TYPE_NOISE_REDUCTION, nc);
    EXPECT_NE(anrAlgo, nullptr);
    anrAlgo.reset();

    auto sfAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        AlgoType::AUDIO_NODE_TYPE_SOUND_FIELD, nc);
    EXPECT_NE(sfAlgo, nullptr);
    sfAlgo.reset();

    auto aissAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION, nc);
    EXPECT_NE(aissAlgo, nullptr);
    aissAlgo.reset();

    auto vbAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        AlgoType::AUDIO_NODE_TYPE_VOICE_BEAUTIFIER, nc);
    EXPECT_NE(vbAlgo, nullptr);
    vbAlgo.reset();

    auto unknownAlgo = AudioSuiteAlgoInterface::CreateAlgoInterface(
        static_cast<AlgoType>(-1), nc);
    EXPECT_EQ(unknownAlgo, nullptr);
    unknownAlgo.reset();
}

}