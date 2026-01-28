/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

import audioNapi from 'libentry.so';
import { saveFileBuffer } from '../utils/ExportFile';
import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import { audio } from '@kit.AudioKit';

const TAG: string = 'AudioEditTestApp_PlayAudio';

@Component
export struct PlayAudio {
    @Prop pcmBufferLength:number = 0;
    @Prop pitchTotalAudioTime:number = 0;
    @State playCurrentBufferSize: number = 0;
    @State @Watch('isPlayStop') playCurrentTime: number = 0;
    @State fileName: string | Resource = '';
    @Link isPlay: boolean;
    // 播放状态
    @State renderState: number = 0;
    private isCallbackRegistered: boolean = false;

    aboutToAppear(): void {
        Logger.info(TAG, 'aboutToAppear');
        if (!this.isCallbackRegistered) {
            Logger.info(TAG, 'aboutToAppear registerAudioRendererCallback');
            // audioNapi.registerAudioRendererCallback((result: number) => {
            //     this.playCurrentBufferSize = result;
            //     Logger.info(TAG, `playCurrentBufferSize: ${this.playCurrentBufferSize}`);
            //     this.getCurrentTime();
            // });
            this.isCallbackRegistered = true;
        }
    }

    aboutToDisappear(): void {
        Logger.info(TAG, 'aboutToDisappear');
    }

    build() {
        Column() {
            Column() {
                Row() {
                    Text(`${this.fileName}`)
                        .fontSize($r('app.float.common_font'))
                        .fontWeight(StyleConst.FONT_WEIGHT_500)
                        .fontColor($r('sys.color.font_primary'))
                        .fontFamily($r('sys.string.ohos_id_text_font_family_medium'))
                    Image(this.renderState === audio.AudioState.STATE_RUNNING ?
                        $r('app.media.ic_record_playing') : $r('app.media.ic_record_paused'))
                        .width($r('app.float.common_image'))
                        .height($r('app.float.common_image'))
                        .id('playing_state')
                        .onClick(() => {
                            if (this.renderState === audio.AudioState.STATE_RUNNING) {
                                audioNapi.audioRendererPause();
                                this.renderState = audioNapi.getRendererState();
                            } else {
                                audioNapi.audioRendererStart();
                                this.renderState = audioNapi.getRendererState();
                            }
                        })
                }
                .width(StyleConst.NINETY_PERCENT_WIDTH)
                .height($r('app.float.common_height'))
                .justifyContent(FlexAlign.SpaceBetween)
                Row() {
                    Text(this.renderState === audio.AudioState.STATE_RUNNING ? '正在播放...' : '暂停')
                        .fontSize($r('app.float.small_size'))
                        .fontFamily($r('sys.string.ohos_id_text_font_family_medium'))
                        .fontColor($r('sys.color.font_primary'))
                        .opacity(StyleConst.COMMON_OPACITY)
                    Text(this.getTimesBySecond(this.playCurrentTime) + '/' + this.getTimesBySecond(this.pitchTotalAudioTime))
                        .fontSize($r('app.float.small_size'))
                        .fontFamily($r('sys.string.ohos_id_text_font_family_medium'))
                        .fontColor($r('sys.color.font_primary'))
                        .opacity(StyleConst.COMMON_OPACITY)
                }
                .justifyContent(FlexAlign.SpaceBetween)
                .width(StyleConst.NINETY_PERCENT_WIDTH)
                .height($r('app.float.common_height'))
                Row() {
                    Progress({
                        value: this.playCurrentTime,
                        total: this.pitchTotalAudioTime,
                        type: ProgressType.Linear
                    })
                        .width(StyleConst.NINETY_PERCENT_WIDTH)
                        .margin({top: $r('app.float.margin_5')})
                        .style({strokeWidth: 6, enableSmoothEffect: true})
                }
            }.visibility(this.isPlay ? Visibility.Visible : Visibility.None)
        }
        .width(StyleConst.FULL_WIDTH)
        .margin($r('app.float.margin_35'))
    }

    getCurrentTime() {
        this.playCurrentTime = (this.playCurrentBufferSize / this.pcmBufferLength) * this.pitchTotalAudioTime / 2;
        Logger.info(TAG, `playCurrentTime: ${this.playCurrentTime}`);
    }

    getTimesBySecond(t: number) {
        let h = Math.floor(t / 60 / 60 % 24);
        let m = Math.floor(t / 60 % 60);
        let s = Math.floor(t % 60);
        let hs = h < 10 ? '0' + h : h;
        let ms = m < 10 ? '0' + m : m;
        let ss = s < 10 ? '0' + s : s;
        return `${hs}:${ms}:${ss}`;
    }

    isPlayStop() {
        if (this.playCurrentTime >= this.pitchTotalAudioTime) {
            this.renderState = audioNapi.getRendererState();
            this.isPlay = false;
        }
    }
}