/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */
import { Decimal } from '@kit.ArkTS';
import { media } from '@kit.MediaKit';
import { Logger } from '../utils/Logger';
import { JSON, taskpool, util } from '@kit.ArkTS';
import buffer from '@ohos.buffer';
import { common } from '@kit.AbilityKit';
import { fileIo as fs, fileUri, ListFileOptions, picker, ReadOptions } from '@kit.CoreFileKit';
import { NodeType, ColorMap, CaseInfo, OutputFile, InputFile, ExpectedOutput, ExecuteReport, NodeInfo } from '../utils/autoTest/Interface'
import { BusinessError } from '@kit.BasicServicesKit';
import { saveFileBuffer } from '../utils/ExportFile';
import audioNapi from 'libentry.so';
import { cryptoFramework } from '@kit.CryptoArchitectureKit';

const TAG: string = 'AudioEditTestApp_AudioEdit';
const SUCCESS = 0;

@Builder
export function AutoTestBuilder() {
  AutoTest()
}

function getNodeTypeFromString(originStr: string): NodeType {
  let ret: NodeType = NodeType.VALID;
  switch (originStr) {
    case "AISS":
      ret = NodeType.AISS;
      break;
    case "NR":
      ret = NodeType.NR;
      break;
    case "VB":
      ret = NodeType.VB;
      break;
    case "MIXER":
      ret = NodeType.MIXER;
      break;
    case "ENV":
      ret = NodeType.ENV;
      break;
    case "EQ":
      ret = NodeType.EQ;
      break;
    case "FIELD":
      ret = NodeType.FIELD;
      break;
  }
  return ret;
}

function getNumberArrayFromString(originStr: string): number[] {
  let ret: number[] = [];
  originStr.replace("[", "").replace("]", "").split(" ").forEach((num) => {
    ret.push(parseInt(num));
  })
  return ret;
}

@Component
export struct AutoTest {
  @State nodeInfoShow: boolean = false
  @State executeReportShow: boolean = false
  @State mixerProcessList: NodeType[] = [];
  @State text: string = "请选择预定义场景"
  @State index: number = 0
  @State nodeList: string[] = []
  @State fileList: string[] = []
  @State selectList: SelectOption[] = []
  @State inputFileList: string[] = []
  @State effectChainMap: Map<string, NodeType[]> = new Map()
  @State displayFormat: string = ".wav"
  @State displayChannels: number = 2
  @State displayBitDepth: number = 32
  @State displaySampleRate: number = 48000
  @State selectedFile: string = ""
  @State selectedCase: string = ""
  jsonDataMap: Map<string, CaseInfo> = new Map();
  pageInfos: NavPathStack = new NavPathStack();
  executeReport: ExecuteReport[] = [];
  singleReport: ExecuteReport = {
    caseName: '',
    startTime: '',
    endTime: '',
    inputFileReport: '',
    outFileReport: '',
    costTime: 0,
    result: false,
    startTimeStamp: 0,
    endTimeStamp: 0,
    effectProcess: ''
  };
  csvReportHeaderText = '用例名称,开始执行时间,结束执行时间,总耗时,输入文件,输出文件,执行结果\n';
  errorDetail: string = "";
  rootPath = '/storage/Users/currentUser/Download/src.main.audiodemo';
  jsonPath = '${this.rootPath}/case_json';
  outputPath = '${this.rootPath}/output_files';
  inputPath = '${this.rootPath}/input_files';
  expectedOutputPath = '${this.rootPath}/expected_output';
  dirCheckList: string[] = ["expected_output", "output_files", "input_files", "case_json"]
  outputFileInfo_: OutputFile = {
    format: '',
    sampleRate: 0,
    bitDepth: 0,
    channels: 0,
    nameTemplate: []
  };
  inputIdList_: string[] = [];
  caseNameList: string[] = [];
  aissRenderFrameFlag: boolean = false;

  async aboutToAppear(): Promise<void> {
    Logger.info(TAG, 'AutoTest aboutToAppear');
    try {
      let fileList:string[] = fs.listFileSync(this.rootPath);
      this.dirCheckList.forEach((check: string) => {
        if (!fileList.find((ele: string) => ele != "init.txt" && ele === check)) {
          Logger.info(TAG, 'dir:${check} not existed');
          let dirPath = '${this.rootPath}/${check}';
          try {
            fs.mkdirSync(dirPath);
          } catch (err) {
          }
        }
      })
    }catch (e) {
      Logger.error(TAG,"rootPath is null, need init rootPath");
      await this.initAutoTestDir();
    }

    // 创建引擎，创建管线
    let result = audioNapi.audioEditNodeInit(1);
    Logger.info(TAG, 'audioEffectNodeTestSuccess01 audioEditNodeInit result: ${result}');
    if (result !== SUCCESS) {
      this.getUIContext().showAlertDialog({
        message: '引擎初始化失败\n请重启应用',
        autoCancel: true
      })
      return;
    }


    const dirPath = new fileUri.FileUri(this.rootPath + '/case_json').path;
    let listFileOption: ListFileOptions = {
      filter: {
        suffix: [".json"]
      }
    };
    try {
      this.fileList = fs.listFileSync(dirPath, listFileOption);
    }catch (e) {
    }
    // 遍历解析路径下所有json文件
    this.fileList.forEach((jsonFile: string) => {
      const outputFile: OutputFile = {
        sampleRate: 0,
        bitDepth: 0,
        channels: 0,
        format: '',
        nameTemplate: []
      };
      const caseInfo: CaseInfo = {
        filePath: '',
        sceneName: '',
        description: '',
        inputFiles: [],
        mixerInfo: [],
        expectedOutput: {
          targetFile: []
        },
        outputFile: {
          sampleRate: 0,
          bitDepth: 0,
          channels: 0,
          format: '',
          nameTemplate: []
        }
      };

      const expectedOutput: ExpectedOutput = {
        targetFile: []
      }
      const jsonFilePath = '${this.rootPath}/case_json/${jsonFile}'
      try {
        let file = fs.openSync(jsonFilePath, fs.OpenMode.READ_WRITE);
        let fsStat = fs.statSync(jsonFilePath);
        let buf = new ArrayBuffer(fsStat.size);
        fs.readSync(file.fd, buf);
        let fileData: string = buffer.from(buf).toString();
        let obj = JSON.parse(fileData);
        caseInfo.sceneName = Object(obj)["scene_name"];
        caseInfo.filePath = jsonFile;
        this.selectList.push({ value: '${caseInfo.sceneName}' })
        this.caseNameList.push(caseInfo.sceneName);
        caseInfo.description = Object(obj)["description"];
        caseInfo.inputFiles = []
        caseInfo.mixerInfo = []
        let inputLen: number = (Object(obj)["input_files"])["length"];
        let mixerLen: number = (Object(obj)["mixer_info"])["length"];
        for (let i = 0; i < inputLen; i++) {
          const inputFile: InputFile = {
            path: '',
            sampleRate: 0,
            bitDepth: 0,
            channels: 0,
            effect: []
          };
          let originInputFileStr: string = (Object(obj)["input_files"])[i]["path"];
          inputFile.path = originInputFileStr;
          inputFile.sampleRate = (Object(obj)["input_files"])[i]["sample_rate"];
          inputFile.bitDepth = (Object(obj)["input_files"])[i]["bit_depth"];
          inputFile.channels = (Object(obj)["input_files"])[i]["channels"];
          inputFile.format = (Object(obj)["input_files"])[i]["format"];
          inputFile.effect = []

          let effectLen: number = (Object(obj)["input_files"])[i]["effect"]["length"];

          for (let j = 0; j < effectLen; j++) {
            let nodeInfo: NodeInfo = {
              type: '',
              gains: []
            }
            nodeInfo.type = (Object(obj)["input_files"])[i]["effect"][j]["type"];
            let mode: number = (Object(obj)["input_files"])[i]["effect"][j]["mode"];
            if (mode != undefined) {
              nodeInfo.mode = mode;
            }
            if ((Object(obj)["input_files"])[i]["effect"][j]["gains"] != undefined) {
              let gains: number[] = getNumberArrayFromString((Object(obj)["input_files"])[i]["effect"][j]["gains"]);
              nodeInfo.gains = gains;
            }
            inputFile.effect.push(nodeInfo);
          }
          caseInfo.inputFiles.push(inputFile)
        }
        if(!Array.isArray(Object(obj)["expected_output"]["target_file"])){
          expectedOutput.targetFile.push((Object(obj)["expected_output"])["target_file"]);
        }else{
          expectedOutput.targetFile = (Object(obj)["expected_output"])["target_file"];
        }
        caseInfo.expectedOutput = expectedOutput;
        for (let i = 0; i < mixerLen; i++) {
          let nodeInfo: NodeInfo = {
            type: '',
            gains: []
          }
          nodeInfo.type = (Object(obj)["mixer_info"])[i]["type"];
          let mode: number = (Object(obj)["mixer_info"])[i]["mode"];
          if (mode != undefined) {
            nodeInfo.mode = mode;
          }
          if ((Object(obj)["mixer_info"])[i]["gains"] != undefined) {
            let gains: number[] = getNumberArrayFromString((Object(obj)["mixer_info"])[i]["gains"]);
            nodeInfo.gains = gains;
          }

          caseInfo.mixerInfo?.push(nodeInfo);
        }
        caseInfo.outputFile.nameTemplate.push(Object(obj)["output_file"]["name_template"]);
        caseInfo.outputFile.format = (Object(obj)["output_file"])["format"];
        caseInfo.outputFile.sampleRate = (Object(obj)["output_file"])["sample_rate"];
        caseInfo.outputFile.bitDepth = (Object(obj)["output_file"])["bit_depth"];
        caseInfo.outputFile.channels = (Object(obj)["output_file"])["channels"];

        this.jsonDataMap.set(Object(obj)["scene_name"], caseInfo);
      } catch (e) {
        Logger.error(TAG, 'Open file:${jsonFilePath} Failed, error info:${e}')
      }
    })
    if (this.jsonDataMap.size > 0) {
      Logger.info(TAG, "Parse json file success");
    } else {
      Logger.error(TAG, "Parse json file failed");
    }
  }

  build() {
    NavDestination() {
      Row() {
        Column() {
          Text('选择场景：')
        }.width('30%')

        Column() {
          Select(this.selectList)
            .width('100%')
            .selected(this.index)
            .value(this.text)
            .font({ size: 16, weight: 500 })
            .fontColor('#182431')
            .selectedOptionFont({ size: 16, weight: 400 })
            .optionFont({ size: 16, weight: 400 })
            .menuAlign(MenuAlignType.START, { dx: 0, dy: 0 })
            .optionWidth(200)
            .optionHeight(300)
            .onSelect((index: number, text: string) => {
              this.index = index;
              this.text = text
              this.inputFileList = [];
              this.mixerProcessList = [];
              this.effectChainMap = new Map();
              let caseInfo = this.jsonDataMap.get(text);
              if (caseInfo) {
                this.selectedFile = caseInfo.filePath;
                this.selectedCase = caseInfo.sceneName;
              }
              caseInfo?.inputFiles?.forEach((inputFile: InputFile) => {
                let inputStr: string = inputFile.path + "#" + util.generateRandomUUID(true);
                this.inputFileList.push(inputStr);
                this.effectChainMap.set(inputStr, [NodeType.INPUT]);
                inputFile.effect.forEach((effect: NodeInfo) => {
                  this.effectChainMap.get(inputStr)?.push(getNodeTypeFromString(effect.type));
                })
              })
              caseInfo?.mixerInfo?.forEach((mixer: NodeInfo) => {
                this.mixerProcessList.push(getNodeTypeFromString(mixer.type))
              })
              this.mixerProcessList.push(NodeType.OUTPUT)
            })
        }.width('70%')
      }.height('10%')


      List() {
        ListItem() {
          this.NodeGraphArea()
        }

        ListItem() {
          this.MixerGraphArea()
        }

      }.height('75%').width('100%').padding(2)

      // .border({ width: '2vp', color: Color.Pink, radius: '10vp' })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceAround, alignItems: ItemAlign.Center }) {
        Button() {
          Text('执行选中用例')
        }
        .onClick(() => {
          this.singleReport.caseName = this.selectedCase;
          let caseExecuteStartTimestamp = new Date().getTime();
          let caseExecuteStartTime: string = this.getDateStringWithTimeStamp(caseExecuteStartTimestamp);
          this.singleReport.startTime = caseExecuteStartTime;
          this.singleReport.startTimeStamp = caseExecuteStartTimestamp;

          let status = this.executeSingleCase('${this.selectedCase}');

        })
        .backgroundColor(0x317aff)
        .borderRadius(10)
        .width(100)
        .height(50)

        Button() {
          Text('执行全部用例')
        }
        .onClick(() => {
          this.caseNameList.forEach((caseName:string)=>{
            let caseInfo = this.jsonDataMap.get(caseName);
            if(caseInfo == undefined){
              return;
            }
            this.singleReport.caseName = caseInfo?.sceneName;
            let caseExecuteStartTimestamp = new Date().getTime();
            let caseExecuteStartTime: string = this.getDateStringWithTimeStamp(caseExecuteStartTimestamp);
            this.singleReport.startTime = caseExecuteStartTime;
            this.singleReport.startTimeStamp = caseExecuteStartTimestamp;
            let status = this.executeSingleCase('${caseName}');
          })
        })
        .backgroundColor(0x317aff)
        .borderRadius(10)
        .width(100)
        .height(50)
        .margin({ left: 20 })
      }.height('20%').width('100%')
      .bindSheet(this.executeReportShow, this.ExecuteReportDialog(this.executeReport), {
        height: '80%',
        backgroundColor: Color.White,
        maskColor: Color.Transparent,
        showClose: false,
        onWillAppear: () => {
        },
        onAppear: () => {
        },
        onWillDisappear: () => {
        },
        onDisappear: () => {
          this.executeReportShow = false;
        },
        onWillDismiss: (DismissSheetAction: DismissSheetAction) => {
          if (DismissSheetAction.reason != DismissReason.TOUCH_OUTSIDE) {
            DismissSheetAction.dismiss()
          }
        }
      })

    }
    .onReady((context: NavDestinationContext) => {
      this.pageInfos = context.pathStack;
    })
    .title('自动化测试')
    .height('100%')
    .width('100%')
  }

  @Builder
  NodeGraphArea() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
      if (this.inputFileList.length > 0) {
        Row() {
          Text("Effect Area")
        }
      }
      Column() {
        ForEach(this.inputFileList, (inputFileName: string) => {
          Row() {
            List() {
              ForEach(this.effectChainMap.get(inputFileName), (value: NodeType) => {
                ListItem() {
                  this.NodeButton(value)
                }
              })
            }
            .width('100%')
            .listDirection(Axis.Horizontal)
            .alignListItem(ListItemAlign.Center)
            .focusWrapMode(FocusWrapMode.WRAP_WITH_ARROW)
            .scrollBar(BarState.Off)
            .margin({ bottom: 10 })
          }
        })
      }
      .border({ width: '1vp', color: Color.Blue, radius: '10vp' })

    }

  }

  @Builder
  MixerGraphArea() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
      if (this.mixerProcessList.length > 0) {
        Row() {
          Text("Output Area")
        }
      }


      Row() {
        List() {
           ForEach(this.mixerProcessList, (value: NodeType) => {
              ListItem() {
                this.NodeButton(value)
              }
           })
        }
        .width('100%')
        .listDirection(Axis.Horizontal)
        .alignListItem(ListItemAlign.Center)
        .focusWrapMode(FocusWrapMode.WRAP_WITH_ARROW)
        .scrollBar(BarState.Off)
        .border({ width: '1vp', color: Color.Blue, radius: '10vp' })
        .margin({ bottom: 10 })
      }
    }

  }

  @Builder
  NodeInfoDialog() {
    Column() {
      Row() {
        Column() {
          Image($r('app.media.close_window'))
            .width(20)
            .height(20)
        }
        .width('20%')
        .onClick(() => {
          this.nodeInfoShow = false;
        })

        Column() {
          Text('Node Info').fontSize(20)
        }.width('80%')
      }

      Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
        Text('sample_rate: ${this.displaySampleRate}').fontSize(20)
        Text('bit_depth: ${this.displayBitDepth}').fontSize(20)
        Text('channels: ${this.displayChannels}').fontSize(20)
        Text('format: ${this.displayFormat}').fontSize(20)
      }.height('80%')
    }

  }
  
  @Builder
  ExecuteReportDialog(report: ExecuteReport[]) {
    Column() {
      Row() {
        Column() {
          Image($r('app.media.close_window'))
            .width(20)
            .height(20)
        }
        .width('20%')
        .onClick(() => {
          this.executeReportShow = false;
        })

        Column() {
          Text('用例执行结果报告').fontSize(20)
        }.width('80%')
      }

      ForEach(report, (single: ExecuteReport) => {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
          Text('用例场景: ${single.caseName}').fontSize(20)
          Text('测试开始时间: ${single.startTime}').fontSize(20)
          Text('测试结束时间: ${single.endTime}').fontSize(20)
          // Text('输入文件信息: ${single.inputFile}').fontSize(20)
          // Text('音效处理级联关系: ${single.effectProcess}').fontSize(20)
          Text('输出文件信息: ${single.outFileReport}').fontSize(20)
          Text('处理耗时: ${single.costTime / 1000} s').fontSize(20)
          Text('测试结果: ${single.result ? "成功" : "异常"}\n').fontSize(20)
        }.height('80%')
      })

    }

  }

  @Builder
  NodeButton(text: NodeType) {
    Column() {
      Button() {
        Text('${text}').wordBreak(WordBreak.BREAK_WORD).lineBreakStrategy(LineBreakStrategy.BALANCED)
      }
      .backgroundColor(ColorMap.get(text))
      .borderRadius(10)
      .width(60)
      .height(60)
      .margin({ left: 10 })
    }
    .onClick(() => {
      this.nodeInfoShow = true;
    })
    .bindSheet(this.nodeInfoShow, this.NodeInfoDialog(), {
      height: '50%',
      backgroundColor: Color.White,
      maskColor: Color.Transparent,
      showClose: false,
      onWillAppear: () => {
      },
      onAppear: () => {
      },
      onWillDisappear: () => {
      },
      onDisappear: () => {
        this.nodeInfoShow = false;
      },
      onWillDismiss: (DismissSheetAction: DismissSheetAction) => {
        if (DismissSheetAction.reason != DismissReason.TOUCH_OUTSIDE) {
          DismissSheetAction.dismiss()
        }
      }
    })
  }

  async executeSingleCase(caseName: string): Promise<number> {
    let flag: number = 0;
    this.jsonDataMap.get(caseName)?.inputFiles?.forEach((ele: InputFile) => {
      let inputId = this.singleFileInput(ele.path);
      if (inputId == "VALID") {
        flag = 1;
        return;
      }
      this.inputIdList_.push(inputId);

      ele.effect.forEach((effect: NodeInfo) => {
        let nodeType = getNodeTypeFromString(effect.type);

        switch (nodeType) {
          case NodeType.NR:
            let NRId: string = util.generateRandomUUID(true);
            let ret: number = audioNapi.addNoiseReduction(NRId, inputId);
            if (ret != 0) {
              Logger.error(TAG, 'addNoiseReduction error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加降噪节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'addNoiseReduction node successful!!!');
            }
            break;
          case NodeType.AISS:
            let AISSId: string = util.generateRandomUUID(true);
            ret = audioNapi.addAudioSeparation(effect.mode, AISSId, inputId);
            if (ret != 0) {
              Logger.error(TAG, 'addAudioSeparation error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加音源分离节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'addAudioSeparation node successful!!!');
            }
            this.aissRenderFrameFlag = true
            break;
          case NodeType.ENV:
            let ENVId: string = util.generateRandomUUID(true);
            ret = audioNapi.startEnvEffect(inputId, ENVId, effect.mode);
            if (ret != 0) {
              Logger.error(TAG, 'startEnvEffect error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加环境节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'startEnvEffect node successful!!!');
            }
            break;
          case NodeType.FIELD:
            let FIELDId: string = util.generateRandomUUID(true);
            ret = audioNapi.startFieldEffect(inputId, effect.mode, FIELDId);
            if (ret != 0) {
              Logger.error(TAG, 'startFieldEffect error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加环境节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'startEnvEffect node successful!!!');
            }
            break;
          case NodeType.VB:
            let VBId: string = util.generateRandomUUID(true);
            ret = audioNapi.startVBEffect(inputId, effect.mode, VBId);
            if (ret != 0) {
              Logger.error(TAG, 'startVBEffect error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加环境节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'startEnvEffect node successful!!!');
            }
            break;
          case NodeType.EQ:
            let EQId: string = util.generateRandomUUID(true);
            if(effect.gains.length==0){
              ret = audioNapi.setEquailizerMode(effect.mode, EQId, inputId);
            }else{
              ret = audioNapi.setEquailizerFrequencyBandGains(effect.gains, EQId, inputId);
            }
            if (ret != 0) {
              Logger.error(TAG, 'startVBEffect error number is ${ret}');
              this.getUIContext().showAlertDialog({
                message: '添加环境节点失败\n请单击弹窗外任意处关闭弹窗',
                autoCancel: true
              })
              flag = 1;
              return;
            } else {
              Logger.info(TAG, 'startEnvEffect node successful!!!');
            }
            break;
        }
      })
    })

    let outputInfo = this.jsonDataMap.get(caseName)?.outputFile;
    if (outputInfo != undefined) {
      this.outputFileInfo_ = outputInfo;
      audioNapi.setFormat(outputInfo?.channels, outputInfo?.sampleRate, outputInfo?.bitDepth);
      taskpool.execute(saveFileBuffer).then(async (buffer: object) => {
        let date: Date = new Date();
        let fileName: string =
          '${this.selectedFile.replace(".json", "")}_${this.getDateStringWithTimeStamp(date.getTime(), 1)}.wav';
        let tapBuffer:ArrayBuffer = audioNapi.getAudioOfTap();
        if ((buffer as ArrayBuffer).byteLength === 0 || tapBuffer.byteLength === 0) {
          Logger.info(TAG, 'SaveBuffer error: byteLength==0');
        }
        this.saveSingleFile([buffer as ArrayBuffer,tapBuffer], ".wav", this.outputFileInfo_)
      }).then(() => {
        this.inputIdList_.forEach((inputId: string) => {
          let res = audioNapi.deleteSong(inputId);
          Logger.info(TAG, 'loop deleteSong result and inputId is ${res}, ${inputId}');
        })
        this.aissRenderFrameFlag = false;
      }).catch(()=>{
        Logger.error(TAG,"taskpool execute ERROR");
      })
    }
    return new Promise((resolve: Function) =>{resolve(flag);});;
  }

  async saveSingleFile(pcmBuffer: ArrayBuffer[], audioFormate: string, outputInfo: OutputFile) {
    try {
      let newFileNames = '${outputInfo.nameTemplate}${this.aissRenderFrameFlag?"_Voice":""}_${this.getDateStringWithTimeStamp(new Date().getTime(), 1)}.wav';
      let newFileNames2 = '${outputInfo.nameTemplate}${this.aissRenderFrameFlag?"_Accompaniment":""}_${this.getDateStringWithTimeStamp(new Date().getTime(), 1)}.wav';
      const outputFilePath = this.aissRenderFrameFlag?['${this.outputPath}/${newFileNames}','${this.outputPath}/${newFileNames2}']:['${this.outputPath}/${newFileNames}'];
      outputFilePath.forEach((path:string, index:number)=>{
        let file = fs.openSync(path, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        Logger.info(TAG, 'SaveBuffer filePath : ${outputFilePath}');
        if (audioFormate === '.wav') {
          // 处理wav
          let wavBuffer =
            this.writeWavFileHeader(pcmBuffer[index], outputInfo.sampleRate, outputInfo.channels, outputInfo.bitDepth);
          let writeLen = fs.writeSync(file.fd, wavBuffer);
          Logger.info(TAG, 'SaveWavBuffer writeLen : ${writeLen}');
          fs.closeSync(file);
        } else {
          // 处理pcm
          let writeLen = fs.writeSync(file.fd, pcmBuffer[0]);
          Logger.info(TAG, 'SavePcmBuffer writeLen : ${writeLen}');
          fs.closeSync(file);
        }
      })

      let expectedFileMd5: string[]|undefined =this.jsonDataMap.get(this.selectedCase)?.expectedOutput.targetFile.map((name:string)=>{
        return '${this.expectedOutputPath}/' + name;
      }).map((nameForMD5:string)=>{
        return this.getMD5OfFile(nameForMD5);
      })


      let outputFileMd5: string[] = [];
      try {
        outputFileMd5 = outputFilePath.map((path:string)=>{
          return this.getMD5OfFile(path);
        })
      } catch (e) {
        Logger.error(TAG, 'Get MD5 failed, error : ${e}')
      }
      if (expectedFileMd5!=undefined && this.checkMd5IsEqual(outputFileMd5,expectedFileMd5)) {
        Logger.error(TAG, 'file MD5 compare ERROR --- file1:${outputFilePath}; file2:${this.jsonDataMap.get(this.selectedCase)?.expectedOutput.targetFile}');
        this.getUIContext().showAlertDialog({
          message: '输出文件与预期不符失败\n请单击弹窗外任意处关闭弹窗',
          autoCancel: true
        })
        this.singleReport.result = false;
      } else {
        this.singleReport.result = true;
        Logger.info(TAG, 'file MD5 compare SUCCESS --- file1:${outputFilePath}; file2:${this.jsonDataMap.get(this.selectedCase)?.expectedOutput.targetFile}');
      }

      let caseExecuteEndTimestamp = new Date().getTime()
      let caseExecuteEndTime: string = this.getDateStringWithTimeStamp(caseExecuteEndTimestamp)
      this.singleReport.endTime = caseExecuteEndTime
      this.singleReport.endTimeStamp = caseExecuteEndTimestamp;
      this.singleReport.costTime = this.singleReport.endTimeStamp - this.singleReport.startTimeStamp;

      this.executeReport = [];
      outputInfo.nameTemplate = outputFilePath;
      this.singleReport.outFileReport = JSON.stringify(outputInfo);
      this.executeReport.push(this.singleReport);
      this.executeReportShow = true;
      let fileInfoText = await this.getCsvTextFromSingleReport(this.singleReport);
      let csvText = this.csvReportHeaderText + fileInfoText;
      let file = fs.openSync(outputFilePath[0].replace("_Voice","").replace(".wav", ".csv"), fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      let writeLen = fs.writeSync(file.fd, csvText);
      Logger.info(TAG, 'Save CSV file suncess writeLen : ${writeLen}');
      fs.closeSync(file);

    } catch (e) {
      Logger.error(TAG, 'SaveBuffer catch: ${JSON.stringify(e)}')
    }
  }

  writeWavFileHeader(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number, bitsPerSample: number): ArrayBuffer {
    Logger.info(TAG,
      'writeWavFileHeader sampleRate : ${sampleRate} --- channels : ${channels} --- bitsPerSample : ${bitsPerSample}');
    const header = new ArrayBuffer(44);
    const dv = new DataView(header);
    // 写入RIFF快
    this.writeString(dv, 0, 'RIFF');
    let totalDataLen = pcmBuffer.byteLength + 44;
    dv.setUint32(4, totalDataLen, true);
    this.writeString(dv, 8, 'WAVE');
    // 写入 fmt 快
    this.writeString(dv, 12, 'fmt ');
    // fmt快大小
    dv.setUint32(16, 16, true);
    // 格式类别（1 - PCM， 3 - IEEE 浮点数）
    dv.setUint16(20, 3, true);
    if (bitsPerSample === 32) {
      dv.setUint16(20, 3, true);
    } else {
      dv.setUint16(20, 1, true);
    }
    // 声道数
    dv.setUint16(22, channels, true);
    // 采样率
    dv.setUint32(24, sampleRate, true);
    // 比特率
    let byteRate = bitsPerSample * sampleRate * channels / 8;
    dv.setUint32(28, byteRate, true);
    // 每个采样点的字节数
    dv.setUint16(32, channels * bitsPerSample / 8, true);
    // 位深
    dv.setUint16(34, bitsPerSample, true);
    // 写入data 快
    this.writeString(dv, 36, 'data');
    // 数据快大小
    dv.setUint32(40, pcmBuffer.byteLength, true);

    let resultBuffer = this.concatArrayBuffer(header, pcmBuffer);
    return resultBuffer;
  }

  writeString(dv: DataView, offset: number, str: string) {
    for (let i = 0; i < str.length; i++) {
      dv.setUint8(offset + i, str.charCodeAt(i));
    }
  }

  concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
    const viewWavHeader = new Uint8Array(wavHeaderBuffer);
    const viewPcm = new Uint8Array(pcmBuffer);

    // 创建一个新的 ArrayBuffer, 大小为两个原始缓冲区之和
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);

    // 将两个视图的数据复制到新的视图中
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);

    return resultBuffer;
  }

  singleFileInput(audioFile: string): string {
    const inputId = util.generateRandomUUID(true);
    const outputId = util.generateRandomUUID(true);
    const mixerId = util.generateRandomUUID(true);

    // 获取文件内容
    let inputPath = '${this.inputPath}/${audioFile}';
    let file = fs.openSync(inputPath, fs.OpenMode.READ_WRITE);
    let fsStat = fs.statSync(inputPath);
    let buffer = new ArrayBuffer(fsStat.size);
    fs.readSync(file.fd, buffer);

    Logger.info(TAG, 'input FIle: ${inputPath}, buffer.length: ${buffer.byteLength}');

    const wavView = new Uint8Array(buffer);
    let wavHeaderBuffer = wavView.slice(0, 44).buffer;
    const dataView = new DataView(wavHeaderBuffer);
    // 获取通道数, 通道数位于22字节
    const channels: number = dataView.getUint16(22, true);
    // 获取采样率, 采样率位于第 24 到28 字节
    const sampleRate: number = dataView.getUint32(24, true);
    // 获取位深，位深位于 34字节
    const bitsPerSample: number = dataView.getUint16(34, true);
    // 格式类别  int 还是 float（3） ...
    const formatCategory = dataView.getUint16(20, true);
    // 获取音频的长度
    // 获取音频的长度
    const fmtSize = dataView.getUint32(16, true); // fmt chunk size
    let offset = 20 + fmtSize;
    let pcmLength = 0;
    while (offset <= buffer.byteLength) {
    const chunkId = dataView.getUint32(offset, true);
    offset += 4;
    const chunkSize = dataView.getUint32(offset, true);
    offset += 4;
    // 'data' 在 ASCII 小端字节
    if (chunkId === 0x61746164) {
        // 找到了 data 块，返回其大小
        pcmLength = chunkSize;
        break;
    }
    offset += chunkSize;
    }
    Logger.info(TAG, 'pcmLength: ${pcmLength}');
    Logger.info(TAG,
      'audioEffectNodeTestSuccess01 sampleRate: ${sampleRate}, bitsPerSample: ${bitsPerSample}, channels: ${channels}');
    const pcmBuffer = wavView.slice(44).buffer;
    let result =
      audioNapi.audioInAndOutInit(inputId, outputId, mixerId, channels, sampleRate, bitsPerSample, formatCategory,
        pcmLength, pcmBuffer);
    if (result !== SUCCESS) {
      Logger.error(TAG, 'Input audio FAILED : ${audioFile}')
      this.getUIContext().showAlertDialog({
        message: '音频导入失败\n请单击弹窗外任意处关闭弹窗',
        autoCancel: true
      })
      return "VALID";
    }
    Logger.info(TAG, 'audioEffectNodeTest beforeEach end');
    return inputId;
  }

  getMD5OfFile(filePath: string): string {
    // 获取文件内容
    let file = fs.openSync(filePath, fs.OpenMode.READ_WRITE);
    let buf = new ArrayBuffer(40960);
    fs.readSync(file.fd, buf);
    fs.closeSync(file);

    let mdAlgName = 'MD5'; // 摘要算法名。
    let md = cryptoFramework.createMd(mdAlgName);
    // 数据量较少时，可以只做一次update，将数据全部传入，接口未对入参长度做限制。
    md.updateSync({ data: new Uint8Array(buf) });
    let mdResult = md.digestSync();
    Logger.info(TAG, '[Sync]:Md result:' + mdResult.data);
    let md5Str = buffer.from(mdResult.data.buffer).toString('hex');
    Logger.info(TAG, '[Sync]:Md string result:' + md5Str);
    return md5Str;
  }

  getDateStringWithTimeStamp(timestamp: number, mode: number = 0): string {
    let date = new Date(timestamp);
    const year = date.getFullYear();
    const month = ('0' + (date.getMonth() + 1)).slice(-2);
    const day = ('0' + date.getDate()).slice(-2);
    const hour = date.getHours();
    const min = date.getMinutes();
    const sec = date.getSeconds();
    return mode == 0 ? '${year}/${month}/${day} ${hour}:${min}:${sec}' :
      '${year}-${month}-${day} ${hour}-${min}-${sec}';
  }

  async getCsvTextFromSingleReport(singleReport: ExecuteReport): Promise<string> {
    let inputFileReportText = '';
    let outputFileReportText = '';
    await this.getMetadataCsvTextOfInputFiles(this.jsonDataMap.get(singleReport.caseName)?.inputFiles).then((inputCsvRet:string)=>{
      inputFileReportText = inputCsvRet;
    })
    await this.getMetadataCsvTextOfoutputFiles(this.jsonDataMap.get(singleReport.caseName)?.outputFile.nameTemplate).then((outputCsvRet:string)=>{
      outputFileReportText = outputCsvRet;
    })

    let ret: string = 
      '${singleReport.caseName},${singleReport.startTime},${singleReport.endTime},${singleReport.costTime /
        1000} s,${inputFileReportText},${outputFileReportText},${singleReport.result ? "执行成功" :
        "执行异常"}\n';
    return new Promise((resolve: Function) =>{resolve(ret);});
  }

  checkMd5IsEqual(outputFileMd5:string[],expectedFileMd5:string[]):boolean{
    let ret = true;
    outputFileMd5.forEach((outputMd5:string)=>{
      if(expectedFileMd5.find((expected:string)=>expected===outputMd5)==undefined){
        ret = false;
      }
    })
    return ret;
  }

  async getMetadataCsvTextOfInputFiles(inputFiles:InputFile[]|undefined): Promise<string>{
    if(inputFiles == undefined){
      return "InputFiles open FAILED";
    }
    let ret:string = '';
    for(const input of inputFiles){
      Decimal.set({
        precision: 4,
      });
      let inputFilePath = '${this.inputPath}/${input.path}';
      let file = fs.openSync(inputFilePath, fs.OpenMode.READ_WRITE);
      let fsStat = fs.statSync(inputFilePath);
      let buffer = new ArrayBuffer(fsStat.size);
      fs.readSync(file.fd, buffer);
      const wavView = new Uint8Array(buffer);
      let wavHeaderBuffer = wavView.slice(0, 44).buffer;
      const dataView = new DataView(wavHeaderBuffer);
      // 获取通道数, 通道数位于22字节
      const channels: number = dataView.getUint16(22, true);
      // 获取采样率, 采样率位于第 24 到28 字节
      const sampleRate: number = dataView.getUint32(24, true);
      // 获取位深，位深位于 34字节
      const bitsPerSample: number = dataView.getUint16(34, true);

      let avMetadataExtractor = await media.createAVMetadataExtractor();
      // 设置fdSrc
      avMetadataExtractor.fdSrc = fs.openSync(inputFilePath);
      // 获取元数据（promise模式）
      let metadata = await avMetadataExtractor.fetchMetadata();
      let fileLen = new Decimal(fsStat.size/1024/1024*100).ceil();
      console.info(TAG, 'get meta data, media length: ${Number(metadata.duration)/1000} s');
      ret+='--------------------------\n
      输入文件路径: ${inputFilePath}\n
      文件大小: ${(Number(fileLen)/100).toString()} MB\n
      音频长度: ${Number(metadata.duration)/1000} s\n
      音频格式(采样率/声道数/位深): ${sampleRate}/${channels}/${bitsPerSample}\n'
    }
    return new Promise((resolve: Function) =>{resolve(ret);});
  }

  async getMetadataCsvTextOfoutputFiles(outputFiles:string[]|undefined): Promise<string>{
    if(outputFiles == undefined){
      return "OutputFiles open FAILED";
    }
    let ret:string = '';
    for(const outputPath of outputFiles){
      Decimal.set({
        precision: 4,
      });
      let file = fs.openSync(outputPath, fs.OpenMode.READ_WRITE);
      let fsStat = fs.statSync(outputPath);
      let buffer = new ArrayBuffer(fsStat.size);
      fs.readSync(file.fd, buffer);
      const wavView = new Uint8Array(buffer);
      let wavHeaderBuffer = wavView.slice(0, 44).buffer;
      const dataView = new DataView(wavHeaderBuffer);
      // 获取通道数, 通道数位于22字节
      const channels: number = dataView.getUint16(22, true);
      // 获取采样率, 采样率位于第 24 到28 字节
      const sampleRate: number = dataView.getUint32(24, true);
      // 获取位深，位深位于 34字节
      const bitsPerSample: number = dataView.getUint16(34, true);

      let avMetadataExtractor = await media.createAVMetadataExtractor();
      // 设置fdSrc
      avMetadataExtractor.fdSrc = fs.openSync(outputPath);
      // 获取元数据（promise模式）
      let metadata = await avMetadataExtractor.fetchMetadata();
      let fileLen = new Decimal(fsStat.size/1024/1024*100).ceil();
      console.info(TAG, 'get meta data, media length: ${Number(metadata.duration)/1000} s');
      ret+='--------------------------\n
      输出文件路径: ${outputPath}\n
      文件大小: ${(Number(fileLen)/100).toString()} MB\n
      音频长度: ${Number(metadata.duration)/1000} s\n
      音频格式(采样率/声道数/位深): ${sampleRate}/${channels}/${bitsPerSample}\n'
    }
    return new Promise((resolve: Function) =>{resolve(ret);});
  }

  async initAutoTestDir(){
    let documentSaveOptions = new picker.DocumentSaveOptions();
    documentSaveOptions.pickerMode = 1;
    let context = getContext() as common.Context;
    let documentPicker = new picker.DocumentViewPicker(context);
    documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
      if (err) {
        Logger.error(TAG,
          'DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}');
        return;
      }
      Logger.info(TAG,
        'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
      let uri = documentSaveResult[0];
      let filePath = new fileUri.FileUri(uri + '/init.txt').path;
      // documentSaveResult数组中只有一个路径
      let file = fs.openSync(filePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      fs.writeSync(file.fd, new ArrayBuffer(0));
      Logger.info(TAG, 'audiodemo dir init success');
      fs.closeSync(file);
      this.dirCheckList.forEach((dir: string) => {
        let dirPath = new fileUri.FileUri(${uri}/${dir}).path;
        fs.mkdirSync(dirPath);
      })
    })
  }

}