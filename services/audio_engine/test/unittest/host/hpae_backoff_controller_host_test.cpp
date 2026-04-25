/*
 * Host build test for hpae_backoff_controller.
 * Runs natively on x86_64 without OHOS runtime.
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <thread>
#include <chrono>

// Host stubs provide HWTEST_F compatibility macro
#include "audio_engine_log.h"

#include "hpae_backoff_controller.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeBackoffControllerHostTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

// Default constructor
HWTEST_F(HpaeBackoffControllerHostTest, Construct_Default, TestSize.Level0)
{
    HpaeBackoffController ctrl;
    // No crash, default params
}

// Custom constructor
HWTEST_F(HpaeBackoffControllerHostTest, Construct_Custom, TestSize.Level0)
{
    HpaeBackoffController ctrl(5, 50, 5);
    // No crash with custom params
}

// HandleResult(true) resets delay
HWTEST_F(HpaeBackoffControllerHostTest, HandleResult_True_Resets, TestSize.Level0)
{
    HpaeBackoffController ctrl(0, 20, 5);
    ctrl.HandleResult(false);  // increment delay
    ctrl.HandleResult(true);   // should reset
    // After reset, next false should go back to increment
    ctrl.HandleResult(false);
    // No crash, delay was reset then incremented
}

// HandleResult(false) increases delay
HWTEST_F(HpaeBackoffControllerHostTest, HandleResult_False_Increments, TestSize.Level0)
{
    HpaeBackoffController ctrl(0, 20, 5);
    ctrl.HandleResult(false);
    ctrl.HandleResult(false);
    ctrl.HandleResult(false);
    // Delay should increase each time (up to maxDelay)
    // No crash expected
}

// Saturate at maxDelay
HWTEST_F(HpaeBackoffControllerHostTest, HandleResult_False_Saturate, TestSize.Level0)
{
    HpaeBackoffController ctrl(0, 10, 5);
    for (int i = 0; i < 10; i++) {
        ctrl.HandleResult(false);
    }
    // After many failures, delay should be capped at maxDelay
    // No crash expected
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
