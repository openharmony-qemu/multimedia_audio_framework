/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */

import audioNapi from 'libentry.so';
import { Logger } from '../utils/Logger';
import fs from '@ohos.file.fs';
import { common } from '@kit.AbilityKit';
import { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import { SongInfo } from '../utils/importSongs/InterfaceInfo';
import { MainTitleTextModifier, saveFileBuffer } from '../utils/ExportFile';
import { taskpool } from '@kit.ArkTS';
import { SymbolGlyphModifier, TextModifier } from '@kit.ArkUI';
import { getDateStringWithTimeStamp } from '../utils/Util';

const TAG: string = 'AudioEditTestApp_Export';

@Builder
export function ExportBuilder() {
  Export()
}

class RadioParam {
  public value: string = '';
  public groupName: string = '';
  public isChecked: boolean = false;
  public label: string = '';
  public onChange: VoidCallback = () => {
  }; // Click the radio button
}

@Component
export struct Export {
  pageInfos: NavPathStack = new NavPathStack();
  scroller: Scroller = new Scroller();
  // Main Title Style Modifier
  @State mainTitleModifier: MainTitleTextModifier = new MainTitleTextModifier();
  private symbolModifier: SymbolGlyphModifier = new SymbolGlyphModifier($r('sys.symbol.chevron_backward')).fontColor([$r('app.color.index_title_text_color')])
  @StorageLink('playList') playList: Map<string, SongInfo> = new Map();
  @StorageLink('songWaveList') songWaveList: Map<string, SongInfo> = new Map();
  private newAudioFileName: string = '';
  private audioFormate: string = '.wav';
  // sampleRate
  private sampleRate: number = 8000;
  private sampleRates: number[] =
    [8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000];
  // channels
  private channels: number = 1;
  // bit depth
  private bitsPerSample: number = 16;
  private bitsPerSamples: number[] = [8, 16, 24, 32];
  // Exporting...
  @State isExported: boolean = false;

  aboutToAppear(): void {
    Logger.info(TAG, 'aboutToAppear');
  }

  build() {
    NavDestination() {
      Scroll(this.scroller) {
        Column() {
          Column() {
            Column() {
              Text('音频格式')
                .fontWeight(FontWeight.Bold)
                .fontColor($r('app.color.index_title_text_color'))
            }

            Column() {
              Row() {
                this.createRadio({
                  value: 'pcm',
                  groupName: 'radioGroupFormat',
                  isChecked: false,
                  label: 'pcm',
                  onChange: () => {
                    this.audioFormate = '.pcm';
                  }
                })
                this.createRadio({
                  value: 'wav',
                  groupName: 'radioGroupFormat',
                  isChecked: false,
                  label: 'wav',
                  onChange: () => {
                    this.audioFormate = '.wav';
                  }
                })
              }
              
            }.margin({ bottom: $r('app.float.margin_35') })

            Column() {
              Text('采样率')
                .fontWeight(FontWeight.Bold)
                .fontColor($r('app.color.index_title_text_color'))
            }

            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center,
              wrap: FlexWrap.Wrap
            }) {

              ForEach(this.sampleRates, (item: number) => {
                Row() {
                  this.createRadio({
                    value: `${item}`,
                    groupName: 'radioGroupRate',
                    isChecked: this.sampleRate === 16000 ? true : false,
                    label: `${item} Hz`,
                    onChange: () => {
                      this.sampleRate = item;
                    }
                  })
                }.width('100')
              })

            }.margin({ bottom: $r('app.float.margin_35') })

            Column() {
              Text('声道')
                .fontWeight(FontWeight.Bold)
                .fontColor($r('app.color.index_title_text_color'))
            }.margin({ bottom: $r('app.float.margin_10') })

            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center,
              wrap: FlexWrap.Wrap
            }) {
              Row() {
                this.createRadio({
                  value: '1',
                  groupName: 'radioGroupChannels',
                  isChecked: this.channels === 1 ? true : false,
                  label: '单声道',
                  onChange: () => {
                    this.channels = 1;
                  }
                })
              }

              Row() {
                this.createRadio({
                  value: '2',
                  groupName: 'radioGroupChannels',
                  isChecked: this.channels === 2 ? true : false,
                  label: '双声道',
                  onChange: () => {
                    this.channels = 2;
                  }
                })
              }
            }.margin({ bottom: $r('app.float.margin_35') })

            Column() {
              Text('位深')
                .fontWeight(FontWeight.Bold)
                .fontColor($r('app.color.index_title_text_color'))
            }.margin({ bottom: $r('app.float.margin_10') })

            Flex({
              direction: FlexDirection.Row,
              justifyContent: FlexAlign.SpaceBetween,
              alignItems: ItemAlign.Center,
              wrap: FlexWrap.Wrap
            }) {
              ForEach(this.bitsPerSamples, (item: number) => {
                Row() {
                  this.createRadio({
                    value: `${item}`,
                    groupName: 'radioGroupBitsPerSample',
                    isChecked: this.bitsPerSample === 32 ? true : false,
                    label: `${item}`,
                    onChange: () => {
                      this.bitsPerSample = item;
                    }
                  })
                }
              })
            }.margin({ bottom: $r('app.float.margin_10') })

            Column() {
              Text('输入保存的音频文件名')
                .margin({ right: '20vp' })
                .margin({ bottom: $r('app.float.margin_10') })
                .fontWeight(FontWeight.Bold)
                .fontColor($r('app.color.index_title_text_color'))
              TextInput({ text: this.newAudioFileName, placeholder: '输入保存的音频文件名' })
                .backgroundColor($r('app.color.index_title_text_color'))
                .onChange((value: string) => {
                  this.newAudioFileName = value;
                })
            }.margin({ top: $r('app.float.margin_10'), bottom: $r('app.float.margin_10') })
          }
          .alignItems(HorizontalAlign.Start)
          .margin({ top: $r('app.float.margin_35'), left: $r('app.float.margin_10'), right: $r('app.float.margin_10') })
          Column() {
            Button('保存音频格式').onClick((event: ClickEvent) => {
              Logger.info(TAG,
                `save sampleRate: ${this.sampleRate}, channels: ${this.channels}, bitsPerSample: ${this.bitsPerSample}`)
              audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, 1);
            })
              .backgroundColor($r('app.color.index_audio_clip_card'))
              .margin({bottom: $r('app.float.margin_10')})
            Button('确认导出')
              .enabled(!this.isExported)
              .backgroundColor($r('app.color.index_audio_clip_card'))
              .onClick(async () => {
                audioNapi.setFormat(this.channels, this.sampleRate, this.bitsPerSample, 1);
                this.isExported = true;
                try {
                  const value = await taskpool.execute(saveFileBuffer);
                  Logger.info(TAG, 'taskpool execute success, saving buffer...');
                  this.SaveBuffer(value as ArrayBuffer);
                  this.pageInfos.pop(this.channels);
                  this.isExported = false;
                  let tapValue: ArrayBuffer = audioNapi.getAudioOfTap();
                  if (tapValue.byteLength === 0) {
                    Logger.error(TAG, 'tapValue is blank, skipping save');
                    return;
                  }
                  Logger.info(TAG, 'Saving tap buffer...');
                  this.SaveTapBuffer(tapValue);
                  Logger.info(TAG, 'Export completed successfully.');
                } catch (error) {
                  Logger.error(TAG, `Export failed: ${error.message || error}`);
                  this.isExported = false;
                }
                Logger.info(TAG, 'onClick end');
              })

          }.alignItems(HorizontalAlign.Center)
          .padding('10vp')
        }
      }
      .edgeEffect(EdgeEffect.None)
    }
    .title('导出音频', {mainTitleModifier: this.mainTitleModifier})
    .backButtonIcon(this.symbolModifier)
    .backgroundColor($r('app.color.main_backgroundColor'))
    .onReady((context: NavDestinationContext) => {
        this.pageInfos = context.pathStack;
    })
  }

  @Builder
  createRadio(param: RadioParam) {
    Radio({
      value: param.value,
      group: param.groupName,
      indicatorType: RadioIndicatorType.DOT,
    })
      .width(20)
      .height(20)
      .margin({ right: 10 })
      .checked(param.isChecked)
      .radioStyle({
        checkedBackgroundColor: $r('app.color.index_audio_clip_card'),
      })
      .onChange((isChecked: boolean) => {
        if (isChecked) {
          param.onChange();
        }
      })
    Text(param.label).margin({ right: '20vp' }).fontSize(14).fontColor($r('app.color.index_title_text_color'))
  }

  async SaveBuffer(value: ArrayBuffer) {
    try {
      Logger.info(TAG,
        `SaveBuffer start, audioFormate: ${this.audioFormate}, sampleRate: ${this.sampleRate}, channels: ${this.channels}, bitsPerSample: ${this.bitsPerSample}`);
      let pcmBuffer = value;
      Logger.info(TAG, `SaveBuffer pcmBuffer length : ${pcmBuffer.byteLength}`);
      if (pcmBuffer.byteLength === 0) {
        Logger.info(TAG, 'SaveBuffer error');
      }
      // Picker selector, save PCM file
      let documentSaveOptions = new picker.DocumentSaveOptions();
      if (this.newAudioFileName === '') {
        this.newAudioFileName = getDateStringWithTimeStamp(new Date().getTime());
      } else {
        this.newAudioFileName = this.newAudioFileName + ' ' + getDateStringWithTimeStamp(new Date().getTime());
      }
      let newFileNames = `${this.newAudioFileName}${this.audioFormate}`;
      Logger.info(TAG, `SaveBuffer newFileNames is: ${newFileNames}`);
      documentSaveOptions.newFileNames = [newFileNames];
      let context = getContext() as common.Context; // Please ensure that getContext(this) returns a result of UIAbilityContext
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
        if (err) {
          Logger.error(TAG,
            `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
          return;
        }
        Logger.info(TAG,
          'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
        let filePath = documentSaveResult;
        // There is only one path in the documentSaveResult array
        try {
          let file = fs.openSync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
          Logger.info(TAG, `SaveBuffer filePath : ${filePath}`);
          try {
            if (this.audioFormate === '.wav') {
              // Processing WAV
              let wavBuffer = this.writeWavFileHeader(pcmBuffer);
              let writeLen = fs.writeSync(file.fd, wavBuffer);
              Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
              fs.closeSync(file);
            } else {
              // Processing PCM
              let writeLen = fs.writeSync(file.fd, pcmBuffer);
              Logger.info(TAG, `SavePcmBuffer writeLen : ${writeLen}`);
              fs.closeSync(file);
            }
          } catch (e) {
            fs.closeSync(file);
            Logger.error(TAG, `SavePcmBuffer writeLen error: ${e}`);
          }
        } catch (e) {
          Logger.error(TAG, `SaveBuffer filePath error: ${e}`);
        } 
      })
    } catch (e) {
      Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
    }
  }

  async SaveTapBuffer(value: ArrayBuffer) {
    try {
      Logger.info(TAG,
        `SaveBuffer start, audioFormate: ${this.audioFormate}, sampleRate: ${this.sampleRate}, channels: ${this.channels}, bitsPerSample: ${this.bitsPerSample}`);
      let pcmBuffer = value;
      Logger.info(TAG, `SaveBuffer pcmBuffer length : ${pcmBuffer.byteLength}`);
      if (pcmBuffer.byteLength === 0) {
        Logger.info(TAG, 'SaveBuffer error');
      }
      // Picker selector, save PCM file
      let documentSaveOptions = new picker.DocumentSaveOptions();
      let newFileNames = `tap_output${this.audioFormate}`;
      Logger.info(TAG, `SaveBuffer newFileNames is: ${newFileNames}`);
      documentSaveOptions.newFileNames = [newFileNames];
      let context = getContext() as common.Context; // Please ensure that getContext(this) returns a result of UIAbilityContext
      let documentPicker = new picker.DocumentViewPicker(context);
      documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
        if (err) {
          Logger.error(TAG,
            `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
          return;
        }
        Logger.info(TAG,
          'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
        let filePath = documentSaveResult;
        // There is only one path in the documentSaveResult array
        try {
          let file = fs.openSync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
          Logger.info(TAG, `SaveTapBuffer filePath : ${filePath}`);
          try {
            if (this.audioFormate === '.wav') {
              // Processing WAV
              let wavBuffer = this.writeWavFileHeader(pcmBuffer);
              let writeLen = fs.writeSync(file.fd, wavBuffer);
              Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
            } else {
              // Processing PCM
              let writeLen = fs.writeSync(file.fd, pcmBuffer);
              Logger.info(TAG, `SavePcmBuffer writeLen : ${writeLen}`);
            }
          } catch (e) {
            fs.closeSync(file);
            Logger.error(TAG, `SaveWavBuffer writeLen error: ${e}`);
          }
        } catch (e) {
          Logger.error(TAG, `SaveTapBuffer filePath error: ${e}`);
        }   
      })
    } catch (e) {
      Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
    }
  }

  writeString(dv: DataView, offset: number, str: string) {
    for (let i = 0; i < str.length; i++) {
      dv.setUint8(offset + i, str.charCodeAt(i));
    }
  }

  concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
    const viewWavHeader = new Uint8Array(wavHeaderBuffer);
    const viewPcm = new Uint8Array(pcmBuffer);

    // Create a new ArrayBuffer with a size equal to the sum of the two raw buffers
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);

    // Copy the data from two views into a new view
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);

    return resultBuffer;
  }

  // Define the method for writing WAV header file information
  writeWavFileHeader(pcmBuffer: ArrayBuffer): ArrayBuffer {
    Logger.info(TAG,
      `writeWavFileHeader sampleRate : ${this.sampleRate} --- channels : ${this.channels} --- bitsPerSample : ${this.bitsPerSample}`);
    const header = new ArrayBuffer(44);
    const dv = new DataView(header);
    // Writing RIFF Blocks
    this.writeString(dv, 0, 'RIFF');
    let totalDataLen = pcmBuffer.byteLength + 44;
    dv.setUint32(4, totalDataLen, true);
    this.writeString(dv, 8, 'WAVE');
    // writing fmt block
    this.writeString(dv, 12, 'fmt ');
    // fmt block size
    dv.setUint32(16, 16, true);
    // Format category (1 - int, 3 - IEEE floating point)
    if (this.bitsPerSample === 32) {
      dv.setUint16(20, 3, true);
    } else {
      dv.setUint16(20, 1, true);
    }

    // Number of audio channels
    dv.setUint16(22, this.channels, true);
    // sampling fraction
    dv.setUint32(24, this.sampleRate, true);
    // bit rate
    let byteRate = this.bitsPerSample * this.sampleRate * this.channels / 8;
    dv.setUint32(28, byteRate, true);
    // Number of bytes per sampling point
    dv.setUint16(32, this.channels * this.bitsPerSample / 8, true);
    // bit depth
    dv.setUint16(34, this.bitsPerSample, true);
    // writing data block
    this.writeString(dv, 36, 'data');
    // data block size
    dv.setUint32(40, pcmBuffer.byteLength, true);

    let resultBuffer = this.concatArrayBuffer(header, pcmBuffer);
    return resultBuffer;
  }
}
