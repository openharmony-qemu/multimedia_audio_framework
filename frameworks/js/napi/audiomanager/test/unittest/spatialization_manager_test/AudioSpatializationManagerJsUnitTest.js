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

const numberParameter = 12345678;
describe("AudioSpatializationManagerJsTest", function () {
  let audioManager = audio.getAudioManager();
  let audioSpatializationManager = audioManager.getSpatializationManager();
  const ERROR_NO_PERMISSION = '201';
  const ERROR_INPUT_INVALID = '401';
  const ERROR_INVALID_PARAM = '6800101';
  let deviceDescriptor = {
    address: "123",
    deviceRole: 2,
    deviceType: 1,
    id: 123,
    name: "123",
    sampleRates: [18],
    channelCounts: [2, 6],
    channelMasks: [2, 6],
    networkId: "123",
    interruptGroupId: 12,
    volumeGroupId: 12,
    displayName: "123"
  }

  beforeAll(async function () {
    console.info('beforeAll called')
  })

  afterAll(function () {
    console.info('afterAll called')
  })

  beforeEach(function () {
    console.info('beforeEach called')
  })

  afterEach(function () {
    console.info('afterEach called')
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001
   * @tc.desc:isSpatializationEnabled success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001", 0, async function (done) {
    try {
      let isEnabled = audioSpatializationManager.isSpatializationEnabled();
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001 SUCCESS: ${isEnabled}.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001 ERROR: ${err}`);
      expect(false).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001
   * @tc.desc:setSpatializationEnabled enable success - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001", 0, async function (done) {
    try {
      await audioSpatializationManager.setSpatializationEnabled(true);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002
   * @tc.desc:setSpatializationEnabled disable success - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002", 0, async function (done) {
    try {
      await audioSpatializationManager.setSpatializationEnabled(false);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003
   * @tc.desc:setSpatializationEnabled no parameter - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003", 0, async function (done) {
    try {
      await audioSpatializationManager.setSpatializationEnabled();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003 parameter check ERROR.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003 ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003 check no parameter PASS`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004
   * @tc.desc:setSpatializationEnabled check number parameter - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004", 0, async function (done) {
    try {
      await audioSpatializationManager.setSpatializationEnabled(numberParameter);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004 parameter check ERROR.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004 ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004 check number parameter PASS`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001
   * @tc.desc:setSpatializationEnabled enable success - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001", 0, async function (done) {
    audioSpatializationManager.setSpatializationEnabled(true, (err) => {
      if (err) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_001 SUCCESS`);
      done();
    })
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002
   * @tc.desc:setSpatializationEnabled disable success - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002", 0, async function (done) {
    audioSpatializationManager.setSpatializationEnabled(false, (err) => {
      if (err) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_002 SUCCESS`);
      done();
    })
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003
   * @tc.desc:setSpatializationEnabled no parameter - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003", 0, async function (done) {
    try {
      audioSpatializationManager.setSpatializationEnabled((err) => {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003 check no parameter ERROR`);
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_003 check no parameter PASS, \
        errorcode ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004
   * @tc.desc:setSpatializationEnabled check number parameter - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004", 0, async function (done) {
    try {
      audioSpatializationManager.setSpatializationEnabled(numberParameter, (err) => {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004 check number parameter ERROR`);
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_ENABLED_004 check number parameter PASS, \
        errorcode ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001
   * @tc.desc:isHeadTrackingEnabled success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001", 0, async function (done) {
    try {
      let isEnabled = audioSpatializationManager.isHeadTrackingEnabled();
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001 SUCCESS: ${isEnabled}.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001 ERROR: ${err}`);
      expect(false).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001
   * @tc.desc:setHeadTrackingEnabled enable success - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001", 0, async function (done) {
    try {
      await audioSpatializationManager.setHeadTrackingEnabled(true);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002
   * @tc.desc:setHeadTrackingEnabled disable success - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002", 0, async function (done) {
    try {
      await audioSpatializationManager.setHeadTrackingEnabled(false);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003
   * @tc.desc:setHeadTrackingEnabled no parameter - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003", 0, async function (done) {
    try {
      await audioSpatializationManager.setHeadTrackingEnabled();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003 parameter check ERROR.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003 ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003 check no parameter PASS`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004
   * @tc.desc:setHeadTrackingEnabled check number parameter - promise
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004", 0, async function (done) {
    try {
      await audioSpatializationManager.setHeadTrackingEnabled(numberParameter);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004 parameter check ERROR.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004 ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004 check number parameter PASS`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001
   * @tc.desc:setHeadTrackingEnabled enable success - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001", 0, async function (done) {
    audioSpatializationManager.setHeadTrackingEnabled(true, (err) => {
      if (err) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_001 SUCCESS`);
      done();
    })
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002
   * @tc.desc:setHeadTrackingEnabled disable success - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002", 0, async function (done) {
    audioSpatializationManager.setHeadTrackingEnabled(false, (err) => {
      if (err) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        done();
        return;
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_002 SUCCESS`);
      done();
    })
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003
   * @tc.desc:setHeadTrackingEnabled no parameter - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003", 0, async function (done) {
    try {
      audioSpatializationManager.setHeadTrackingEnabled((err) => {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003 check no parameter ERROR`);
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_003 check no parameter PASS, \
        errorcode ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004
   * @tc.desc:setHeadTrackingEnabled check number parameter - callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004", 0, async function (done) {
    try {
      audioSpatializationManager.setHeadTrackingEnabled(numberParameter, (err) => {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004 check number parameter ERROR`);
        expect().assertFail();
        done();
      })
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_HEAD_TRACKING_ENABLED_004 check number parameter PASS, \
        errorcode ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_001
   * @tc.desc:isSpatializationSupported success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_001", 0, async function (done) {
    try {
      let isSupported = audioSpatializationManager.isSpatializationSupported();
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_001 SUCCESS: ${isSupported}.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_001 ERROR: ${err}`);
      expect(false).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_001
   * @tc.desc:isHeadTrackingSupported success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_001", 0, async function (done) {
    try {
      let isSupported = audioSpatializationManager.isHeadTrackingSupported();
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_001 SUCCESS: ${isSupported}.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_001 ERROR: ${err}`);
      expect(false).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_001
   * @tc.desc:isSpatializationSupportedForDevice success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_001", 0, async function (done) {
    try {
      let isSupported = audioSpatializationManager.isSpatializationSupportedForDevice(deviceDescriptor);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_001 PASS: ${isSupported}`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_001 ERROR ${err.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_002
   * @tc.desc:isSpatializationSupportedForDevice no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_002", 0, async function (done) {
    try {
      let isSupported = audioSpatializationManager.isSpatializationSupportedForDevice();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_002 check \
        no parameter fail: ${isSupported}.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_002 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_003
   * @tc.desc:isSpatializationSupportedForDevice invalid parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_003", 0, async function (done) {
    deviceDescriptor.deviceType = -2;
    try {
      let isSupported = audioSpatializationManager.isSpatializationSupportedForDevice(deviceDescriptor);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_003 check \
        invalid parameter fail: ${isSupported}.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE_003 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_001
   * @tc.desc:isHeadTrackingSupportedForDevice success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_001", 0, async function (done) {
    deviceDescriptor.deviceType = 1;
    try {
      let isSupported = audioSpatializationManager.isHeadTrackingSupportedForDevice(deviceDescriptor);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_001 PASS: ${isSupported}`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_001 ERROR ${err.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_002
   * @tc.desc:isHeadTrackingSupportedForDevice no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_002", 0, async function (done) {
    try {
      let isSupported = audioSpatializationManager.isHeadTrackingSupportedForDevice();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_002 check \
        no parameter fail: ${isSupported}.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_002 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_003
   * @tc.desc:isHeadTrackingSupportedForDevice invalid parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_003", 0, async function (done) {
    deviceDescriptor.deviceType = -2;
    try {
      let isSupported = audioSpatializationManager.isHeadTrackingSupportedForDevice(deviceDescriptor);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_003 check \
        invalid parameter fail: ${isSupported}.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE_003 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_001
   * @tc.desc:on(spatializationEnabledChange) success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_001", 0, async function (done) {
    try {
      audioSpatializationManager.on("spatializationEnabledChange", (data) => {
      });
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_002
   * @tc.desc:on(spatializationEnabledChange) check number parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_002", 0, async function (done) {
    try {
      audioSpatializationManager.on("spatializationEnabledChange", numberParameter, (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_002 number parameter failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_002 check number parameter \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_SPATIALIZATION_ENABLED_CHANGE_002 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_001
   * @tc.desc:off(spatializationEnabledChange) success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_001", 0, async function (done) {
    try {
      audioSpatializationManager.off("spatializationEnabledChange", (data) => {
      });
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_002
   * @tc.desc:off(spatializationEnabledChange) check number parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_002", 0, async function (done) {
    try {
      audioSpatializationManager.off("spatializationEnabledChange", numberParameter, (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_002 number parameter failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_002 check number parameter \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_SPATIALIZATION_ENABLED_CHANGE_002 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_001
   * @tc.desc:on(headTrackingEnabledChange) success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_001", 0, async function (done) {
    try {
      audioSpatializationManager.on("headTrackingEnabledChange", (data) => {
      });
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_002
   * @tc.desc:on(headTrackingEnabledChange) check number parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_002", 0, async function (done) {
    try {
      audioSpatializationManager.on("headTrackingEnabledChange", numberParameter, (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_002 number parameter failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_002 check number parameter \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_HEAD_TRACKING_ENABLED_CHANGE_002 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_001
   * @tc.desc:off(headTrackingEnabledChange) success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_001", 0, async function (done) {
    try {
      audioSpatializationManager.off("headTrackingEnabledChange", (data) => {
      });
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_001 SUCCESS.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_001 ERROR: ${err.message}`);
      expect().assertFail();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_002
   * @tc.desc:off(headTrackingEnabledChange) check number parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_002", 0, async function (done) {
    try {
      audioSpatializationManager.off("headTrackingEnabledChange", numberParameter, (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_002 number parameter failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INPUT_INVALID) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_002 check number parameter \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_HEAD_TRACKING_ENABLED_CHANGE_002 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_ON_UNKNOWN_CALLBACK_001
   * @tc.desc:on(unknownCallback) check unknown callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_ON_UNKNOWN_CALLBACK_001", 0, async function (done) {
    try {
      audioSpatializationManager.on("unknownCallback", (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_UNKNOWN_CALLBACK_001 check unknown callback failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INVALID_PARAM) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_UNKNOWN_CALLBACK_001 check unknown callback \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_ON_UNKNOWN_CALLBACK_001 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_UNKNOWN_CALLBACK_001
   * @tc.desc:off(unknownCallback) check unknown callback
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_UNKNOWN_CALLBACK_001", 0, async function (done) {
    try {
      audioSpatializationManager.off("unknownCallback", (data) => {
      });
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_UNKNOWN_CALLBACK_001 check unknown callback failed.`);
      expect().assertFail();
      done();
    } catch (err) {
      if (err.code != ERROR_INVALID_PARAM) {
        console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_UNKNOWN_CALLBACK_001 check unknown callback \
          ERROR: ${err.message}`);
        expect().assertFail();
        done();
      }
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_OFF_UNKNOWN_CALLBACK_001 PASS: ${err.message}`);
      expect(true).assertTrue();
      done();
    }
  })

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_001
   * @tc.desc:updateSpatialDeviceState success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_001", 0, async function (done) {
    let spatialDeviceState = {
      address: "1234",
      isSpatializationSupported: true,
      isHeadTrackingSupported: true,
      spatialDeviceType: 1
    }
    try {
      audioSpatializationManager.updateSpatialDeviceState(spatialDeviceState);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_001 updateSpatialDeviceState SUCCESS`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_001 ERROR: ${err.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_002
   * @tc.desc:updateSpatialDeviceState no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_002", 0, async function (done) {
    try {
      audioSpatializationManager.updateSpatialDeviceState();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_002 check no parameter fail.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_002 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_003
   * @tc.desc:updateSpatialDeviceState invalid parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_003", 0, async function (done) {
    let spatialDeviceState = {
      address: "1234",
      isSpatializationSupported: true,
      isHeadTrackingSupported: true,
      spatialDeviceType: -1
    }
    try {
      audioSpatializationManager.updateSpatialDeviceState(spatialDeviceState);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_003 check invalid parameter fail.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_UPDATE_SPATIAL_DEVICE_STATE_003 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_GET_SPATIALIZATION_SCENE_TYPE_001
   * @tc.desc:getSpatializationSceneType success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_GET_SPATIALIZATION_SCENE_TYPE_001", 0, async function (done) {
    try {
      let sceneType = audioSpatializationManager.getSpatializationSceneType();
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_GET_SPATIALIZATION_SCENE_TYPE_001 SUCCESS: ${sceneType}.`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_GET_SPATIALIZATION_SCENE_TYPE_001 ERROR: ${err}`);
      expect(false).assertTrue();
      done();
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_001
   * @tc.desc:setSpatializationSceneType success
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_001", 0, async function (done) {
    try {
      audioSpatializationManager.setSpatializationSceneType(0);
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_001 PASS`);
      expect(true).assertTrue();
      done();
    } catch (err) {
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_001 ERROR ${err.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_002
   * @tc.desc:setSpatializationSceneType no parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_002", 0, async function (done) {
    try {
      audioSpatializationManager.setSpatializationSceneType();
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_002 check no parameter fail.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_002 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_003
   * @tc.desc:setSpatializationSceneType invalid parameter
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_003", 0, async function (done) {
    try {
      audioSpatializationManager.setSpatializationSceneType(-1);
      console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_003 check invalid parameter fail.`);
      expect(false).assertTrue();
      done();
    } catch (err) {
      console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_SET_SPATIALIZATION_SCENE_TYPE_003 PASS: ${err.code}`);
      expect(err.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });
})
