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

#include "hdi_monitor_aiut.h"
#include "aiut_log.h"
#include "manager/hdi_monitor.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void HdiMonitorAiut::SetUpTestCase(void) {}
void HdiMonitorAiut::TearDownTestCase(void) {}
void HdiMonitorAiut::SetUp(void) {}
void HdiMonitorAiut::TearDown(void) {}

// ==================== ReportHdiException Tests ====================

/**
 * @tc.name  : Test ReportHdiException with LOCAL type
 * @tc.number: HdiMonitor_ReportHdiException_001
 * @tc.desc  : Test ReportHdiException does not crash with LOCAL HDI type.
 */
HWTEST_F(HdiMonitorAiut, HdiMonitor_ReportHdiException_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    HdiMonitor::ReportHdiException(HdiType::LOCAL, ErrorCase::CALL_HDI_FAILED, -1, "test_local_error");
}

/**
 * @tc.name  : Test ReportHdiException with A2DP type
 * @tc.number: HdiMonitor_ReportHdiException_002
 * @tc.desc  : Test ReportHdiException does not crash with A2DP HDI type.
 */
HWTEST_F(HdiMonitorAiut, HdiMonitor_ReportHdiException_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_TIMEOUT, -2, "test_a2dp_error");
}

/**
 * @tc.name  : Test ReportHdiException with empty description
 * @tc.number: HdiMonitor_ReportHdiException_004
 * @tc.desc  : Test ReportHdiException does not crash with empty description.
 */
HWTEST_F(HdiMonitorAiut, HdiMonitor_ReportHdiException_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    HdiMonitor::ReportHdiException(HdiType::LOCAL, ErrorCase::CALL_HDI_FAILED, 0, "");
}

} // namespace AudioStandard
} // namespace OHOS
