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
#ifndef AUDIO_SAFE_BLOCK_QUEUE_H
#define AUDIO_SAFE_BLOCK_QUEUE_H

namespace OHOS {
namespace AudioStandard {

/**
 * @brief Provides interfaces for thread-safe blocking queues.
 *
 * The interfaces can be used to perform blocking and non-blocking push and
 * pop operations on queues.
 */
template <typename T>
class AudioSafeBlockQueue {
public:
    explicit AudioSafeBlockQueue(int capacity) : maxSize_(capacity)
    {
    }

    /**
     * @brief Inserts an element at the end of this queue in blocking mode.
     *
     * If the queue is full, the thread of the push operation will be blocked
     * until the queue has space.
     * If the queue is not full, the push operation can be performed and one of the
     * pop threads (blocked when the queue is empty) is woken up.
     *
     * @param elem Indicates the element to insert.
     */
    virtual void Push(T const& elem)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        while (queueT_.size() >= maxSize_) {
            // If the queue is full, wait for jobs to be taken.
            cvNotFull_.wait(lock, [&]() { return (queueT_.size() < maxSize_); });
        }

        // Insert the element into the queue if the queue is not full.
        queueT_.push(elem);
        cvNotEmpty_.notify_all();
    }

    /**
     * @brief Removes the first element from this queue in blocking mode.
     *
     * If the queue is empty, the thread of the pop operation will be blocked
     * until the queue has elements.
     * If the queue is not empty, the pop operation can be performed, the first
     * element of the queue is returned, and one of the push threads (blocked
     * when the queue is full) is woken up.
     */
    T Pop()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);

        while (queueT_.empty()) {
            // If the queue is empty, wait for elements to be pushed in.
            cvNotEmpty_.wait(lock, [&] { return !queueT_.empty(); });
        }

        T elem = queueT_.front();
        queueT_.pop();
        cvNotFull_.notify_all();
        return elem;
    }

    /**
     * @brief Inserts an element at the end of this queue in non-blocking mode.
     *
     * If the queue is full, <b>false</b> is returned directly.
     * If the queue is not full, the push operation can be performed, one of the
     * pop threads (blocked when the queue is empty) is woken up, and <b>true</b>
     * is returned.
     *
     * @param elem Indicates the element to insert.
     */
    virtual bool PushNoWait(T const& elem)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        if (queueT_.size() >= maxSize_) {
            return false;
        }
        // Insert the element if the queue is not full.
        queueT_.push(elem);
        cvNotEmpty_.notify_all();
        return true;
    }

    /**
     * @brief Removes the first element from this queue in non-blocking mode.
     *
     * If the queue is empty, <b>false</b> is returned directly.
     * If the queue is not empty, the pop operation can be performed, one of the
     * push threads (blocked when the queue is full) is woken up, and <b>true</b>
     * is returned.
     *
     * @param outtask Indicates the data of the pop operation.
     */
    bool PopNotWait(T& outtask)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        if (queueT_.empty()) {
            return false;
        }
        outtask = queueT_.front();
        queueT_.pop();

        cvNotFull_.notify_all();

        return true;
    }

    std::queue<T> PopAllNotWait()
    {
        std::queue<T> retQueue = {};
        std::unique_lock<std::mutex> lock(mutexLock_);
        retQueue.swap(queueT_);

        cvNotFull_.notify_all();

        return retQueue;
    }

    unsigned int Size()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.size();
    }

    template< class Rep, class Period >
    void WaitNotEmptyFor(const std::chrono::duration<Rep, Period>& rel_time)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        cvNotEmpty_.wait_for(lock, rel_time, [this] {
            return !queueT_.empty();
        });
    }

    bool IsEmpty()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.empty();
    }

    bool IsFull()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.size() == maxSize_;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        queueT_ = {};
        cvNotFull_.notify_all();
    }

    virtual ~AudioSafeBlockQueue() {}

protected:
    unsigned long maxSize_;  // Capacity of the queue
    std::mutex mutexLock_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;
    std::queue<T> queueT_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SAFE_BLOCK_QUEUE_H