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

import audio from '@ohos.multimedia.audio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

const TAG = "[AudioLoopbackJsUnitTest]";

describe("AudioLoopbackJsUnitTest", function() {
    beforeAll(async function () {
        // input testsuit setup step, setup invoked before all testcases
        console.info(TAG + 'beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step, teardown invoked after all testcases
        console.info(TAG + 'afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step, setup invoked before each testcases
        console.info(TAG + 'beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step, teardown invoked after each testcases
        console.info(TAG + 'afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_LOOPBCAK_IS_SUPPORTED_TEST_001
     * @tc.desc:isAudioLoopbackSupported success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_LOOPBCAK_IS_SUPPORTED_TEST_001', 0, async function (done) {
        let audioStreamManager = null;
        try {
            audioStreamManager = audio.getAudioManager().getStreamManager();
            let isSupported = audioStreamManager.isAudioLoopbackSupported(audio.AudioLoopbackMode.HARDWARE);
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_IS_SUPPORTED_TEST_001 SUCCESS: ${isSupported}`);
            expect(true).assertTrue();
            done();
        } catch (e) {
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_IS_SUPPORTED_TEST_001 ERROR: ${e.message}`);
            expect(e.code).assertEqual(ERROR_INVALID_PARAM);
            done();
            return;
        }
    })

    /*
     * @tc.name:SUB_AUDIO_LOOPBCAK_CREATE_TEST_001
     * @tc.desc:createAudioLoopback success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_LOOPBCAK_CREATE_TEST_001', 0, async function (done) {
        let audioLoopback = null;
        try {
            audioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_CREATE_TEST_001 SUCCESS: ${audioLoopback}`);
            expect(true).assertTrue();
            done();
        } catch (e) {
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_CREATE_TEST_001 ERROR: ${e.message}`);
            expect(e.code).assertEqual(ERR_PERMISSION_DENIED);
            done();
            return;
        }
    })

    /*
     * @tc.name:SUB_AUDIO_LOOPBCAK_GETSTATUS_TEST_001
     * @tc.desc:getStatus success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_LOOPBCAK_GETSTATUS_TEST_001', 0, async function (done) {
        let audioLoopback = null;
        try {
            audioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
            let status = await audioLoopback.getStatus();
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_GETSTATUS_TEST_001 SUCCESS: ${status}`);
            expect(true).assertTrue();
            done();
        } catch (e) {
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_GETSTATUS_TEST_001 ERROR: ${e.message}`);
            expect().assertFail();
            done();
            return;
        }
    })

    /*
     * @tc.name:SUB_AUDIO_LOOPBCAK_SETVOLUME_TEST_001
     * @tc.desc:setVolume success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_LOOPBCAK_SETVOLUME_TEST_001', 0, async function (done) {
        let audioLoopback = null;
        try {
            audioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
            await audioLoopback.setVolume(0.5);
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_SETVOLUME_TEST_001 SUCCESS`);
            expect(true).assertTrue();
            done();
        } catch (e) {
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_SETVOLUME_TEST_001 ERROR: ${e.message}`);
            expect(e.code).assertEqual(ERROR_INVALID_PARAM);
            done();
            return;
        }
    })

    /*
     * @tc.name:SUB_AUDIO_LOOPBCAK_ENABLE_TEST_001
     * @tc.desc:enable success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_LOOPBCAK_ENABLE_TEST_001', 0, async function (done) {
        let audioLoopback = null;
        try {
            audioLoopback = await audio.createAudioLoopback(audio.AudioLoopbackMode.HARDWARE);
            let enable = await audioLoopback.enable(true);
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_ENABLE_TEST_001 SUCCESS`);
            expect(enable).assertTrue();
            await audioLoopback.enable(false);
            done();
        } catch (e) {
            console.info(`${TAG}: SUB_AUDIO_LOOPBCAK_ENABLE_TEST_001 ERROR: ${e.message}`);
            expect().assertFail();
            done();
            return;
        }
    })
})