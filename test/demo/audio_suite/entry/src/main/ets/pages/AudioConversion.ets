import { Logger } from '../utils/Logger';
import { StyleConst } from '../utils/StyleConst';
import audioNapi from 'libentry.so';
import { common } from '@kit.AbilityKit';
import { picker } from '@kit.CoreFileKit';
import { BusinessError } from '@kit.BasicServicesKit';
import fs from '@ohos.file.fs';
import { fileUri } from '@kit.CoreFileKit';
import fileio from '@ohos.fileio';
import { SongInfo } from '../utils/InterfaceInfo';
import { taskpool } from '@kit.ArkTS';

const TAG: string = 'AudioConversion';

const bufferSize = 100 * 1024 * 1024; // 100MB
const zeroBuffer = new ArrayBuffer(bufferSize);

// Input status enum
enum OH_AudioConverter_InputStatus {
  AUDIOCONVERTER_INPUT_HAVE_DATA = 1,
  AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA = 2,
  AUDIOCONVERTER_INPUT_DATA_FINISHED = 3
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

// Audio converter format
interface OH_AudioConverter_Format {
  encodingType: number;
  samplingRate: number;
  channelLayout: number;
  sampleFormat: number;
}

// Converter output parameters
interface ConverterHandle {
  handle: number;
}

// OutputSize output parameters
interface OutputSize {
  value: number;
}

@Builder
function AudioConversionBuilder() {
  AudioConversion()
}

export { AudioConversionBuilder };

@Concurrent
export async function saveFileBuffer(): Promise<ArrayBuffer> {
  audioNapi.setCurrentTime(0);
  let ConverterBuffer = audioNapi.saveFileBuffer();
  return ConverterBuffer;
}

@Concurrent
export async function audioConverterProcess(converterHandle: number): Promise<ArrayBuffer> {
  let result = audioNapi.OH_AudioConverter_Process(converterHandle);
  return result;
}

@Component
export struct AudioConversion {
  pageInfos: NavPathStack = new NavPathStack();
  @StorageLink('playList') playList: Map<string, SongInfo> = new Map();
  @State message: string = '音频格式转换';
  @State converterHandle: number = 0;
  @State createResult: string = '';
  @State callbackResult: string = '';
  @State processResult: string = '';
  @State destroyResult: string = '';
  @State selectedFileUri: string = '';
  @State convertedData: ArrayBuffer | null = null;
  @State selectedFileName: string = '';
  @State inputFormat: OH_AudioConverter_Format = {
    encodingType: AudioEncodingType.AUDIO_ENCODING_TYPE_RAW,
    samplingRate: AudioSampleRate.AUDIO_SAMPLE_RATE_44100,
    channelLayout: 3,
    sampleFormat: AudioSampleFormat.AUDIO_SAMPLE_S16LE
  };
  @State outputFormat: OH_AudioConverter_Format = {
    encodingType: AudioEncodingType.AUDIO_ENCODING_TYPE_RAW,
    samplingRate: AudioSampleRate.AUDIO_SAMPLE_RATE_48000,
    channelLayout: 3,
    sampleFormat: AudioSampleFormat.AUDIO_SAMPLE_S16LE
  };
  @State isProcessing: boolean = false;
  @State isConverting: boolean = false;
  private context = getContext(this) as common.UIAbilityContext;
  private fileHandle: number = 0;
  private fileData: ArrayBuffer = new ArrayBuffer(0);
  private readOffset: number = 0;

  build() {
    NavDestination() {
      Scroll() {
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

          Row() {
            Column() {
              this.FormatConfigSection('输入格式', this.inputFormat, (format: OH_AudioConverter_Format) => {
                this.inputFormat = format;
              }, true)
            }
            .layoutWeight(1)

            Column() {
              this.FormatConfigSection('输出格式', this.outputFormat, (newFormat: OH_AudioConverter_Format) => {
                this.outputFormat = {
                  encodingType: newFormat.encodingType,
                  samplingRate: newFormat.samplingRate,
                  channelLayout: newFormat.channelLayout,
                  sampleFormat: newFormat.sampleFormat
                };
              }, false)
            }
            .layoutWeight(1)
          }
          .width('90%')
          .margin({ top: 5, bottom: 10 })

          if (!this.isConverting) {
            Button('创建转换器')
              .width('90%')
              .height(40)
              .margin({ top: 5 })
              .onClick(() => {
                this.OH_AudioConverter_Create();
              })

            Text(this.createResult)
              .fontSize(12)
              .fontColor(Color.Yellow)
              .margin({ top: 3, bottom: 3 })

            Button('设置回调')
              .width('90%')
              .height(40)
              .margin({ top: 5 })
              .onClick(() => {
                this.OH_AudioConverter_SetInputCallback();
              })

            Text(this.callbackResult)
              .fontSize(12)
              .fontColor(Color.Yellow)
              .margin({ top: 3, bottom: 3 })

            Button('开始转换')
              .width('90%')
              .height(40)
              .margin({ top: 5 })
              .onClick(() => {
                this.OH_AudioConverter_Process();
              })

            Text(this.processResult)
              .fontSize(12)
              .fontColor(Color.Yellow)
              .margin({ top: 3, bottom: 3 })

            Button('释放转换器')
              .width('90%')
              .height(40)
              .margin({ top: 5 })
              .onClick(() => {
                this.OH_AudioConverter_Destroy();
              })

            Text(this.destroyResult)
              .fontSize(12)
              .fontColor(Color.Yellow)
              .margin({ top: 3, bottom: 3 })

            Button('保存转换文件')
              .width('90%')
              .height(40)
              .margin({ top: 5 })
              .onClick(() => {
                if (this.convertedData) {
                  this.saveConvertedFile(this.convertedData);
                } else {
                  this.showToast('没有可保存的转换数据');
                }
              })
              .margin({ bottom: 20 })
          } else {
            Text('转换中...')
              .fontSize(18)
              .fontColor(Color.Green)
              .margin({ top: 20, bottom: 20 })
          }
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
  FormatConfigSection(title: string, format: OH_AudioConverter_Format, onChange: (format: OH_AudioConverter_Format) => void, isInput: boolean) {
    Column() {
      Text(title)
        .fontSize(16)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 10, bottom: 5 })

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
          { value: AudioSampleRate.AUDIO_SAMPLE_RATE_96000.toString() }
        ])
          .selected(this.getSampleRateIndex(isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate))
          .onSelect((index: number) => {

            Logger.info(TAG, `${isInput ? 'Input' : 'Output'} samplingRate selected: index=${index},
            value=${[8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000][index]}`);

            const rates = [8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 64000, 88200, 96000];
            let newFormat: OH_AudioConverter_Format = {
              encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
              samplingRate: rates[index],
              channelLayout: isInput ? this.inputFormat.channelLayout : this.outputFormat.channelLayout,
              sampleFormat: isInput ? this.inputFormat.sampleFormat : this.outputFormat.sampleFormat
            };
            onChange(newFormat);
            Logger.info(TAG, `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);

          })
      }
      .width('100%')
      .margin({ bottom: 5 })

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
          .selected(this.getChannelLayoutIndex(isInput ? this.inputFormat.channelLayout : this.outputFormat.channelLayout))
          .onSelect((index: number) => {
            Logger.info(TAG, `${isInput ? 'Input' : 'Output'} channelLayout selected: index=${index},
            value=${[4,3,1610612736,11,259,7,15,263,1539,51,206158430211,271,1543,55,
              206158430219, 206158430215, 1551,63,1799, 20495,1731,311,1807,319,1739,1591,1735,1599,1847,206158431759,255,255][index]}`);

            const layouts = [4,3,1610612736,11,259,7,15,263,1539,51,206158430211,271,1543,55,
              206158430219, 206158430215, 1551,63,1799, 20495,1731,311,1807,319,1739,1591,1735,1599,1847,206158431759,255,255];
            let newFormat: OH_AudioConverter_Format = {
              encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
              samplingRate: isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate,
              channelLayout: layouts[index],
              sampleFormat: isInput ? this.inputFormat.sampleFormat : this.outputFormat.sampleFormat
            };
            onChange(newFormat);
            Logger.info(TAG, `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);
          })
      }
      .width('100%')
      .margin({ bottom: 5 })

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
            Logger.info(TAG, `${isInput ? 'Input' : 'Output'} sampleFormat selected: index=${index},
            value=${[AudioSampleFormat.AUDIO_SAMPLE_U8, AudioSampleFormat.AUDIO_SAMPLE_S16LE, AudioSampleFormat.AUDIO_SAMPLE_S24LE, AudioSampleFormat.AUDIO_SAMPLE_S32LE, AudioSampleFormat.AUDIO_SAMPLE_F32LE][index]}`);
            Logger.info(TAG, `Current ${isInput ? 'inputFormat' : 'outputFormat'}.samplingRate before update: ${isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate}`);

            const sampleFormats = [AudioSampleFormat.AUDIO_SAMPLE_U8, AudioSampleFormat.AUDIO_SAMPLE_S16LE, AudioSampleFormat.AUDIO_SAMPLE_S24LE, AudioSampleFormat.AUDIO_SAMPLE_S32LE, AudioSampleFormat.AUDIO_SAMPLE_F32LE];
            let newFormat: OH_AudioConverter_Format = {
              encodingType: isInput ? this.inputFormat.encodingType : this.outputFormat.encodingType,
              samplingRate: isInput ? this.inputFormat.samplingRate : this.outputFormat.samplingRate,
              channelLayout: isInput ? this.inputFormat.channelLayout : this.outputFormat.channelLayout,
              sampleFormat: sampleFormats[index]
            };
            onChange(newFormat);
            Logger.info(TAG, `${isInput ? 'Input' : 'Output'}Format updated: ${JSON.stringify(newFormat)}`);
          })
      }
      .width('100%')
      .margin({ bottom: 5 })
    }
    .width('100%')
    .padding(8)
    .backgroundColor(0x33000000)
    .borderRadius(8)
  }

  getSampleRateIndex(rate: number): number {
    const rates = [8000, 11025, 12000, 16000, 22050, 24000, 32000,
      44100, 48000, 64000, 88200, 96000];
    return rates.indexOf(rate);
  }

  getChannelLayoutIndex(layout: number): number {
    const layouts = [4,3,1610612736,11,259,7,15,263,1539,51,206158430211,271,1543,55,
      206158430219, 206158430215, 1551,63,1799, 20495,1731,311,1807,319,1739,1591,1735,1599,1847,206158431759,255,255];
    return layouts.indexOf(layout);
  }

  getSampleFormatIndex(sampleFormat: number): number {
    const sampleFormats = [AudioSampleFormat.AUDIO_SAMPLE_U8, AudioSampleFormat.AUDIO_SAMPLE_S16LE, AudioSampleFormat.AUDIO_SAMPLE_S24LE, AudioSampleFormat.AUDIO_SAMPLE_S32LE, AudioSampleFormat.AUDIO_SAMPLE_F32LE];
    return sampleFormats.indexOf(sampleFormat);
  }


  selectAudioFile(): void {
    Logger.info(TAG, 'selectAudioFile called');
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
          Logger.info(TAG, `selected file: ${this.selectedFileUri}`);
        }
      });
    } catch (e) {
      Logger.error(TAG, `selectAudioFile error: ${JSON.stringify(e)}`);
    }
  }

  OH_AudioConverter_Create(): void {
    Logger.info(TAG, 'OH_AudioConverter_Create 接口调用');
    try {
      Logger.info(TAG, `InputFormat: encodingType=${this.inputFormat.encodingType},
      samplingRate=${this.inputFormat.samplingRate},
      channelLayout=${this.inputFormat.channelLayout},
      sampleFormat=${this.inputFormat.sampleFormat}`);
      Logger.info(TAG, `OutputFormat: encodingType=${this.outputFormat.encodingType},
      samplingRate=${this.outputFormat.samplingRate},
      channelLayout=${this.outputFormat.channelLayout},
      sampleFormat=${this.outputFormat.sampleFormat}`);

      // Add detailed information about channel count and format conversion
      const inputChannels = this.getChannelCount(this.inputFormat.channelLayout);
      const outputChannels = this.getChannelCount(this.outputFormat.channelLayout);
      const inputBits = this.getBitsPerSample(this.inputFormat.sampleFormat);
      const outputBits = this.getBitsPerSample(this.outputFormat.sampleFormat);

      Logger.info(TAG, `Channel conversion: ${inputChannels} -> ${outputChannels}`);
      Logger.info(TAG, `Sample rate conversion: ${this.inputFormat.samplingRate} Hz -> ${this.outputFormat.samplingRate} Hz`);
      Logger.info(TAG, `Bit depth conversion: ${inputBits} bits -> ${outputBits} bits`);

      // Detect special format conversions
      if (inputChannels !== outputChannels) {
        Logger.warn(TAG, `Channel count conversion: ${inputChannels} -> ${outputChannels}`);
      }
      if (this.inputFormat.samplingRate !== this.outputFormat.samplingRate) {
        Logger.warn(TAG, `Sample rate conversion: ${this.inputFormat.samplingRate} -> ${this.outputFormat.samplingRate}`);
      }
      if (inputBits !== outputBits) {
        Logger.warn(TAG, `Bit depth conversion: ${inputBits} -> ${outputBits}`);
      }

      let path: string = new fileUri.FileUri(this.selectedFileUri).path;
      Logger.info(TAG, `getPcmFileBuffer path: ${JSON.stringify(path)}`);
      let file  = fs.openSync(path, fs.OpenMode.READ_ONLY);
      let fsStat  = fs.statSync(path);
      Logger.info(TAG, `fileSize : ${fsStat.size}`);

      const result = audioNapi.OH_AudioConverter_Create(this.inputFormat, this.outputFormat, file.fd);

      // Note: The C++ layer has already read the data and closed the file descriptor, so there is no need to close it again here.
      // Check if the return value is a valid converter handle (non-zero and negative values indicate an error).
      if (result > 0) {
        this.converterHandle = result;  // Storage converter handle.
        this.createResult = `Create 调用成功, converter handle: ${result}`;
        Logger.info(TAG, `OH_AudioConverter_Create success, converter handle: ${result}`);
      } else {
        this.converterHandle = 0;  // Resetting Handles
        this.createResult = `Create 调用失败, error code: ${result}`;
        Logger.error(TAG, `OH_AudioConverter_Create failed, error code: ${result}`);
      }
    } catch (e) {
      this.createResult = `Create 调用失败: ${JSON.stringify(e)}`;
      Logger.info(TAG, `err: ${e}`);
      Logger.error(TAG, `OH_AudioConverter_Create error: ${JSON.stringify(e)}`);
    }
  }

  OH_AudioConverter_SetInputCallback(): void {
    Logger.info(TAG, 'OH_AudioConverter_SetCallback 接口调用');
    try {
      if (this.selectedFileUri === '') {
        this.callbackResult = '请先选择音频文件';
        return;
      }

      if (this.converterHandle === 0) {
        this.callbackResult = '请先创建转换器';
        Logger.error(TAG, 'Converter handle is invalid, please create converter first');
        return;
      }

      // Define callback function
      const callback = (data: ArrayBuffer) => {
        Logger.info(TAG, `Received callback data, size: ${data.byteLength}`);
        this.callbackResult = `Received callback data, size: ${data.byteLength} bytes`;
      };

      // Call C++ interface, passing callback function and user data
      const result = audioNapi.OH_AudioConverter_SetInputCallback(this.converterHandle, callback, 0);
      this.callbackResult = `SetInputCallback 调用成功, result: ${result}`;
      Logger.info(TAG, `OH_AudioConverter_SetInputCallback result: ${result}`);
    } catch (e) {
      this.callbackResult = `SetInputCallback 调用失败: ${JSON.stringify(e)}`;
      Logger.error(TAG, `OH_AudioConverter_SetInputCallback error: ${JSON.stringify(e)}`);
    }
  }

  OH_AudioConverter_Process(): void {
    Logger.info(TAG, 'OH_AudioConverter_Process 接口调用');

    // Checking whether the converter handle is valid.
    if (this.converterHandle === 0) {
      this.processResult = '请先创建转换器';
      Logger.error(TAG, 'Converter handle is invalid, please create converter first');
      return;
    }

    this.isConverting = true;
    this.processResult = '正在转换...';


    taskpool.execute(audioConverterProcess, this.converterHandle).then((value: object) => {
      const result = value as ArrayBuffer;
      if (result && result.byteLength > 0) {

        this.convertedData = result;
        this.processResult = `Process 完成，输出大小: ${result.byteLength} 字节`;
        Logger.info(TAG, `OH_AudioConverter_Process 成功, 输出大小: ${result.byteLength}`);
      } else {

        this.convertedData = null;
        this.processResult = `Process 失败`;
        Logger.error(TAG, `OH_AudioConverter_Process 失败`);
      }
      this.isConverting = false;
    }).catch((error: Error) => {
      this.convertedData = null;
      this.processResult = `Process 调用失败: ${error.message}`;
      Logger.error(TAG, `OH_AudioConverter_Process error: ${error.message}`);
      this.isConverting = false;
    });
  }

  OH_AudioConverter_Destroy(): void {
    Logger.info(TAG, 'OH_AudioConverter_Destroy 接口调用');
    try {
      if (this.converterHandle === 0) {
        this.destroyResult = '无需销毁，转换器句柄无效';
        Logger.info(TAG, 'Converter handle is already invalid');
        return;
      }

      audioNapi.OH_AudioConverter_Destroy(this.converterHandle);
      this.converterHandle = 0;
      this.destroyResult = 'Destroy 调用成功';
      Logger.info(TAG, 'OH_AudioConverter_Destroy 调用成功');
    } catch (e) {
      this.destroyResult = `Destroy 调用失败: ${JSON.stringify(e)}`;
      Logger.error(TAG, `OH_AudioConverter_Destroy error: ${JSON.stringify(e)}`);
    }
  }

  async saveConvertedFile(pcmData: ArrayBuffer): Promise<void> {
    try {
      Logger.info(TAG, 'saveConvertedFile 开始');
      let context = this.getUIContext().getHostContext() as common.UIAbilityContext;

      if (!context) {
        Logger.error(TAG, 'UIAbilityContext not initialized!');
        this.showToast('保存失败：上下文未初始化');
        return;
      }

      if (!pcmData || pcmData.byteLength === 0) {
        Logger.error(TAG, 'No converted audio data to save.');
        this.showToast('保存失败：无转换数据');
        return;
      }

      Logger.info(TAG, `Converted PCM data length is: ${pcmData.byteLength}`);

      // Use file picker to let user choose save location
      const documentViewPicker = new picker.DocumentViewPicker(context);
      const documentSaveOptions = new picker.DocumentSaveOptions();
      documentSaveOptions.pickerMode = picker.DocumentPickerMode.DOWNLOAD;

      documentViewPicker.save(documentSaveOptions).then((documentSaveResult: Array<string>) => {
        const uri = documentSaveResult[0];
        const fileName = `converted_audio_${Date.now()}.wav`;
        const testFilePath = new fileUri.FileUri(uri + `/` + `${fileName}`).path;

        Logger.info(TAG, `Saving to file: ${testFilePath}`);

        // Generate WAV file header
        Logger.info(TAG, `Preparing to generate WAV file header: pcmData length=${pcmData.byteLength}`);
        Logger.info(TAG, `Output format parameters: samplingRate=${this.outputFormat.samplingRate}, channelLayout=${this.outputFormat.channelLayout}, sampleFormat=${this.outputFormat.sampleFormat}`);
        Logger.info(TAG, `Calculated parameters: channels=${this.getChannelCount(this.outputFormat.channelLayout)}, bitsPerSample=${this.getBitsPerSample(this.outputFormat.sampleFormat)}`);

        const wavFileData = this.writeWavFileHeaderCorrected(
          pcmData,
          this.outputFormat.samplingRate,
          this.getChannelCount(this.outputFormat.channelLayout),
          this.getBitsPerSample(this.outputFormat.sampleFormat),
          this.outputFormat.sampleFormat
        );

        Logger.info(TAG, `WAV file header generation completed, total size=${wavFileData.byteLength}`);

        // Write to file
        let file = fs.openSync(testFilePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        const writeLen = fs.writeSync(file.fd, wavFileData);
        fs.closeSync(file.fd);

        Logger.info(TAG, `文件保存成功，写入字节数: ${writeLen}`);
        this.showToast(`文件保存成功: ${fileName}`);
      }).catch((err: BusinessError) => {
        Logger.error(TAG, `文件保存失败: ${err.code}, ${err.message}`);
        this.showToast('文件保存失败');
      });

    } catch (error) {
      Logger.error(TAG, 'saveConvertedFile 失败:', error);
      this.showToast('保存文件时发生错误');
    }
  }

  // Auxiliary method: obtaining the number of audio channels based on the audio channel layout.
  private getChannelCount(channelLayout: number): number {
    switch (channelLayout) {
      case AudioChannelLayout.CH_LAYOUT_MONO:
        return 1;
      case AudioChannelLayout.CH_LAYOUT_STEREO:
      case AudioChannelLayout.CH_LAYOUT_STEREO_DOWNMIX:
        return 2;
      case AudioChannelLayout.CH_LAYOUT_2POINT1:
        return 3;
      case AudioChannelLayout.CH_LAYOUT_3POINT0:
        return 3;
      case AudioChannelLayout.CH_LAYOUT_SURROUND:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_3POINT1:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_4POINT0:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_QUAD_SIDE:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_QUAD:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_2POINT0POINT2:
        return 4;
      case AudioChannelLayout.CH_LAYOUT_4POINT1:
        return 5;
      case AudioChannelLayout.CH_LAYOUT_5POINT0:
        return 5;
      case AudioChannelLayout.CH_LAYOUT_5POINT0_BACK:
        return 5;
      case AudioChannelLayout.CH_LAYOUT_2POINT1POINT2:
        return 5;
      case AudioChannelLayout.CH_LAYOUT_3POINT0POINT2:
        return 5;
      case AudioChannelLayout.CH_LAYOUT_5POINT1:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_5POINT1_BACK:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_6POINT0:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_3POINT1POINT2:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_6POINT0_FRONT:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_HEXAGONAL:
        return 6;
      case AudioChannelLayout.CH_LAYOUT_6POINT1:
        return 7;
      case AudioChannelLayout.CH_LAYOUT_6POINT1_BACK:
        return 7;
      case AudioChannelLayout.CH_LAYOUT_6POINT1_FRONT:
        return 7;
      case AudioChannelLayout.CH_LAYOUT_7POINT0:
        return 7;
      case AudioChannelLayout.CH_LAYOUT_7POINT0_FRONT:
        return 7;
      case AudioChannelLayout.CH_LAYOUT_7POINT1:
        return 8;
      case AudioChannelLayout.CH_LAYOUT_OCTAGONAL:
        return 8;
      case AudioChannelLayout.CH_LAYOUT_5POINT1POINT2:
        return 8;
      case AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE:
        return 8;
      case AudioChannelLayout.CH_LAYOUT_7POINT1_WIDE_BACK:
        return 8;
      default:
        Logger.warn(TAG, `Unknown channel layout: ${channelLayout}, using default value 2`);
        return 2; // Default to stereo
    }
  }

  // Auxiliary method: get bit depth based on sample format
  private getBitsPerSample(sampleFormat: number): number {
    switch (sampleFormat) {
      case AudioSampleFormat.AUDIO_SAMPLE_U8:
        return 8;
      case AudioSampleFormat.AUDIO_SAMPLE_S16LE:
        return 16;
      case AudioSampleFormat.AUDIO_SAMPLE_S24LE:
        return 24;
      case AudioSampleFormat.AUDIO_SAMPLE_S32LE:
      case AudioSampleFormat.AUDIO_SAMPLE_F32LE:
        return 32;
      default:
        return 16;
    }
  }

  // The correct WAV file header generation function can distinguish between S32LE and F32LE.
  private writeWavFileHeaderCorrected(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number, bitsPerSample: number, sampleFormat: number): ArrayBuffer {
    Logger.info(TAG,
      `writeWavFileHeaderCorrected sampleRate: ${sampleRate}, channels: ${channels}, bitsPerSample: ${bitsPerSample}, sampleFormat: ${sampleFormat}`);

    if (channels <= 0 || channels > 8) {
      Logger.error(TAG, `无效的声道数: ${channels}，WAV标准支持1-8声道`);
      throw new Error(`Invalid channel count: ${channels}`);
    }

    if (sampleRate <= 0) {
      Logger.error(TAG, `无效的采样率: ${sampleRate}`);
      throw new Error(`Invalid sample rate: ${sampleRate}`);
    }

    if (bitsPerSample !== 8 && bitsPerSample !== 16 && bitsPerSample !== 24 && bitsPerSample !== 32) {
      Logger.error(TAG, `不支持的位深: ${bitsPerSample}，支持8/16/24/32位`);
      throw new Error(`Unsupported bits per sample: ${bitsPerSample}`);
    }

    const header = new ArrayBuffer(44);
    const dv = new DataView(header);


    const writeString = (dv: DataView, offset: number, string: string) => {
      for (let i = 0; i < string.length; i++) {
        dv.setUint8(offset + i, string.charCodeAt(i));
      }
    };

    writeString(dv, 0, 'RIFF');
    const totalDataLen = pcmBuffer.byteLength + 44;
    dv.setUint32(4, totalDataLen, true);
    writeString(dv, 8, 'WAVE');
    Logger.info(TAG, `RIFF块设置完成，总大小: ${totalDataLen} bytes`);

    // fmt chunk
    writeString(dv, 12, 'fmt ');
    dv.setUint32(16, 16, true);

    // Format Tag: 1 = PCM (integer format), 3 = IEEE Float (floating point format)
    // F32LE uses IEEE floating point format (3), other formats (including S32LE) use PCM format (1)
    if (bitsPerSample === 32 && sampleFormat === AudioSampleFormat.AUDIO_SAMPLE_F32LE) {
      dv.setUint16(20, 3, true);  // IEEE Floating Point
      Logger.info(TAG, `Using IEEE floating point format (F32LE)`);
    } else {
      dv.setUint16(20, 1, true);  // PCM (including U8, S16LE, S24LE, S32LE)
      Logger.info(TAG, `Using PCM format`);
    }

    dv.setUint16(22, channels, true);
    Logger.info(TAG, `声道数: ${channels}`);

    dv.setUint32(24, sampleRate, true);
    Logger.info(TAG, `采样率: ${sampleRate} Hz`);

    const byteRate = sampleRate * channels * (bitsPerSample / 8);
    dv.setUint32(28, byteRate, true);
    Logger.info(TAG, `比特率: ${byteRate} bytes/s`);

    const blockAlign = channels * (bitsPerSample / 8);
    dv.setUint16(32, blockAlign, true);
    Logger.info(TAG, `块对齐: ${blockAlign} bytes`);

    dv.setUint16(34, bitsPerSample, true);
    Logger.info(TAG, `位深: ${bitsPerSample} bits`);

    // data chunk
    writeString(dv, 36, 'data');
    dv.setUint32(40, pcmBuffer.byteLength, true);
    Logger.info(TAG, `data块大小: ${pcmBuffer.byteLength} bytes`);

    // Merging the file header and PCM data.
    const viewWavHeader = new Uint8Array(header);
    const viewPcm = new Uint8Array(pcmBuffer);
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);

    Logger.info(TAG, `WAV文件生成完成，总大小: ${resultBuffer.byteLength} bytes`);
    return resultBuffer;
  }

  // Auxiliary method: show toast message
  private showToast(message: string): void {
    // Toast implementation can be added as needed
    Logger.info(TAG, `Toast: ${message}`);
  }
}
