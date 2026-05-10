/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef PACING_TEST_SINK_H
#define PACING_TEST_SINK_H

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <unistd.h>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class PacingTestSink {
public:
    using ProcessFunc = std::function<void()>;
    static constexpr uint32_t usPerSec = 1000000;
    static constexpr uint32_t msPerSec = 1000;

    PacingTestSink(ProcessFunc processFunc, uint32_t sampleRate = 48000,
                   uint32_t frameSize = 480)
        : processFunc_(processFunc),
          intervalUs_(sampleRate != 0 ? frameSize * usPerSec / sampleRate : 0),
          running_(false),
          processedFrames_(0) {}

    ~PacingTestSink() { Stop(); }

    void Start()
    {
        running_ = true;
        processedFrames_ = 0;
        thread_ = std::thread(&PacingTestSink::RunLoop, this);
    }

    void Stop()
    {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void RunFor(uint32_t durationMs)
    {
        Start();
        usleep(durationMs * usPerSec / msPerSec);
        Stop();
    }

    void RunUntil(std::function<bool()> condition, uint32_t timeoutMs = 5000)
    {
        Start();
        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            usleep(intervalUs_);
            if (condition()) {
                Stop();
                return;
            }
        }
        Stop();
    }

    uint32_t GetProcessedFrameCount() const { return processedFrames_; }

private:
    void RunLoop()
    {
        while (running_) {
            processFunc_();
            processedFrames_++;
            usleep(intervalUs_);
        }
    }

    ProcessFunc processFunc_;
    uint32_t intervalUs_;
    std::atomic<bool> running_;
    std::atomic<uint32_t> processedFrames_;
    std::thread thread_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // PACING_TEST_SINK_H