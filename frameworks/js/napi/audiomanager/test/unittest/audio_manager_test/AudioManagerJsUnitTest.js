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


describe("AudioManagerJsUnitTest", function () {
    let audioManager = audio.getAudioManager();

    beforeAll(async function () {

        // input testsuit setup step，setup invoked before all testcases
        console.info('AudioManagerJsUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('AudioManagerJsUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('AudioManagerJsUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('AudioManagerJsUnitTest:afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_001
     * @tc.desc:getAudioScene success - AUDIO_SCENE_DEFAULT
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_001", 0, async function (done) {
        audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT, (err) => {
            if (err) {
                console.error(`001.Failed to set the audio scene mode. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('001.invoked to indicate a successful setting of the audio scene mode.');
            expect(true).assertTrue();

            try {
                let value = audioManager.getAudioSceneSync();
                console.info(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_001 SUCCESS: ${value}.`);
                expect(value).assertEqual(audio.AudioScene.AUDIO_SCENE_DEFAULT);
                done();
            } catch (err) {
                console.error(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_001 ERROR: ${err}`);
                expect(false).assertTrue();
                done();
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_002
     * @tc.desc:getAudioScene success - AUDIO_SCENE_RINGING
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_002", 0, async function (done) {
        audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_RINGING, (err) => {
            if (err) {
                console.error(`002.Failed to set the audio scene mode. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('002.invoked to indicate a successful setting of the audio scene mode.');
            expect(true).assertTrue();

            try {
                let value = audioManager.getAudioSceneSync();
                console.info(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_002 SUCCESS: ${value}.`);
                expect(value).assertEqual(audio.AudioScene.AUDIO_SCENE_RINGING);
            } catch (err) {
                console.error(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_002 ERROR: ${err}`);
                expect(false).assertTrue();
            } finally {
                audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT, (err) => {
                    if (err) {
                        console.error(`002.Failed to reset the audio scene mode to AUDIO_SCENE_DEFAULT. ${err}`);
                    }
                    done();
                })
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_003
     * @tc.desc:getAudioScene success - AUDIO_SCENE_PHONE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_003", 0, async function (done) {
        audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_PHONE_CALL, (err) => {
            if (err) {
                console.error(`003.Failed to set the audio scene mode. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('003.invoked to indicate a successful setting of the audio scene mode.');
            expect(true).assertTrue();

            try {
                let value = audioManager.getAudioSceneSync();
                console.info(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_003 SUCCESS: ${value}.`);
                expect(value).assertEqual(audio.AudioScene.AUDIO_SCENE_PHONE_CALL);
            } catch (err) {
                console.error(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_003 ERROR: ${err}`);
                expect(false).assertTrue();
            } finally {
                audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT, (err) => {
                    if (err) {
                        console.error(`003.Failed to reset the audio scene mode to AUDIO_SCENE_DEFAULT. ${err}`);
                    }
                    done();
                })
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_004
     * @tc.desc:getAudioScene success - AUDIO_SCENE_PHONE_CALL
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_004", 0, async function (done) {
        audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_VOICE_CHAT, (err) => {
            if (err) {
                console.error(`004.Failed to set the audio scene mode. ${err}`);
                expect(false).assertTrue();
                done();
                return;
            }
            console.info('004.invoked to indicate a successful setting of the audio scene mode.');
            expect(true).assertTrue();

            try {
                let value = audioManager.getAudioSceneSync();
                console.info(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_004 SUCCESS: ${value}.`);
                expect(value).assertEqual(audio.AudioScene.AUDIO_SCENE_VOICE_CHAT);
            } catch (err) {
                console.error(`SUB_AUDIO_MANAGER_GET_AUDIO_SCENE_SYNC_004 ERROR: ${err}`);
                expect(false).assertTrue();
            } finally {
                audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT, (err) => {
                    if (err) {
                        console.error(`004.Failed to reset the audio scene mode to AUDIO_SCENE_DEFAULT. ${err}`);
                    }
                    done();
                })
            }
        })
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_001
     * @tc.desc:setExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_001", 0, async function (done) {
        try {
            await audioManager.setExtraParameters('mmi');
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_001 SUCCESS: ${err}.`);
            expect(err.code).assertEqual('401');
        }
        done();
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_002
     * @tc.desc:setExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_002", 0, async function (done) {
        try {
            await audioManager.setExtraParameters('mmi', 1);
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_002 SUCCESS: ${err}.`);
            expect(err.code).assertEqual('401');
        }
        done();
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_003
     * @tc.desc:setExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_003", 0, function (done) {
        try {
            audioManager.setExtraParameters('mmi', { 'test': 'on' }).then(() => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_003 FAIL`);
                expect(false).assertTrue();
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_003 SUCCESS: ${err}.`);
                expect(err.code).assertEqual(6800101);
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_003 FAIL: ${err}.`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_004
     * @tc.desc:setExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_004", 0, function (done) {
        try {
            audioManager.setExtraParameters('mmi', { }).then(() => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_004 FAIL`);
                expect(false).assertTrue();
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_004 SUCCESS: ${err}.`);
                expect(err.code).assertEqual(6800101);
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_004 FAIL: ${err}.`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_005
     * @tc.desc:setExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_005", 0, function (done) {
        try {
            audioManager.setExtraParameters('mmi', { 'mmi_test': 'on' }).then(() => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_005 SUCCESS`);
                expect(true).assertTrue();
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_005 FAIL: ${err}.`);
                expect(true).assertTrue();
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_SET_EXTRA_PARAMETERS_005 try-catch FAIL: ${err}.`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_001
     * @tc.desc:getExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_001", 0, async function (done) {
        try {
            await audioManager.getExtraParameters();
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_001 SUCCESS: ${err}.`);
            expect(err.code).assertEqual('401');
        }
        done();
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_002
     * @tc.desc:getExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_002", 0, async function (done) {
        try {
            await audioManager.getExtraParameters('mmi', 1);
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_002 SUCCESS: ${err}.`);
            expect(err.code).assertEqual('401');
        }
        done();
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_003
     * @tc.desc:getExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_003", 0, function (done) {
        try {
            audioManager.getExtraParameters('mmi', ['unittest']).then((value) => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_003 FAIL`);
                expect(false).assertTrue();
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_003 SUCCESS: ${err}.`);
                expect(err.code).assertEqual(6800101);
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_003 FAIL try-catch`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_004
     * @tc.desc:getExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_004", 0, function (done) {
        try {
            audioManager.getExtraParameters('mmi').then((value) => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_004 SUCCESS ` + value.length);
                expect(value.length).assertEqual(17);
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_004 FAIL: ${err.code}.`);
                expect(true).assertTrue();
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_004 FAIL try-catch: ${err.code}.`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_005
     * @tc.desc:getExtraParameters - Promise
     * @tc.type: FUNC
     * @tc.require: SR20231218728353
     */
    it("SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_005", 0, function (done) {
        try {
            audioManager.getExtraParameters('mmi', ['getSmartPANV']).then((value) => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_005 SUCCESS ` + value.length);
                expect(value.length).assertEqual(1);
                done();
            }).catch(err => {
                console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_005 FAIL: ${err.code}.`);
                expect(true).assertTrue();
                done();
            });
        } catch (err) {
            console.info(`SUB_AUDIO_MANAGER_GET_EXTRA_PARAMETERS_005 FAIL try-catch: ${err}.`);
            expect(false).assertTrue();
            done();
        }
    })
})