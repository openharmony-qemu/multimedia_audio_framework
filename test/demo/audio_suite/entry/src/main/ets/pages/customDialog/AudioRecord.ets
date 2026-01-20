/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { JSON, taskpool, util } from '@kit.ArkTS';
import { Logger } from '../../utils/Logger';
import audioNapi from 'libentry.so';
import { BusinessError } from '@kit.BasicServicesKit';
import { getTimesBySecond, getDate, getDateStringWithTimeStamp } from '../../utils/Util';
import { common } from '@kit.AbilityKit';
import { fileIo as fs, fileUri, picker } from '@kit.CoreFileKit';
import { PromptAction } from '@kit.ArkUI';
import { AudioTrack, BitsPerSampleMode } from '../../utils/InterfaceInfo';
import { saveFileBuffer } from '../../utils/ExportFile';

const TAG: string = 'AudioEditTestApp_CheckNodeDialog';
const INTERVAL_TIME = 1000;
const documentSaveOptions = new picker.DocumentSaveOptions();
// 配置保存的模式为DOWNLOAD，若配置了DOWNLOAD模式，此时配置的其他documentSaveOptions参数将不会生效。
documentSaveOptions.pickerMode = picker.DocumentPickerMode.DOWNLOAD;

@CustomDialog
export struct CustomDialogAudioRecord {
  controller?: CustomDialogController;
  promptAction: PromptAction = this.getUIContext().getPromptAction();

  @State date: string = '';
  @Link recordSec: number;
  @Link recordState: string;
  @Link realPlayRecord: string;
  @Link showTime: string;
  @Link interval: number;
  @Link isRecordOver: boolean;
  @Link audioTrackList: AudioTrack[];

  @Prop sampleRate: number = -1;
  @Prop channels: number = -1;
  @Prop bitsPerSample: number = -1;
  @Prop bitsPerSampleMode: number = BitsPerSampleMode.FLOAT;
  @Prop startTime: number;
  @Prop isPlay: boolean;
  @Prop inputId: string;
  @Prop mixerId: string;
  @Prop outPutId: string;
  private timerId: number = -1;

  build() {
    Column() {
      if (this.recordState === 'started' || this.recordState === 'continued'){
        this.StartedRecord();
      } else if (this.recordState === 'paused'){
        this.PausedRecord();
      }
    }
  }

  @Builder
  StartedRecord() {
    Column() {
      Text(this.showTime).fontSize(21).fontWeight(500).margin({ bottom: 8 })
    }.id('show_time_txt')

    Row() {
      Column() {
        Image($r('app.media.ic_record_circle_2')).width(24).height(24)
          .id('stop_record_btn')
          .onClick(() => {
            if (this.controller != undefined) {
              this.controller.close();
            }
            if (this.timerId !== -1) {
              clearTimeout(this.timerId);
            }
            this.timerId = setTimeout(() => {
              this.capturerStop();
            }, 10)
          })
        Text($r('app.string.STOP')).fontSize(12).fontWeight(400).id('pause_record_btn').margin({ top: 2 })
      }
      .alignItems(HorizontalAlign.Center)
      .justifyContent(FlexAlign.Center)

      Column() {
        Image($r('app.media.ic_record_paused_green')).width(24).height(24)
          .onClick(() => {
            this.capturerPause()
          })
        Text($r('app.string.PAUSE')).fontSize(12).fontWeight(400).id('pause_record_btn').margin({ top: 2 })
      }
      .alignItems(HorizontalAlign.Center)
      .justifyContent(FlexAlign.Center)
      .margin({ left: 16 })
    }
  }

  //录制停止
  capturerStop() {
    try {
      audioNapi.audioCapturerStop();
      audioNapi.audioCapturerRelease();
      this.recordState = 'init';
      if (this.isPlay) {
        this.realPlayRecord = 'finish';
      }
      clearInterval(this.interval);
      this.saveAsWavFile();
    } catch (err) {
      let error = err as BusinessError;
      this.recordState = 'init';
      Logger.info(TAG, `AudioRecording:audioCapturer stop err=${JSON.stringify(error)}`);
    }
    this.isRecordOver = true;
  }

  //Determine whether there is audio on the track and whether the audio is playing.
  judgeMixAudio(): boolean {
    return this.audioTrackList.some(track => track.audioAssetArray?.length !== 0);
  }

  //保存wave文件
  // 新增：直接将录制的音频保存为 WAV 文件，并弹出提示框显示路径
  async saveAsWavFile(): Promise<void> {
    try {
      // 确保 context 已初始化
      let uri: string = '';
      // 请在组件内获取context，确保this.getUIContext().getHostContext()返回结果为UIAbilityContext
      let context = this.getUIContext().getHostContext() as common.UIAbilityContext;
      this.date = getDate(1);
      const documentViewPicker = new picker.DocumentViewPicker(context);
      const documentSaveOptions = new picker.DocumentSaveOptions();
      documentSaveOptions.pickerMode = picker.DocumentPickerMode.DOWNLOAD;
      if (!context) {
        Logger.error(TAG, `UIAbilityContext not initialized!`);
        this.showToast('保存失败：上下文未初始化');
        return;
      }
      //judge audioTrackList is playing song
      let pcmData: ArrayBuffer = new ArrayBuffer(0);
      if (this.judgeMixAudio()) {
        audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, this.bitsPerSampleMode);
        //mix audio
        if (this.isPlay){
          //获取实时播放录音的buffer
          pcmData = audioNapi.realPlayRecordBuffer(this.inputId);
        } else {
          Logger.warn(TAG,`this.inputId:${this.inputId}, this.mixerId:${this.mixerId}, this.outPutId:${this.outPutId}`)
          audioNapi.mixRecordBuffer(this.inputId, this.mixerId, this.outPutId);
          // pcmData = audioNapi.saveFileBuffer();
          pcmData = await taskpool.execute(saveFileBuffer) as ArrayBuffer;
        }
        //Destroy the temporary input node created by the recording.
        let result = audioNapi.deleteNode(this.inputId);
        if (result !== 0) {
          Logger.error(TAG, `delete record inputNode error: ${result}`);
        }
        audioNapi.clearByInputId(this.inputId, this.startTime);
      } else {
        //only record audio
        pcmData = audioNapi.getAudioFrames();
      }
      if (!pcmData || pcmData.byteLength === 0) {
        Logger.error(TAG, `No audio data to save.`);
        this.showToast('保存失败：无音频数据');
        return;
      }
      Logger.info(TAG, `PCM data length is: ${pcmData.byteLength}`);

      // Construct the WAV header.
      const wavHeader: ArrayBuffer = new ArrayBuffer(44);
      const view: DataView = new DataView(wavHeader);

      // RIFF identifier
      view.setUint32(0, 0x52494646, false); // "RIFF"
      // file length (header + data)
      view.setUint32(4, 36 + pcmData.byteLength, true);
      // RIFF type
      view.setUint32(8, 0x57415645, false); // "WAVE"
      // format chunk identifier
      view.setUint32(12, 0x666D7420, false); // "fmt "
      // format chunk length
      view.setUint32(16, 16, true);
      // sample format (1 is PCM)
      if (this.bitsPerSample === 32 && this.bitsPerSampleMode === BitsPerSampleMode.FLOAT) {
        view.setUint16(20, 3, true);
      } else {
        view.setUint16(20, 1, true);
      }
      // channel count
      view.setUint16(22, this.channels, true);
      // sample rate
      view.setUint32(24, this.sampleRate, true);
      // byte rate
      view.setUint32(28, this.sampleRate * this.channels * (this.bitsPerSample / 8), true);
      // block align
      view.setUint16(32, this.channels * (this.bitsPerSample / 8), true);
      // bits per sample
      view.setUint16(34, this.bitsPerSample, true);
      // data chunk identifier
      view.setUint32(36, 0x64617461, false); // "data"
      // data chunk length
      view.setUint32(40, pcmData.byteLength, true);

      // Merge WAV header with PCM data
      const totalLength = wavHeader.byteLength + pcmData.byteLength;
      const combinedBuffer = new ArrayBuffer(totalLength);
      const combinedView = new Uint8Array(combinedBuffer);

      combinedView.set(new Uint8Array(wavHeader), 0);
      combinedView.set(new Uint8Array(pcmData), wavHeader.byteLength);

      documentViewPicker.save(documentSaveOptions).then((documentSaveResult: Array<string>) => {
        uri = documentSaveResult[0];
        const fileName = `recording_${getDateStringWithTimeStamp(new Date().getTime(), 0)}_${this.recordSec}s.wav`;
        const testFilePath = new fileUri.FileUri(uri + `/` +`${fileName}`).path;
        let file = fs.openSync(testFilePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        const  writeLen = fs.writeSync(file.fd, combinedBuffer);
        fs.closeSync(file.fd);

        this.showToast(`音频已保存成功：\n${testFilePath}`, 3000);
      }).catch((err: BusinessError) => {
        Logger.info(TAG, `Invoke documentViewPicker.save failed, code is ${err.code}, message is ${err.message}`);
      })
      audioNapi.clearRecordBuffer();
    } catch (error) {
      Logger.info(TAG, 'Failed to save WAV file:', error);
    }
  }

  // 辅助方法：弹出 Toast 提示框
  private showToast(message: string, duration: number = 2000): void {
    this.promptAction.showToast({
      message,
      duration,
      bottom: 80,
      backgroundColor: '#FFFFFF',
      textColor: '#ff0e0c0c',
      alignment: 12
    });
  }


  //录制暂停
  capturerPause() {
    try {
      audioNapi.audioCapturerPause();
      this.recordState = 'paused';
      clearInterval(this.interval);
    } catch (err) {
      let error = err as BusinessError;
      Logger.info(TAG, `AudioRecording:audioCapturer stop err=${JSON.stringify(error)}`);
      return;
    }
  }

  //录制暂停
  @Builder
  PausedRecord() {
    Column() {
      Text(this.showTime).fontSize(21).fontWeight(500).margin({ bottom: 8 })
    }

    Row(){
      Column() {
        Image($r('app.media.ic_record_circle_2')).width(24).height(24)
          .onClick(() => {
            if (this.controller != undefined) {
              this.controller.close();
            }
            if (this.timerId !== -1) {
              clearTimeout(this.timerId);
            }
            setTimeout(() => {
              this.capturerStop();
            }, 10)
          })
        Text($r('app.string.STOP')).fontSize(12).fontWeight(400).id('pause_record_btn').margin({ top: 2 })
      }

      Column() {
        Image($r('app.media.ic_record_circle_bule')).width(24).height(24)
          .onClick(() => {
            this.capturerContinue()
          })
          .id('continue_record_btn')
        Text($r('app.string.CONTINUE')).fontSize(12).fontWeight(400).margin({ top: 2 })
      }
      .alignItems(HorizontalAlign.Center)
      .justifyContent(FlexAlign.Center)
      .margin({left: 16})
    }
  }

  //继续录制
  capturerContinue() {
    try {
      audioNapi.audioCapturerStart();
      this.recordState = 'continued';
      this.interval = setInterval(async () => {
        this.recordSec++;
        this.showTime = getTimesBySecond(this.recordSec);
      }, INTERVAL_TIME);
    } catch (err) {
      let error = err as BusinessError;
      Logger.info(TAG, `AudioRecording:audioCapturer start err=${JSON.stringify(error)}`);
    }
  }
}
