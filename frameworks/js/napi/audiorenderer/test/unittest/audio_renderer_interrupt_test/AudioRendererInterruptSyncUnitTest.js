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

import audio from '@ohos.multimedia.audio';

describe("AudioRendererInterruptSyncUnitTest", function() {
    const ERROR_INPUT_INVALID = '401';
    const ERROR_INVALID_PARAM = '6800101';
    let audioStreamInfo = {
        samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
        channels: audio.AudioChannel.CHANNEL_1,
        sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
        encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    }
    let audioRendererInfo = {
        content: audio.ContentType.CONTENT_TYPE_MUSIC,
        usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
        rendererFlags: 0
    }
    let audioRendererOptions = {
        streamInfo: audioStreamInfo,
        rendererInfo: audioRendererInfo
    }
    let audioRenderer;

    beforeAll(async function () {
        // input testsuit setup step, setup invoked before all testcases
        try {
            audioRenderer = await audio.createAudioRenderer(audioRendererOptions);
            console.info(`${TAG}: AudioRenderer created SUCCESS, state: ${audioRenderer.state}`);
        } catch (err) {
            console.error(`${TAG}: AudioRenderer created ERROR: ${err.message}`);
        }
        console.info(TAG + 'beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step, teardown invoked after all testcases
        audioRenderer.release().then(() => {
            console.info(`${TAG}: AudioRenderer release : SUCCESS`);
        }).catch((err) => {
            console.info(`${TAG}: AudioRenderer release :ERROR : ${err.message}`);
        });
        console.info(TAG + 'afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step, setup invoked before each testcases
        console.info('beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step, teardown invoked after each testcases
        console.info('afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_001
     * @tc.desc:setInterruptModeSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_001', 0, async function (done) {
        try {
            audioRenderer.setInterruptModeSync(audio.InterruptMode.INDEPENDENT_MODE);
            console.info(`setInterruptModeSync success`);
            expect(true).assertTrue();
        } catch (err) {
            console.error(`setInterruptModeSync error: ${err}`);
            expect(false).assertTrue();
        }
        done()
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_002
     * @tc.desc:setInterruptModeSync fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_002', 0, async function (done) {
        try {
            audioRenderer.setInterruptModeSync();
            console.info(`setInterruptModeSync success`);
            expect(false).assertTrue();
        } catch (err) {
            console.error(`setInterruptModeSync error: ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
        }
        done()
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_003
     * @tc.desc:setInterruptModeSync fail(401) - Invalid param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_003', 0, async function (done) {
        try {
            audioRenderer.setInterruptModeSync("Invalid type");
            console.info(`setInterruptModeSync success`);
            expect(false).assertTrue();
        } catch (err) {
            console.error(`setInterruptModeSync error: ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
        }
        done()
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_004
     * @tc.desc:setInterruptModeSync fail(6800101) - Invalid param value : 100
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_INTERRUPT_SYNC_TEST_004', 0, async function (done) {
        try {
            audioRenderer.setInterruptModeSync(100);
            console.info(`setInterruptModeSync success`);
            expect(false).assertTrue();
        } catch (err) {
            console.error(`setInterruptModeSync error: ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
        }
        done()   
    })
})
