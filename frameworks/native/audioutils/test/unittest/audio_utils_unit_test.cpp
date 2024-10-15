/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <thread>
#include <gtest/gtest.h>
#include "audio_utils.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

constexpr int32_t SUCCESS = 0;
constexpr unsigned int QUEUE_SLOTS = 10;
constexpr unsigned int THREAD_NUM = QUEUE_SLOTS + 1;
class AudioUtilsUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioUtilsUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioUtilsUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioUtilsUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioUtilsUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test ClockTime API
* @tc.type  : FUNC
* @tc.number: ClockTime_001
* @tc.desc  : Test ClockTime interface.
*/
HWTEST(AudioUtilsUnitTest, ClockTime_001, TestSize.Level1)
{
    const int64_t CLOCK_TIME = 0;
    int32_t ret = -1;
    ret = ClockTime::AbsoluteSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);

    int64_t nanoTime = 1000;
    ret = ClockTime::AbsoluteSleep(nanoTime);
    EXPECT_EQ(SUCCESS, ret);

    ret = ClockTime::RelativeSleep(CLOCK_TIME);
    EXPECT_EQ(SUCCESS - 1, ret);

    ret = ClockTime::RelativeSleep(nanoTime);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test Trace API
* @tc.type  : FUNC
* @tc.number: Trace_001
* @tc.desc  : Test Trace interface.
*/
HWTEST(AudioUtilsUnitTest, Trace_001, TestSize.Level1)
{
    std::string value = "Test";
    std::shared_ptr<Trace> trace = std::make_shared<Trace>(value);
    trace->End();
    int64_t count = 1;
    Trace::Count(value, count);
}

/**
* @tc.name  : Test PermissionUtil API
* @tc.type  : FUNC
* @tc.number: PermissionUtil_001
* @tc.desc  : Test PermissionUtil interface.
*/
HWTEST(AudioUtilsUnitTest, PermissionUtil_001, TestSize.Level1)
{
    bool ret1 = PermissionUtil::VerifyIsSystemApp();
    EXPECT_EQ(false, ret1);
    bool ret2 = PermissionUtil::VerifySelfPermission();
    EXPECT_EQ(true, ret2);
    bool ret3 = PermissionUtil::VerifySystemPermission();
    EXPECT_EQ(true, ret3);
}

/**
* @tc.name  : Test AdjustStereoToMonoForPCM API
* @tc.type  : FUNC
* @tc.number: AdjustStereoToMonoForPCM_001
* @tc.desc  : Test AdjustStereoToMonoForPCM interface.
*/
HWTEST(AudioUtilsUnitTest, AdjustStereoToMonoForPCM_001, TestSize.Level1)
{
    uint64_t len = 2;

    const int8_t BitRET = 1;
    int8_t arr1[2] = {1, 2};
    int8_t *data1 = &arr1[0];
    AdjustStereoToMonoForPCM8Bit(data1, len);
    EXPECT_EQ(BitRET, data1[0]);
    EXPECT_EQ(BitRET, data1[1]);

    len = 4;
    const int16_t Bit16RET = 1;
    int16_t arr2[2] = {1, 2};
    int16_t *data2 = &arr2[0];
    AdjustStereoToMonoForPCM16Bit(data2, len);
    EXPECT_EQ(Bit16RET, data2[0]);
    EXPECT_EQ(Bit16RET, data2[1]);

    len = 8;
    const int32_t Bit32RET = 1;
    int32_t arr4[2] = {1, 2};
    int32_t *data4 = &arr4[0];
    AdjustStereoToMonoForPCM32Bit(data4, len);
    EXPECT_EQ(Bit32RET, data4[0]);
    EXPECT_EQ(Bit32RET, data4[1]);
}

/**
* @tc.name  : Test AdjustAudioBalanceForPCM API
* @tc.type  : FUNC
* @tc.number: AdjustAudioBalanceForPCM_001
* @tc.desc  : Test AdjustAudioBalanceForPCM interface.
*/
HWTEST(AudioUtilsUnitTest, AdjustAudioBalanceForPCM_001, TestSize.Level1)
{
    float left = 2.0;
    float right = 2.0;
    uint64_t len = 2;

    const int8_t Bit8RET1 = 2;
    const int8_t Bit8RET2 = 4;
    int8_t arr1[2] = {1, 2};
    int8_t *data1 = &arr1[0];
    AdjustAudioBalanceForPCM8Bit(data1, len, left, right);
    EXPECT_EQ(Bit8RET1, data1[0]);
    EXPECT_EQ(Bit8RET2, data1[1]);

    len = 4;
    const int16_t Bit16RET1 = 2;
    const int16_t Bit16RET2 = 4;
    int16_t arr2[2] = {1, 2};
    int16_t *data2 = &arr2[0];
    AdjustAudioBalanceForPCM16Bit(data2, len, left, right);
    EXPECT_EQ(Bit16RET1, data2[0]);
    EXPECT_EQ(Bit16RET2, data2[1]);

    len = 8;
    const int32_t Bit32RET = 2;
    int32_t arr4[2] = {1, 2};
    int32_t *data4 = &arr4[0];
    AdjustAudioBalanceForPCM32Bit(data4, len, left, right);
    EXPECT_EQ(Bit32RET, data4[0]);
    EXPECT_EQ(Bit32RET * 2, data4[1]);
}

/**
* @tc.name  : Test GetSysPara API
* @tc.type  : FUNC
* @tc.number: GetSysPara_001
* @tc.desc  : Test GetSysPara interface.
*/
HWTEST(AudioUtilsUnitTest, GetSysPara_001, TestSize.Level1)
{
    const char *invaildKey = nullptr;
    int32_t value32 = 2;
    bool result = GetSysPara(invaildKey, value32);
    EXPECT_EQ(false, result);
    const char *key = "debug.audio_service.testmodeon";
    bool result1 = GetSysPara(key, value32);
    EXPECT_EQ(true, result1);
    uint32_t valueU32 = 3;
    bool result2 = GetSysPara(key, valueU32);
    EXPECT_EQ(true, result2);
    int64_t value64 = 0;
    bool result3 = GetSysPara(key, value64);
    EXPECT_EQ(true, result3);
    std::string strValue = "100";
    bool result4 = GetSysPara(key, strValue);
    EXPECT_EQ(true, result4);
}

class DemoThreadData {
public:
    DemoThreadData()
    {
        putStatus = false;
        getStatus = false;
    }
    bool putStatus;
    bool getStatus;
    static AudioSafeBlockQueue<int> shareQueue;

    void Put(int j)
    {
        shareQueue.Push(j);
        putStatus = true;
    }

    void Get()
    {
        shareQueue.Pop();
        getStatus = true;
    }
};

AudioSafeBlockQueue<int> DemoThreadData::shareQueue(QUEUE_SLOTS);

void PutHandleThreadData(DemoThreadData& q, int i)
{
    q.Put(i);
}

void GetThreadDatePushedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
                               unsigned int& unpushedIn)
{
    pushedIn = 0;
    unpushedIn = 0;
    for (auto& t : demoDatas) {
        if (t.putStatus) {
            pushedIn++;
        } else {
            unpushedIn++;
        }
    }
}

void GetThreadDateGetedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas,
    unsigned int& getedOut, unsigned int& ungetedOut)
{
    getedOut = 0;
    ungetedOut = 0;
    for (auto& t : demoDatas) {
        if (t.getStatus) {
            getedOut++;
        } else {
            ungetedOut++;
        }
    }
}

void PutHandleThreadDataTime(DemoThreadData& q, int i, std::chrono::system_clock::time_point absTime)
{
    cout << "thread-" << std::this_thread::get_id() << " run time: "
        << std::chrono::system_clock::to_time_t(absTime) << endl;
    std::this_thread::sleep_until(absTime);

    q.Put(i);
}

void GetHandleThreadDataTime(DemoThreadData& q, int i, std::chrono::system_clock::time_point absTime)
{
    cout << "thread-" << std::this_thread::get_id() << " run time: "
        << std::chrono::system_clock::to_time_t(absTime) << endl;
    std::this_thread::sleep_until(absTime);

    q.Get();
}

/*
 * @tc.name: testPut001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
HWTEST(AudioUtilsUnitTest, testPut001, TestSize.Level0)
{
    AudioSafeBlockQueue<int> qi(10);
    int i = 1;
    qi.Push(i);
    EXPECT_EQ(static_cast<unsigned>(1), qi.Size());
}

/*
 * @tc.name: testGet001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
HWTEST(AudioUtilsUnitTest, testGet001, TestSize.Level0)
{
    AudioSafeBlockQueue<int> qi(10);
    for (int i = 0; i < 3; i++) {
        qi.Push(i);
    }
    EXPECT_EQ(static_cast<unsigned>(3), qi.Size());
    int t = qi.Pop();
    ASSERT_EQ(t, 0);
}

static void ThreadsJoin(std::thread (&threads)[THREAD_NUM])
{
    for (auto& t : threads) {
        t.join();
    }
}

static void CheckFullQueueStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
    unsigned int& unpushedIn, unsigned int& getedOut, unsigned int& ungetedOut)
{
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(pushedIn, THREAD_NUM);
    ASSERT_EQ(getedOut, THREAD_NUM - QUEUE_SLOTS);
}

/*
 * @tc.name: testMutilthreadPutAndBlock001
 * @tc.desc: Multiple threads put until blocking runs, one thread gets, all threads finish running normally
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadPutAndBlock001, TestSize.Level0)
{
    std::thread threads[THREAD_NUM];

    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadData, std::ref(demoDatas[i]), i);
    }

    // 1. queue is full and some threads is blocked
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    unsigned int pushedIn = 0, unpushedIn = 0, getedOut = 0, ungetedOut = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, QUEUE_SLOTS);
    ASSERT_EQ(unpushedIn, THREAD_NUM - QUEUE_SLOTS);

    // 2. get one out  and wait some put in
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Get();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    CheckFullQueueStatus(demoDatas, pushedIn, unpushedIn, getedOut, ungetedOut);
    ThreadsJoin(threads);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }

    // here means all thread end ok or if some operation blocked and the testcase blocked
}

static std::time_t GetTimeAddTwoSeconds()
{
    using std::chrono::system_clock;
    std::time_t timeT = system_clock::to_time_t(system_clock::now());
    cout << "start time: " << timeT << endl;
    const int twoSec = 2;
    timeT += twoSec;
    return timeT;
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInblankqueue001
 * @tc.desc: Multi-threaded put() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    // 2. start thread
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    // 1. queue is full and some threads is blocked
    std::this_thread::sleep_for(std::chrono::seconds(4));
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, QUEUE_SLOTS);
    ASSERT_EQ(unpushedIn, THREAD_NUM - QUEUE_SLOTS);

    // 2. get one out  and wait some put in
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Get();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    CheckFullQueueStatus(demoDatas, pushedIn, unpushedIn, getedOut, ungetedOut);
    ThreadsJoin(threads);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
    // here means all thread end ok or if some operation blocked and the testcase blocked
}

static void QueuePushInfull()
{
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

static void QueuePushInnotfull(const unsigned int remain)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS - remain; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInfullqueue001
 * @tc.desc: Multi-threaded put() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInfull();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    // 2. start thread put in full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));
    // 3. now thread is running and all is blocked
    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    ASSERT_EQ(pushedIn, static_cast<unsigned int>(0));
    ASSERT_EQ(unpushedIn, THREAD_NUM);
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        cout << "get out one and then the queue is full" << endl;
        DemoThreadData::shareQueue.Pop();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        ASSERT_EQ(pushedIn, i + 1);
        ASSERT_EQ(unpushedIn, THREAD_NUM - (i + 1));
    }

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInblankqueue001
 * @tc.desc: Multi-threaded get() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    // 2. start thread put in empty queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 3. now thread is running and all is blocked
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, static_cast<unsigned int>(0));
    ASSERT_EQ(ungetedOut, THREAD_NUM);
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    int value = 1;

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        cout << "put in one and then the queue is empty" << endl;
        DemoThreadData::shareQueue.Push(value);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        ASSERT_EQ(getedOut, i + 1);
        ASSERT_EQ(ungetedOut, THREAD_NUM - (i + 1));
    }

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}
/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInfullqueue001
 * @tc.desc: Multi-threaded get() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    int t = 1;
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(t);
    }
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());

    // 2. start thread put in full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime, std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(4));
    // 3. start judge
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);

    ASSERT_EQ(getedOut, QUEUE_SLOTS);
    ASSERT_EQ(ungetedOut, THREAD_NUM - QUEUE_SLOTS);

    // 2.  put one in  and wait some get out
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Put(t);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // queue is full and some threads is blocked and is not joined
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, THREAD_NUM);
    ASSERT_EQ(ungetedOut, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded get() on the notfull queue. When n threads are waiting to reach a certain
 * time-point, everyone get concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndBlockInnotfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    const unsigned int REMAIN_SLOTS = 5;
    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInnotfull(REMAIN_SLOTS);

    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, QUEUE_SLOTS - REMAIN_SLOTS);
    ASSERT_EQ(ungetedOut, THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS));

    // 3. put ungetedOut
    for (unsigned int i = 0; i < ungetedOut; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    ASSERT_EQ(getedOut, THREAD_NUM);
    ASSERT_EQ(ungetedOut, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded put() on the not full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentPutAndBlockInnotfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    using std::chrono::system_clock;

    const unsigned int REMAIN_SLOTS = 5;
    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    QueuePushInnotfull(REMAIN_SLOTS);

    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    unsigned int putedin = 0;
    unsigned int unputedin = 0;
    GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
    ASSERT_EQ(putedin, REMAIN_SLOTS);
    ASSERT_EQ(unputedin, THREAD_NUM - REMAIN_SLOTS);

    // 3. put ungetedOut
    for (unsigned int i = 0; i < unputedin; i++) {
        DemoThreadData::shareQueue.Pop();
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
    ASSERT_EQ(putedin, THREAD_NUM);
    ASSERT_EQ(unputedin, static_cast<unsigned int>(0));

    ThreadsJoin(threads);
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

static void CheckQueueStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas)
{
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);

    ASSERT_EQ(pushedIn, THREAD_NUM);
    ASSERT_EQ(getedOut, THREAD_NUM);
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInblankqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the empty queue. When all threads are waiting to reach
 * a certain time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndPopInblankqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    std::thread threadsin[THREAD_NUM];

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());

    // 2. start thread put and get in empty queue

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsout[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsin[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    CheckQueueStatus(demoDatas);

    ThreadsJoin(threadsout);
    ThreadsJoin(threadsin);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInfullqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the full queue.
 * When all threads are waiting to reach a certain
 * time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
HWTEST(AudioUtilsUnitTest, testMutilthreadConcurrentGetAndPopInfullqueue001, TestSize.Level0)
{
    // 1. prepare
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    demoDatas.fill(DemoThreadData());

    std::thread threadsin[THREAD_NUM];

    using std::chrono::system_clock;

    std::time_t timeT = GetTimeAddTwoSeconds();
    ASSERT_TRUE(DemoThreadData::shareQueue.IsEmpty());
    int t = 1;
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(t);
    }
    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    // 2. start thread put in not full queue
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsin[i] = std::thread(PutHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threadsout[i] = std::thread(GetHandleThreadDataTime,
            std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    ASSERT_TRUE(DemoThreadData::shareQueue.IsFull());
    CheckQueueStatus(demoDatas);

    ThreadsJoin(threadsout);
    ThreadsJoin(threadsin);

    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}
} // namespace AudioStandard
} // namespace OHOS