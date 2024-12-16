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
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

const TAG = "[AudioRendererJsUnitTest]";

describe("AudioRendererJsUnitTest", function() {
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
        console.info(TAG + 'beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step, teardown invoked after each testcases
        console.info(TAG + 'afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_STREAM_INFO_SYNC_TEST_001
     * @tc.desc:getStreamInfoSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_STREAM_INFO_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getStreamInfoSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_STREAM_INFO_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(data.samplingRate).assertEqual(audio.AudioSamplingRate.SAMPLE_RATE_48000);
            expect(data.channels).assertEqual(audio.AudioChannel.CHANNEL_1);
            expect(data.sampleFormat).assertEqual(audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE);
            expect(data.encodingType).assertEqual(audio.AudioEncodingType.ENCODING_TYPE_RAW);
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_STREAM_INFO_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_RENDERER_INFO_SYNC_TEST_001
     * @tc.desc:getRendererInfoSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_RENDERER_INFO_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getRendererInfoSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDERER_INFO_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(data.content).assertEqual(audio.ContentType.CONTENT_TYPE_MUSIC);
            expect(data.usage).assertEqual(audio.StreamUsage.STREAM_USAGE_MEDIA);
            expect(data.rendererFlags).assertEqual(0);
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDERER_INFO_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_AUDIO_STREAM_ID_SYNC_TEST_001
     * @tc.desc:getAudioStreamIdSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_AUDIO_STREAM_ID_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getAudioStreamIdSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_AUDIO_STREAM_ID_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(typeof data).assertEqual('number');
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_AUDIO_STREAM_ID_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_BUFFER_SIZE_SYNC_TEST_001
     * @tc.desc:getBufferSizeSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_BUFFER_SIZE_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getBufferSizeSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_BUFFER_SIZE_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(typeof data).assertEqual('number');
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_BUFFER_SIZE_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_MIN_STREAM_VOLUME_SYNC_TEST_001
     * @tc.desc:getMinStreamVolumeSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_MIN_STREAM_VOLUME_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getMinStreamVolumeSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_MIN_STREAM_VOLUME_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(typeof data).assertEqual('number');
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_MIN_STREAM_VOLUME_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_MAX_STREAM_VOLUME_SYNC_TEST_001
     * @tc.desc:getMaxStreamVolumeSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_MAX_STREAM_VOLUME_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getMaxStreamVolumeSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_MAX_STREAM_VOLUME_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(typeof data).assertEqual('number');
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_MAX_STREAM_VOLUME_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_AUDIO_TIME_SYNC_TEST_001
     * @tc.desc:getAudioTimeSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_AUDIO_TIME_SYNC_TEST_001', 0, async function (done) {
        try {
            let audioRenderer = await audio.createAudioRenderer(audioRendererOptions);
            let data = audioRenderer.getAudioTimeSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_AUDIO_TIME_SYNC_TEST_001 SUCCESS, before start: ${data}`);
            expect(data).assertEqual(0);

            await audioRenderer.start();
            data = audioRenderer.getAudioTimeSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_AUDIO_TIME_SYNC_TEST_001 SUCCESS, after start: ${data}`);
            expect(data).assertLarger(0);

            await audioRenderer.stop();
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_AUDIO_TIME_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_001
     * @tc.desc:getRenderRateSync success - RENDER_RATE_NORMAL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_001', 0, async function (done) {
        await audioRenderer.setRenderRate(audio.AudioRendererRate.RENDER_RATE_NORMAL).then(() => {
            console.info('setRenderRate SUCCESS');
            try {
                let data = audioRenderer.getRenderRateSync();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_001 SUCCESS: ${data}`);
                expect(data).assertEqual(audio.AudioRendererRate.RENDER_RATE_NORMAL);
                done();
            } catch (err) {
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_001 ERROR: ${err.message}`);
                expect(false).assertTrue();
                done();
            }
        }).catch((err) => {
            console.error(`setRenderRate ERROR: ${err}`);
        });
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_002
     * @tc.desc:getRenderRateSync success - RENDER_RATE_DOUBLE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_002', 0, async function (done) {
        await audioRenderer.setRenderRate(audio.AudioRendererRate.RENDER_RATE_DOUBLE).then(() => {
            console.info('setRenderRate SUCCESS');
            try {
                let data = audioRenderer.getRenderRateSync();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_002 SUCCESS: ${data}`);
                expect(data).assertEqual(audio.AudioRendererRate.RENDER_RATE_DOUBLE);
                done();
            } catch (err) {
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_002 ERROR: ${err.message}`);
                expect(false).assertTrue();
                done();
            }
        }).catch((err) => {
            console.error(`setRenderRate ERROR: ${err}`);
        });
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_003
     * @tc.desc:getRenderRateSync success - RENDER_RATE_DOUBLE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_003', 0, async function (done) {
        await audioRenderer.setRenderRate(audio.AudioRendererRate.RENDER_RATE_HALF).then(() => {
            console.info('setRenderRate SUCCESS');
            try {
                let data = audioRenderer.getRenderRateSync();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_003 SUCCESS: ${data}`);
                expect(data).assertEqual(audio.AudioRendererRate.RENDER_RATE_HALF);
                done();
            } catch (err) {
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_RENDER_RATE_SYNC_TEST_003 ERROR: ${err.message}`);
                expect(false).assertTrue();
                done();
            }
        }).catch((err) => {
            console.error(`setRenderRate ERROR: ${err}`);
        });
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_UNDERFLOW_COUNT_SYNC_TEST_001
     * @tc.desc:getUnderflowCountSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it('SUB_AUDIO_RENDERER_GET_UNDERFLOW_COUNT_SYNC_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getUnderflowCountSync();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_UNDERFLOW_COUNT_SYNC_TEST_001 SUCCESS: ${data}`);
            expect(typeof data).assertEqual('number');
            done();
        } catch (err) {
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_UNDERFLOW_COUNT_SYNC_TEST_001 ERROR: ${err.message}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_SET_SPEED_TEST_001
     * @tc.desc:setSpeed and getSpeed success
     * @tc.type: FUNC
     * @tc.require: I8OIJL
     */
    it('SUB_AUDIO_RENDERER_SET_SPEED_TEST_001', 0, async function (done) {
        try {
            let speed = 2.0
            audioRenderer.setSpeed(speed);
            let data = audioRenderer.getSpeed();
            expect(data).assertEqual(speed);
            done();
        } catch (error) {
            console.error(`setSpeed ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_VOLUME_TEST_001
     * @tc.desc:getVolume success
     * @tc.type: FUNC
     * @tc.require: I8OIJL
     */
    it('SUB_AUDIO_RENDERER_GET_VOLUME_TEST_001', 0, async function (done) {
        try {
            let data = audioRenderer.getVolume();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_VOLUME_TEST_001 SUCCESS: ${data}`);
            expect(data).assertEqual(1);
            done();
        } catch (error) {
            console.error(`setVolume ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_VOLUME_TEST_002
     * @tc.desc:setVolume and getVolume success
     * @tc.type: FUNC
     * @tc.require: I8OIJL
     */
    it('SUB_AUDIO_RENDERER_GET_VOLUME_TEST_002', 0, async function (done) {
        await audioRenderer.setVolume(0).then(() => {
            try {
                let data = audioRenderer.getVolume();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_VOLUME_TEST_002 SUCCESS: ${data}`);
                expect(data).assertEqual(0);
                done();
            } catch (error) {
                console.error(`setVolume ERROR: ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_VOLUME_TEST_003
     * @tc.desc:setVolume and getVolume success
     * @tc.type: FUNC
     * @tc.require: I8OIJL
     */
    it('SUB_AUDIO_RENDERER_GET_VOLUME_TEST_003', 0, async function (done) {
        await audioRenderer.setVolume(0.5).then(() => {
            try {
                let data = audioRenderer.getVolume();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_VOLUME_TEST_003 SUCCESS: ${data}`);
                expect(data).assertEqual(0.5);
                done();
            } catch (error) {
                console.error(`setVolume ERROR: ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_VOLUME_TEST_004
     * @tc.desc:setVolume and getVolume success
     * @tc.type: FUNC
     * @tc.require: I8OIJL
     */
    it('SUB_AUDIO_RENDERER_GET_VOLUME_TEST_004', 0, async function (done) {
        await audioRenderer.setVolume(1).then(() => {
            try {
                let data = audioRenderer.getVolume();
                console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_VOLUME_TEST_004 SUCCESS: ${data}`);
                expect(data).assertEqual(1);
                done();
            } catch (error) {
                console.error(`setVolume ERROR: ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_RENDERER_GET_SILENT_MODE_AND_MIX_WITH_OTHERS_TEST_001
     * @tc.desc:setSilentModeAndMixWithOthers and getSilentModeAndMixWithOthers success
     * @tc.type: FUNC
     * @tc.require: I9P7F9
     */
    it('SUB_AUDIO_RENDERER_GET_SILENT_MODE_AND_MIX_WITH_OTHERS_TEST_001', 0, async function (done) {
        try {
            audioRenderer.setSilentModeAndMixWithOthers(true);
            let data = audioRenderer.getSilentModeAndMixWithOthers();
            console.info(`${TAG}: SUB_AUDIO_RENDERER_GET_SILENT_MODE_AND_MIX_WITH_OTHERS_TEST_001 SUCCESS: ${data}`);
            expect(data).assertEqual(true);
            done();
        } catch (err) {
            console.error(`${TAG}: SUB_AUDIO_RENDERER_GET_SILENT_MODE_AND_MIX_WITH_OTHERS_TEST_001 ERROR: ${err}`);
            done();
        }
    })
})
