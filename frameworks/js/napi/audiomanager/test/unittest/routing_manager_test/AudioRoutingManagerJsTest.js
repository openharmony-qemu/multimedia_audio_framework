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

  /*
   * @tc.name:getdevice002
   * @tc.desc:getdevice - promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getdevice002", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let AudioDeviceDescriptors = await routingManager.getDevices(audio.DeviceFlag.INPUT_DEVICES_FLAG);
      console.info(`${TAG} getDevices002 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(AudioDeviceDescriptors.length).assertLarger(0);
      for (let i = 0; i < AudioDeviceDescriptors.length; i++) {
        expect(AudioDeviceDescriptors[i].displayName!==""
        && AudioDeviceDescriptors[i].displayName!==undefined).assertTrue();
      }
      done();
    } catch (e) {
      console.error(`${TAG} getdevice002 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:setCommunicationDevice001
   * @tc.desc:setCommunicationDevice - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("setCommunicationDevice001", 0, async function (done) {
    let audioManager = audio.getAudioManager();
    let rendererInfo = {
      content: audio.ContentType.CONTENT_TYPE_SPEECH,
      usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
      rendererFlags: 0
    };
    let flag = false;
    console.info(`${TAG} setCommunicationDevice001 start`);
    audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_PHONE_CALL, (err) => {
      console.info(`${TAG} setAudioScene enter`);
      if (err) {
        console.error(`${TAG} setAudioScene ERROR: ${err.message}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`${TAG} setAudioScene success`);
      let routingManager = audioManager.getRoutingManager();
      routingManager.getDevices(audio.DeviceFlag.OUTPUT_DEVICES_FLAG, async (err, value) => {
        console.info(`${TAG}_001.getDevices return: ` + JSON.stringify(value));
        if (err) {
          console.error(`${TAG} getDevices ERROR: ${err.message}`);
          expect(false).assertTrue();
          await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
          done();
          return;
        }
        console.info(`${TAG}_001.getDevices value.length: ` + JSON.stringify(value.length));
        for (let i = 0; i < value.length; i++) {
          if (value[i].deviceType == audio.DeviceType.EARPIECE) {
            flag = true;
            break;
          }
        }
        console.info(`${TAG}_001.getDevices flag: ` + flag);
        if (!flag) {
          console.error(`${TAG}_001.This device does not have a eapiece`);
          expect(true).assertTrue();
          await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
          done();
          return;
        }
        routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo, async (err, value) => {
          console.info(`${TAG}_001.getPreferredOutputDeviceForRendererInfo return: ` + JSON.stringify(value));
          if (err) {
            console.error(`${TAG}_001.getPreferredOutputDeviceForRendererInfo ERROR: ${err.message}`);
            expect(false).assertTrue();
            await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
            done();
            return;
          }
          if (value[0].deviceType != audio.DeviceType.EARPIECE) {
            console.error(`${TAG}_001.getPrefer device is not EARPIECE`);
            expect(false).assertTrue();
            await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
            done();
            return;
          }
          routingManager.setCommunicationDevice(audio.CommunicationDeviceType.SPEAKER, false, async (err) => {
            console.info(`${TAG}_001.setCommunicationDevice enter`);
            if (err) {
              console.error(`${TAG}_001.setCommunicationDevice ERROR: ${err.message}`);
              expect(false).assertTrue();
              await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
              done();
              return;
            }
            routingManager.isCommunicationDeviceActive(audio.CommunicationDeviceType.SPEAKER, async (err, value) => {
              console.info(`${TAG}_001.isCommunicationDeviceActive return: `+ JSON.stringify(value));
              if (err) {
                console.error(`${TAG}_001.isCommunicationDeviceActive ERROR: ${err.message}`);
                expect(false).assertTrue();
                await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
                done();
                return;
              }
              if (value) {
                console.error(`${TAG}_001.isCommunicationDeviceActive reurn true`);
                expect(false).assertTrue();
                await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
                done();
                return;
              }
              expect(true).assertTrue();
              await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
              done();
              return;
            });
          });
        });
      });
    });
  });

  /*
   * @tc.name:setCommunicationDevice002
   * @tc.desc:setCommunicationDevice - callback
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("setCommunicationDevice002", 0, async function (done) {
    let audioManager = audio.getAudioManager();
    let rendererInfo = {
      content: audio.ContentType.CONTENT_TYPE_SPEECH,
      usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
      rendererFlags: 0
    };
    let flag = false;
    console.info(`${TAG} setCommunicationDevice002 start`);
    audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_PHONE_CALL, (err) => {
      console.info(`${TAG} setAudioScene enter`);
      if (err) {
        console.error(`${TAG} setAudioScene ERROR: ${err.message}`);
        expect(false).assertTrue();
        done();
        return;
      }
      let routingManager = audioManager.getRoutingManager();
      routingManager.getDevices(audio.DeviceFlag.OUTPUT_DEVICES_FLAG, async (err, value) => {
        console.info(`${TAG}_002.getDevices return: ` + JSON.stringify(value));
        if (err) {
          console.error(`${TAG}_002.getDevices ERROR: ${err.message}`);
          expect(false).assertTrue();
          await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
          done();
          return;
        }
        console.info(`${TAG}_002.getDevices value.length: ` + JSON.stringify(value.length));
        for (let i = 0; i < value.length; i++) {
          if (value[i].deviceType == audio.DeviceType.EARPIECE) {
            flag = true;
            break;
          }
        }
        console.info(`${TAG}_002.getDevices flag: ` + flag);
        if (!flag) {
          console.error(`${TAG}_002.This device does not have a earpiece`);
          expect(true).assertTrue();
          await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
          done();
          return;
        }
        routingManager.getPreferredOutputDeviceForRendererInfo(rendererInfo, async (err, value) => {
          console.info(`${TAG}_002.getPreferredOutputDeviceForRendererInfo return: ` + JSON.stringify(value));
          if (err) {
            console.error(`${TAG}_002.getPreferredOutputDeviceForRendererInfo ERROR: ${err.message}`);
            expect(false).assertTrue();
            await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
            done();
            return;
          }
          if (value[0].deviceType != audio.DeviceType.EARPIECE) {
            console.error(`${TAG}_002.getPrefer device is not EARPIECE`);
            expect(false).assertTrue();
            await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
            done();
            return;
          }
          routingManager.setCommunicationDevice(audio.CommunicationDeviceType.SPEAKER, true, async (err) => {
            console.info(`${TAG}_002.setCommunicationDevice enter`);
            if (err) {
              console.error(`${TAG}_002.setCommunicationDevice ERROR: ${err.message}`);
              expect(false).assertTrue();
              await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
              done();
              return;
            }
            routingManager.isCommunicationDeviceActive(audio.CommunicationDeviceType.SPEAKER, async (err, value) => {
              console.info(`${TAG}_002.isCommunicationDeviceActive return: `+ JSON.stringify(value));
              if (err) {
                console.error(`${TAG}_002.isCommunicationDeviceActive ERROR: ${err.message}`);
                expect(false).assertTrue();
                await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
                done();
                return;
              }
              if (!value) {
                console.error(`${TAG}_002.isCommunicationDeviceActive reurn false`);
                expect(false).assertTrue();
                await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
                done();
                return;
              }
              expect(true).assertTrue();
              await audioManager.setAudioScene(audio.AudioScene.AUDIO_SCENE_DEFAULT);
              done();
              return;
            });
          })
        });
      });
    });
  });

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest001
   * @tc.desc:Get preferred input device - promise
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest001", 0, async function (done) {
    let capturerInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      capturerFlags : 0 }

    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredInputDeviceForCapturerInfo(capturerInfo);
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfo SUCCESS`+JSON.stringify(data));
      expect(true).assertTrue();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredInputDeviceForCapturerInfo ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest002
   * @tc.desc:Get preferred input device no parameter- promise
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest002", 0, async function (done) {
      let routingManager = audio.getAudioManager().getRoutingManager();
      try {
        let data = await routingManager.getPreferredInputDeviceForCapturerInfo();
        console.error(`${TAG} getPreferredInputDeviceForCapturerInfo parameter check ERROR: ${JSON.stringify(data)}`);
        expect().assertFail();
      } catch(e) {
        if (e.code != ERROR_INPUT_INVALID) {
          console.error(`${TAG} getPreferredInputDeviceForCapturerInfo ERROR: ${e.message}`);
          expect().assertFail();
          done();
        }
        console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check no parameter PASS`);
        expect(true).assertTrue();
      }
      done();
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest003
   * @tc.desc:Get preferred input device check number parameter- promise
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest003", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredInputDeviceForCapturerInfo(numberParameter);
      console.error(`${TAG} getPreferredInputDeviceForCapturerInfo parameter check ERROR: `+JSON.stringify(data));
      expect().assertFail();
    } catch(e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} getPreferredInputDeviceForCapturerInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check number parameter PASS`);
      expect(true).assertTrue();
    }
    done();
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest004
   * @tc.desc:Get preferred input device check string parameter- promise
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest004", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      let data = await routingManager.getPreferredInputDeviceForCapturerInfo(stringParameter);
      console.error(`${TAG} getPreferredInputDeviceForCapturerInfo parameter check ERROR: `+JSON.stringify(data));
      expect().assertFail();
    } catch(e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} getPreferredInputDeviceForCapturerInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check string parameter PASS`);
      expect(true).assertTrue();
    }
    done();
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest005
   * @tc.desc:Get preferred input device - callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest005", 0, async function (done) {
    let capturerInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      capturerFlags : 0 }

    let routingManager = audio.getAudioManager().getRoutingManager();
      routingManager.getPreferredInputDeviceForCapturerInfo(capturerInfo, (e, data)=>{
        if (e) {
          console.error(`${TAG} getPreferredInputDeviceForCapturerInfo ERROR: ${e.message}`);
          expect(false).assertTrue();
          done();
        }
        console.info(`${TAG} getPreferredInputDeviceForCapturerInfo SUCCESS`);
        expect(true).assertTrue();
        done();
      });
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest006
   * @tc.desc:Get preferred input device check number parameter- callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest006", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.getPreferredInputDeviceForCapturerInfo(numberParameter, (e, data)=>{
        console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check number parameter ERROR`);
        expect().assertFail();
        done();
      });
    } catch (e) {
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check number parameter PASS, errorcode ${e.code}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoTest007
   * @tc.desc:Get preferred input device check string parameter- callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoTest007", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.getPreferredInputDeviceForCapturerInfo(stringParameter, (e, data)=>{
        console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check string parameter ERROR`);
        expect().assertFail();
        done();
      });
    } catch (e) {
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfo check string parameter PASS, errorcode ${e.code}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:on_preferredInputDeviceChangeForCapturerInfoTest001
   * @tc.desc:On preferred input device - callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("on_preferredInputDeviceChangeForCapturerInfoTest001", 0, async function (done) {
    let capturerInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      capturerFlags : 0 }

    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredInputDeviceChangeForCapturerInfo', capturerInfo, (data)=>{});
      expect(true).assertTrue();
      done();
    } catch (e) {
        console.error(`${TAG} on_preferredInputDeviceChangeForCapturerInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
    }
  })

  /*
   * @tc.name:on_preferredInputDeviceChangeForCapturerInfoTest002
   * @tc.desc:On preferred input device check string parameter- callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("on_preferredInputDeviceChangeForCapturerInfoTest002", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredInputDeviceChangeForCapturerInfo', stringParameter, (data)=>{});
      console.error(`${TAG} on_preferredInputDeviceChangeForCapturerInfo with string patameter ERROR: ${e.message}`);
      expect().assertFail();
      done();
    } catch (e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} on_preferredInputDeviceChangeForCapturerInfo check string parameter ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} on_preferredInputDeviceChangeForCapturerInfo PASS: ${e.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:on_preferredInputDeviceChangeForCapturerInfoTest003
   * @tc.desc:On preferred input device check number parameter- callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("on_preferredInputDeviceChangeForCapturerInfoTest003", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.on('preferredInputDeviceChangeForCapturerInfo', numberParameter, (data)=>{});
      console.error(`${TAG} on_preferredInputDeviceChangeForCapturerInfo with number patameter ERROR: ${e.message}`);
      expect().assertFail();
      done();
    } catch (e) {
      if (e.code != ERROR_INPUT_INVALID) {
        console.error(`${TAG} on_preferredInputDeviceChangeForCapturerInfo check number parameter ERROR: ${e.message}`);
        expect().assertFail();
        done();
      }
      console.info(`${TAG} on_preferredInputDeviceChangeForCapturerInfo PASS: ${e.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:off_preferredInputDeviceChangeForCapturerInfoTest001
   * @tc.desc:Off preferred input device - callback
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("off_preferredInputDeviceChangeForCapturerInfoTest001", 0, async function (done) {
    let routingManager = audio.getAudioManager().getRoutingManager();
    try {
      routingManager.off('preferredInputDeviceChangeForCapturerInfo', (data)=>{});
      console.info(`${TAG} off_preferredInputDeviceChangeForCapturerInfo SUCCESS`);
      expect(true).assertTrue();
      done();
    } catch (e) {
        console.error(`${TAG} off_preferredInputDeviceChangeForCapturerInfo ERROR: ${e.message}`);
        expect().assertFail();
        done();
    }
  })

  /*
   * @tc.name:isCommunicationDeviceActiveSync001
   * @tc.desc:Get isCommunicationDeviceActiveSync success - SPEAKER
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isCommunicationDeviceActiveSync001", 0, async function (done) {
    let audioRoutingManager = null;

    try {
      audioRoutingManager = audio.getAudioManager().getRoutingManager();
      await audioRoutingManager.setCommunicationDevice(audio.CommunicationDeviceType.SPEAKER, true);
      let isActive = audioRoutingManager.isCommunicationDeviceActiveSync(audio.CommunicationDeviceType.SPEAKER);
      console.info(`The active status of the device is obtained ${isActive}.`);
      expect(isActive).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isCommunicationDeviceActiveSync001 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isCommunicationDeviceActiveSync002
   * @tc.desc:Get isCommunicationDeviceActiveSync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isCommunicationDeviceActiveSync002", 0, async function (done) {
    let audioRoutingManager = null;

    try {
      audioRoutingManager = audio.getAudioManager().getRoutingManager();
      let isActive = audioRoutingManager.isCommunicationDeviceActiveSync();
      console.info(`The active status of the device is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isCommunicationDeviceActiveSync002 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:isCommunicationDeviceActiveSync003
   * @tc.desc:Get isCommunicationDeviceActiveSync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isCommunicationDeviceActiveSync003", 0, async function (done) {
    let audioRoutingManager = null;

    try {
      audioRoutingManager = audio.getAudioManager().getRoutingManager();
      let isActive = audioRoutingManager.isCommunicationDeviceActiveSync("Invalid type");
      console.info(`The active status of the device is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isCommunicationDeviceActiveSync003 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:isCommunicationDeviceActiveSync004
   * @tc.desc:Get isCommunicationDeviceActiveSync fail(6800101) - Invalid param value : 100
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isCommunicationDeviceActiveSync004", 0, async function (done) {
    let invalidDeviceType = 100;
    let audioRoutingManager = null;

    try {
      audioRoutingManager = audio.getAudioManager().getRoutingManager();
      let isActive = audioRoutingManager.isCommunicationDeviceActiveSync(invalidDeviceType);
      console.info(`The active status is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isCommunicationDeviceActiveSync004 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });

  /*
   * @tc.name:getDevicesSync001
   * @tc.desc:getDevicesSync success - INPUT_DEVICES_FLAG
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getDevicesSync001", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let AudioDeviceDescriptors = routingManager.getDevicesSync(audio.DeviceFlag.INPUT_DEVICES_FLAG);
      console.info(`${TAG} getDevicesSync001 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(AudioDeviceDescriptors.length).assertLarger(0);
      for (let i = 0; i < AudioDeviceDescriptors.length; i++) {
        expect(AudioDeviceDescriptors[i].displayName!==""
        && AudioDeviceDescriptors[i].displayName!==undefined).assertTrue();
      }
      done();
    } catch (e) {
      console.error(`${TAG} getDevicesSync001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getDevicesSync010
   * @tc.desc:getDevicesSync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getDevicesSync010", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let AudioDeviceDescriptors = routingManager.getDevicesSync();
      console.info(`${TAG} getDevicesSync010 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(false).assertTrue();
      done();
    } catch (e) {
      console.error(`${TAG} getDevicesSync010 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  });

  /*
   * @tc.name:getDevicesSync011
   * @tc.desc:getDevicesSync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getDevicesSync011", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let AudioDeviceDescriptors = routingManager.getDevicesSync("Invalid type");
      console.info(`${TAG} getDevicesSync011 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(false).assertTrue();
      done();
    } catch (e) {
      console.error(`${TAG} getDevicesSync011 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  });

  /*
   * @tc.name:getDevicesSync012
   * @tc.desc:getDevicesSync fail(6800101) - Invalid param value : 10000
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getDevicesSync012", 0, async function (done) {
    let invalidDeviceFlag = 10000;
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let AudioDeviceDescriptors = routingManager.getDevicesSync(invalidDeviceFlag);
      console.info(`${TAG} getDevicesSync012 SUCCESS:`+ JSON.stringify(AudioDeviceDescriptors));
      expect(false).assertTrue();
      done();
    } catch (e) {
      console.error(`${TAG} getDevicesSync012 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
    }
  });

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoSyncTest001
   * @tc.desc:getPreferredInputDeviceForCapturerInfoSync success
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoSyncTest001", 0, async function (done) {
    let capturerInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      capturerFlags : 0 }

    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let data = routingManager.getPreferredInputDeviceForCapturerInfoSync(capturerInfo);
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest001 SUCCESS`+JSON.stringify(data));
      expect(true).assertTrue();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoSyncTest002
   * @tc.desc:getPreferredInputDeviceForCapturerInfoSync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoSyncTest002", 0, async function (done) {
      try {
        let routingManager = audio.getAudioManager().getRoutingManager();
        let data = routingManager.getPreferredInputDeviceForCapturerInfoSync();
        console.info(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest002 SUCCESS`+JSON.stringify(data));
        expect().assertFail();
        done();
      } catch(e) {
        console.error(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest002 ERROR: ${e.message}`);
        expect(e.code).assertEqual(ERROR_INPUT_INVALID);
        done();
      }
  })

  /*
   * @tc.name:getPreferredInputDeviceForCapturerInfoSyncTest003
   * @tc.desc:getPreferredInputDeviceForCapturerInfoSync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredInputDeviceForCapturerInfoSyncTest003", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let data = routingManager.getPreferredInputDeviceForCapturerInfoSync("Invalid type");
      console.info(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest003 SUCCESS`+JSON.stringify(data));
      expect().assertFail();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredInputDeviceForCapturerInfoSyncTest003 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoSyncTest001
   * @tc.desc:getPreferredOutputDeviceForRendererInfoSync success
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredOutputDeviceForRendererInfoSyncTest001", 0, async function (done) {
    let rendererInfo = {
      content : audio.ContentType.CONTENT_TYPE_MUSIC,
      usage : audio.StreamUsage.STREAM_USAGE_MEDIA,
      rendererFlags : 0 }

    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let data = routingManager.getPreferredOutputDeviceForRendererInfoSync(rendererInfo);
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest001 SUCCESS`+JSON.stringify(data));
      expect(true).assertTrue();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoSyncTest002
   * @tc.desc:getPreferredOutputDeviceForRendererInfoSync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredOutputDeviceForRendererInfoSyncTest002", 0, async function (done) {
      try {
        let routingManager = audio.getAudioManager().getRoutingManager();
        let data = routingManager.getPreferredOutputDeviceForRendererInfoSync();
        console.info(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest002 SUCCESS`+JSON.stringify(data));
        expect().assertFail();
        done();
      } catch(e) {
        console.error(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest002 ERROR: ${e.message}`);
        expect(e.code).assertEqual(ERROR_INPUT_INVALID);
        done();
      }
  })

  /*
   * @tc.name:getPreferredOutputDeviceForRendererInfoSyncTest003
   * @tc.desc:getPreferredOutputDeviceForRendererInfoSync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I7Q56A
   */
  it("getPreferredOutputDeviceForRendererInfoSyncTest003", 0, async function (done) {
    try {
      let routingManager = audio.getAudioManager().getRoutingManager();
      let data = routingManager.getPreferredOutputDeviceForRendererInfoSync("Invalid type");
      console.info(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest003 SUCCESS`+JSON.stringify(data));
      expect().assertFail();
      done();
    } catch(e) {
      console.error(`${TAG} getPreferredOutputDeviceForRendererInfoSyncTest003 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

})
