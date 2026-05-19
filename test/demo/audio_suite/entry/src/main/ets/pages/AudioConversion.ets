/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. 2026-2026. All rights reserved.
 */


import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import audioNapi from 'libentry.so';
import { common } from '@kit.AbilityKit';
import { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import fs from '@ohos.file.fs';
import { fileUri } from '@kit.CoreFileKit';
import { taskpool } from '@kit.ArkTS';

const TAG: string = 'AudioConversion';

// Concurrent function for background audio conversion
@Concurrent
function convertAudioInBackground(
  inputEncodingType: number,
  inputSamplingRate: number,
  inputChannelLayout: number,
  inputSampleFormat: number,
  outputEncodingType: number,
  outputSamplingRate: number,
  outputChannelLayout: number,
  outputSampleFormat: number,
  fd: number
): ESObject {
  let inputFormat: ESObject = {
    encodingType: inputEncodingType,
    samplingRate: inputSamplingRate,
    channelLayout: inputChannelLayout,
    sampleFormat: inputSampleFormat
  };
  let outputFormat: ESObject = {
    encodingType: outputEncodingType,
    samplingRate: outputSamplingRate,
    channelLayout: outputChannelLayout,
    sampleFormat: outputSampleFormat
  };
  return audioNapi.audioFormatConverter(inputFormat, outputFormat, fd);
}

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

// Audio file info interface
interface AudioFileInfo {
  encodingType: number;
  samplingRate: number;
  channelLayout: number;
  sampleFormat: number;
  numChannels: number;
  fileSize: number;
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

// WAV file constants
const WAV_HEADER_SIZE = 44;
const WAV_CHUNK_SIZE = 1024 * 1024; // 1MB
const PROGRESS_UPDATE_INTERVAL = 10; // Update progress every 10 chunks
const MAX_CHANNELS = 8;
const BITS_PER_SAMPLE_8 = 8;
const BITS_PER_SAMPLE_16 = 16;
const BITS_PER_SAMPLE_24 = 24;
const BITS_PER_SAMPLE_32 = 32;

// WAV header offset constants
const WAV_OFFSET_RIFF = 0;
const WAV_OFFSET_FILE_SIZE = 4;
const WAV_OFFSET_WAVE = 8;
const WAV_OFFSET_FMT = 12;
const WAV_OFFSET_FMT_CHUNK_SIZE = 16;
const WAV_OFFSET_AUDIO_FORMAT = 20;
const WAV_OFFSET_NUM_CHANNELS = 22;
const WAV_OFFSET_SAMPLE_RATE = 24;
const WAV_OFFSET_BYTE_RATE = 28;
const WAV_OFFSET_BLOCK_ALIGN = 32;
const WAV_OFFSET_BITS_PER_SAMPLE = 34;
const WAV_OFFSET_DATA = 36;
const WAV_OFFSET_DATA_SIZE = 40;

// WAV audio format constants
const WAV_AUDIO_FORMAT_PCM = 1;
const WAV_AUDIO_FORMAT_IEEE_FLOAT = 3;
const WAV_FMT_CHUNK_SIZE = 16;
const WAV_DATA_CHUNK_HEADER_SIZE = 36;

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

@Component
export struct AudioConversion {
  pageInfos: NavPathStack = new NavPathStack();
  outputFilePath: string = '';
  fileFd: number = -1;
  @State message: string = '音频格式转换';
  @State converterHandle: number = 0;
  @State createResult: string = '';
  @State processResult: string = '';
  @State selectedFileUri: string = '';
  @State selectedFileName: string = '';
  @State fileInfo: AudioFileInfo | null = null;
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

  selectOutputFileAndSave(data: ArrayBuffer | string, isFilePath: boolean, extension: string = '.wav'): Promise<void> {
    return new Promise((resolve, reject) => {
      try {
        let documentSaveOptions = new picker.DocumentSaveOptions();
        documentSaveOptions.newFileNames = [`converted_audio_${Date.now()}${extension}`];

        let context = getContext() as common.Context;
        let documentPicker = new picker.DocumentViewPicker(context);

        documentPicker.save(documentSaveOptions, async (err: BusinessError, documentSaveResult: Array<string>) => {
          if (err) {
            Logger.error(TAG, `DocumentViewPicker.save failed: ${err.code}, ${err.message}`);
            this.processResult = 'Save dialog cancelled or failed';
            resolve();
            return;
          }

          if (documentSaveResult && documentSaveResult.length > 0) {
            this.outputFilePath = documentSaveResult[0];
            const uri = new fileUri.FileUri(this.outputFilePath);
            this.outputFileName = uri.name || 'Output File';
            Logger.info(TAG, `User selected save path: ${this.outputFilePath}`);

            if (isFilePath) {
              await this.saveToFileFromPath(data as string);
            } else {
              await this.saveToOutputPath(data as ArrayBuffer);
            }
          }
          resolve();
        });
      } catch (err) {
        Logger.error(TAG, `selectOutputFileAndSave error: ${JSON.stringify(err)}`);
        this.processResult = 'Save dialog error';
        reject(err);
      }
    });
  }

  async parseAudioFileFormat(): Promise<void> {
    if (!this.selectedFileUri) {
      Logger.error(TAG, 'No file selected');
      return;
    }

    const file = fs.openSync(this.selectedFileUri, fs.OpenMode.READ_ONLY);

    try {
      const buffer = new ArrayBuffer(44);
      const bytesRead = fs.readSync(file.fd, buffer);
      fs.closeSync(file.fd);

      if (bytesRead < 44) {
        Logger.error(TAG, 'Failed to read WAV header');
        this.createResult = 'Failed to read WAV header';
        return;
      }

      const fileInfo = this.parseWavHeader(buffer);

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

  private readLittleEndian16(buffer: ArrayBuffer, offset: number): number {
    const view = new DataView(buffer);
    return view.getUint16(offset, true);
  }

  private readLittleEndian32(buffer: ArrayBuffer, offset: number): number {
    const view = new DataView(buffer);
    return view.getUint32(offset, true);
  }

  private getSampleFormatFromBits(bitsPerSample: number, audioFormat: number): number {
    switch (bitsPerSample) {
      case 8:
        return AudioSampleFormat.AUDIO_SAMPLE_U8;
      case 16:
        return AudioSampleFormat.AUDIO_SAMPLE_S16LE;
      case 24:
        return AudioSampleFormat.AUDIO_SAMPLE_S24LE;
      case 32:
        if (audioFormat === 3) {
          return AudioSampleFormat.AUDIO_SAMPLE_F32LE;
        } else {
          return AudioSampleFormat.AUDIO_SAMPLE_S32LE;
        }
      default:
        Logger.warn(TAG, `Unsupported bits per sample: ${bitsPerSample}, defaulting to F32LE`);
        return AudioSampleFormat.AUDIO_SAMPLE_F32LE;
    }
  }

  private getChannelLayoutFromChannels(numChannels: number): number {
    switch (numChannels) {
      case 1:
        return AudioChannelLayout.CH_LAYOUT_MONO;
      case 2:
        return AudioChannelLayout.CH_LAYOUT_STEREO;
      case 3:
        return AudioChannelLayout.CH_LAYOUT_2POINT1;
      case 4:
        return AudioChannelLayout.CH_LAYOUT_QUAD;
      case 5:
        return AudioChannelLayout.CH_LAYOUT_5POINT0;
      case 6:
        return AudioChannelLayout.CH_LAYOUT_5POINT1;
      case 7:
        return AudioChannelLayout.CH_LAYOUT_6POINT1;
      case 8:
        return AudioChannelLayout.CH_LAYOUT_7POINT1;
      default:
        return AudioChannelLayout.CH_LAYOUT_STEREO;
    }
  }

  private parseWavHeader(buffer: ArrayBuffer): AudioFileInfo | null {
    const WAV_HEADER_SIZE = 44;
    const WAV_AUDIO_FORMAT_OFFSET = 20;
    const WAV_NUM_CHANNELS_OFFSET = 22;
    const WAV_SAMPLE_RATE_OFFSET = 24;
    const WAV_BITS_PER_SAMPLE_OFFSET = 34;

    if (buffer.byteLength < WAV_HEADER_SIZE) {
      Logger.error(TAG, 'Buffer too small to be a valid WAV file');
      return null;
    }

    const view = new Uint8Array(buffer);
    if (view[0] !== 0x52 || view[1] !== 0x49 || view[2] !== 0x46 || view[3] !== 0x46) {
      Logger.error(TAG, 'Invalid WAV file header');
      return null;
    }

    const audioFormat = this.readLittleEndian16(buffer, WAV_AUDIO_FORMAT_OFFSET);
    const numChannels = this.readLittleEndian16(buffer, WAV_NUM_CHANNELS_OFFSET);
    const samplingRate = this.readLittleEndian32(buffer, WAV_SAMPLE_RATE_OFFSET);
    const bitsPerSample = this.readLittleEndian16(buffer, WAV_BITS_PER_SAMPLE_OFFSET);

    const sampleFormat = this.getSampleFormatFromBits(bitsPerSample, audioFormat);
    const channelLayout = this.getChannelLayoutFromChannels(numChannels);

    return {
      encodingType: AudioEncodingType.AUDIO_ENCODING_TYPE_RAW,
      samplingRate: samplingRate,
      channelLayout: channelLayout,
      sampleFormat: sampleFormat,
      numChannels: numChannels,
      fileSize: buffer.byteLength
    };
  }

  private async createConverter(): Promise<boolean> {
    let path: string = new fileUri.FileUri(this.selectedFileUri).path;
    let file = fs.openSync(path, fs.OpenMode.READ_ONLY);

    try {
      // Execute conversion in background thread using taskpool
      const result = await taskpool.execute(
        convertAudioInBackground,
        this.inputFormat.encodingType,
        this.inputFormat.samplingRate,
        this.inputFormat.channelLayout,
        this.inputFormat.sampleFormat,
        this.outputFormat.encodingType,
        this.outputFormat.samplingRate,
        this.outputFormat.channelLayout,
        this.outputFormat.sampleFormat,
        file.fd
      );

      if (!result) {
        this.createResult = 'Conversion failed';
        this.isConverting = false;
        Logger.error(TAG, 'AudioConverterCreate failed: returned null');
        return false;
      }

      // result is the internal output file path
      Logger.info(TAG, `Conversion output file: ${result}`);

      // Show save dialog and copy file to user selected location (always save as WAV)
      await this.selectOutputFileAndSave(result as string, true, '.wav');

      return true;
    } finally {
      fs.closeSync(file.fd);
    }
  }

  private async saveToOutputPath(data: ArrayBuffer): Promise<void> {
    try {
      if (!data || data.byteLength === 0) {
        Logger.error(TAG, 'No converted data to save');
        this.processResult = 'Save failed: No converted data';
        return;
      }

      Logger.info(TAG, `Converted data size: ${data.byteLength} bytes`);

      let outputPath: string;
      let fileName: string;

      if (this.outputFilePath) {
        outputPath = this.outputFilePath;
        fileName = this.outputFileName;
      } else {
        Logger.error(TAG, 'No output path selected');
        this.processResult = 'Save failed: No output path selected';
        return;
      }

      const channels = this.getChannelCount(this.outputFormat.channelLayout);
      const bitsPerSample = this.getBitsPerSample(this.outputFormat.sampleFormat);
      const sampleRate = this.outputFormat.samplingRate;

      Logger.info(TAG, `WAV params: sampleRate=${sampleRate}, channels=${channels}, bitsPerSample=${bitsPerSample}`);

      const wavFileData = this.writeWavFileHeaderCorrected(
        data, sampleRate, channels, bitsPerSample, this.outputFormat.sampleFormat
      );

      Logger.info(TAG, `WAV file size: ${wavFileData.byteLength} bytes`);

      let file = fs.openSync(outputPath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      const writeLen = fs.writeSync(file.fd, wavFileData);
      fs.closeSync(file.fd);

      Logger.info(TAG, `Save file success: ${writeLen} bytes`);
      this.processResult = `Conversion complete and saved to: ${fileName}`;
    } catch (error) {
      Logger.error(TAG, `saveToOutputPath failed: ${JSON.stringify(error)}`);
      this.processResult = 'Save file error';
    }
  }

  private async saveToFileFromPath(tempFilePath: string): Promise<void> {
    try {
      Logger.info(TAG, `saveToFileFromPath: ${tempFilePath}`);

      let outputPath: string;
      let fileName: string;

      if (this.outputFilePath) {
        outputPath = this.outputFilePath;
        fileName = this.outputFileName;
      } else {
        Logger.error(TAG, 'No output path selected');
        this.processResult = 'Save failed: No output path selected';
        return;
      }

      const channels = this.getChannelCount(this.outputFormat.channelLayout);
      const bitsPerSample = this.getBitsPerSample(this.outputFormat.sampleFormat);
      const sampleRate = this.outputFormat.samplingRate;

      Logger.info(TAG, `WAV params: sampleRate=${sampleRate}, channels=${channels}, bitsPerSample=${bitsPerSample}`);

      // Get temp file size
      const tempFile = await fs.open(tempFilePath, fs.OpenMode.READ_ONLY);
      const tempFileStat = await fs.stat(tempFile.fd);
      const tempFileSize = tempFileStat.size;
      Logger.info(TAG, `Temp file size: ${tempFileSize} bytes`);

      const dataSize = tempFileSize;
      const totalSize = dataSize + WAV_HEADER_SIZE; // WAV header is 44 bytes

      // Create WAV header
      const header = this.createWavHeader(
        dataSize, sampleRate, channels, bitsPerSample, this.outputFormat.sampleFormat
      );

      // Open output file
      const outputFile = await fs.open(outputPath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);

      // Write header
      await fs.write(outputFile.fd, header);

      // Copy PCM data in chunks to avoid blocking UI
      const buffer = new ArrayBuffer(WAV_CHUNK_SIZE);
      let totalRead = 0;

      while (totalRead < tempFileSize) {
        const readSize = Math.min(WAV_CHUNK_SIZE, tempFileSize - totalRead);
        const bytesRead = await fs.read(tempFile.fd, buffer, { length: readSize });
        if (bytesRead === 0) break;

        await fs.write(outputFile.fd, buffer, { length: bytesRead });
        totalRead += bytesRead;

        // Yield to UI thread after each chunk to prevent blocking
        await new Promise<void>(resolve => setTimeout(resolve, 0));

        // Log progress periodically
        if (totalRead % (PROGRESS_UPDATE_INTERVAL * WAV_CHUNK_SIZE) === 0) {
          Logger.info(TAG, `Progress: ${totalRead}/${tempFileSize} bytes (${((totalRead / tempFileSize) * 100).toFixed(1)}%)`);
        }
      }

      await fs.close(tempFile.fd);
      await fs.close(outputFile.fd);

      // Delete temp file
      try {
        await fs.unlink(tempFilePath);
        Logger.info(TAG, `Temp file deleted: ${tempFilePath}`);
      } catch (e) {
        Logger.warn(TAG, `Failed to delete temp file: ${e}`);
      }

      Logger.info(TAG, `Save file success: ${totalSize} bytes`);
      this.processResult = `Conversion complete and saved to: ${fileName}`;
    } catch (error) {
      Logger.error(TAG, `saveToFileFromPath failed: ${JSON.stringify(error)}`);
      this.processResult = 'Save file error';
    }
  }

  private cleanupOnError(): void {
    this.createResult = `Conversion failed`;
    this.processResult = `Conversion failed`;
    this.isConverting = false;

    this.converterHandle = 0;
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

      const result = await this.createConverter();

      if (result) {
        this.createResult = 'Conversion completed';
      }

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
        return BITS_PER_SAMPLE_8;
      case AudioSampleFormat.AUDIO_SAMPLE_S16LE:
        return BITS_PER_SAMPLE_16;
      case AudioSampleFormat.AUDIO_SAMPLE_S24LE:
        return BITS_PER_SAMPLE_24;
      case AudioSampleFormat.AUDIO_SAMPLE_S32LE:
      case AudioSampleFormat.AUDIO_SAMPLE_F32LE:
        return BITS_PER_SAMPLE_32;
      default:
        return BITS_PER_SAMPLE_16;
    }
  }

  private validateWavParameters(channels: number, sampleRate: number, bitsPerSample: number): void {
    if (channels <= 0 || channels > MAX_CHANNELS) {
      Logger.error(TAG, `unknown channel: ${channels}, WAV Supports 1~${MAX_CHANNELS} channels.`);
      throw new Error(`Invalid channel count: ${channels}`);
    }

    if (sampleRate <= 0) {
      Logger.error(TAG, `unknown sampleRate: ${sampleRate}`);
      throw new Error(`Invalid sample rate: ${sampleRate}`);
    }

    if (bitsPerSample !== BITS_PER_SAMPLE_8 && bitsPerSample !== BITS_PER_SAMPLE_16 &&
      bitsPerSample !== BITS_PER_SAMPLE_24 && bitsPerSample !== BITS_PER_SAMPLE_32) {
      Logger.error(TAG, `unknown bitsPerSample: ${bitsPerSample}, Supports ${BITS_PER_SAMPLE_8},${BITS_PER_SAMPLE_16},${BITS_PER_SAMPLE_24},${BITS_PER_SAMPLE_32}-bit.`);
      throw new Error(`Unsupported bits per sample: ${bitsPerSample}`);
    }
  }

  private createWavHeader(dataSize: number, sampleRate: number, channels: number,
    bitsPerSample: number, sampleFormat: number): ArrayBuffer {
    const header = new ArrayBuffer(WAV_HEADER_SIZE);
    const dv = new DataView(header);

    const writeString = (dv: DataView, offset: number, string: string) => {
      for (let i = 0; i < string.length; i++) {
        dv.setUint8(offset + i, string.charCodeAt(i));
      }
    };

    writeString(dv, WAV_OFFSET_RIFF, 'RIFF');
    const totalDataLen = dataSize + WAV_DATA_CHUNK_HEADER_SIZE;
    dv.setUint32(WAV_OFFSET_FILE_SIZE, totalDataLen, true);
    writeString(dv, WAV_OFFSET_WAVE, 'WAVE');

    writeString(dv, WAV_OFFSET_FMT, 'fmt ');
    dv.setUint32(WAV_OFFSET_FMT_CHUNK_SIZE, WAV_FMT_CHUNK_SIZE, true);

    if (bitsPerSample === BITS_PER_SAMPLE_32 && sampleFormat === AudioSampleFormat.AUDIO_SAMPLE_F32LE) {
      dv.setUint16(WAV_OFFSET_AUDIO_FORMAT, WAV_AUDIO_FORMAT_IEEE_FLOAT, true);
    } else {
      dv.setUint16(WAV_OFFSET_AUDIO_FORMAT, WAV_AUDIO_FORMAT_PCM, true);
    }

    dv.setUint16(WAV_OFFSET_NUM_CHANNELS, channels, true);
    dv.setUint32(WAV_OFFSET_SAMPLE_RATE, sampleRate, true);

    const byteRate = sampleRate * channels * (bitsPerSample / 8);
    dv.setUint32(WAV_OFFSET_BYTE_RATE, byteRate, true);

    const blockAlign = channels * (bitsPerSample / 8);
    dv.setUint16(WAV_OFFSET_BLOCK_ALIGN, blockAlign, true);
    dv.setUint16(WAV_OFFSET_BITS_PER_SAMPLE, bitsPerSample, true);

    writeString(dv, WAV_OFFSET_DATA, 'data');
    dv.setUint32(WAV_OFFSET_DATA_SIZE, dataSize, true);

    return header;
  }

  private writeWavFileHeaderCorrected(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number,
    bitsPerSample: number, sampleFormat: number): ArrayBuffer {
    this.validateWavParameters(channels, sampleRate, bitsPerSample);
    const header = this.createWavHeader(pcmBuffer.byteLength, sampleRate, channels, bitsPerSample, sampleFormat);
    const viewWavHeader = new Uint8Array(header);
    const viewPcm = new Uint8Array(pcmBuffer);
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);
    return resultBuffer;
  }
}
