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

describe("AudioRendererInterruptUnitTest", function() {
    beforeAll(async function () {
        // input testsuit setup step, setup invoked before all testcases
        console.info('AudioRendererInterruptUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step, teardown invoked after all testcases
        console.info('AudioRendererInterruptUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step, setup invoked before each testcases
        console.info('AudioRendererInterruptUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step, teardown invoked after each testcases
        console.info('AudioRendererInterruptUnitTest:afterEach called')
    })

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

    let renderInfo = {
        'MUSIC': {
            rendererFlags: 0,
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
        },
        'VOICE_CALL': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
            rendererFlags: 0
        },
        'RINGTONE': {
            rendererFlags: 0,
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_NOTIFICATION_RINGTONE,
        },
        'VOICE_ASSISTANT': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_VOICE_ASSISTANT,
            rendererFlags: 0
        },
        'ULTRASONIC': {
            rendererFlags: 0,
            content: audio.ContentType.CONTENT_TYPE_ULTRASONIC,
            usage: audio.StreamUsage.STREAM_USAGE_SYSTEM,
        },
        'ALARM': {
            content: audio.ContentType.CONTENT_TYPE_MUSIC,
            usage: audio.StreamUsage.STREAM_USAGE_ALARM,
            rendererFlags: 0
        },
        'ACCESSIBILITY': {
            rendererFlags: 0,
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_ACCESSIBILITY,
        },
        'SPEECH': {
            content: audio.ContentType.CONTENT_TYPE_SPEECH,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
            rendererFlags: 0
        },
        'MOVIE': {
            rendererFlags: 0,
            content: audio.ContentType.CONTENT_TYPE_MOVIE,
            usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
        },
        'UNKNOW': {
            content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
            usage: audio.StreamUsage.STREAM_USAGE_UNKNOWN,
            rendererFlags: 0
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
            console.log(" AudioRendererInterrupt:createAudioRenderer success.")
        } catch (err) {
            console.log(" AudioRendererInterrupt:createAudioRenderer err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
        return render
    }

    async function start(render,done) {
        try {
            await render.start()
            console.log(" AudioRendererInterrupt:start success.")
        } catch (err) {
            await release(render,done)
            console.log(" AudioRendererInterrupt:start err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            done()
        }
    }

    async function startFail(render,done,render1) {
        try {
            await render.start()
            console.log(" AudioRendererInterrupt:start success.")
        } catch (err) {
            console.log(" AudioRendererInterrupt:start err:" + JSON.stringify(err))
            await release(render,done)
            await release(render1,done)
            expect(true).assertEqual(true)
            done()
        }
    }

    async function stop(render,done) {
        try {
            await render.stop()
            console.log(" AudioRendererInterrupt:stop success.")
        } catch (err) {
            console.log(" AudioRendererInterrupt:stop err:" + JSON.stringify(err))
            expect(false).assertEqual(true)
            await release(render,done)
            done()
        }
    }

    async function release(render,done) {
        if (render.state == audio.AudioState.STATE_RELEASED) {
            console.log(" AudioRendererInterrupt:release render state: " + render.state)
            return
        }
        try {
            await render.release()
            console.log(" AudioRendererInterrupt:release success.")
        } catch (err) {
            console.log(" AudioRendererInterrupt:release err:" + JSON.stringify(err))
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
        } else {
        }
    }

    function sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_001', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt",async (eventAction) => {
            console.log("1.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_002', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async (eventAction) => {
            console.log("2.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_003', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async(eventAction) => {
            console.log("3.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_004', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt",async (eventAction) => {
            console.log("4.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let audiorenderer = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await audiorenderer.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audiorenderer, done)
        await sleep(500)
        await release(render1, done)
        await release(audiorenderer, done)
        done()    
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_006', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async(eventAction) => {
            console.log("6.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_007', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async(eventAction) => {
            console.log("7.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render2, done)
        await sleep(500)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_008', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt",async (eventAction) => {
            console.log("8.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_009', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt",async (eventAction) => {
            console.log("9.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_010', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt",async (eventAction) => {
            console.log("10.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    // VOICE_CALL
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_011', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("11-2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_012', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("12_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_013', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("13_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_014', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("14_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_016', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("16_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_017', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("17.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(audioRender1, done)

        let render2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_018', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("18_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_019', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("19_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_020', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("20_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    // RINGTONE
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_021', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("21_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_022', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("22.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_023', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("23_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_024', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("24_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_026', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("26_2.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await startFail(render2,done,render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_027', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async(eventAction) => {
            console.log("27.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_028', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("28_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_029', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("29_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_030', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("30_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    // VOICE_ASSISTANT
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_031', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("31.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_032', 0, async function (done) {
        let audioRender = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender.on("audioInterrupt", async(eventAction) => {
            console.log("32.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRender, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_033', 0, async function (done) {
        let audioRenderer = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await audioRenderer.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRenderer.on("audioInterrupt", async(eventAction) => {
            console.log("33.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRenderer, done)

        let audioRender2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRenderer, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_034', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("34.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
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

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_036', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("36.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_037', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("37.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let accessibilityRender = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await accessibilityRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(accessibilityRender, done)
        await sleep(500)
        await release(render1, done)
        await release(accessibilityRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_038', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("38.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_039', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("39.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_040', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("40.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    // ALARM
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_051', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("51.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)
        let audioRender = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(render1, done)
        await release(audioRender, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_052', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("52.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_053', 0, async function (done) {
        let audioRenderer = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await audioRenderer.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRenderer.on("audioInterrupt", async(eventAction) => {
            console.log("53.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRenderer, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(audioRenderer, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_054', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("54.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_056', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("56.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
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

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_057', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("57.eventAction=" + JSON.stringify(eventAction))
            interruptPauseType(eventAction);
        })
        await start(render1, done)

        let audioRender = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await audioRender.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender, done)
        await sleep(500)
        await release(audioRender, done)
        await sleep(500)
        await release(render1, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_058', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("58.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender1, done)
        let audioRender2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_059', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("59.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender1, done)
        let audioRender2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_060', 0, async function (done) {
        let audioRender1 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['44100'])
        await audioRender1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        audioRender1.on("audioInterrupt", async(eventAction) => {
            console.log("60.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(audioRender1, done)
        let audioRender2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await audioRender2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(audioRender2, done)
        await sleep(500)
        await release(audioRender1, done)
        await release(audioRender2, done)
        done()
    })

    // ACCESSIBILITY
    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_061', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MUSIC'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("61_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_062', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_CALL'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("62_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_063', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("63.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['RINGTONE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_064', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['VOICE_ASSISTANT'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("64_2.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await startFail(render2, done, render1)
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_066', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['ALARM'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("66_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_067', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render1.on("audioInterrupt", async(eventAction) => {
            console.log("67.eventAction=" + JSON.stringify(eventAction))
            expect(eventAction.hintType).assertEqual(audio.InterruptHint.INTERRUPT_HINT_STOP)
        })
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_068', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['SPEECH'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("68_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_069', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['MOVIE'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("69_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })

    it('SUB_AUDIO_RENDERER_INTERRUPT_TEST_070', 0, async function (done) {
        let render1 = await createAudioRenderer(renderInfo['ACCESSIBILITY'], streamInfo['44100'])
        await render1.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        await start(render1, done)

        let render2 = await createAudioRenderer(renderInfo['UNKNOW'], streamInfo['48000'])
        await render2.setInterruptMode(audio.InterruptMode.INDEPENDENT_MODE)
        render2.on("audioInterrupt", async(eventAction) => {
            console.log("70_2.eventAction=" + JSON.stringify(eventAction))
            interruptType(eventAction);
        })
        await start(render2, done)
        await sleep(500)
        await release(render1, done)
        await release(render2, done)
        done()
    })
})
