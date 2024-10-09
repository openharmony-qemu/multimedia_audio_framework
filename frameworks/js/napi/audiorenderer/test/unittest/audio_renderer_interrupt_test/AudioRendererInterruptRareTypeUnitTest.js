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

describe("AudioRendererInterruptRareTypeUnitTest", function() {
    beforeAll(async function () {
        // input testsuit setup step, setup invoked before all testcases
        console.info('AudioRendererInterruptRareTypeUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step, teardown invoked after all testcases
        console.info('AudioRendererInterruptRareTypeUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step, setup invoked before each testcases
        console.info('AudioRendererInterruptRareTypeUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step, teardown invoked after each testcases
        console.info('AudioRendererInterruptRareTypeUnitTest:afterEach called')
    })

    let renderInfo = {
        'MUSIC': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0,
        },
        'VOICE_CALL': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
            rendererFlags: 0
        },
        'RINGTONE': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_NOTIFICATION_RINGTONE,
            rendererFlags: 0,
        },
        'VOICE_ASSISTANT': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_ASSISTANT,
            rendererFlags: 0
        },
        'ULTRASONIC': {
            content: audio.ContentType.CONTENT_TYPE_ULTRASONIC,
            usage: audio.StreamUsage.STREAM_USAGE_SYSTEM,
            rendererFlags: 0
        },
        'ALARM': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_ALARM,
            rendererFlags: 0
        },
        'ACCESSIBILITY': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_ACCESSIBILITY,
            rendererFlags: 0
        },
        'SPEECH': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0
        },
        'MOVIE': {
            content: audio.ContentType.CONTENT_TYPE_MOVIE,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0
        },
        'UNKNOW': {
            content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
            usage: audio.StreamUsage.STREAM_USAGE_UNKNOWN,
            rendererFlags: 0
        },
    }

    let streamInfo = {
        '44100': {
            samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_44100,
            channels: audio.AudioChannel.CHANNEL_2,
            sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
            encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
        },
        '48000' : {
            samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
            channels: audio.AudioChannel.CHANNEL_2,
            sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S32LE,
            encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
        },
    }

    async function createAudioRenderer(AudioRendererInfo, AudioStreamInfo, done) {
        let render = null;

        var AudioRendererOptions = {
            streamInfo: AudioStreamInfo,
            rendererInfo: AudioRendererInfo
        }
        try {
            render = await audio.createAudioRenderer(AudioRendererOptions)
            console.log(" AudioRendererInterruptRareType:createAudioRenderer success.")
        } catch (err) {
            console.log(" AudioRendererInterruptRareType:createAudioRenderer err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
        return render
    }

    async function start(render,done) {
        try {
            await render.start()
            console.log(" AudioRendererInterruptRareType:start success.")
        } catch (err) {
            await release(render,done)
            console.log(" AudioRendererInterruptRareType:start err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    async function interruptPauseType(eventAction) {   
        if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_BEGIN) {
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_PAUSE)
        } else if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_END) {
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_RESUME)
        } else {}
    }

    async function interruptType(eventAction) {   
        if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_BEGIN) {
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_DUCK)
        } else if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_END) {
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_UNDUCK)
        } else {}
    }

    async function startFail(render,done,render1) {
        try {
            await render.start()
            console.log("AudioRendererInterruptRareType:start success.")
        } catch (err) {
            console.log("AudioRendererInterruptRareType:start err:" + JSON.stringify(err))
            await release(render,done)
            await release(render1,done)
            expect(true).assertEqual(true)
            done()
        }
    }


    async function stop(render,done) {
        try {
            await render.stop()
            console.log("AudioRendererInterruptRareType:stop success.")
        } catch (err) {
            console.log("AudioRendererInterruptRareType:stop err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            await release(render,done)
            done()
        }
    }

    async function release(render,done) {
        if (render.state == audio.AudioState.STATE_RELEASED) {
            console.log("AudioRendererInterruptRareType:release render state: " + render.state)
            return
        }
        try {
            await render.release()
            console.log("AudioRendererInterruptRareType:release success.")
        } catch (err) {
            console.log("AudioRendererInterruptRareType:release err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    function sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    // SPEECH
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_071', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt",async (eventAction) => {
            console.log("71.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_072', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async (eventAction) => {
            console.log("72.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(audioRender, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_073', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("73.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(audioRender, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_074', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("74.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let assistantRender = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await assistantRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(assistantRender, done)
        await sleep(500)
        await release(render1, done)
        await release(assistantRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_076', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("76.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_077', 0, async function (done) {
        let audiorender1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await audiorender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audiorender1.on("audioInterrupt", async(eventAction) => {
            console.log("77.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(audiorender1, done)

        let audiorender2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await audiorender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audiorender2, done)
        await sleep(500)
        await release(audiorender2, done)
        await sleep(500)
        await release(audiorender1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_078', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("78.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let speechRender = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await speechRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(speechRender, done)
        await sleep(500)
        await release(render1, done)
        await release(speechRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_079', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt",async (eventAction) => {
            console.log("79.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender1, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_080', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("80.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender1, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender1, done)
        done()
    })

    // MOVIE
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_081', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("81.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let musicRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await musicRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(musicRender, done)
        await sleep(500)
        await release(render1, done)
        await release(musicRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_082', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async (eventAction) => {
            console.log("82.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_083', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("83.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_084', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt",async (eventAction) => {
            console.log("84.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(audioRender1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_086', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("86.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_087', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("87.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_088', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("88.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_089', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("89.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let movieRender = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await movieRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(movieRender, done)
        await sleep(500)
        await release(render1, done)
        await release(movieRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_090', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("90.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender2, done)
        done()
    })

    // UNKNOW
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_091', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("91.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_092', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async (eventAction) => {
            console.log("92.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_093', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("93.eventAction=" + JSON.stringify(eventAction))
            if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_BEGIN) {
                expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_PAUSE)
            } else if (eventAction.eventType == audio.InterruptType.INTERRUPT_TYPE_END) {
                expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_RESUME)
            } else {}
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_094', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("94.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let render = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await render.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render, done)
        await sleep(500)
        await release(render1, done)
        await release(render, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_096', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("96.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let alarmRender = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await alarmRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(alarmRender, done)
        await sleep(500)
        await release(render1, done)
        await release(alarmRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_097', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("97.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let accessibityRender = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await accessibityRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(accessibityRender, done)
        await sleep(500)
        await release(accessibityRender, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_098', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("98.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_099', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("99.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_100', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("100.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    // 两个stream同时为share mode
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_101', 0, async function (done) {
        let render1_callback = false
        let render2_callback = false
        let render1 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.SHARE_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("101.eventAction=" + JSON.stringify(eventAction))
            render1_callback = true
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.SHARE_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("101_2.eventAction=" + JSON.stringify(eventAction))
            render1_callback = true
        })
        await start(render2, done)
        await sleep(500)
        console.log("render1_callback = " + render1_callback + ", render2_callback = " + render2_callback)
        expect(render1_callback == false && render2_callback == false).assertTrue()
        await sleep(100)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    // 第一个为share mode, 第二个为Independe mode
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_102', 0, async function (done) {
        let render1_callback = false
        let render2_callback = false
        let render1 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.SHARE_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("102.eventAction=" + JSON.stringify(eventAction))
            render1_callback = true
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("102_2.eventAction=" + JSON.stringify(eventAction))
            render2_callback == true
        })
        await start(render2, done)
        await sleep(500)
        console.log("102.render1_callback = " + render1_callback + ", render2_callback = " + render2_callback)
        expect(render1_callback == true && render2_callback == false).assertTrue()
        await sleep(100)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    // 第一个为independ mode, 第二个为share mode
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_103', 0, async function (done) {
        let render1_callback = false
        let render2_callback = false
        let render1 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("103.eventAction=" + JSON.stringify(eventAction))
            render1_callback = true
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("103_2.eventAction=" + JSON.stringify(eventAction))
            render2_callback = true
        })
        await start(render2, done)
        await sleep(500)
        console.log("103.render1_callback = " + render1_callback + ", render2_callback = " + render2_callback)
        expect(render1_callback == true && render2_callback == false).assertTrue()
        await sleep(100)
        await release(render1, done)
        await release(render2, done)
        done()
    })

})
