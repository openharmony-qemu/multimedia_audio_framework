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

const TAG = "[AudioRoutingManagerJsTest]";
const stringParameter = 'stringParameter';
const numberParameter = 12345678;

describe("AudioRoutingManagerJsTest", function () {
  const ERROR_INPUT_INVALID = '401';
  const ERROR_INVALID_PARAM = '6800101';

  beforeAll(async function () {

    console.info(TAG + "AudioRoutingManagerJsTest:beforeAll called");
  })

  afterAll(function () {
    console.info(TAG + 'AudioRoutingManagerJsTest:afterAll called')
  })

  beforeEach(function () {
    console.info(TAG + 'AudioRoutingManagerJsTest:beforeEach called')
  })

  afterEach(function () {
    console.info(TAG + 'AudioRoutingManagerJsTest:afterEach called')
  })

  function sleep(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
  }

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest001
   * @tc.desc:Get prefer output device - promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest001", 0, async function (done) {
    let rendererInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      rendererFlags : 0 }
    
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo);
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfo SUCCESS`+JSON.stringify(data));
      expect(true).assertTrue();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredOutputDeviceForRendererInfo ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest002
   * @tc.desc:Get prefer output device no parameter- promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest002", 0, async function (done) {
      let routingManager = audio.getAudioManager().getRoutingManager();
      try {
        let data = await routingManager.getPreferredOutputDeviceForRendererInfo();
        console.error(`${TAG} getPreferredOutputDeviceForRendererInfo parameter check ERROR: ${JSON.stringify(data)}`);
        expect().assertFail();
      } catch(e) {
        if (e.code != ERROR_INPUT_INVALID) {
          console.error(`${TAG} getPreferredOutputDeviceForRendererInfo ERROR: ${e.message}`);
          expect().assertFail();
          done();
        }
        console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check no parameter PASS`);
        expect(true).assertTrue();
      }
      done();
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest003
   * @tc.desc:Get prefer output device check number parameter- promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest003", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredOutputDeviceForRendererInfo(numberParameter);
      console.error(`${TAG} getPreferredOutputDeviceForRendererInfo parameter check ERROR: `+JSON.stringify(data));
      expect().assertFail();
    } catch(e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} getPreferredOutputDeviceForRendererInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check number parameter PASS`);
      expect(true).assertTrue();
    }
    done();
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest004
   * @tc.desc:Get prefer output device check string parameter- promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest004", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredOutputDeviceForRendererInfo(stringParameter);
      console.error(`${TAG} getPreferredOutputDeviceForRendererInfo parameter check ERROR: `+JSON.stringify(data));
      expect().assertFail();
    } catch(e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} getPreferredOutputDeviceForRendererInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check string parameter PASS`);
      expect(true).assertTrue();
    }
    done();
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest005
   * @tc.desc:Get prefer output device - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest005", 0, async function (done) {
    let rendererInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      rendererFlags : 0 }
    
    let routingManager = audio.getAudioManager().getRoutingManager();
      routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo, (e, data)=>{
        if (e) {
          console.error(`${TAG} getPreferredOutputDeviceForRendererInfo ERROR: ${e.message}`);
          expect(false).assertTrue();
          done();
        }
        console.info(`${TAG} getPreferredOutputDeviceForRendererInfo SUCCESS`);
        expect(true).assertTrue();
        done();
      });
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest006
   * @tc.desc:Get prefer output device check number parameter- callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest006", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.getPreferredOutputDeviceForRendererInfo(numberParameter, (e, data)=>{
        console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check number parameter ERROR`);
        expect().assertFail();
        done();
      });
    } catch (e) {
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check number parameter PASS, errorcode ${e.code}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoTest007
   * @tc.desc:Get prefer output device check string parameter- callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getPreferredOutputDeviceForRendererInfoTest007", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.getPreferredOutputDeviceForRendererInfo(stringParameter, (e, data)=>{
        console.error(`${TAG} getPreferredOutputDeviceForRendererInfo check string parameter ERROR`);
        expect().assertFail();
        done();
      });
    } catch (e) {
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfo check string parameter PASS, errorcode ${e.code}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:on_referOutputDeviceForRendererInfoTest001
   * @tc.desc:On prefer output device - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("on_preferredOutputDeviceForRendererInfoTest001", 0, async function (done) {
    let rendererInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      rendererFlags : 0 }
    
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredOutputDeviceChangeForRendererInfo', rendererInfo, (data)=>{});
      expect(true).assertTrue();
      done();
    } catch (e) {
        console.error(`${TAG} on_referOutputDeviceForRendererInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
    }
  })

  /*
   * @tc.name:on_referOutputDeviceForRendererInfoTest002
   * @tc.desc:On prefer output device check string parameter- callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("on_preferredOutputDeviceForRendererInfoTest002", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredOutputDeviceChangeForRendererInfo', stringParameter, (data)=>{});
      console.error(`${TAG} on_referOutputDeviceForRendererInfo with string patameter ERROR: ${e.message}`);
      expect().assertFail();
      done();
    } catch (e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} on_referOutputDeviceForRendererInfo check string parameter ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} on_referOutputDeviceForRendererInfo PASS: ${e.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:on_referOutputDeviceForRendererInfoTest003
   * @tc.desc:On prefer output device check number parameter- callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("on_preferredOutputDeviceForRendererInfoTest003", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredOutputDeviceChangeForRendererInfo', numberParameter, (data)=>{});
      console.error(`${TAG} on_referOutputDeviceForRendererInfo with number patameter ERROR: ${e.message}`);
      expect().assertFail();
      done();
    } catch (e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} on_referOutputDeviceForRendererInfo check number parameter ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} on_referOutputDeviceForRendererInfo PASS: ${e.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:off_referOutputDeviceForRendererInfoTest001
   * @tc.desc:Off prefer output device - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("off_preferredOutputDeviceForRendererInfoTest001", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.off('preferredOutputDeviceChangeForRendererInfo', (data)=>{});
      console.info(`${TAG} off_referOutputDeviceForRendererInfo SUCCESS`);
      expect(true).assertTrue();
      done();
    } catch (e) {
        console.error(`${TAG} off_referOutputDeviceForRendererInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
    }
  })

  /*
   * @tc.name:getdevice001
   * @tc.desc:getdevice - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getdevice001", 0, async function (done) {
    let routingManager = null;
    try {
      routingManager = audio.getAudioManager().getRoutingManager();
      expect(true).assertTrue();
      done();
    } catch (e) {
      console.error(`${TAG} getdevice001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }

    routingManager.getDevices(audio.DeviceFlag.INPUT_DEVICES_FLAG, (err, AudioDeviceDescriptors)=>{
      if (err) {
        console.error(`${TAG} first getDevices ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`${TAG} getDevices001 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(AudioDeviceDescriptors.length).assertLarger(0);
      for (let i = 0; i < AudioDeviceDescriptors.length; i++) {
        expect(AudioDeviceDescriptors[i].displayName!==""
        && AudioDeviceDescriptors[i].displayName!==undefined).assertTrue();
      }
      done();
    })
  });

})
