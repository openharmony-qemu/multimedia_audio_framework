/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */
import { Decimal } from '@kit.ArkTS';
import { media } from '@kit.MediaKit';
import { Logger } from '../utils/Logger';
import { JSON, taskpool, util } from '@kit.ArkTS';
import buffer from '@ohos.buffer';
import { common } from '@kit.AbilityKit';
import { fileUri, ListFileOptions, picker } from '@kit.CoreFileKit';
import fs from '@ohos.file.fs';
import {NodeType, colorMap, inputStringToNodeTypeMap} from '../utils/importSongs/InterfaceInfo'
import { SceneInfo, OutputFile, InputFile, ExpectedOutput, ExecuteReport,
  NodeInfo, PipelineInfo, JsonFileInfo, DisplayPipelineInfo } from '../utils/autoTest/Interface'
import { BusinessError } from '@kit.BasicServicesKit';
import { multiPipelineSaveFileBuffer, writeWavFileHeader, multiPipelineRealTimeSaveFileBuffer } from '../utils/ExportFile';
import audioNapi from 'libentry.so';
import { cryptoFramework } from '@kit.CryptoArchitectureKit';
import { initAllInputNode } from '../utils/autoTest/AsyncTask'
import { getDateStringWithTimeStamp } from '../utils/Util'

// root path of Auto Test APP
const rootPath = `/storage/Users/currentUser/Download/src.main.audiodemo`;
// the path to store all output audio files
const outputPath = `${rootPath}/output_files`;
// the path to store all input audio files of every test scene
const inputDir = `${rootPath}/input_files`;

const TAG: string = 'AudioEditTestApp_AudioEdit';
const SUCCESS: number = 0;
const FAILED: number = 1;
const csvReportHeaderText = `执行管线,开始执行时间,结束执行时间,总耗时,输入文件,输入音频时长,输出文件,执行结果\n`;

const jsonSummaryToJsonFileInfoMap: Map<string, JsonFileInfo> = new Map();
const sceneInfoIdToSceneInfoMap: Map<string, SceneInfo> = new Map();
const pipelineIdToPipelineInfoMap: Map<string, PipelineInfo> = new Map();
const nodeInfoIdToNodeInfoMap: Map<string, NodeInfo> = new Map();
const pipelineIdToSingleReportMap: Map<string, ExecuteReport> = new Map();

const dirCheckList: string[] = ["expected_output", "output_files", "input_files", "case_json"];
let preparedPipelineList: string[] = [];

// init all folder required for running Auto Test
async function initAutoTestDir() {
  let documentSaveOptions = new picker.DocumentSaveOptions();
  documentSaveOptions.pickerMode = 1;
  let context = getContext() as common.Context;
  let documentPicker = new picker.DocumentViewPicker(context);
  documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
    if (err) {
      Logger.error(TAG,
        `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
      return;
    }
    Logger.info(TAG, 'documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
    let uri = documentSaveResult[0];
    let filePath = new fileUri.FileUri(uri + `/init.txt`).path;
    try {
      let file = fs.openSync(filePath, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
      fs.writeSync(file.fd, new ArrayBuffer(0));
      Logger.info(TAG, `audiodemo dir init success`);
      fs.closeSync(file);
      dirCheckList.forEach((dir: string) => {
        let dirPath = new fileUri.FileUri(`${uri}/${dir}`).path;
        fs.mkdirSync(dirPath);
      })
    } catch (err) {
      Logger.error(TAG, `initAutoTestDir ERROR: ${err}`);
    }
  })
}

function getNodeTypeFromString(originStr: string): NodeType {
  let ret = inputStringToNodeTypeMap.get(originStr);
  if (ret === undefined) {
    ret = NodeType.VALID;
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

function parseJsonObject(jsonObjectStr: string): SceneInfo {
  const sceneInfo: SceneInfo = {
    sceneName: '',
    description: '',
    inputFiles: [],
    mixerInfo: [],
    expectedOutput: {
      md5Value: []
    },
    outputFile: {
      sampleRate: 0,
      bitDepth: 0,
      channels: 0,
      format: '',
      nameTemplate: ''
    },
    sceneInfoId: util.generateRandomUUID(true),
    pipelineId: ''
  };
  const expectedOutput: ExpectedOutput = {
    md5Value: []
  }
  sceneInfo.sceneName = Object(jsonObjectStr)["scene_name"];
  sceneInfo.description = Object(jsonObjectStr)["description"];
  sceneInfo.inputFiles = []
  sceneInfo.mixerInfo = []
  let inputLen: number = (Object(jsonObjectStr)["input_files"])["length"];
  let mixerLen: number = (Object(jsonObjectStr)["mixer_info"])["length"];
  for (let i = 0; i < inputLen; i++) {
    const inputFile: InputFile = {
      path: '',
      sampleRate: 0,
      bitDepth: 0,
      channels: 0,
      effect: [],
      inputPathId: '',
      inputId: util.generateRandomUUID(true),
      effectStr: []
    };
    let originInputFileStr: string = (Object(jsonObjectStr)["input_files"])[i]["path"];
    inputFile.path = originInputFileStr;
    inputFile.inputPathId = util.generateRandomUUID(true);
    inputFile.sampleRate = (Object(jsonObjectStr)["input_files"])[i]["sample_rate"];
    inputFile.bitDepth = (Object(jsonObjectStr)["input_files"])[i]["bit_depth"];
    inputFile.channels = (Object(jsonObjectStr)["input_files"])[i]["channels"];
    inputFile.format = (Object(jsonObjectStr)["input_files"])[i]["format"];
    inputFile.effect = []

    let effectLen: number = (Object(jsonObjectStr)["input_files"])[i]["effect"]["length"];
    inputFile.effectStr = (Object(jsonObjectStr)["input_files"])[i]["effect"];

    for (let j = 0; j < effectLen; j++) {
      let nodeInfo: NodeInfo = {
        type: '',
        gains: [],
        nodeInfoId: util.generateRandomUUID(true)
      }
      nodeInfo.type = (Object(jsonObjectStr)["input_files"])[i]["effect"][j]["type"];
      let mode: number = (Object(jsonObjectStr)["input_files"])[i]["effect"][j]["mode"];
      if (mode != undefined) {
        nodeInfo.mode = mode;
      }
      if ((Object(jsonObjectStr)["input_files"])[i]["effect"][j]["gains"] != undefined) {
        let gains: number[] = getNumberArrayFromString((Object(jsonObjectStr)["input_files"])[i]["effect"][j]["gains"]);
        nodeInfo.gains = gains;
      }
      inputFile.effect.push(nodeInfo);

    }
    sceneInfo.inputFiles.push(inputFile)
  }
  if (!Array.isArray(Object(jsonObjectStr)["expected_output"]["md5Value"])) {
    expectedOutput.md5Value.push((Object(jsonObjectStr)["expected_output"])["md5Value"]);
  } else {
    expectedOutput.md5Value = (Object(jsonObjectStr)["expected_output"])["md5Value"];
  }
  sceneInfo.expectedOutput = expectedOutput;
  for (let i = 0; i < mixerLen; i++) {
    let nodeInfo: NodeInfo = {
      type: '',
      gains: [],
      nodeInfoId: util.generateRandomUUID(true)
    }
    nodeInfo.type = (Object(jsonObjectStr)["mixer_info"])[i]["type"];
    let mode: number = (Object(jsonObjectStr)["mixer_info"])[i]["mode"];
    if (mode != undefined) {
      nodeInfo.mode = mode;
    }
    if ((Object(jsonObjectStr)["mixer_info"])[i]["gains"] != undefined) {
      let gains: number[] = getNumberArrayFromString((Object(jsonObjectStr)["mixer_info"])[i]["gains"]);
      nodeInfo.gains = gains;
    }

    sceneInfo.mixerInfo?.push(nodeInfo);
  }
  sceneInfo.outputFile.nameTemplate = Object(jsonObjectStr)["output_file"]["name_template"];
  sceneInfo.outputFile.format = (Object(jsonObjectStr)["output_file"])["format"];
  sceneInfo.outputFile.sampleRate = (Object(jsonObjectStr)["output_file"])["sample_rate"];
  sceneInfo.outputFile.bitDepth = (Object(jsonObjectStr)["output_file"])["bit_depth"];
  sceneInfo.outputFile.channels = (Object(jsonObjectStr)["output_file"])["channels"];

  return sceneInfo;
}

function getMD5OfFile(filePath: string): string {
  // 获取文件内容
  let file = fs.openSync(filePath, fs.OpenMode.READ_WRITE);
  let buf = new ArrayBuffer(40960);
  fs.readSync(file.fd, buf);
  fs.closeSync(file);

  let mdAlgName = 'MD5';
  let md = cryptoFramework.createMd(mdAlgName);
  md.updateSync({ data: new Uint8Array(buf) });
  let mdResult = md.digestSync();
  Logger.info(TAG, '[Sync]:Md result:' + mdResult.data);
  let md5Str = buffer.from(mdResult.data.buffer).toString('hex');
  Logger.info(TAG, '[Sync]:Md string result:' + md5Str);
  return md5Str;
}

function checkMd5IsEqual(outputFileMd5: string[], expectedFileMd5: string[]): boolean {
  let ret = true;
  outputFileMd5.forEach((outputMd5: string) => {
    if (expectedFileMd5.find((expected: string) => expected === outputMd5) == undefined) {
      ret = false;
    }
  })
  return ret;
}

@Builder
export function AutoTestBuilder() {
  AutoTest()
}

@Component
export struct AutoTest {
  @State text: string = "请选择预定义场景";
  @State index: number = 0;
  @State selectedFileSummary: string = "";
  @State selectOptionList: SelectOption[] = [];
  @State selectedNodeId: string = "";
  @State nodeInfoShow: boolean = false;
  @State choosedNode: NodeInfo = {
    nodeInfoId: '',
    type: ''
  }
  @State displayPipelineInfoList: DisplayPipelineInfo[] = [];
  @State autoTestProcessMap: Map<string, number> = new Map();
  @State exeOneStateEffect: boolean = true;
  @State executeProcessState: boolean = false;
  pageInfos: NavPathStack = new NavPathStack();

  async aboutToAppear(): Promise<void> {
    Logger.info(TAG, 'AutoTest aboutToAppear');
    try {
      let fileList: string[] = fs.listFileSync(rootPath);
      dirCheckList.forEach((check: string) => {
        if (!fileList.find((ele: string) => ele != "init.txt" && ele === check)) {
          Logger.info(TAG, `dir:${check} not existed`);
          let dirPath = `${rootPath}/${check}`;
          fs.mkdirSync(dirPath);
        }
      })
    } catch (e) {
      Logger.error(TAG, "rootPath is null, need init rootPath");
      await initAutoTestDir();
    }

    const dirPath = new fileUri.FileUri(rootPath + `/case_json`).path;
    let listFileOption: ListFileOptions = {
      filter: {
        suffix: [".json"]
      }
    };
    let fileList = fs.listFileSync(dirPath, listFileOption);
    fileList.forEach((jsonFile: string) => {
      let jsonFileInfo: JsonFileInfo = {
        jsonInfoId: util.generateRandomUUID(true),
        filePath: '',
        sceneInfos: [],
        summary: '',
        realTime: false
      }
      const jsonFilePath = `${rootPath}/case_json/${jsonFile}`
      jsonFileInfo.filePath = jsonFilePath;
      try {
        let file = fs.openSync(jsonFilePath, fs.OpenMode.READ_WRITE);
        let fsStat = fs.statSync(jsonFilePath);
        let buf = new ArrayBuffer(fsStat.size);
        fs.readSync(file.fd, buf);
        let fileData: string = buffer.from(buf).toString();
        let obj = JSON.parse(fileData);
        jsonFileInfo.summary = Object(obj)["summary"];
        this.selectOptionList.push({ value: jsonFileInfo.summary });
        jsonSummaryToJsonFileInfoMap.set(jsonFileInfo.summary, jsonFileInfo);
        const caseNum: number = Object(obj)["scenes"]["length"];
        if (caseNum > 0) {
          for (let i = 0; i < caseNum; i++) {
            const sceneInfo: SceneInfo = parseJsonObject(Object(obj)["scenes"][i]);
            sceneInfoIdToSceneInfoMap.set(sceneInfo.sceneInfoId, sceneInfo);
            jsonFileInfo.sceneInfos.push(sceneInfo.sceneInfoId);
          }
        }
      } catch (e) {
        Logger.error(TAG, `Open file:${jsonFilePath} Failed, error info:${e}`)
      }
    })
    if (jsonSummaryToJsonFileInfoMap.size > 0 && sceneInfoIdToSceneInfoMap.size > 0) {
      Logger.info(TAG, "Parse json file success");
    } else {
      Logger.error(TAG, "Parse json file failed");
    }
  }

  aboutToDisappear(): void {
    Logger.info(TAG, 'aboutToDisappear');
    // 销毁管线，销毁引擎
    this.destroyPipeline();
  }

  build() {
    NavDestination() {
      Flex({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
        Column() {
          Text('选择场景：')
        }.width('30%')
      }.height('5%')

      Row() {
        Select(this.selectOptionList)
          .width('100%')
          .selected(this.index)
          .value(this.text)
          .font({ size: 16, weight: 500 })
          .fontColor('#182431')
          .selectedOptionFont({ size: 16, weight: 400 })
          .optionFont({ size: 16, weight: 400 })
          .menuAlign(MenuAlignType.START, { dx: 0, dy: 0 })
          .optionWidth(350)
          .optionHeight(300)
          .onSelect((index: number, text: string) => {
            this.index = index;
            this.text = text
            this.selectedFileSummary = text;
            const sceneInfos = jsonSummaryToJsonFileInfoMap.get(text)?.sceneInfos;
            if (sceneInfos == undefined) {
              return;
            }
            this.displayPipelineInfoList = sceneInfos.map((sceneInfoId: string): DisplayPipelineInfo => {
              const sceneInfo = sceneInfoIdToSceneInfoMap.get(sceneInfoId);
              const displayPipelineInfo: DisplayPipelineInfo = {
                effectChain: new Map(),
                mixerChain: [],
                inputNodeIdList: []
              }
              if (sceneInfo == undefined) {
                return displayPipelineInfo;
              }

              sceneInfo?.inputFiles?.forEach((inputFile: InputFile) => {
                let inputNode: NodeInfo = {
                  nodeInfoId: util.generateRandomUUID(true),
                  type: "INPUT",
                  format: inputFile.format,
                  sampleRate: inputFile.sampleRate,
                  bitDepth: inputFile.bitDepth,
                  channels: inputFile.channels
                }
                nodeInfoIdToNodeInfoMap.set(inputNode.nodeInfoId, inputNode);
                displayPipelineInfo.inputNodeIdList.push(inputNode.nodeInfoId);
                displayPipelineInfo.effectChain.set(inputNode.nodeInfoId, []);
                inputFile.effect.forEach((effectNode: NodeInfo) => {
                  nodeInfoIdToNodeInfoMap.set(effectNode.nodeInfoId, effectNode);
                  displayPipelineInfo.effectChain.get(inputNode.nodeInfoId)?.push(effectNode.nodeInfoId);
                })
              })
              sceneInfo?.mixerInfo?.forEach((mixerNode: NodeInfo) => {
                nodeInfoIdToNodeInfoMap.set(mixerNode.nodeInfoId, mixerNode);
                displayPipelineInfo.mixerChain.push(mixerNode.nodeInfoId)
              })

              let outputNode: NodeInfo = {
                nodeInfoId: util.generateRandomUUID(true),
                type: "OUTPUT",
                format: sceneInfo.outputFile.format,
                sampleRate: sceneInfo.outputFile.sampleRate,
                bitDepth: sceneInfo.outputFile.bitDepth,
                channels: sceneInfo.outputFile.channels
              }
              nodeInfoIdToNodeInfoMap.set(outputNode.nodeInfoId, outputNode);
              displayPipelineInfo.mixerChain.push(outputNode.nodeInfoId)
              return displayPipelineInfo;
            })
          })
      }.height('5%')


      Column() {
        if (this.displayPipelineInfoList.length > 0) {
          List() {
            ForEach(this.displayPipelineInfoList, (displayPipelineInfo: DisplayPipelineInfo, index: number) => {
              ListItem() {
                this.PipelineGraphArea(displayPipelineInfo, index+1)
              }
            })
          }
        }
      }.height('75%').width('100%').padding(2)

      // .border({ width: '2vp', color: Color.Pink, radius: '10vp' })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceAround, alignItems: ItemAlign.Center }) {
        Button() {
          Text(this.exeOneStateEffect?`执行选中用例`:`执行中...`)
        }
        .onClick(async () => {
          this.exeOneStateEffect = false;
          this.executeProcessState = true;
          await this.executeSingleFileWithSyncInit(`${this.selectedFileSummary}`);
          this.exeOneStateEffect = true;
        })
        .backgroundColor(0x317aff)
        .borderRadius(10)
        .width(100)
        .height(50)
        .enabled(this.exeOneStateEffect)

        Button() {
          Text(`执行全部用例`)
        }
        .onClick(() => {
          this.executeAllFile();
        })
        .backgroundColor(0x317aff)
        .borderRadius(10)
        .width(100)
        .height(50)
        .margin({ left: 20 })
      }.height('20%').width('100%')

    }
    .bindSheet(this.executeProcessState && this.autoTestProcessMap.size > 0, this.ExecuteProcessDialog(), {
      height: '100%',
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
        this.executeProcessState = false;
      },
      onWillDismiss: (DismissSheetAction: DismissSheetAction) => {
        if (DismissSheetAction.reason != DismissReason.TOUCH_OUTSIDE) {
          DismissSheetAction.dismiss()
        }
      }
    })
    .onReady((context: NavDestinationContext) => {
      this.pageInfos = context.pathStack;
    })
    .title('自动化测试')
    .height('100%')
    .width('100%')
  }

  @Builder
  PipelineGraphArea(displayPipelineInfo: DisplayPipelineInfo, index: number) {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
      Row() {
        Text(`pipeline ${index}`).fontColor($r('app.color.index_audio_clip_card'))
      }.width('100%').justifyContent(FlexAlign.Center)
      this.EffectGraphArea(displayPipelineInfo.inputNodeIdList, displayPipelineInfo.effectChain)
      this.MixerGraphArea(displayPipelineInfo.mixerChain)
    }
    .border({ width: '3vp', color: $r('app.color.index_audio_clip_card'), radius: '10vp' })
    .padding({ left: '10vp', right: '10vp' })
    .margin({ bottom: '10vp' })
    .bindSheet(this.nodeInfoShow, this.NodeInfoDialog(this.makeNodeMapNotBlank(this.selectedNodeId)), {
      height: '50%',
      backgroundColor: ($r('app.color.selected_node_info_background_color')),
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

  @Builder
  EffectGraphArea(inputNodeList: string[], effectChainMap: Map<string, string[]>) {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
      if (this.displayPipelineInfoList.length > 0) {
        Row() {
          Text("Effect Area")
        }
      }
      Column() {
        ForEach(inputNodeList, (inputNodeId: string) => {
          Row() {
            List() {
              ListItem() {
                this.NodeButton(this.makeNodeMapNotBlank(inputNodeId))
              }

              ForEach(effectChainMap.get(inputNodeId), (effectNodeId: string) => {
                ListItem() {
                  this.NodeButton(this.makeNodeMapNotBlank(effectNodeId))
                }
              })
            }
            .width('100%')
            .listDirection(Axis.Horizontal)
            .alignListItem(ListItemAlign.Center)
            .focusWrapMode(FocusWrapMode.WRAP_WITH_ARROW)
            .scrollBar(BarState.Off)
            .margin({ bottom: inputNodeList.length > 1 ? 10 : 0 })
          }
        })
      }
      .border({ width: '1vp', color: Color.Blue, radius: '10vp' })
      .backgroundColor(Color.Gray)

    }
  }

  @Builder
  MixerGraphArea(mixerNodeChain: string[]) {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
      if (this.displayPipelineInfoList.length > 0) {
        Row() {
          Text("Output Area")
        }
      }

      Row() {
        List() {
          ForEach(mixerNodeChain, (nodeId: string) => {
            ListItem() {
              this.NodeButton(this.makeNodeMapNotBlank(nodeId))
            }
          })
        }
        .width('100%')
        .listDirection(Axis.Horizontal)
        .alignListItem(ListItemAlign.Center)
        .focusWrapMode(FocusWrapMode.WRAP_WITH_ARROW)
        .scrollBar(BarState.Off)
        .border({ width: '1vp', color: Color.Blue, radius: '10vp' })
        .backgroundColor(Color.Gray)
      }
    }
    .margin({ bottom: 5 })

  }

  @Builder
  NodeButton(nodeInfo: NodeInfo) {
    Column() {
      Button() {
        Text(`${getNodeTypeFromString(nodeInfo.type)}`)
          .wordBreak(WordBreak.BREAK_WORD)
          .lineBreakStrategy(LineBreakStrategy.BALANCED)
      }
      .backgroundColor(colorMap.get(getNodeTypeFromString(nodeInfo.type)))
      .borderRadius(10)
      .width(40)
      .height(20)
      .margin({ left: 10 })
    }
    .onClick(() => {
      this.nodeInfoShow = true;
      this.selectedNodeId = nodeInfo.nodeInfoId;
    })
  }

  @Builder
  NodeInfoDialog(nodeInfo: NodeInfo) {
    Column() {
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start }) {
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
        }.width('50%')
      }

      Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceAround }) {
        if (nodeInfo.type) {
          Text(`node_type: ${nodeInfo.type}`).fontSize(20)
        }
        if (nodeInfo.mode) {
          Text(`mode: ${nodeInfo.mode}`).fontSize(20)
        }
        if (nodeInfo.gains && nodeInfo.gains.length > 0) {
          Text(`gains: ${nodeInfo.gains.toString()}`).fontSize(20)
        }
        if (nodeInfo.format) {
          Text(`format: ${nodeInfo.format}`).fontSize(20)
        }
        if (nodeInfo.sampleRate) {
          Text(`sampleRate: ${nodeInfo.sampleRate}`).fontSize(20)
        }
        if (nodeInfo.channels) {
          Text(`channels: ${nodeInfo.channels}`).fontSize(20)
        }
        if (nodeInfo.bitDepth) {
          Text(`bitDepth: ${nodeInfo.bitDepth}`).fontSize(20)
        }
      }.height('80%')
    }

  }

  @Builder
  ExecuteProcessDialog() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceBetween }) {
      Row() {
        Image($r('app.media.close_window'))
          .width(20)
          .height('8%')
          .onClick(() => {
            this.executeProcessState = false;
          })
        Text(`用例执行进度`).fontColor($r('app.color.index_audio_clip_card')).fontSize(30).margin({left:50})
      }

      Text(`${this.selectedFileSummary}`).fontColor($r('app.color.index_audio_clip_card')).fontSize(25)
      ForEach(Array.from(this.autoTestProcessMap.keys()), (pipelineId:string)=>{
        Text(`pipeline：${pipelineIdToPipelineInfoMap.get(pipelineId)?.sceneInfoName}`).fontColor($r('app.color.index_audio_clip_card'))
        Row() {
          Progress({ value: this.autoTestProcessMap.get(pipelineId), total: 100, type: ProgressType.Linear }).width(200).height(50)
          Text(`${this.autoTestProcessMap.get(pipelineId)}%`)
            .fontSize(20)
            .margin({ left: 20 })
        }
      })
    }
  }

  async executeAllFile() {
    Logger.info(TAG, "executeAllFile start");
    const jsonFiles = Array.from(jsonSummaryToJsonFileInfoMap.values());
    for (const jsonFile of jsonFiles) {
      Logger.info(TAG, `before task jsonFile=${jsonFile.summary}`);
      this.selectedFileSummary = jsonFile.summary;
      let ret = await this.executeSingleFileWithSyncInit(jsonFile.summary, false);
      Logger.info(TAG, `before task ret = ${ret}`);
    }
    await this.saveAllCaseExecuteReport();
    this.destroyPipeline();
    Logger.info(TAG, "executeAllFile end");
  }

  async executeSingleFileWithSyncInit(fileSummary: string, reportFlag: boolean = true): Promise<number> {
    Logger.info(TAG, `executeSingleFileWithSyncInit start fileSummary:${fileSummary}`);
    this.autoTestProcessMap.clear();
    const jsonFileInfo = jsonSummaryToJsonFileInfoMap.get(fileSummary);
    if (jsonFileInfo == undefined || jsonFileInfo.sceneInfos.length == 0) {
      return FAILED;
    } else if (jsonFileInfo.sceneInfos.length >= 1) {
      let status = this.initAllPipeline(jsonFileInfo);
      if (status != SUCCESS) {
        Logger.error(TAG, `initAllPipeline ERROR`);
        return FAILED;
      }
      let taskExeStatus = await taskpool.execute(initAllInputNode, jsonFileInfo, sceneInfoIdToSceneInfoMap, pipelineIdToPipelineInfoMap);
      if (taskExeStatus != SUCCESS) {
        Logger.error(TAG, `initAllInputNode ERROR`);
        return FAILED;
      }
      Logger.info(TAG, `initAllInputNode successed, ready to initAllEffectNode`);
      status = this.initAllEffectNode(jsonFileInfo);
      if (status != SUCCESS) {
        Logger.error(TAG, `initAllEffectNode ERROR`);
        return FAILED;
      }
      Logger.info(TAG, `initAllEffectNode successed, ready to render frame`);
      let timer = setInterval(()=>{
        Logger.error(TAG, `getAutoTestProcess start`);
        let processObj: Record<string, number> = audioNapi.getAutoTestProcess();
        let newMap = new Map(Object.entries(processObj));
        let continueFlag: boolean = false;
        if (newMap.size == 0) {
          Logger.error(TAG, `newMap size is 0`);
          clearInterval(timer);
          this.destroyPipeline();
          return;
        }
        Logger.error(TAG, `newMap size is ${newMap.size}`);
        if (this.autoTestProcessMap.size == 0) {
          this.autoTestProcessMap = new Map(newMap);
        } else if (newMap.size >= this.autoTestProcessMap.size) {
          this.autoTestProcessMap = new Map(newMap);
        }

        this.autoTestProcessMap.forEach((value, key)=>{
          Logger.error(TAG, `key:${key}----value:${value}`);
          if (value < 100) {
            continueFlag = true;
          }
        })
        if (!continueFlag) {
          Logger.info(TAG, `process done, ready to clear timer.`);
          clearInterval(timer);
          this.destroyPipeline();
        }
      }, 200);
      let scenePromises = jsonFileInfo.sceneInfos.map(async (sceneInfoId: string) => {
        const sceneInfo = sceneInfoIdToSceneInfoMap.get(sceneInfoId);
        const pipelineId = sceneInfo?.pipelineId;
        if (sceneInfo == undefined || pipelineId == undefined) {
          Logger.error(TAG, `executeSingleFile get null sceneInfo`);
          return FAILED;
        }
        const pipelineInfo = pipelineIdToPipelineInfoMap.get(pipelineId);
        if (pipelineInfo == undefined) {
          Logger.error(TAG, `executeSingleFile get null pipelineInfo`);
          return FAILED;
        }

        let outputInfo = sceneInfo.outputFile;
        const multiRenderFrameFlag = pipelineInfo.multiRenderFrameFlag;
        if (jsonFileInfo.realTime) {
          const buffer: object = await taskpool.execute(multiPipelineRealTimeSaveFileBuffer, pipelineId, outputInfo, multiRenderFrameFlag);
          await this.saveSingleFile(pipelineId, buffer as ArrayBuffer[], ".wav", outputInfo, multiRenderFrameFlag)
        } else {
          const buffer: object = await taskpool.execute(multiPipelineSaveFileBuffer, pipelineId, outputInfo, multiRenderFrameFlag);
          await this.saveSingleFile(pipelineId, buffer as ArrayBuffer[], ".wav", outputInfo, multiRenderFrameFlag)
        }
        this.generateSingleReportInfo(pipelineId);
        return SUCCESS;
      })
      await Promise.all(scenePromises).then(async () => {
        Logger.info(TAG, `scenePromises done `);
        if (reportFlag) {
          await this.saveSingleCaseExecuteReport(fileSummary);
        }
      })
    }
    return SUCCESS;
  }
  // sync init pipeline/inputNode/effectNode
  initAllPipeline(jsonFileInfo:JsonFileInfo): number {
    Logger.info(TAG, `initAllPipeline start`);
    for (const sceneInfoId of jsonFileInfo.sceneInfos) {
      const sceneInfo = sceneInfoIdToSceneInfoMap.get(sceneInfoId);
      if (sceneInfo == undefined) {
        return FAILED;
      }
      const pipelineId = util.generateRandomUUID(true);
      const pipelineInfo: PipelineInfo = {
        pipelineId: pipelineId,
        inputIdList: [],
        outputId: util.generateRandomUUID(true),
        mixerId: util.generateRandomUUID(true),
        multiRenderFrameFlag: false,
        sceneInfoId: sceneInfoId,
        sceneInfoName: sceneInfo.sceneName
      }
      const start = new Date().getTime();
      const singleReport: ExecuteReport = {
        sceneInfoId: jsonFileInfo.sceneInfos[0],
        sceneName: sceneInfo.sceneName,
        startTime: getDateStringWithTimeStamp(start, 0),
        startTimeStamp: start,
        endTime: '',
        endTimeStamp: 0,
        costTime: 0,
        result: false,
        exeReportPath: '',
      }
      pipelineIdToSingleReportMap.set(pipelineId, singleReport);
      pipelineIdToPipelineInfoMap.set(pipelineId, pipelineInfo);
      sceneInfo.pipelineId = pipelineId;
      let pipelineStatus = audioNapi.audioEditNodeInitMultiPipeline(pipelineId)
      if (pipelineStatus != SUCCESS) {
        Logger.error(TAG, `pipeline init ERROR, pipelineId :${pipelineId}`);
        return FAILED;
      }
      sceneInfo.pipelineId = pipelineId;
      preparedPipelineList.push(pipelineId);
      pipelineIdToPipelineInfoMap.set(pipelineId, pipelineInfo);
      Logger.info(TAG, `pipeline inited, pipelineId: ${pipelineId}`);
    }
    Logger.info(TAG, `all pipeline inited`);
    return SUCCESS;
  }

  initAllEffectNode(jsonFileInfo:JsonFileInfo): number {
    Logger.info(TAG, `initAllEffectNode start`);
    for (const sceneInfoId of jsonFileInfo.sceneInfos) {
      const sceneInfo = sceneInfoIdToSceneInfoMap.get(sceneInfoId);
      const pipelineId = sceneInfo?.pipelineId;
      if (sceneInfo == undefined || pipelineId == undefined) {
        Logger.error(TAG, `initAllEffectNode get null sceneInfo`);
        return FAILED;
      }
      const pipelineInfo = pipelineIdToPipelineInfoMap.get(pipelineId);
      if (pipelineInfo == undefined) {
        Logger.error(TAG, `initAllEffectNode get null pipelineInfo`);
        return FAILED;
      }
      let status = audioNapi.multiPipelineEnvPrepare(pipelineId);
      if (status != SUCCESS) {
        Logger.error(TAG, `multiPipelineEnvPrepare ERROR while initAllInputNode`);
        return FAILED;
      }
      for (const inputFile of sceneInfo.inputFiles) {
        const inputId = inputFile.inputId;
        let effectIndex = 0;
        for (const effect of inputFile.effect) {
          const effectObj = inputFile.effectStr[effectIndex++];
          const nodeType: NodeType = getNodeTypeFromString(effect.type);
          switch (nodeType) {
            case NodeType.NR:
              let NRId: string = util.generateRandomUUID(true);
              let status: number = audioNapi.addNoiseReduction(NRId, inputId);
              if (status != SUCCESS) {
                Logger.error(TAG, `addNoiseReduction error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `addNoiseReduction node successful!!!`);
              }
              break;
            case NodeType.SEPARATION:
              let AISSId: string = util.generateRandomUUID(true);
              status = audioNapi.addAudioSeparation(0, AISSId, inputId);
              if (status != SUCCESS) {
                Logger.error(TAG, `addAudioSeparation error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `addAudioSeparation node successful!!!`);
              }
              pipelineInfo.multiRenderFrameFlag = true
              break;
            case NodeType.ENV:
              let ENVId: string = util.generateRandomUUID(true);
              const envMode:number = Object(effectObj)["mode"];
              Logger.info(TAG, `multiStartEnvEffect mode=${envMode}`);
              status = audioNapi.startEnvEffect(inputId, ENVId, envMode);
              if (status != SUCCESS) {
                Logger.error(TAG, `multiStartEnvEffect error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `multiStartEnvEffect node successful!!!`);
              }
              break;
            case NodeType.FIELD:
              let FIELDId: string = util.generateRandomUUID(true);
              const fieldMode:number = Object(effectObj)["mode"];
              Logger.info(TAG, `multiStartFieldEffect mode=${fieldMode}`);
              status = audioNapi.startFieldEffect(inputId, fieldMode, FIELDId);
              if (status != SUCCESS) {
                Logger.error(TAG, `startFieldEffect error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `startFieldEffect node successful!!!`);
              }
              break;
            case NodeType.VB:
              let VBId: string = util.generateRandomUUID(true);
              const vbMode:number = Object(effectObj)["mode"];
              Logger.info(TAG, `multiStartVBEffect mode=${vbMode}`);
              status = audioNapi.startVBEffect(inputId, vbMode, VBId);
              if (status != SUCCESS) {
                Logger.error(TAG, `startVBEffect error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `startVBEffect node successful!!!`);
              }
              break;
            case NodeType.EQ:
              let EQId: string = util.generateRandomUUID(true);
              const eqMode = effect.mode;
              Logger.info(TAG, `multiStartVBEffect mode=${eqMode}`);
              if (effect.gains == undefined || effect.gains.length == 0) {
                status = audioNapi.setEquailizerMode(eqMode, EQId, inputId);
              } else {
                status = audioNapi.setEqualizerFrequencyBandGains(effect.gains, EQId, inputId);
              }
              if (status != SUCCESS) {
                Logger.error(TAG, `EQ Effect error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `EQ Effect node successful!!!`);
              }
              break;
            case NodeType.GENERAL_VOICE_CHANGE:
              let generalId: string = util.generateRandomUUID(true);
              const generalMode = effect.mode;
              Logger.info(TAG, `startGeneralVoiceChange mode=${generalMode}`);
              status = audioNapi.startGeneralVoiceChange(inputId, generalMode, generalId);
              if (status != SUCCESS) {
                Logger.error(TAG, `startGeneralVoiceChange error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `startGeneralVoiceChange successful!!!`);
              }
              break;
            case NodeType.PURE_VOICE_CHANGE:
              let pureId: string = util.generateRandomUUID(true);
              const gender:number = Object(effectObj)["gender"];
              const changeType:number = Object(effectObj)["changeType"];
              status = audioNapi.startPureVoiceChange(inputId, pureId, gender, 0, changeType);
              if (status != SUCCESS) {
                Logger.error(TAG, `startPureVoiceChange error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `startPureVoiceChange successful!!!`);
              }
              break;
            case NodeType.SOUND_SPEED_TONE:
              let soundSpeedToneId: string = util.generateRandomUUID(true);
              const pureMode = effect.mode;
              Logger.info(TAG, `startPureVoiceChange mode=${pureMode}`);
              const soundSpeed:number = Object(effectObj)["soundSpeed"];
              const soundTone:number = Object(effectObj)["soundTone"];
              status = audioNapi.setSoundSpeedTone(inputId, soundSpeedToneId, soundSpeed, soundTone);
              if (status != SUCCESS) {
                Logger.error(TAG, `setSoundSpeedTone error number is ${status}`);
                return FAILED;
              } else {
                Logger.info(TAG, `setSoundSpeedTone successful!!!`);
              }
              break;
            // case NodeType.SPACE_RENDER:
            //   let spaceRenderId: string = util.generateRandomUUID(true);
            //   const spaceRenderMode = effect.mode;
            //   const x:number = Object(effectObj)["x"];
            //   const y:number = Object(effectObj)["y"];
            //   const z:number = Object(effectObj)["z"];
            //   status = FAILED;
            //   if (spaceRenderMode === 0) {
            //     status = audioNapi.startFixedPositionEffect(x, y, z, spaceRenderId, inputId);
            //   }
            //   if (spaceRenderMode === 1) {
            //     const surroundTime:number = Object(effectObj)["surroundTime"];
            //     const surroundDirection:number = Object(effectObj)["surroundDirection"];
            //     status = audioNapi.startDynamicRenderEffect(x,  y, z, surroundTime, surroundDirection, spaceRenderId, inputId);
            //   }
            //   if (spaceRenderMode === 2) {
            //     const extRadius:number = Object(effectObj)["extRadius"];
            //     const extAngle:number = Object(effectObj)["extAngle"];
            //     status = audioNapi.startExpandEffect(extRadius, extAngle, spaceRenderId, inputId);
            //   }
            //   if (status != SUCCESS) {
            //     Logger.error(TAG, `SPACE_RENDER error number is ${status}`);
            //     return FAILED;
            //   } else {
            //     Logger.info(TAG, `add SPACE_RENDER successful!!!`);
            //   }
            //   break;
          }
        }
        Logger.info(TAG, `all effect node of input inited, inputId :${inputId}, pipelineId:${pipelineId}`);
      }
      Logger.info(TAG, `all effect node of pipeline inited, pipelineId:${pipelineId}`);
    }
    return SUCCESS;
  }

  destroyPipeline() {
    Logger.info(TAG, "destroyPipeline start");
    if (preparedPipelineList.length == 0) {
      return;
    }
    preparedPipelineList.forEach((pipelineId: string) => {
      pipelineIdToPipelineInfoMap.get(pipelineId)?.inputIdList.forEach((inputId: string) => {
        audioNapi.multiPipelineEnvPrepare(pipelineId);
        audioNapi.multiDeleteSong(inputId);
      })
    })
    if (preparedPipelineList.length >= 1) {
      audioNapi.destroyMultiPipeline();
    } else {
      Logger.error(TAG, 'pipelineList is blank while destroy');
    }
    preparedPipelineList.length = 0;
    Logger.info(TAG, "destroyPipeline end");
  }

  async saveSingleFile(pipelineId: string, pcmBuffer: ArrayBuffer[], audioFormate: string, outputInfo: OutputFile,
    multiRenderFrameFlag: boolean): Promise<number> {

    pcmBuffer.forEach((value: ArrayBuffer, index) => {
      Logger.info(TAG, `saveSingleFile pcmBuffer of ---${index}---length : ${value.byteLength}`)
    })
    try {
      // let newFileName1 = `${outputInfo.nameTemplate}${multiRenderFrameFlag ? "_Voice" :
      //   ""}_${getDateStringWithTimeStamp(new Date().getTime())}.wav`;
      // let newFileName2 = `${outputInfo.nameTemplate}${multiRenderFrameFlag ? "_Accompaniment" :
      //   ""}_${getDateStringWithTimeStamp(new Date().getTime())}.wav`;
      let newFileName1 = `${outputInfo.nameTemplate}.wav`;
      let newFileName2 = `${outputInfo.nameTemplate}.wav`;
      const outputFilePath = multiRenderFrameFlag ? [`${outputPath}/${newFileName1}`, `${outputPath}/${newFileName2}`] :
        [`${outputPath}/${newFileName1}`];
      outputFilePath.forEach((path: string, index: number) => {
        let file = fs.openSync(path, fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
        Logger.info(TAG, `SaveBuffer filePath : ${outputFilePath}`);
        if (audioFormate === '.wav') {
          // 处理wav
          let wavBuffer =
            writeWavFileHeader(pcmBuffer[index], outputInfo.sampleRate, outputInfo.channels, outputInfo.bitDepth);
          let writeLen = fs.writeSync(file.fd, wavBuffer);
          Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
          fs.closeSync(file);
        } else {
          // 处理pcm
          let writeLen = fs.writeSync(file.fd, pcmBuffer[0]);
          Logger.info(TAG, `SavePcmBuffer writeLen : ${writeLen}`);
          fs.closeSync(file);
        }
      })

      outputInfo.outputFileList = outputFilePath;
      const sceneInfoId = pipelineIdToPipelineInfoMap.get(pipelineId)?.sceneInfoId;
      if (sceneInfoId === undefined) {
        return 1;
      }
      let expectedFileMd5: string[] | undefined =
        sceneInfoIdToSceneInfoMap.get(sceneInfoId)?.expectedOutput.md5Value;


      let outputFileMd5: string[] = [];
      try {
        outputFileMd5 = outputFilePath.map((path: string) => {
          return getMD5OfFile(path);
        })
      } catch (e) {
        Logger.error(TAG, `Get MD5 failed, error : ${e}`)
      }
      if (expectedFileMd5 != undefined && !checkMd5IsEqual(outputFileMd5, expectedFileMd5)) {
        Logger.error(TAG,
          `file MD5 compare ERROR --- file:${outputFilePath}; md5Value:${outputFileMd5}; expectedMd5:${expectedFileMd5}`);
        this.getUIContext().showAlertDialog({
          message: '输出文件与预期不符失败\n请单击弹窗外任意处关闭弹窗',
          autoCancel: true
        })
      } else {
        Logger.info(TAG,
          `file MD5 compare SUCCESS --- file1:${outputFilePath}; md5Value:${outputFileMd5};`);
      }
      return 0;
    } catch (e) {
      Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
      return 1;
    }
  }

  makeNodeMapNotBlank(nodeId: string): NodeInfo {
    let nodeInfo = nodeInfoIdToNodeInfoMap.get(nodeId);
    if (nodeInfo == undefined) {
      nodeInfo = { nodeInfoId: "", type: "" }
    }
    return nodeInfo;
  }

  generateSingleReportInfo(pipelineId: string) {
    const endTime = new Date().getTime();
    const csvFileSavePath = `execute_report_${getDateStringWithTimeStamp(endTime)}.csv`;
    const singleReport = pipelineIdToSingleReportMap.get(pipelineId);
    if (singleReport == undefined) {
      return;
    }
    singleReport.endTimeStamp = endTime;
    singleReport.endTime = getDateStringWithTimeStamp(endTime, 0);
    singleReport.costTime = endTime - singleReport.startTimeStamp;
    singleReport.result = true;
    singleReport.exeReportPath = csvFileSavePath;
    Logger.info(TAG, `singleReport of pipeline:${pipelineId} generated;`);
  }

  async saveSingleCaseExecuteReport(summary: string) {
    Logger.info(TAG, `saveSingleCaseExecuteReport start summary=${summary}`);
    const csvFileSavePath = `${outputPath}/execute_report_${getDateStringWithTimeStamp(new Date().getTime())}.csv`;
    let csvFileInfoText = await this.getCsvTextOfSingeCase(summary);
    let csvText = `执行用例: ${summary},,,,,,,\n` + csvReportHeaderText + csvFileInfoText + ",,,,,,,\n";
    const csvTextWithBom = this.addBOMToStartOfString(csvText);
    let file = fs.openSync(csvFileSavePath,
      fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
    let writeLen = fs.writeSync(file.fd, csvTextWithBom);
    fs.closeSync(file);
    Logger.info(TAG, `Save CSV file suncess writeLen : ${writeLen}`);
  }

  async saveAllCaseExecuteReport() {
    Logger.info(TAG, "saveAllCaseExecuteReport start");
    const csvFileSavePath = `${outputPath}/execute_report_${getDateStringWithTimeStamp(new Date().getTime())}.csv`;
    let csvText = ``;
    const summaryArray = Array.from(jsonSummaryToJsonFileInfoMap.keys());
    const promises = summaryArray.map(async (summary: string) => {
      let csvFileInfoText = await this.getCsvTextOfSingeCase(summary);
      csvText += `执行用例: ${summary},,,,,,,\n` + csvReportHeaderText + csvFileInfoText + ",,,,,,,\n";
      return Promise.resolve();
    })
    await Promise.all(promises);
    const csvTextWithBom = this.addBOMToStartOfString(csvText);
    let file = fs.openSync(csvFileSavePath,
      fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
    let writeLen = fs.writeSync(file.fd, csvTextWithBom);
    fs.closeSync(file);
    Logger.info(TAG, `Save CSV file suncess writeLen : ${writeLen}`);
  }

  async getCsvTextOfSingeCase(summary: string): Promise<string> {
    let csvText = ``;
    let texts: string[] = [];
    const promises = jsonSummaryToJsonFileInfoMap.get(summary)?.sceneInfos.map(async (scene) => {
      let pipelineId = sceneInfoIdToSceneInfoMap.get(scene)?.pipelineId;
      if (pipelineId == undefined) {
        return;
      }
      let singleReport = pipelineIdToSingleReportMap.get(pipelineId)
      if (singleReport == undefined) {
        return;
      }
      let text = await this.getCsvTextFromSingleReport(singleReport);
      texts.push(text);
    })
    if (promises == undefined) {
      return "VALID";
    }
    await Promise.all(promises);
    texts.forEach((text) => {
      csvText += `${text}\n`;
    })
    return csvText;
  }

  async getCsvTextFromSingleReport(singleReport: ExecuteReport): Promise<string> {
    let inputFileReportText = ``;
    let outputFileReportText = ``;
    let inputDurationText = ``;
    await this.getMetadataCsvTextOfInputFiles(sceneInfoIdToSceneInfoMap.get(singleReport.sceneInfoId)?.inputFiles)
      .then((inputCsvRet: string[]) => {
        inputFileReportText = inputCsvRet[0];
        inputDurationText = inputCsvRet[1];
      })
    await this.getMetadataCsvTextOfOutputFiles(sceneInfoIdToSceneInfoMap.get(singleReport.sceneInfoId)?.outputFile.outputFileList)
      .then((outputCsvRet: string) => {
        outputFileReportText = outputCsvRet;
      })

    let ret: string =
      `"${singleReport.sceneName}","${singleReport.startTime}","${singleReport.endTime}","${singleReport.costTime /
        1000} s","${inputFileReportText}","${inputDurationText}","${outputFileReportText}","${singleReport.result ?
        "执行成功" : "执行异常"}"`;
    return ret;
  }

  async getMetadataCsvTextOfInputFiles(inputFiles: InputFile[] | undefined): Promise<string[]> {
    if (inputFiles == undefined) {
      return ["InputFiles open FAILED", ""];
    }
    let ret: string = ``;
    let durationStr: string = ``;
    const promises = inputFiles.map(async (input) => {
      Decimal.set({
        precision: 4,
      });
      let inputFilePath = `${inputDir}/${input.path}`;
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
      return media.createAVMetadataExtractor().then(async (avMetadataExtractor) => {
        // 设置fdSrc
        avMetadataExtractor.fdSrc = fs.openSync(inputFilePath);
        // 获取元数据（promise模式）
        let metadata = await avMetadataExtractor.fetchMetadata();
        let fileLen = new Decimal(fsStat.size / 1024 / 1024 * 100).ceil();
        console.info(TAG, `get meta data, media length: ${Number(metadata.duration) / 1000} s`);
        ret += `--------------------------\r
        输入文件路径: ${inputFilePath}\r
        文件大小: ${(Number(fileLen) / 100).toString()} MB\r
        音频长度: ${Number(metadata.duration) / 1000} s\r
        音频格式(采样率/声道数/位深): ${sampleRate}/${channels}/${bitsPerSample}\r
        `
        durationStr += `--------------------------\r
      ${Number(metadata.duration) / 1000} s\r
      `
      })
    })
    await Promise.all(promises);
    return [ret, durationStr];
  }

  async getMetadataCsvTextOfOutputFiles(outputFiles: string[] | undefined): Promise<string> {
    if (outputFiles == undefined) {
      return "OutputFiles open FAILED";
    }
    let ret: string = ``;
    for (const outputPath of outputFiles) {
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
      let fileLen = new Decimal(fsStat.size / 1024 / 1024 * 100).ceil();
      console.info(TAG, `get meta data, media length: ${Number(metadata.duration) / 1000} s`);
      ret += `--------------------------\r
      输出文件路径: ${outputPath}\r
      文件大小: ${(Number(fileLen) / 100).toString()} MB\r
      音频长度: ${Number(metadata.duration) / 1000} s\r
      音频格式(采样率/声道数/位深): ${sampleRate}/${channels}/${bitsPerSample}\r
      `
    }
    return new Promise((resolve: Function) => {
      resolve(ret);
    });
  }

  addBOMToStartOfString(originStr:string):string {
    const encoder = util.TextEncoder.create();
    const bom = encoder.encode('\uFEFF');
    const content = encoder.encode(originStr);
    const resultArray = new Uint8Array(bom.length + content.length);
    resultArray.set(bom, 0);
    resultArray.set(content, bom.length);
    const decoder = util.TextDecoder.create();
    return decoder.decode(resultArray);
  }
}