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


describe("AudioGroupManagerJsUnitTest", function () {
    const ACCESSIBILITY = 5;
    const ALARM = 4;
    let audioManager = audio.getAudioManager();
    let audioVolumeManager = audioManager.getVolumeManager();
    const GROUP_ID = audio.DEFAULT_VOLUME_GROUP_ID;
    let audioVolumeGroupManager;
    let audioRenderer;
    let inputDeviceDesc;
    let outputDeviceDesc;
    const ERROR_INPUT_INVALID = '401';
    const ERROR_INVALID_PARAM = '6800101';
    const MIN_VOLUME_LEVEL = 0;
    const MAX_VOLUME_LEVEL = 15;

    let audioStreamInfo = {
        samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
        channels: audio.AudioChannel.CHANNEL_2,
        sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
        encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    }
    let audioRendererInfo = {
        content: audio.ContentType.CONTENT_TYPE_MUSIC,
        usage: audio.StreamUsage.STREAM_USAGE_MUSIC,
        rendererFlags: 0
    }
    let audioRendererOptions = {
        streamInfo: audioStreamInfo,
        rendererInfo: audioRendererInfo
    }
    let bufferSize;

    beforeAll(async function () {
        audioVolumeGroupManager = await audioVolumeManager.getVolumeGroupManager(GROUP_ID).catch((err) => {
            console.error("Create audioVolumeManager error " + JSON.stringify(err));
        });
        console.info("Create audioVolumeManager finished");

        // input testsuit setup step，setup invoked before all testcases
        console.info('AudioGroupManagerJsUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('AudioGroupManagerJsUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('AudioGroupManagerJsUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('AudioGroupManagerJsUnitTest:afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_001
     * @tc.desc:verify alarm Volume set successfully
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_001", 0, async function (done) {
        let volume = 4;
        audioVolumeGroupManager.setVolume(ALARM, volume, (err) => {
            if (err) {
                console.error(`Failed to set ALARM volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful volume ALARM setting.');
            expect(true).assertTrue();

            audioVolumeGroupManager.getVolume(ALARM, (err, value) => {
                if (err) {
                    console.error(`Failed to obtain ALARM volume. ${err}`);
                    expect(false).assertTrue();
                    done();
                    return;
                }
                console.info(`get alarm volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            })
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_002
     * @tc.desc:Verify whether the abnormal volume setting is successful
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_002", 0, async function (done) {
        let volume = -1;
        audioVolumeGroupManager.setVolume(ALARM, volume, (err) => {
            if (err) {
                console.error(`Failed to set ALARM volume. ${err}`);
                expect(true).assertTrue();
                done();
                return;
            }
            console.info('Callback invoked to indicate a successful volume setting.');
            expect(false).assertTrue();
            done();
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_003
     * @tc.desc:Verify whether the abnormal volume setting is successful
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_003", 0, async function (done) {
        let volume = 17;
        audioVolumeGroupManager.setVolume(ALARM, volume, (err) => {
            if (err) {
                console.error(`Failed to set ALARM volume. ${err}`);
                expect(true).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful ALARM volume setting.');
            expect(false).assertTrue();
            done();
        })
    });

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_004
     * @tc.desc:verify alarm Volume set successfully
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_004", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(ACCESSIBILITY, volume, (err) => {
            if (err) {
                console.error(`Failed to set ACCESSIBILITY volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful ACCESSIBILITY volume setting.');
            expect(true).assertTrue();

            audioVolumeGroupManager.getVolume(ACCESSIBILITY, (err, value) => {
                if (err) {
                    console.error(`Failed to obtain ACCESSIBILITY volume. ${err}`);
                    expect(false).assertTrue();
                    done();
                    return;
                }
                console.info(`get ACCESSIBILITY volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            })
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_005
     * @tc.desc:Verify whether the abnormal volume setting is successful
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_005", 0, async function (done) {
        let volume = -3;
        audioVolumeGroupManager.setVolume(ACCESSIBILITY, volume, (err) => {
            if (err) {
                console.error(`005.Failed to set ALARM volume. ${err}`);
                expect(true).assertTrue();
                done();
                return;
            }
            console.info('005.invoked to indicate a successful ACCESSIBILITY volume setting.');
            expect(false).assertTrue();
            done();
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_006
     * @tc.desc:Verify whether the abnormal volume setting is successful
     * @tc.type: FUNC
     * @tc.require: issueNumber
     */
    it("SUB_AUDIO_GROUP_MANAGER_SET_VOLUME_006", 0, async function (done) {
        let volume = 16;
        audioVolumeGroupManager.setVolume(ACCESSIBILITY, volume, (err) => {
            if (err) {
                console.error(`006.Failed to set ALARM volume. ${err}`);
                expect(true).assertTrue();
                done();
                return;
            }
            console.info('006.invoked to indicate a successful ACCESSIBILITY volume setting.');
            expect(false).assertTrue();
            done();
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_001
     * @tc.desc:verify getVolumeSync get volume successfully - VOICE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_001", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.VOICE_CALL, volume, (err) => {
            if (err) {
                console.error(`Failed to set VOICE_CALL volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful VOICE_CALL volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.VOICE_CALL);
                console.info(`get VOICE_CALL volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain VOICE_CALL volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_002
     * @tc.desc:verify getVolumeSync get volume successfully - RINGTONE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_002", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.RINGTONE);
            console.info(`get MEDIA volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value <= MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain RINGTONE volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_003
     * @tc.desc:verify getVolumeSync get volume successfully - MEDIA
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_003", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.MEDIA, volume, (err) => {
            if (err) {
                console.error(`Failed to set MEDIA volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful MEDIA volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.MEDIA);
                console.info(`get MEDIA volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain MEDIA volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_004
     * @tc.desc:verify getVolumeSync get volume successfully - ALARM
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_004", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.ALARM, volume, (err) => {
            if (err) {
                console.error(`Failed to set ALARM volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful ALARM volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.ALARM);
                console.info(`get ALARM volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain ALARM volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_005
     * @tc.desc:verify getVolumeSync get volume successfully - ACCESSIBILITY
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_005", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.ACCESSIBILITY, volume, (err) => {
            if (err) {
                console.error(`Failed to set ACCESSIBILITY volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful ACCESSIBILITY volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.ACCESSIBILITY);
                console.info(`get ACCESSIBILITY volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain ACCESSIBILITY volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_006
     * @tc.desc:verify getVolumeSync get volume successfully - VOICE_ASSISTANT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_006", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.VOICE_ASSISTANT, volume, (err) => {
            if (err) {
                console.error(`Failed to set VOICE_ASSISTANT volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful VOICE_ASSISTANT volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.VOICE_ASSISTANT);
                console.info(`get VOICE_ASSISTANT volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain VOICE_ASSISTANT volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_007
     * @tc.desc:verify getVolumeSync get volume successfully - ULTRASONIC
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_007", 0, async function (done) {
        let volume = 6;
        audioVolumeGroupManager.setVolume(audio.AudioVolumeType.ULTRASONIC, volume, (err) => {
            if (err) {
                console.error(`Failed to set ULTRASONIC volume. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate a successful ULTRASONIC volume setting.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.ULTRASONIC);
                console.info(`get ULTRASONIC volume is obtained ${value}.`);
                expect(value).assertEqual(volume);
                done();
            } catch (err) {
                console.error(`Failed to obtain ULTRASONIC volume. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_008
     * @tc.desc:verify getVolumeSync get volume successfully - ALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_008", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getVolumeSync(audio.AudioVolumeType.ULTRASONIC);
            console.info(`get ULTRASONIC volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value <= MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain ALL volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_009
     * @tc.desc:verify getVolumeSync get volume fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_009", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getVolumeSync();
            console.info(`get volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_010
     * @tc.desc:verify getVolumeSync get volume fail(401) - Invalid param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_010", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getVolumeSync("Invalid type");
            console.info(`get volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_011
     * @tc.desc:verify getVolumeSync get volume fail(6800101) - Invalid param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_VOLUME_SYNC_011", 0, async function (done) {
        let invalidVolumeType = 10000;
        try {
            let value = audioVolumeGroupManager.getVolumeSync(invalidVolumeType);
            console.info(`get volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_001
     * @tc.desc:verify getMinVolumeSync get min volume successfully - VOICE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_001", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.VOICE_CALL);
            console.info(`get VOICE_CALL min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain VOICE_CALL min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_002
     * @tc.desc:verify getMinVolumeSync get min volume successfully - RINGTONE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_002", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.RINGTONE);
            console.info(`get RINGTONE min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain RINGTONE min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_003
     * @tc.desc:verify getMinVolumeSync get min volume successfully - MEDIA
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_003", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.MEDIA);
            console.info(`get MEDIA min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain MEDIA min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_004
     * @tc.desc:verify getMinVolumeSync get min volume successfully - ALARM
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_004", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.ALARM);
            console.info(`get ALARM min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain ALARM min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_005
     * @tc.desc:verify getMinVolumeSync get min volume successfully - ACCESSIBILITY
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_005", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.ACCESSIBILITY);
            console.info(`get ACCESSIBILITY min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain ACCESSIBILITY min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_006
     * @tc.desc:verify getMinVolumeSync get min volume successfully - VOICE_ASSISTANT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_006", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.VOICE_ASSISTANT);
            console.info(`get VOICE_ASSISTANT min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain VOICE_ASSISTANT min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_007
     * @tc.desc:verify getMinVolumeSync get min volume successfully - ULTRASONIC
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_007", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.ULTRASONIC);
            console.info(`get ULTRASONIC min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain ULTRASONIC min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_008
     * @tc.desc:verify getMinVolumeSync get min volume successfully - ALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_008", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(audio.AudioVolumeType.ALL);
            console.info(`get ALL min volume is obtained ${value}.`);
            expect(value >= MIN_VOLUME_LEVEL && value < MAX_VOLUME_LEVEL).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain ALL min volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_009
     * @tc.desc:verify getMinVolumeSync get min volume fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_009", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync();
            console.info(`get min volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain min volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_010
     * @tc.desc:verify getMinVolumeSync get volume fail(401) - Invalid param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_010", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync("Invalid type");
            console.info(`get min volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain min volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_011
     * @tc.desc:verify getMinVolumeSync get min volume fail(6800101) - Invalid param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MIN_VOLUME_SYNC_011", 0, async function (done) {
        let invalidVolumeType = 10000;
        try {
            let value = audioVolumeGroupManager.getMinVolumeSync(invalidVolumeType);
            console.info(`get min volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain min volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_001
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - VOICE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_001", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.VOICE_CALL);
            console.info(`get VOICE_CALL max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain VOICE_CALL max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_002
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - RINGTONE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_002", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.RINGTONE);
            console.info(`get RINGTONE max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain RINGTONE max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_003
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - MEDIA
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_003", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.MEDIA);
            console.info(`get MEDIA max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain MEDIA max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_004
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - ALARM
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_004", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.ALARM);
            console.info(`get ALARM max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain ALARM max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_005
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - ACCESSIBILITY
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_005", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.ACCESSIBILITY);
            console.info(`get ACCESSIBILITY max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain ACCESSIBILITY max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_006
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - VOICE_ASSISTANT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_006", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.VOICE_ASSISTANT);
            console.info(`get VOICE_ASSISTANT max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain VOICE_ASSISTANT max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_007
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - ULTRASONIC
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_007", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.ULTRASONIC);
            console.info(`get ULTRASONIC max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain ULTRASONIC max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_008
     * @tc.desc:verify getMaxVolumeSync get max volume successfully - ALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_008", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(audio.AudioVolumeType.ALL);
            console.info(`get ALL max volume is obtained ${value}.`);
            expect(value).assertEqual(MAX_VOLUME_LEVEL);
            done();
        } catch (err) {
            console.error(`Failed to obtain ALL max volume. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_009
     * @tc.desc:verify getMaxVolumeSync get max volume fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_009", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync();
            console.info(`get max volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain max volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_010
     * @tc.desc:verify getMaxVolumeSync get volume fail(401) - Invalid param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_010", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync("Invalid type");
            console.info(`get max volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain max volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_011
     * @tc.desc:verify getMaxVolumeSync get max volume fail(6800101) - Invalid param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_VOLUME_SYNC_011", 0, async function (done) {
        let invalidVolumeType = 10000;
        try {
            let value = audioVolumeGroupManager.getMaxVolumeSync(invalidVolumeType);
            console.info(`get max volume is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain max volume. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_001
     * @tc.desc:verify isMuteSync get mute status successfully - VOICE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_001", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.VOICE_CALL, true, (err) => {
            if (err) {
                console.error(`Failed to mute VOICE_CALL stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that VOICE_CALL stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.VOICE_CALL);
                console.info(`The mute status of VOICE_CALL stream is obtained ${value}.`);
                expect(value).assertEqual(false);
                done();
            } catch (err) {
                console.error(`Failed to obtain VOICE_CALL mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_002
     * @tc.desc:verify isMuteSync get mute status successfully - RINGTONE
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_002", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.RINGTONE);
            console.info(`The mute status of RINGTONE stream is obtained ${value}.`);
            expect(typeof value).assertEqual('boolean');
            done();
        } catch (err) {
            console.error(`Failed to obtain RINGTONE mute status. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_003
     * @tc.desc:verify isMuteSync get mute status successfully - MEDIA
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_003", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.MEDIA, true, (err) => {
            if (err) {
                console.error(`Failed to mute MEDIA stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that MEDIA stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.MEDIA);
                console.info(`The mute status of MEDIA stream is obtained ${value}.`);
                expect(value).assertTrue();
                done();
            } catch (err) {
                console.error(`Failed to obtain MEDIA mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_004
     * @tc.desc:verify isMuteSync get mute status successfully - ALARM
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_004", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.ALARM, true, (err) => {
            if (err) {
                console.error(`Failed to mute ALARM stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that ALARM stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.ALARM);
                console.info(`The mute status of ALARM stream is obtained ${value}.`);
                expect(value).assertEqual(false);
                done();
            } catch (err) {
                console.error(`Failed to obtain ALARM mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_005
     * @tc.desc:verify isMuteSync get mute status successfully - ACCESSIBILITY
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_005", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.ACCESSIBILITY, true, (err) => {
            if (err) {
                console.error(`Failed to mute ACCESSIBILITY stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that ACCESSIBILITY stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.ACCESSIBILITY);
                console.info(`The mute status of ACCESSIBILITY stream is obtained ${value}.`);
                expect(value).assertEqual(false);
                done();
            } catch (err) {
                console.error(`Failed to obtain ACCESSIBILITY mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_006
     * @tc.desc:verify isMuteSync get mute status successfully - VOICE_ASSISTANT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_006", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.VOICE_ASSISTANT, true, (err) => {
            if (err) {
                console.error(`Failed to mute VOICE_ASSISTANT stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that VOICE_ASSISTANT stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.VOICE_ASSISTANT);
                console.info(`The mute status of VOICE_ASSISTANT stream is obtained ${value}.`);
                expect(value).assertEqual(false);
                done();
            } catch (err) {
                console.error(`Failed to obtain VOICE_ASSISTANT mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_007
     * @tc.desc:verify isMuteSync get mute status successfully - ULTRASONIC
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_007", 0, async function (done) {
        audioVolumeGroupManager.mute(audio.AudioVolumeType.ULTRASONIC, true, (err) => {
            if (err) {
                console.error(`Failed to mute ULTRASONIC stream. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('invoked to indicate that ULTRASONIC stream is muted.');
            expect(true).assertTrue();

            try {
                let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.ULTRASONIC);
                console.info(`The mute status of ULTRASONIC stream is obtained ${value}.`);
                expect(value).assertTrue();
                done();
            } catch (err) {
                console.error(`Failed to obtain ULTRASONIC mute status. ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_008
     * @tc.desc:verify isMuteSync get mute status successfully - ALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_008", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.isMuteSync(audio.AudioVolumeType.ALL);
            console.info(`The mute status of ALL stream is obtained ${value}.`);
            expect(typeof value).assertEqual('boolean');
            done();
        } catch (err) {
            console.error(`Failed to obtain ALL mute status. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_009
     * @tc.desc:verify isMuteSync get mute status fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_009", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.isMuteSync();
            console.info(`The mute status of the stream is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain mute status. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_010
     * @tc.desc:verify isMuteSync get mute status fail(401) - Invalid param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_010", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.isMuteSync("Invalid type");
            console.info(`The mute status of the stream is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain mute status. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_011
     * @tc.desc:verify isMuteSync get mute status fail(6800101) - Invalid param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MUTE_SYNC_011", 0, async function (done) {
        let invalidVolumeType = 10000;
        try {
            let value = audioVolumeGroupManager.isMuteSync(invalidVolumeType);
            console.info(`The mute status of the stream is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain mute status. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_RINGER_MODE_SYNC_001
     * @tc.desc:verify getRingerModeSync get ringer mode successfully
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_RINGER_MODE_SYNC_001", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getRingerModeSync();
            console.info(`invoked to indicate that the ringer mode is obtained ${value}.`);
            expect(typeof value).assertEqual('number');
            done();
        } catch (err) {
            console.error(`Failed to obtain the ringer mode. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_IS_MICROPHONE_MUTE_SYNC_001
     * @tc.desc:verify isMicrophoneMuteSync get microphone mute status successfully
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_IS_MICROPHONE_MUTE_SYNC_001", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.isMicrophoneMuteSync();
            console.info(`invoked to indicate that the mute status of the microphone is obtained ${value}.`);
            expect(typeof value).assertEqual('boolean');
            done();
        } catch (err) {
            console.error(`Failed to obtain the mute status of the microphone. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_001
     * @tc.desc:verify getSystemVolumeInDbSync get volume db successfully - <VOICE_CALL, SPEAKER>
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_001", 0, async function (done) {
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL, volumeLevel,
                audio.DeviceType.SPEAKER);
            console.info(`get <VOICE_CALL, SPEAKER> volume db is obtained ${value}.`);
            expect(typeof value).assertEqual('number');
            done();
        } catch (err) {
            console.error(`Failed to obtain <VOICE_CALL, SPEAKER> volume db. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_100
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid param count : 0
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_100", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync();
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_101
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid param count : 1
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_101", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_102
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid param count : 2
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_102", 0, async function (done) {
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL,
                volumeLevel);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_103
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid first param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_103", 0, async function (done) {
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync("Invalid type", volumeLevel,
                audio.DeviceType.SPEAKER);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_104
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid second param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_104", 0, async function (done) {
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL,
                "Invalid type", audio.DeviceType.SPEAKER);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_105
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(401) - Invalid third param type : "Invalid type"
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_105", 0, async function (done) {
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL,
                volumeLevel, "Invalid type");
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INPUT_INVALID);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_106
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(6800101) - Invalid first param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_106", 0, async function (done) {
        let invalidVolumeType = 10000;
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(invalidVolumeType, volumeLevel,
                audio.DeviceType.SPEAKER);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_107
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(6800101) - Invalid second param value : 100
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_107", 0, async function (done) {
        let invalidVolumeLevel = 100;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL,
                invalidVolumeLevel, audio.DeviceType.SPEAKER);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_108
     * @tc.desc:verify getSystemVolumeInDbSync get volume db fail(6800101) - Invalid third param value : 10000
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_SYSTEM_VOLUME_IN_DB_SYNC_108", 0, async function (done) {
        let invalidDeviceType = 10000;
        let volumeLevel = 3;
        try {
            let value = audioVolumeGroupManager.getSystemVolumeInDbSync(audio.AudioVolumeType.VOICE_CALL,
                volumeLevel, invalidDeviceType);
            console.info(`get volume db is obtained ${value}.`);
            expect(false).assertTrue();
            done();
        } catch (err) {
            console.error(`Failed to obtain volume db. ${err}`);
            expect(err.code).assertEqual(ERROR_INVALID_PARAM);
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_OUTPUT_DEVICE_001
     * @tc.desc:verify getMaxAmplitudeForOutputDevice get outputDevice max amplitude successfully - <SPEAKER>
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_OUTPUT_DEVICE_001", 0, async function (done) {
        let routingManager = audio.getAudioManager().getRoutingManager();
        let rendererInfo = {
            content : audio.ContentType.CONTENT_TYPE_MUSIC,
            usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags : 0 }

        await routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo, (e, desc)=>{
            if (e) {
                console.error(`get ouput device failed ERROR: ${e.message}`);
                expect(false).assertTrue();
                done();
            }
            if (desc.length === 0) {
                console.error("get ouput device failed");
                return;
            }
            outputDeviceDesc = desc[0];
            console.info("get outputDeviceId finished");
        });

        audioRenderer = await audio.createAudioRenderer(audioRendererOptions).catch((err) => {
            console.error("Create audioRenderer error " + JSON.stringify(err));
            return;
        });
        console.info("Create audioRenderer finished");

        try {
            bufferSize = await audioRenderer.getBufferSize();
            await audioRenderer.start();
        } catch (err) {
            console.err("audioRenderer getBufferSize/start error" + JSON.stringify(err));
            return;
        }
        console.info("audioRenderer getBufferSize/start finished");
        
        try {
            let buff = new ArrayBuffer(bufferSize);
            let num = 0;
            const uInt8 = new Uint8Array(buff);
            for (let i = 0; i < buff.byteLength; ++i) {
                uInt8[i] = Math.ceil((Math.random() * 100)) + 1; // data from 1 ~ 101
            }
            while (num < 10) { // add 10 frames;
                await audioRenderer.write(buff);
                num++;
            }
            try {
                await audioVolumeGroupManager.getMaxAmplitudeForOutputDevice(outputDeviceDesc);
                await audioVolumeGroupManager.getMaxAmplitudeForOutputDevice(outputDeviceDesc).then(
                    (maxAmplitude) => {
                        console.info(`get maxAmplitude finished ${maxAmplitude}.`);
                        expect(maxAmplitude >= 0).assertTrue();
                        done();
                    }).catch((err) => {
                        console.error("get maxAmplitude error" + JSON.stringify(err));
                        return;
                    });
            } catch (err) {
                console.error(`Failed to getMaxAmplitude. ${err}`);
                expect(false).assertTrue();
                done();
            }
        } catch (err) {
            console.error("play render failed error " + JSON.stringify(err));
        };

        try {
            audioRenderer.stop();
            audioRenderer.release();
            console.info("audioRenderer stop/release finished");
        } catch (err) {
            console.error("audioRenderer stop/release error " + JSON.stringify(err));
        };
    })

    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_OUTPUT_DEVICE_002
     * @tc.desc:verify getMaxAmplitudeForOutputDevice get output devive max amplitude without render successfully - <Random Id>
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_OUTPUT_DEVICE_002", 0, async function (done) {
        let routingManager = audio.getAudioManager().getRoutingManager();
        let rendererInfo = {
            content : audio.ContentType.CONTENT_TYPE_MUSIC,
            usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags : 0 }
 
        routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo).then(
            (desc) => {
                if (desc.length === 0) {
                    console.error("get ouput device failed");
                    return;
                } else {
                    outputDeviceDesc = desc[0];
                }
                console.info("get outputDeviceId finished");
            }).catch((err) => {
                console.error("get outputDeviceId error" + JSON.stringify(err));
                return;
            });
 
        try {
            audioVolumeGroupManager.getMaxAmplitudeForOutputDevice(outputDeviceDesc).then(
                (maxAmplitude) => {
                    console.info(`get maxAmplitude finished ${maxAmplitude}.`);
                    expect(maxAmplitude == 0).assertTrue();
                    done();
                }).catch((err) => {
                    console.error("get maxAmplitude error" + JSON.stringify(err));
                    return;
                });
        } catch (err) {
            console.error(`Failed to getMaxAmplitude. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })
 
    /*
     * @tc.name:SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_INPUT_DEVICE_003
     * @tc.desc:verify getMaxAmplitude get inputDevice max amplitude successfully - <MIC>
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_GROUP_MANAGER_GET_MAX_AMPLITUDE_FOR_INPUT_DEVICE_003", 0, async function (done) {
        let routingManager = audio.getAudioManager().getRoutingManager();
        let capturerInfo = {
            content : audio.ContentType.CONTENT_TYPE_MUSIC,
            usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
            capturerFlags : 0 }
 
        routingManager.getPreferredInputDeviceForCapturerInfo(capturerInfo).then(
            (desc) => {
                if (desc.length === 0) {
                    console.error("get ouput device failed");
                    return;
                } else {
                    inputDeviceDesc = desc[0];
                }
                console.info("get inputDeviceId finished");
            }).catch((err) => {
                console.error("get inputDeviceId error" + JSON.stringify(err));
                return;
            });
 
        try {
            audioVolumeGroupManager.getMaxAmplitudeForInputDevice(inputDeviceDesc).then(
                (maxAmplitude) => {
                    console.info(`get maxAmplitude finished ${maxAmplitude}.`);
                    expect(maxAmplitude >= 0).assertTrue();
                    done();
                }).catch((err) => {
                    console.error("get maxAmplitude error" + JSON.stringify(err));
                    return;
                });
        } catch (err) {
            console.error(`Failed to getMaxAmplitude. ${err}`);
            expect(false).assertTrue();
            done();
        }
    })
})