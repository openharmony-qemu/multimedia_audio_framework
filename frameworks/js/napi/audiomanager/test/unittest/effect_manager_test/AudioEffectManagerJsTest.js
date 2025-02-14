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

const TAG = "[AudioEffectManagerJsTest]";

describe("AudioEffectManagerJsTest", function () {
  const ERROR_NO_PERMISSSION = '201';
  const ERROR_INPUT_INVALID = '401';
  const ERROR_INVALID_PARAM = '6800101';
  const ERROR_SYSTEM = '6800301';

  beforeAll(async function () {
    console.info(TAG + "AudioEffectManagerJsTest:beforeAll called");
  })

  afterAll(function () {
    console.info(TAG + 'AudioEffectManagerJsTest:afterAll called')
  })

  beforeEach(function () {
    console.info(TAG + 'AudioEffectManagerJsTest:beforeEach called')
  })

  afterEach(function () {
    console.info(TAG + 'AudioEffectManagerJsTest:afterEach called')
  })

  function sleep(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
  }

  /*
   * @tc.name:getSupportedAudioEffectProperty001
   * @tc.desc:Get getSupportedAudioEffectProperty success - check repeats data
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getSupportedAudioEffectProperty001", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let audioEffectArray = audioEffectManager.getSupportedAudioEffectProperty();
      console.info(`${TAG} getSupportedAudioEffectProperty001 success:${JSON.stringify(audioEffectArray)}`);
      for (let i = 0; i < audioEffectArray.length; i++) {
        expect(audioEffectArray[i].category !== ""
          && audioEffectArray[i].category !== undefined).assertTrue();
        expect(audioEffectArray[i].name !== ""
          && audioEffectArray[i].name !== undefined).assertTrue();
        expect(audioEffectArray[i].flag !== "0"
          && audioEffectArray[i].flag !== "1").assertTrue();
      }
      done();
    } catch (e) {
      console.error(`${TAG} catch getSupportedAudioEffectProperty001 exception: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getAudioEffectProperty001
   * @tc.desc:Get getAudioEffectProperty success - check repeats data
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectProperty001", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let audioEffectArray = audioEffectManager.getAudioEffectProperty();
      console.info(`${TAG} getAudioEffectProperty success:${JSON.stringify(audioEffectArray)}`);
      for (let i = 0; i < audioEffectArray.length; i++) {
        expect(audioEffectArray[i].category !== ""
          && audioEffectArray[i].category !== undefined).assertTrue();
        expect(audioEffectArray[i].name !== ""
          && audioEffectArray[i].name !== undefined).assertTrue();
        expect(audioEffectArray[i].flag !== "0"
          && audioEffectArray[i].flag !== "1").assertTrue();
      }
      done();
    } catch (e) {
      console.error(`${TAG} catch getAudioEffectProperty001 exception: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:setAudioEffectProperty001
   * @tc.desc:Get setAudioEffectProperty success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("setAudioEffectProperty001", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let audioEffectArray = audioEffectManager.getAudioEffectProperty();
      console.info(`${TAG} get supported effect property SUCCESS:${JSON.stringify(audioEffectArray)}`);
      if (audioEffectArray.length > 0) {
        audioEffectManager.setAudioEffectProperty(audioEffectArray);
        console.info(`${TAG} setAudioEffectProperty001 SUCCESS`);
      }
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty001 exception: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:setAudioEffectProperty002
   * @tc.desc:Get setAudioEffectProperty no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("setAudioEffectProperty002", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let audioEffectArray = [];
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty002 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty002 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

    /*
   * @tc.name:setAudioEffectProperty003
   * @tc.desc:Get setAudioEffectProperty no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
    it("setAudioEffectProperty003", 0, async function (done) {
      try {
        let audioEffectManager = audio.getAudioManager().getEffectManager();
        let audioEffectArray = 0;
        audioEffectManager.setAudioEffectProperty(audioEffectArray);
        console.error(`${TAG} setAudioEffectProperty003 check invalid parameter fail`);
        done();
      } catch (e) {
        console.error(`${TAG} catch setAudioEffectProperty003 exception: ${e.message}`);
        expect(e.code).assertEqual(ERROR_INPUT_INVALID);
        done();
      }
    });

  /*
   * @tc.name:setAudioEffectProperty004
   * @tc.desc:Get setAudioEffectProperty invalid parameter - repeats data
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("setAudioEffectProperty004", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let effect_1 = { category: "NROFF", name: "voip_down", flag:0 };
      let effect_2 = { category: "NROFF", name: "record", flag:0 };
      let audioEffectArray = [effect_1, effect_2];
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty004 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty004 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

  /*
   * @tc.name:setAudioEffectProperty005
   * @tc.desc:Get setAudioEffectProperty invalid parameter - empty property value
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("setAudioEffectProperty005", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let effectProperty = { category: "", name: "", flag:0 };
      let audioEffectArray = [effectProperty];
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty005 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty005 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

  /*
 * @tc.name:setAudioEffectProperty006
 * @tc.desc:Get setAudioEffectProperty invalid parameter - upper limit 
 * @tc.type: FUNC
 * @tc.require: I7V04L
 */
  it("setAudioEffectProperty006", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let audioEffectArray = [];
      for (let i = 0; i < audio.AUDIO_EFFECT_COUNT_UPPER_LIMIT + 1; i++) {
        let effectProperty = { category: 'testClass' + i, name: 'testProp' + i, flag:0 };
        audioEffectArray.push(effectProperty);
      }
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty006 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty006 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });
  
  /*
 * @tc.name:setAudioEffectProperty007
 * @tc.desc:Get setAudioEffectProperty invalid parameter - upper limit 
 * @tc.type: FUNC
 * @tc.require: I7V04L
 */
  it("setAudioEffectProperty007", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let effect_1 = { category: "NROFF", name: "voip_down", flag:1 };
      let effect_2 = { category: "NROFF", name: "record", flag:1 };
      let audioEffectArray = [effect_1, effect_2];
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty007 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty007 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

  /*
 * @tc.name:setAudioEffectProperty008
 * @tc.desc:Get setAudioEffectProperty invalid parameter - upper limit 
 * @tc.type: FUNC
 * @tc.require: I7V04L
 */
    it("setAudioEffectProperty008", 0, async function (done) {
      try {
        let audioEffectManager = audio.getAudioManager().getEffectManager();
        let effect_1 = { category: "NROFF", name: "voip_down", flag:0 };
        let effect_2 = { category: "NROFF", name: "record", flag:1 };
        let audioEffectArray = [effect_1, effect_2];
        audioEffectManager.setAudioEffectProperty(audioEffectArray);
        console.error(`${TAG} setAudioEffectProperty008 check invalid parameter fail`);
        done();
      } catch (e) {
        console.error(`${TAG} catch setAudioEffectProperty008 exception: ${e.message}`);
        expect(e.code).assertEqual(ERROR_INVALID_PARAM);
        done();
      }
    });
    
  /*
 * @tc.name:setAudioEffectProperty009
 * @tc.desc:Get setAudioEffectProperty invalid parameter - upper limit 
 * @tc.type: FUNC
 * @tc.require: I7V04L
 */
  it("setAudioEffectProperty009", 0, async function (done) {
    try {
      let audioEffectManager = audio.getAudioManager().getEffectManager();
      let effect_1 = { category: "test1", name: "test1", flag:0 };
      let effect_2 = { category: "test2", name: "test2", flag:1 };
      let audioEffectArray = [effect_1, effect_2];
      audioEffectManager.setAudioEffectProperty(audioEffectArray);
      console.error(`${TAG} setAudioEffectProperty009 check invalid parameter fail`);
      done();
    } catch (e) {
      console.error(`${TAG} catch setAudioEffectProperty009 exception: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

})
