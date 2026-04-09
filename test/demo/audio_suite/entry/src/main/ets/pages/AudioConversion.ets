/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. 2026-2026. All rights reserved.
 */


import { Logger } from '../utils/Logger';
import audioNapi from 'libentry.so';
import { common } from '@kit.AbilityKit';
import { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import fs from '@ohos.file.fs';
import { fileUri } from '@kit.CoreFileKit';
import { taskpool } from '@kit.ArkTS';

const TAG: string = 'AudioConversion';

// Sample format enum
enum AudioSampleFormat {
  AUDIO_SAMPLE_U8 = 0,
  AUDIO_SAMPLE_S16LE = 1,
  AUDIO_SAMPLE_S24LE = 2,
  AUDIO_SAMPLE_S32LE = 3,
  AUDIO_SAMPLE_F32LE = 4
}

// Sample rate enum
enum AudioSampleRate {
  AUDIO_SAMPLE_RATE_8000 = 8000,
  AUDIO_SAMPLE_RATE_11025 = 11025,
  AUDIO_SAMPLE_RATE_12000 = 12000,
  AUDIO_SAMPLE_RATE_16000 = 16000,
  AUDIO_SAMPLE_RATE_22050 = 22050,
  AUDIO_SAMPLE_RATE_24000 = 24000,
  AUDIO_SAMPLE_RATE_32000 = 32000,
  AUDIO_SAMPLE_RATE_44100 = 44100,
  AUDIO_SAMPLE_RATE_48000 = 48000,
  AUDIO_SAMPLE_RATE_64000 = 64000,
  AUDIO_SAMPLE_RATE_88200 = 88200,
  AUDIO_SAMPLE_RATE_96000 = 96000,
  AUDIO_SAMPLE_RATE_176400 = 176400,
  AUDIO_SAMPLE_RATE_192000 = 192000
}

// Encoding type enum
enum AudioEncodingType {
  AUDIO_ENCODING_TYPE_RAW = 0
}

// Channel layout enum
enum AudioChannelLayout {
  CH_LAYOUT_MONO = 4,
  CH_LAYOUT_STEREO = 3,
  CH_LAYOUT_STEREO_DOWNMIX = 1610612736,
  CH_LAYOUT_2POINT1 = 11,
  CH_LAYOUT_3POINT0 = 259,
  CH_LAYOUT_SURROUND = 7,
  CH_LAYOUT_3POINT1 = 15,
  CH_LAYOUT_4POINT0 = 263,
  CH_LAYOUT_QUAD_SIDE = 1539,
  CH_LAYOUT_QUAD = 51,
  CH_LAYOUT_2POINT0POINT2 = 206158430211,
  CH_LAYOUT_4POINT1 = 271,
  CH_LAYOUT_5POINT0 = 1543,
  CH_LAYOUT_5POINT0_BACK = 55,
  CH_LAYOUT_2POINT1POINT2 = 206158430219,
  CH_LAYOUT_3POINT0POINT2 = 206158430215,
  CH_LAYOUT_5POINT1 = 1551,
  CH_LAYOUT_5POINT1_BACK = 63,
  CH_LAYOUT_6POINT0 = 1799,
  CH_LAYOUT_3POINT1POINT2 = 20495,
  CH_LAYOUT_6POINT0_FRONT = 1731,
  CH_LAYOUT_HEXAGONAL = 311,
  CH_LAYOUT_6POINT1 = 1807,
  CH_LAYOUT_6POINT1_BACK = 319,
  CH_LAYOUT_6POINT1_FRONT = 1739,
  CH_LAYOUT_7POINT0 = 1591,
  CH_LAYOUT_7POINT0_FRONT = 1735,
  CH_LAYOUT_7POINT1 = 1599,
  CH_LAYOUT_OCTAGONAL = 1847,
  CH_LAYOUT_5POINT1POINT2 = 206158431759,
  CH_LAYOUT_7POINT1_WIDE = 255,
  CH_LAYOUT_7POINT1_WIDE_BACK = 255
}

// Bits per sample constants
const BITS_PER_SAMPLE_U8 = 8;
const BITS_PER_SAMPLE_S16LE = 16;
const BITS_PER_SAMPLE_S24LE = 24;
const BITS_PER_SAMPLE_S32LE = 32;

// WAV file format constants
const WAV_HEADER_SIZE = 44;
const WAV_DATA_OFFSET = 36;
const WAV_FORMAT_CHUNK_SIZE = 16;

// WAV header offset constants
const WAV_OFFSET_FILE_SIZE = 4;
const WAV_OFFSET_FORMAT_ID = 8;
const WAV_OFFSET_FORMAT_CHUNK_ID = 12;
const WAV_OFFSET_FORMAT_CHUNK_SIZE = 16;
const WAV_OFFSET_AUDIO_FORMAT = 20;
const WAV_OFFSET_NUM_CHANNELS = 22;
const WAV_OFFSET_SAMPLE_RATE = 24;
const WAV_OFFSET_BYTE_RATE = 28;
const WAV_OFFSET_BLOCK_ALIGN = 32;
const WAV_OFFSET_BITS_PER_SAMPLE = 34;
const WAV_OFFSET_DATA_CHUNK_ID = 36;
const WAV_OFFSET_DATA_SIZE = 40;

// WAV audio format constants
const WAV_AUDIO_FORMAT_PCM = 1;
const WAV_AUDIO_FORMAT_IEEE_FLOAT = 3;

// Channel count limits
const MIN_CHANNELS = 1;
const MAX_CHANNELS = 8;

// Audio converter format
interface AudioConverterFormat {
  encodingType: number;
  samplingRate: number;
  channelLayout: number;
  sampleFormat: number;
}

@Builder
function AudioConversionBuilder() {
  AudioConversion()
}

export { AudioConversionBuilder };

@Concurrent
export async function audioConverterProcess(converterHandle: number): Promise<ArrayBuffer> {
  let result = audioNapi.audioConverterProcess(converterHandle);
  return result;
}

@Component
export struct AudioConversion {
  pageInfos: NavPathStack = new NavPathStack();
  @State message: string = '音频格式转换';
  @State converterHandle: number = 0;
  @State createResult: string = '';
  @State processResult: string = '';
  @State selectedFileUri: string = '';
  @State selectedFileName: string = '';
  @State fileInfo: audioNapi.AudioFileInfo | null = null;
  @State fileFd: number = -1;
  @State outputFileName: string = '';
  @State inputFormat: AudioConverterFormat = {
    encodingType: AudioEncodingType.AUDIO_ENCODING_TYPE_RAW,
    samplingRate: AudioSampleRate.AUDIO_SAMPLE_RATE_44100,
    channelLayout: 3,
    sampleFormat: AudioSampleFormat.AUDIO_SAMPLE_S16LE
  };
  @State outputFormat: AudioConverterFormat = {
    encodingType: AudioEncodingType.AUDIO_ENCODING_TYPE_RAW,
    samplingRate: AudioSampleRate.AUDIO_SAMPLE_RATE_48000,
    channelLayout: 3,
    sampleFormat: AudioSampleFormat.AUDIO_SAMPLE_S16LE
  };
  @State isConverting: boolean = false;
  private context = getContext(this) as common.UIAbilityContext;

  @Builder
  buildHeaderSection() {
    Column() {
      Text(this.message)
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 10 })

      Button('选择音频文件')
        .width('90%')
        .height(40)
        .margin({ top: 5 })
        .onClick(() => {
          this.selectAudioFile();
        })

      Text(this.selectedFileName)
        .fontSize(12)
        .fontColor(Color.Yellow)
        .margin({ top: 3, bottom: 5 })
    }
    .width('90%')
  }

  @Builder
  buildInputFormatSection() {
    if (this.fileInfo) {
      Column() {
        Text('输入格式（自动解析）')
          .fontSize(14)
          .fontWeight(FontWeight.Bold)
          .margin({ bottom: 5 })
          .fontColor(Color.Green)

        Row() {
          Text(`采样率: ${this.inputFormat.samplingRate}Hz`)
            .fontSize(12)
            .fontColor(Color.Gray)
          Blank()
          Text(`声道: ${this.fileInfo?.numChannels ?? 0}`)
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
        .margin({ bottom: 3 })

        Row() {
          Text(`采样格式: ${this.getSampleFormatName(this.inputFormat.sampleFormat)}`)
            .fontSize(12)
            .fontColor(Color.Gray)
        }
        .width('100%')
      }
      .width('90%')
      .padding(10)
      .backgroundColor('#1A1A1A')
      .borderRadius(8)
      .margin({ top: 5, bottom: 5 })
    }
  }

  @Builder
  buildOutputConfigSection() {
    Column() {
      this.FormatConfigSection('输出格式', this.outputFormat, (newFormat: AudioConverterFormat) => {
        this.outputFormat = {
          encodingType: newFormat.encodingType,
          samplingRate: newFormat.samplingRate,
          channelLayout: newFormat.channelLayout,
          sampleFormat: newFormat.sampleFormat
        };
      }, false)
    }
    .width('90%')
    .margin({ top: 5, bottom: 10 })
  }

  @Builder
  buildConversionControls() {
    if (this.createResult) {
      Text(this.createResult)
        .fontSize(12)
        .fontColor(Color.Yellow)
        .margin({ top: 3, bottom: 3 })
    }

    if (!this.isConverting) {
      Button('开始转换')
        .width('90%')
        .height(40)
        .margin({ top: 5 })
        .enabled(this.selectedFileUri !== '' && this.fileInfo !== null)
        .onClick(() => {
          this.startConversion();
        })

      if (this.processResult) {
        Text(this.processResult)
          .fontSize(12)
          .fontColor(Color.Yellow)
          .margin({ top: 3, bottom: 3 })
      }
    } else {
      Text('转换中...')
        .fontSize(18)
        .fontColor(Color.Green)
        .margin({ top: 20, bottom: 20 })
    }
  }

  build() {
    NavDestination() {
      Scroll() {
        Column() {
          this.buildHeaderSection()
          this.buildInputFormatSection()
          this.buildOutputConfigSection()
          this.buildConversionControls()
        }
        .width('100%')
        .padding({ left: 10, right: 10 })
      }
      .width('100%')
      .height('100%')
      .backgroundColor('#8161c4')
    }
    .onReady((context: NavDestinationContext) => {
      this.pageInfos = context.pathStack;
    })
    .title('音频格式转换')
    .height('100%')
    .width('100%')
  }

  @Builder
  buildSampleRateSelector(isInput: boolean, onChange: (format: AudioConverterFormat) => void) {
    Row() {
      Text('采样率:')
        .width('30%')
      Select([
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_8000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_11025.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_12000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_16000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_22050.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_24000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_32000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_44100.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_48000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_64000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_88200.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_96000.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_176400.toString() },
        { value: AudioSampleRate.AUDIO_SAMPLE_RATE_192000.toString() }
      ])
        .selected(this.getSampleRateIndex(isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate))
        .onSelect((index: number) => {
          const rates = [8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000,
            88200, 96000, 176400, 192000];
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'} samplingRate selected: index=${index}, ` +
              `value=${rates[index]}`);
          let newFormat: AudioConverterFormat = {
            encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
            samplingRate: rates[index],
            channelLayout: isInput ? this.inputFormat.channelLayout : this.outputFormat.channelLayout,
            sampleFormat: isInput ? this.inputFormat.sampleFormat : this.outputFormat.sampleFormat
          };
          onChange(newFormat);
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);
        })
    }
    .width('100%')
    .margin({ bottom: 5 })
  }

  @Builder
  buildChannelLayoutSelector(isInput: boolean, onChange: (format: AudioConverterFormat) => void) {
    Row() {
      Text('声道布局:')
        .width('30%')
      Select([
        { value: 'MONO' },
        { value: 'STEREO' },
        { value: 'STEREO_DOWNMIX' },
        { value: '2POINT1' },
        { value: '3POINT0' },
        { value: 'SURROUND' },
        { value: '3POINT1' },
        { value: '4POINT0' },
        { value: 'QUAD_SIDE' },
        { value: 'QUAD' },
        { value: '2POINT0POINT2' },
        { value: '4POINT1' },
        { value: '5POINT0' },
        { value: '5POINT0_BACK' },
        { value: '2POINT1POINT2' },
        { value: '3POINT0POINT2' },
        { value: '5POINT1' },
        { value: '5POINT1_BACK' },
        { value: '6POINT0' },
        { value: '3POINT1POINT2' },
        { value: '6POINT0_FRONT' },
        { value: 'HEXAGONAL' },
        { value: '6POINT1' },
        { value: '6POINT1_BACK' },
        { value: '6POINT1_FRONT' },
        { value: '7POINT0' },
        { value: '7POINT0_FRONT' },
        { value: '7POINT1' },
        { value: 'OCTAGONAL' },
        { value: '5POINT1POINT2' },
        { value: '7POINT1_WIDE' },
        { value: '7POINT1_WIDE_BACK' }
      ])
        .selected(this.getChannelLayoutIndex(isInput ? this.inputFormat.channelLayout :
          this.outputFormat.channelLayout))
        .onSelect((index: number) => {
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'} channelLayout selected: index=${index}, ` +
              `value=${[4, 3, 1610612736, 11, 259, 7, 15, 263, 1539, 51, 206158430211, 271, 1543, 55,
                206158430219, 206158430215, 1551, 63, 1799, 20495, 1731, 311, 1807, 319, 1739, 1591, 1735, 1599, 1847,
                206158431759, 255, 255][index]}`);

          const layouts = [4, 3, 1610612736, 11, 259, 7, 15, 263, 1539, 51, 206158430211, 271, 1543, 55,
            206158430219, 206158430215, 1551, 63, 1799, 20495, 1731, 311, 1807, 319, 1739, 1591, 1735, 1599, 1847,
            206158431759, 255, 255];
          let newFormat: AudioConverterFormat = {
            encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
            samplingRate: isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate,
            channelLayout: layouts[index],
            sampleFormat: isInput ? this.inputFormat.sampleFormat : this.outputFormat.sampleFormat
          };
          onChange(newFormat);
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);
        })
    }
    .width('100%')
    .margin({ bottom: 5 })
  }

  @Builder
  buildSampleFormatSelector(isInput: boolean, onChange: (format: AudioConverterFormat) => void) {
    Row() {
      Text('采样格式:')
        .width('30%')
      Select([
        { value: 'U8' },
        { value: 'S16LE' },
        { value: 'S24LE' },
        { value: 'S32LE' },
        { value: 'F32LE' }
      ])
        .selected(this.getSampleFormatIndex(isInput ? this.inputFormat.sampleFormat : this.outputFormat.sampleFormat))
        .onSelect((index: number) => {
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'} sampleFormat selected: index=${index}, ` +
              `value=${[AudioSampleFormat.AUDIO_SAMPLE_U8, AudioSampleFormat.AUDIO_SAMPLE_S16LE,
                AudioSampleFormat.AUDIO_SAMPLE_S24LE, AudioSampleFormat.AUDIO_SAMPLE_S32LE,
                AudioSampleFormat.AUDIO_SAMPLE_F32LE][index]}`);

          const sampleFormats = [
            AudioSampleFormat.AUDIO_SAMPLE_U8,
            AudioSampleFormat.AUDIO_SAMPLE_S16LE,
            AudioSampleFormat.AUDIO_SAMPLE_S24LE,
            AudioSampleFormat.AUDIO_SAMPLE_S32LE,
            AudioSampleFormat.AUDIO_SAMPLE_F32LE
          ];
          let newFormat: AudioConverterFormat = {
            encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
            samplingRate: isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate,
            channelLayout: isInput ? this.inputFormat.channelLayout : this.outputFormat.channelLayout,
            sampleFormat: sampleFormats[index]
          };
          onChange(newFormat);
          Logger.info(TAG,
            `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);
        })
    }
    .width('100%')
    .margin({ bottom: 5 })
  }

  @Builder
  FormatConfigSection(title: string, format: AudioConverterFormat, onChange: (format: AudioConverterFormat) => void,
    isInput: boolean)
  {
    Column() {
      Text(title)
        .fontSize(16)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 })

      this.buildSampleRateSelector(isInput, onChange)
      this.buildChannelLayoutSelector(isInput, onChange)
      this.buildSampleFormatSelector(isInput, onChange)
    }
    .width('100%')
    .padding(8)
    .backgroundColor(0x33000000)
    .borderRadius(8)
  }

  getSampleRateIndex(rate: number): number {
    const rates = [
      AudioSampleRate.AUDIO_SAMPLE_RATE_8000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_11025,
      AudioSampleRate.AUDIO_SAMPLE_RATE_12000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_16000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_22050,
      AudioSampleRate.AUDIO_SAMPLE_RATE_24000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_32000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_44100,
      AudioSampleRate.AUDIO_SAMPLE_RATE_48000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_64000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_88200,
      AudioSampleRate.AUDIO_SAMPLE_RATE_96000,
      AudioSampleRate.AUDIO_SAMPLE_RATE_176400,
      AudioSampleRate.AUDIO_SAMPLE_RATE_192000];
    return rates.indexOf(rate);
  }

  getChannelLayoutIndex(layout: number): number {
    const layouts = [
      AudioChannelLayout.CH_LAYOUT_MONO,
      AudioChannelLayout.CH_LAYOUT_STEREO,
      AudioChannelLayout.CH_LAYOUT_STEREO_DOWNMIX,
      AudioChannelLayout.CH_LAYOUT_2POINT1,
      AudioChannelLayout.CH_LAYOUT_3POINT0,
      AudioChannelLayout.CH_LAYOUT_SURROUND,
      AudioChannelLayout.CH_LAYOUT_3POINT1,
      AudioChannelLayout.CH_LAYOUT_4POINT0,
      AudioChannelLayout.CH_LAYOUT_QUAD_SIDE,
      AudioChannelLayout.CH_LAYOUT_QUAD,
      AudioChannelLayout.CH_LAYOUT_2POINT0POINT2,
      AudioChannelLayout.CH_LAYOUT_4POINT1,
      AudioChannelLayout.CH_LAYOUT_5POINT0,
      AudioChannelLayout.CH_LAYOUT_5POINT0_BACK,
      AudioChannelLayout.CH_LAYOUT_2POINT1POINT2,
      AudioChannelLayout.CH_LAYOUT_3POINT0POINT2,
      AudioChannelLayout.CH_LAYOUT_5POINT1,
      AudioChannelLayout.CH_LAYOUT_5POINT1_BACK,
      AudioChannelLayout.CH_LAYOUT_6POINT0,
      AudioChannelLayout.CH_LAYOUT_3POINT1POINT2,
      AudioChannelLayout.CH_LAYOUT_6POINT0_FRONT,
      AudioChannelLayout.CH_LAYOUT_HEXAGONAL,
      AudioChannelLayout.CH_LAYOUT_6POINT1,
      AudioChannelLayout.CH_LAYOUT_6POINT1_BACK,
      AudioChannelLayout.CH_LAYOUT_6POINT1_FRONT,
      AudioChannelLayout.CH_LAYOUT_7POINT0,
      AudioChannelLayout.CH_LAYOUT_7POINT0_FRONT,
      AudioChannelLayout.CH_LAYOUT_7POINT1,
      AudioChannelLayout.CH_LAYOUT_OCTAGONAL,
      AudioChannelLayout.CH_LAYOUT_5POINT1POINT2,
      AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE,
      AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE_BACK];
    return layouts.indexOf(layout);
  }

  getSampleFormatIndex(sampleFormat: number): number {
    const sampleFormats =
      [AudioSampleFormat.AUDIO_SAMPLE_U8, AudioSampleFormat.AUDIO_SAMPLE_S16LE, AudioSampleFormat.AUDIO_SAMPLE_S24LE,
        AudioSampleFormat.AUDIO_SAMPLE_S32LE, AudioSampleFormat.AUDIO_SAMPLE_F32LE];
    return sampleFormats.indexOf(sampleFormat);
  }


  selectAudioFile(): void {
    try {
      let documentPicker = new picker.DocumentViewPicker(this.context);
      documentPicker.select((err: ESObject, documentSelectResult: Array<string>) => {
        if (err) {
          Logger.error(TAG, `selectAudioFile error: ${JSON.stringify(err)}`);
          return;
        }
        if (documentSelectResult && documentSelectResult.length > 0) {
          this.selectedFileUri = documentSelectResult[0];
          const uri = new fileUri.FileUri(this.selectedFileUri);
          this.selectedFileName = uri.name || 'Selected File';
          this.parseAudioFileFormat();
        }
      });
    } catch (e) {
      Logger.error(TAG, `selectAudioFile error: ${JSON.stringify(e)}`);
    }
  }

  

  async parseAudioFileFormat(): Promise<void> {
    if (!this.selectedFileUri) {
      Logger.error(TAG, 'No file selected');
      return;
    }

    const file = fs.openSync(this.selectedFileUri, fs.OpenMode.READ_ONLY);
    this.fileFd = file.fd;

    try {
      const fileInfo = audioNapi.audioConverterGetFileInfo(this.fileFd);

      if (fileInfo) {
        this.fileInfo = fileInfo;
        this.inputFormat = {
          encodingType: fileInfo.encodingType,
          samplingRate: fileInfo.samplingRate,
          channelLayout: fileInfo.channelLayout,
          sampleFormat: fileInfo.sampleFormat
        };

        Logger.info(TAG, `Parsed input format: encodingType=${fileInfo.encodingType}, ` +
          `samplingRate=${fileInfo.samplingRate}, channelLayout=${fileInfo.channelLayout}, ` +
          `sampleFormat=${fileInfo.sampleFormat}, numChannels=${fileInfo.numChannels}`);

        this.createResult =
          `Auto parsed: ${fileInfo.samplingRate}Hz, ` +
            `${fileInfo.numChannels} channels, ` +
            `${this.getSampleFormatName(fileInfo.sampleFormat)}`;
      } else {
        Logger.error(TAG, 'Failed to parse audio file');
        this.createResult = 'Failed to parse audio file format';
      }
    } catch (e) {
      Logger.error(TAG, `parseAudioFileFormat error: ${JSON.stringify(e)}`);
      this.createResult = 'Error parsing audio file format';
    } finally {
      fs.closeSync(file.fd);
    }
  }

  getChannelCount(channelLayout: number): number {
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_MONO) {
      return 1;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_STEREO ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_STEREO_DOWNMIX) {
      return 2;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_2POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_3POINT0 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_SURROUND) {
      return 3;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_2POINT0POINT2 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_3POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_4POINT0 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_QUAD ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_QUAD_SIDE) {
      return 4;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_2POINT1POINT2 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_4POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_5POINT0 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_5POINT0_BACK ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_3POINT0POINT2) {
      return 5;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_5POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_5POINT1_BACK ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_6POINT0 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_HEXAGONAL ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_6POINT0_FRONT ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_3POINT1POINT2) {
      return 6;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_6POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_6POINT1_BACK ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_6POINT1_FRONT ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_7POINT0 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_7POINT0_FRONT) {
      return 7;
    }
    if (channelLayout === AudioChannelLayout.CH_LAYOUT_5POINT1POINT2 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_7POINT1 ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE_BACK ||
      channelLayout === AudioChannelLayout.CH_LAYOUT_OCTAGONAL) {
      return 8;
    }
    return 2;
  }

  getSampleFormatName(sampleFormat: number): string {
    switch (sampleFormat) {
      case AudioSampleFormat.AUDIO_SAMPLE_U8:
        return 'U8';
      case AudioSampleFormat.AUDIO_SAMPLE_S16LE:
        return 'S16LE';
      case AudioSampleFormat.AUDIO_SAMPLE_S24LE:
        return 'S24LE';
      case AudioSampleFormat.AUDIO_SAMPLE_S32LE:
        return 'S32LE';
      case AudioSampleFormat.AUDIO_SAMPLE_F32LE:
        return 'F32LE';
      default:
        return 'Unknown';
    }
  }

  private async createConverter(): Promise<boolean> {
    let path: string = new fileUri.FileUri(this.selectedFileUri).path;
    let file = fs.openSync(path, fs.OpenMode.READ_ONLY);

    try {
      const result = audioNapi.audioConverterCreate(
        this.inputFormat, this.outputFormat, file.fd);

      if (result <= 0) {
        this.createResult = `Create converter failed, error code: ${result}`;
        this.isConverting = false;
        Logger.error(TAG, `AudioConverterCreate failed: ${result}`);
        return false;
      }

      this.converterHandle = result;
      return true;
    } finally {
      fs.closeSync(file.fd);
    }
  }

  private async processConversion(): Promise<void> {
    this.processResult = 'Converting...';
    const convertedData = await taskpool.execute(
      audioConverterProcess, this.converterHandle);

    if (convertedData) {
      const data = convertedData as ArrayBuffer;
      this.processResult = `Conversion complete, output size: ${data.byteLength} bytes`;
      await this.saveToOutputPath(data);
    } else {
      this.processResult = 'Conversion failed';
      Logger.error(TAG, 'AudioConverterProcess failed');
    }
  }

  private async saveToOutputPath(data: ArrayBuffer): Promise<void> {
    try {
      if (!data || data.byteLength === 0) {
        Logger.error(TAG, 'No converted data to save');
        this.processResult = '保存失败: 没有转换数据';
        return;
      }

      Logger.info(TAG, `Converted data size: ${data.byteLength} bytes`);

      const channels = this.getChannelCount(this.outputFormat.channelLayout);
      const bitsPerSample = this.getBitsPerSample(this.outputFormat.sampleFormat);
      const sampleRate = this.outputFormat.samplingRate;

      Logger.info(TAG, `WAV params: sampleRate=${sampleRate}, channels=${channels}, bitsPerSample=${bitsPerSample}`);

      const wavFileData = this.writeWavFileHeaderCorrected(
        data, sampleRate, channels, bitsPerSample, this.outputFormat.sampleFormat
      );

      Logger.info(TAG, `WAV file size: ${wavFileData.byteLength} bytes`);

      // 使用 DocumentViewPicker 让用户选择保存位置
      const fileName = `converted_audio_${Date.now()}.wav`;
      let documentSaveOptions = new picker.DocumentSaveOptions();
      documentSaveOptions.newFileNames = [fileName];

      let context = getContext() as common.Context;
      let documentPicker = new picker.DocumentViewPicker(context);

      documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
        if (err) {
          Logger.error(TAG, `DocumentViewPicker.save failed: ${err.code}, ${err.message}`);
          this.processResult = `保存失败: ${err.message}`;
          return;
        }

        if (documentSaveResult && documentSaveResult.length > 0) {
          const outputPath = documentSaveResult[0];
          Logger.info(TAG, `Output path: ${outputPath}`);

          try {
            let file = fs.openSync(outputPath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
            const writeLen = fs.writeSync(file.fd, wavFileData);

            // 验证写入的字节数是否正确
            if (writeLen !== wavFileData.byteLength) {
              Logger.error(TAG, `Write incomplete: expected ${wavFileData.byteLength} bytes, wrote ${writeLen} bytes`);
              fs.closeSync(file.fd);
              this.processResult = `保存失败: 写入不完整 (${writeLen}/${wavFileData.byteLength} 字节)`;
              return;
            }

            // 同步数据到磁盘
            fs.fsyncSync(file.fd);
            fs.closeSync(file.fd);

            // 验证文件是否真的被创建
            try {
              const stat = fs.statSync(outputPath);
              Logger.info(TAG, `File verified: size=${stat.size} bytes at ${outputPath}`);
            } catch (statError) {
              Logger.error(TAG, `File verification failed: ${JSON.stringify(statError)}`);
              this.processResult = `保存失败: 文件验证失败`;
              return;
            }

            Logger.info(TAG, `Save file success: ${writeLen} bytes to ${outputPath}`);

            // 显示完整保存路径
            this.outputFileName = fileName;
            this.processResult = `转换成功！\n文件已保存到:\n${outputPath}`;
          } catch (error) {
            Logger.error(TAG, `Save file failed: ${JSON.stringify(error)}`);
            this.processResult = `保存文件失败: ${error}`;
          }
        }
      });
    } catch (error) {
      Logger.error(TAG, `saveToOutputPath failed: ${JSON.stringify(error)}`);
      this.processResult = `保存文件失败: ${error}`;
    }
  }

  private destroyConverter(): void {
    audioNapi.audioConverterDestroy(this.converterHandle);
    this.converterHandle = 0;
  }

  private cleanupOnError(): void {
    this.createResult = `Conversion failed`;
    this.processResult = `Conversion failed`;
    this.isConverting = false;

    if (this.converterHandle !== 0) {
      try {
        audioNapi.audioConverterDestroy(this.converterHandle);
        this.converterHandle = 0;
      } catch (destroyError) {
        Logger.error(TAG, `Failed to destroy converter: ${JSON.stringify(destroyError)}`);
      }
    }
  }

  async startConversion(): Promise<void> {
    this.isConverting = true;
    this.createResult = 'Converting...';

    try {
      if (this.selectedFileUri === '') {
        this.createResult = 'Please select audio file first';
        this.isConverting = false;
        return;
      }

      if (!await this.createConverter()) {
        return;
      }

      audioNapi.audioConverterSetInputCallback(this.converterHandle);

      await this.processConversion();
      this.destroyConverter();

      this.createResult = 'Conversion completed';
      this.isConverting = false;

    } catch (e) {
      Logger.error(TAG, `startConversion error: ${JSON.stringify(e)}`);
      this.cleanupOnError();
    }
  }

  // Auxiliary method: get bit depth based on sample format
  private getBitsPerSample(sampleFormat: number): number {
    switch (sampleFormat) {
      case AudioSampleFormat.AUDIO_SAMPLE_U8:
        return BITS_PER_SAMPLE_U8;
      case AudioSampleFormat.AUDIO_SAMPLE_S16LE:
        return BITS_PER_SAMPLE_S16LE;
      case AudioSampleFormat.AUDIO_SAMPLE_S24LE:
        return BITS_PER_SAMPLE_S24LE;
      case AudioSampleFormat.AUDIO_SAMPLE_S32LE:
      case AudioSampleFormat.AUDIO_SAMPLE_F32LE:
        return BITS_PER_SAMPLE_S32LE;
      default:
        return BITS_PER_SAMPLE_S16LE;
    }
  }

  private validateWavParameters(channels: number, sampleRate: number, bitsPerSample: number): void {
    if (channels < MIN_CHANNELS || channels > MAX_CHANNELS) {
      Logger.error(TAG, `unknown channel: ${channels}, WAV Supports ${MIN_CHANNELS}~${MAX_CHANNELS} channels.`);
      throw new Error(`Invalid channel count: ${channels}`);
    }

    if (sampleRate <= 0) {
      Logger.error(TAG, `unknown sampleRate: ${sampleRate}`);
      throw new Error(`Invalid sample rate: ${sampleRate}`);
    }

    if (bitsPerSample !== BITS_PER_SAMPLE_U8 && bitsPerSample !== BITS_PER_SAMPLE_S16LE &&
        bitsPerSample !== BITS_PER_SAMPLE_S24LE && bitsPerSample !== BITS_PER_SAMPLE_S32LE) {
      Logger.error(TAG, `unknown bitsPerSample: ${bitsPerSample}, Supports 8,16,24,32-bit.`);
      throw new Error(`Unsupported bits per sample: ${bitsPerSample}`);
    }
  }

  private createWavHeader(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number,
    bitsPerSample: number, sampleFormat: number): ArrayBuffer {
    const header = new ArrayBuffer(WAV_HEADER_SIZE);
    const dv = new DataView(header);

    const writeString = (dv: DataView, offset: number, string: string) => {
      for (let i = 0; i < string.length; i++) {
        dv.setUint8(offset + i, string.charCodeAt(i));
      }
    };

    writeString(dv, 0, 'RIFF');
    const totalDataLen = pcmBuffer.byteLength + WAV_DATA_OFFSET;
    dv.setUint32(WAV_OFFSET_FILE_SIZE, totalDataLen, true);
    writeString(dv, WAV_OFFSET_FORMAT_ID, 'WAVE');

    writeString(dv, WAV_OFFSET_FORMAT_CHUNK_ID, 'fmt ');
    dv.setUint32(WAV_OFFSET_FORMAT_CHUNK_SIZE, WAV_FORMAT_CHUNK_SIZE, true);

    if (bitsPerSample === BITS_PER_SAMPLE_S32LE && sampleFormat === AudioSampleFormat.AUDIO_SAMPLE_F32LE) {
      dv.setUint16(WAV_OFFSET_AUDIO_FORMAT, WAV_AUDIO_FORMAT_IEEE_FLOAT, true);
    } else {
      dv.setUint16(WAV_OFFSET_AUDIO_FORMAT, WAV_AUDIO_FORMAT_PCM, true);
    }

    dv.setUint16(WAV_OFFSET_NUM_CHANNELS, channels, true);
    dv.setUint32(WAV_OFFSET_SAMPLE_RATE, sampleRate, true);

    const byteRate = sampleRate * channels * (bitsPerSample / BITS_PER_SAMPLE_S16LE);
    dv.setUint32(WAV_OFFSET_BYTE_RATE, byteRate, true);

    const blockAlign = channels * (bitsPerSample / BITS_PER_SAMPLE_S16LE);
    dv.setUint16(WAV_OFFSET_BLOCK_ALIGN, blockAlign, true);
    dv.setUint16(WAV_OFFSET_BITS_PER_SAMPLE, bitsPerSample, true);

    writeString(dv, WAV_OFFSET_DATA_CHUNK_ID, 'data');
    dv.setUint32(WAV_OFFSET_DATA_SIZE, pcmBuffer.byteLength, true);

    return header;
  }

  private mergeHeaderAndData(header: ArrayBuffer, pcmBuffer: ArrayBuffer): ArrayBuffer {
    const viewWavHeader = new Uint8Array(header);
    const viewPcm = new Uint8Array(pcmBuffer);
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);
    return resultBuffer;
  }

  private writeWavFileHeaderCorrected(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number,
    bitsPerSample: number, sampleFormat: number): ArrayBuffer {
    this.validateWavParameters(channels, sampleRate, bitsPerSample);
    const header = this.createWavHeader(pcmBuffer, sampleRate, channels, bitsPerSample, sampleFormat);
    return this.mergeHeaderAndData(header, pcmBuffer);
  }
}
