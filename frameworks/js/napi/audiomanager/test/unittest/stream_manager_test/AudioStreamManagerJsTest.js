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

const TAG = "[AudioStreamManagerJsTest]";

describe("AudioStreamManagerJsTest", function () {
  const ERROR_INPUT_INVALID = '401';
  const ERROR_INVALID_PARAM = '6800101';

  let AudioStreamInfo = {
    samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
    channels: audio.AudioChannel.CHANNEL_2,
    sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S32LE,
    encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
  }

  let AudioRendererInfo = {
      content: audio.ContentType.CONTENT_TYPE_RINGTONE,
      usage: audio.StreamUsage.STREAM_USAGE_NOTIFICATION_RINGTONE,
      rendererFlags: 0
  }

  let AudioRendererOptions = {
      streamInfo: AudioStreamInfo,
      rendererInfo: AudioRendererInfo
  }

  let AudioCapturerInfo = {
    source: audio.SourceType.SOURCE_TYPE_MIC,
    capturerFlags: 0
  }

  let AudioCapturerOptions = {
    streamInfo: AudioStreamInfo,
    capturerInfo: AudioCapturerInfo,
  }

  beforeAll(async function () {
    console.info(TAG + "AudioStreamManagerJsTest:beforeAll called");
  })

  afterAll(function () {
    console.info(TAG + 'AudioStreamManagerJsTest:afterAll called')
  })

  beforeEach(function () {
    console.info(TAG + 'AudioStreamManagerJsTest:beforeEach called')
  })

  afterEach(function () {
    console.info(TAG + 'AudioStreamManagerJsTest:afterEach called')
  })

  function sleep(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
  }

  /*
   * @tc.name:getCurrentAudioRendererInfoArray001
   * @tc.desc:Get getCurrentAudioRendererInfoArray
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray001", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }

    audioStreamManager.getCurrentAudioRendererInfoArray(async (err, audioRendererInfos) => {
      if (err) {
        console.error(`${TAG} getCurrentAudioRendererInfoArray001 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        await audioRenderer.release();
        done();
        return;
      }

      console.info("getCurrentAudioRendererInfoArray001:"+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(1).assertEqual(audioRendererInfos[0].rendererState);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      done();
    })
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArray002
   * @tc.desc:Get getCurrentAudioRendererInfoArray--promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray002", 0, async function (done) {

    try {
      let audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      let audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioRendererInfos= await audioStreamManager.getCurrentAudioRendererInfoArray();
      console.info(`${TAG} getCurrentAudioRendererInfoArray002 SUCCESS`+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(1).assertEqual(audioRendererInfos[0].rendererState);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      done();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray002 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArray003
   * @tc.desc:Get getCurrentAudioRendererInfoArray
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray003", 0, async function (done) {

    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray003 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }

    audioStreamManager.getCurrentAudioRendererInfoArray(async (err, audioRendererInfos) => {
      if (err) {
        console.error(`${TAG} getCurrentAudioRendererInfoArray003 ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        await audioRenderer.release();
        done();
        return;
      }
      console.info("AudioRendererChangeInfoArray++++:"+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(2).assertEqual(audioRendererInfos[0].rendererState);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      done();
    })
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArray004
   * @tc.desc:Get getCurrentAudioRendererInfoArray--promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray004", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
      let audioRendererInfos = await audioStreamManager.getCurrentAudioRendererInfoArray();
      console.info("getCurrentAudioRendererInfoArray004:"+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(2).assertEqual(audioRendererInfos[0].rendererState);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();
      sleep(1)
      await audioRenderer.release();
      done();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray004 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArray005
   * @tc.desc:Get getCurrentAudioRendererInfoArray
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray005", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
      await audioRenderer.stop();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray005 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }

    audioStreamManager.getCurrentAudioRendererInfoArray(async (err, AudioRendererInfoArray) => {
      if (err) {
        console.error(`${TAG} first getCurrentAudioRendererInfoArray ERROR: ${JSON.stringify(err)}`);
        expect(false).assertTrue();
        await audioRenderer.release();
        done();
        return;
      }
      console.info("getCurrentAudioRendererInfoArray005:"+JSON.stringify(AudioRendererInfoArray));
      expect(AudioRendererInfoArray.length).assertLarger(0);
      expect(3).assertEqual(AudioRendererInfoArray[0].rendererState);
      expect(AudioRendererInfoArray[0].deviceDescriptors[0].displayName !== ""
        && AudioRendererInfoArray[0].deviceDescriptors[0].displayName !== undefined).assertTrue();

      await audioRenderer.release();

      audioStreamManager.getCurrentAudioRendererInfoArray(async (err, AudioRendererInfos) => {
        if (err) {
          console.error(`${TAG} sencond getCurrentAudioRendererInfoArray ERROR: ${JSON.stringify(err)}`);
          expect(false).assertTrue();
          await audioRenderer.release();
          done();
          return;
        }
        expect(AudioRendererInfos.length).assertEqual(0);
      })
      done();
    })
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArray006
   * @tc.desc:Get getCurrentAudioRendererInfoArray--promise
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray006", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
      await audioRenderer.stop();
      let audioRendererInfos = await audioStreamManager.getCurrentAudioRendererInfoArray();
      expect(audioRendererInfos.length).assertLarger(0);
      expect(3).assertEqual(audioRendererInfos[0].rendererState);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      audioRendererInfos = await audioStreamManager.getCurrentAudioRendererInfoArray();
      expect(audioRendererInfos.length).assertEqual(0);
      done();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray006 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }
  });

  /*
   * @tc.name:AudioRendererChange007
   * @tc.desc:Get AudioRendererChange info
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray007", 0, async function (done) {
    let count = 0;
    try {
      let audioStreamManager = audio.getAudioManager().getStreamManager();
      audioStreamManager.on('audioRendererChange', (AudioRendererChangeInfoArray) =>{

        expect(AudioRendererChangeInfoArray.length).assertLarger(0);
        count ++;
        expect(AudioRendererChangeInfoArray[0].deviceDescriptors[0].displayName!==""
        || AudioRendererChangeInfoArray[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

        if (count == 2) {
          done();
        }
      })

      let audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      await audioRenderer.release();

    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray007 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
    }
  });

  /*
   * @tc.name:AudioRendererChange008
   * @tc.desc:Get AudioRendererChange info
   * @tc.type: FUNC
   * @tc.require: I6C9VA
   */
  it("getCurrentAudioRendererInfoArray008", 0, async function (done) {
    let count = 0;
    try {
      let audioStreamManager = audio.getAudioManager().getStreamManager();
      let flag = false;
      audioStreamManager.on('audioRendererChange', (AudioRendererChangeInfoArray) =>{

        expect(AudioRendererChangeInfoArray.length).assertLarger(0);
        count ++;
        if (AudioRendererChangeInfoArray[0].deviceDescriptors[0].displayName !== ""
            && AudioRendererChangeInfoArray[0].deviceDescriptors[0].displayName !== undefined) {
          flag = true;
        }
        if (count == 3) {
          done();
        }
      })

      let audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      await audioRenderer.start();
      await audioRenderer.release();
      sleep(1);
      expect(flag).assertTrue();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArray007 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
    }
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArraySync001
   * @tc.desc:Get getCurrentAudioRendererInfoArraySync
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getCurrentAudioRendererInfoArraySync001", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArraySync001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
    sleep(1);
    try {
      let audioRendererInfos = audioStreamManager.getCurrentAudioRendererInfoArraySync();
      console.info("getCurrentAudioRendererInfoArraySync001:"+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      done();
    } catch (err) {
      console.error(`${TAG} getCurrentAudioRendererInfoArraySync001 ERROR: ${JSON.stringify(err)}`);
      expect(false).assertTrue();
      await audioRenderer.release();
      done();
      return;
    }
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArraySync002
   * @tc.desc:Get getCurrentAudioRendererInfoArraySync
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getCurrentAudioRendererInfoArraySync002", 0, async function (done) {

    let audioRenderer = null;
    let audioStreamManager = null;
    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArraySync002 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }

    try {
      let audioRendererInfos = audioStreamManager.getCurrentAudioRendererInfoArraySync();
      console.info("AudioRendererChangeInfoArray++++:"+JSON.stringify(audioRendererInfos));
      expect(audioRendererInfos.length).assertLarger(0);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      done();
    } catch (err) {
      console.error(`${TAG} getCurrentAudioRendererInfoArraySync002 ERROR: ${JSON.stringify(err)}`);
      expect(false).assertTrue();
      await audioRenderer.release();
      done();
    }
  });

  /*
   * @tc.name:getCurrentAudioRendererInfoArraySync003
   * @tc.desc:Get getCurrentAudioRendererInfoArraySync
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getCurrentAudioRendererInfoArraySync003", 0, async function (done) {
    let audioRenderer = null;
    let audioStreamManager = null;

    try {
      audioRenderer = await audio.createAudioRenderer(AudioRendererOptions);
      audioStreamManager = audio.getAudioManager().getStreamManager();
      await audioRenderer.start();
      await audioRenderer.stop();
      let audioRendererInfos = audioStreamManager.getCurrentAudioRendererInfoArraySync();
      expect(audioRendererInfos.length).assertLarger(0);
      expect(audioRendererInfos[0].deviceDescriptors[0].displayName!==""
        && audioRendererInfos[0].deviceDescriptors[0].displayName!==undefined).assertTrue();

      await audioRenderer.release();
      audioRendererInfos = audioStreamManager.getCurrentAudioRendererInfoArraySync();
      expect(audioRendererInfos.length).assertEqual(0);
      done();
    } catch(e) {
      console.error(`${TAG} getCurrentAudioRendererInfoArraySync003 ERROR: ${e.message}`);
      expect().assertFail();
      await audioRenderer.release();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync001
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_UNKNOWN
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync001", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(audio.StreamUsage.STREAM_USAGE_UNKNOWN);
      console.info(`${TAG} getAudioEffectInfoArraySync success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
			expect(audioEffectInfoArray[1]).assertEqual(1);
      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync001 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync002
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_MEDIA
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync002", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(audio.StreamUsage.STREAM_USAGE_MEDIA);
      console.info(`${TAG} getAudioEffectInfoArraySync002 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
			expect(audioEffectInfoArray[1]).assertEqual(1);
      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync002 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync003
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_MUSIC
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync003", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(audio.StreamUsage.STREAM_USAGE_MUSIC);
      console.info(`${TAG} getAudioEffectInfoArraySync003 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
			expect(audioEffectInfoArray[1]).assertEqual(1);
      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync003 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync004
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_VOICE_COMMUNICATION
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync004", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION);
      console.info(`${TAG} getAudioEffectInfoArraySync004 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
			expect(audioEffectInfoArray[1]).assertEqual(1);
      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync004 ERROR: ${e.message}`);
      expect().assertFail();
      done();
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync005
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_VOICE_ASSISTANT
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync005", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(audio.StreamUsage.STREAM_USAGE_VOICE_ASSISTANT);
      console.info(`${TAG} getAudioEffectInfoArraySync005 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
			expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync005 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync006
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_ALARM
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync006", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_ALARM);
      console.info(`${TAG} getAudioEffectInfoArraySync006 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync006 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync007
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_VOICE_MESSAGE
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync007", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_VOICE_MESSAGE);
      console.info(`${TAG} getAudioEffectInfoArraySync007 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync007 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync008
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_NOTIFICATION_RINGTONE
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync008", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_NOTIFICATION_RINGTONE);
      console.info(`${TAG} getAudioEffectInfoArraySync008 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync008 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync009
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_RINGTONE
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync009", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_RINGTONE);
      console.info(`${TAG} getAudioEffectInfoArraySync009 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync009 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync010
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_NOTIFICATION
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync010", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_NOTIFICATION);
      console.info(`${TAG} getAudioEffectInfoArraySync010 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync010 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync011
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_ACCESSIBILITY
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync011", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_ACCESSIBILITY);
      console.info(`${TAG} getAudioEffectInfoArraySync011 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync011 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync012
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_MOVIE
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync012", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_MOVIE);
      console.info(`${TAG} getAudioEffectInfoArraySync012 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync012 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync013
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_GAME
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync013", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_GAME);
      console.info(`${TAG} getAudioEffectInfoArraySync013 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync013 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync014
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_AUDIOBOOK
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync014", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_AUDIOBOOK);
      console.info(`${TAG} getAudioEffectInfoArraySync014 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync014 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync015
   * @tc.desc:Get getAudioEffectInfoArraySync success - STREAM_USAGE_NAVIGATION
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync015", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(
        audio.StreamUsage.STREAM_USAGE_NAVIGATION);
      console.info(`${TAG} getAudioEffectInfoArraySync015 success:${JSON.stringify(audioEffectInfoArray)}`);
      expect(audioEffectInfoArray.length).assertLarger(0);
      expect(audioEffectInfoArray[0]).assertEqual(0);
      expect(audioEffectInfoArray[1]).assertEqual(1);

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync015 ERROR: ${e.message}`);
      expect().assertFail();
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync016
   * @tc.desc:Get getAudioEffectInfoArraySync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync016", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync();
      console.info(`The effect modes is obtained ${audioEffectInfoArray}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync016 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync017
   * @tc.desc:Get getAudioEffectInfoArraySync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync017", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync("Invalid type");
      console.info(`The effect modes is obtained ${audioEffectInfoArray}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync017 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:getAudioEffectInfoArraySync018
   * @tc.desc:Get getAudioEffectInfoArraySync fail(6800101) - Invalid param value : 10000
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("getAudioEffectInfoArraySync018", 0, async function (done) {
    let invalidVolumeType = 10000;
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let audioEffectInfoArray = audioStreamManager.getAudioEffectInfoArraySync(invalidVolumeType);
      console.info(`The effect modes is obtained ${audioEffectInfoArray}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} getAudioEffectInfoArraySync018 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync001
   * @tc.desc:Get isActiveSync success - VOICE_CALL - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync001", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.VOICE_CALL);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync001 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync002
   * @tc.desc:Get isActiveSync success - RINGTONE - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync002", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.RINGTONE);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync002 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync003
   * @tc.desc:Get isActiveSync success - MEDIA - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync003", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.MEDIA);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync003 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync004
   * @tc.desc:Get isActiveSync success - ALARM - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync004", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.ALARM);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync004 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync005
   * @tc.desc:Get isActiveSync success - ACCESSIBILITY - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync005", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.ACCESSIBILITY);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync005 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync006
   * @tc.desc:Get isActiveSync success - VOICE_ASSISTANT - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync006", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.VOICE_ASSISTANT);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync006 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync007
   * @tc.desc:Get isActiveSync success - ULTRASONIC - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync007", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.ULTRASONIC);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync007 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync008
   * @tc.desc:Get isActiveSync success - ALL - When stream is NOT playing
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync008", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(audio.AudioVolumeType.ALL);
      console.info(`The active status is obtained ${isActive}.`);
      expect(isActive).assertEqual(false);

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync008 ERROR: ${e.message}`);
      expect(false).assertTrue();
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync009
   * @tc.desc:Get isActiveSync fail(401) - Invalid param count : 0
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync009", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync();
      console.info(`The active status is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync009 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync010
   * @tc.desc:Get isActiveSync fail(401) - Invalid param type : "Invalid type"
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync010", 0, async function (done) {
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync("Invalid type");
      console.info(`The active status is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync010 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INPUT_INVALID);
      done();
      return;
    }
  });

  /*
   * @tc.name:isActiveSync011
   * @tc.desc:Get isActiveSync fail(6800101) - Invalid param value : 10000
   * @tc.type: FUNC
   * @tc.require: I7V04L
   */
  it("isActiveSync011", 0, async function (done) {
    let invalidVolumeType = 10000;
    let audioStreamManager = null;

    try {
      audioStreamManager = audio.getAudioManager().getStreamManager();
      let isActive = audioStreamManager.isActiveSync(invalidVolumeType);
      console.info(`The active status is obtained ${isActive}.`);
      expect(false).assertTrue();

      done();
    } catch(e) {
      console.error(`${TAG} isActiveSync011 ERROR: ${e.message}`);
      expect(e.code).assertEqual(ERROR_INVALID_PARAM);
      done();
      return;
    }
  });
})
