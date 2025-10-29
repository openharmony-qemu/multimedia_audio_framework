/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. All rights reserved.
 */
import { StyleConst } from "../utils/StyleConst";
import { Node, NodeType, colorMap, SongInfo } from "../utils/importSongs/InterfaceInfo";
import { Logger } from '../utils/Logger';
import audioNapi from 'libentry.so';
import { importSingleSong } from '../utils/importSongs/ImportSongs';
import { SetFormatDialog } from './customDialog/SetFormatDialog';
import { CheckNodeDialog } from './customDialog/CheckNodeDialog';
import { JSON, util } from '@kit.ArkTS';
import { saveFileBuffer, RealTimeSaveFileBuffer } from '../utils/ExportFile';
import { taskpool } from '@kit.ArkTS';
import { fileUri, picker } from '@kit.CoreFileKit';
import { common } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';
import fs, { ReadOptions } from '@ohos.file.fs';
import { AlertDialog } from '@kit.ArkUI';

import { getDateStringWithTimeStamp } from '../utils/Util';

const TAG: string = 'AudioEditTestApp_MixingConsole';

@Builder
export function MixingConsoleBuilder() {
    MixingConsole()
}

@Component
export struct MixingConsole {
    pageInfos: NavPathStack = new NavPathStack();

    @State param: number = 0;
    @State inputList: SongInfo[] = [];
    private MAX_TRACKS: number = 5; // 最大音轨数
    @State nodeList: Map<string, SongInfo> = new Map();
    private wavBuffer: ArrayBuffer = new ArrayBuffer(0);
    // 是否录制
    @State isRecord: boolean = false;
    // 导出中
    @State isExported: boolean = false;
    @State mixerList: Array<Node> = [];
    @State mixerNode: Node = {
        id: util.generateRandomUUID(true),
        type: NodeType.MIXER,
        color: colorMap[NodeType.MIXER]
    }
    @State outputNode: Node = {
        id: util.generateRandomUUID(true),
        type: NodeType.OUTPUT,
        color: colorMap[NodeType.OUTPUT]
    }
    // 是否播放
    @State isPlay: boolean = false;
    @State isPlayRealTime: boolean = false;
    // 导出样式
    @State newAudioFileName: string = '';
    @State audioFormate: string = '.wav';
    @State sampleRate: number = 48000;
    @State channels: number = 2;
    @State bitsPerSample: number = 32;
    // 节点操作弹窗参数
    @State isLeft: boolean = true;
    @State selectedNode: string = '';
    @State nodeType: NodeType = NodeType.INPUT;
    @State finished: boolean = false;

    aboutToAppear(): void {
        Logger.info(TAG, 'aboutToAppear');
        this.isPlayRealTime = false;
        // 初始化空列表（保留5个空位）
        this.inputList = Array(this.MAX_TRACKS).fill(null);
        

        
        audioNapi.registerFinishedCallback((result: boolean) => {
            this.finished = result;
            Logger.info(TAG, `finished: ${JSON.stringify(this.finished)}`);
            if (this.finished) {
                if (this.isRecord) {
                    taskpool.execute(RealTimeSaveFileBuffer).then(async (value: object) => {
                        Logger.info(TAG, 'taskpool onClick then');
                        this.SaveBuffer(value as ArrayBuffer);
                    });
                }
                this.isExported = false;
                this.isRecord = false;
                this.isPlay = false;
            }
        })
    }

    aboutToDisappear(): void {
        Logger.info(TAG, 'aboutToDisappear');
        // 销毁播放器
        try {
            let result = audioNapi.audioRendererDestory();
            Logger.info(TAG, `audioRendererDestory result: ${result}`);
        } catch (e) {
            Logger.info(TAG, `audioRendererDestory error: ${JSON.stringify(e)}`);
        }
        // 销毁管线， 销毁引擎
        try {
            let result = audioNapi.audioEditDestory();
            Logger.info(TAG, `audioEditDestory result: ${result}`);
        } catch (e) {
            Logger.info(TAG, `audioEditDestory error: ${JSON.stringify(e)}`);
        }
    }

    // 设置弹窗
    dialogControllerSet: CustomDialogController | null = new CustomDialogController({
        builder: SetFormatDialog({
            newAudioFileName: this.newAudioFileName,
            sampleRate: this.sampleRate,
            channels: this.channels,
            bitsPerSample: this.bitsPerSample,
            nodeList: this.nodeList,
            cancel: () => {

            },
            confirm: () => {

            }
        }),
    });

    // 查看效果节点信息弹窗
    checkNodeDialogController: CustomDialogController | null = new CustomDialogController({
        builder: CheckNodeDialog({
            isLeft: this.isLeft,
            selectedNode: this.selectedNode,
            mixerList: this.mixerList,
            inputList: this.inputList,
            nodeList: this.nodeList,
            nodeType: this.nodeType,
            isPlayRealTime: this.isPlayRealTime,
            param: this.param,
            cancel: () => {

            },
            confirm: () => {

            }
        }),
    });

    // 添加output节点弹窗
    addOutputDialogController: CustomDialogController = new CustomDialogController({
        builder: AlertDialog({
            primaryTitle: '添加OUTPUT节点',
            primaryButton: {
                value: '取消'
            },
            secondaryButton: {
                value: '确认',
                role: ButtonRole.ERROR,
                action: () => {
                    const outputId = util.generateRandomUUID(true);
                    this.outputNode.id = outputId;
                }
            }
        }),
    });

    build() {
        NavDestination() {
            Column() {
                Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween }) {
                    Row() {
                        Image($r('app.media.ic_set'))
                            .width('30')
                            .height('30')
                            .onClick(() => {
                                // 弹窗， 导出的音频格式
                                this.dialogControllerSet?.open();
                            })

                    }
                    Row() {
                        // 导出（保存）
                        Image(this.isRecord ? $r('app.media.ic_record_circle_2') : $r('app.media.ic_record_circle'))
                            .width('30')
                            .height('30')
                            .margin({right: $r('app.float.margin_5')})
                            .onClick(() => {
                                audioNapi.record();
                                this.isRecord = true;
                            })
                        // 播放
                        Image(!this.isPlay ? $r('app.media.ic_record_playing') : $r('app.media.ic_record_paused'))
                            .width('30')
                            .height('30')
                            .margin({right: $r('app.float.margin_5')})
                            .onClick(() => {
                                this.isPlayRealTime = true;
                                this.isPlay = !this.isPlay;
                                // 播放
                                if (this.isPlay) {
                                    // 初始化 renderer
                                    audioNapi.audioRendererInit();
                                    // 点击播放，才开始 AudioRenderer
                                    audioNapi.audioRendererStart();
                                } else {
                                    // 暂停
                                    audioNapi.audioRendererStop();
                                    // 将buffer的map中的 totalWriteAudioDataSize 置为 0
                                    audioNapi.resetTotalWriteAudioDataSize();
                                }
                            })
                    }
                    .alignItems(VerticalAlign.Center)
                }
            }
            .height("8%")
            .width(StyleConst.FULL_WIDTH)
            .margin({bottom: $r('app.float.margin_10')})
            .padding({left: $r('app.float.margin_35'), right: $r('app.float.margin_35')})

            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween }) {
                // 左侧
                Column() {
                    ForEach(this.inputList, (info: SongInfo, index: number) => {
                        Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start }) {
                            Row() {
                                if(info && this.nodeList.has(info.nodeId || '')) {
                                    ForEach(Array.from(this.nodeList.get(info?.nodeId || '')?.nodes || []), (node: Node) => {
                                        Column() {
                                            this.EffectNode(true, node.type, node.id, info)
                                        }.margin({top: $r('app.float.margin_5')})
                                    })
                                } else {
                                    Column() {
                                        this.Node(true, info);
                                    }.onClick((event: ClickEvent) => {
                                        const inputId: string = util.generateRandomUUID(true);
                                        Logger.info(TAG, `inputId: ${inputId}`);
                                        importSingleSong(this.nodeList, inputId).then((songMap: Map<string, SongInfo>) => {
                                            // 将input节点也要加入到nodes里边
                                            let inputInfo: Node = {
                                                id: inputId,
                                                type: NodeType.INPUT,
                                                color: colorMap.get(NodeType.INPUT)
                                            }
                                            let nodesArray: Array<Node> = [];
                                            nodesArray.push(inputInfo);
                                            let tmpInfo: SongInfo = songMap.entries().next().value;
                                            tmpInfo.nodes = nodesArray;
                                            // 将导入的音频文件存在nodeList中
                                            for (let value of songMap.values()) {
                                                Logger.error(TAG, `${value.nodeId} --- ${inputId}`);
                                                if (value.nodeId === inputId) {
                                                    this.inputList[index] = value;
                                                    this.getPcmFileBuffer(this.inputList[index]);
                                                    if (this.wavBuffer.byteLength !== 0) {
                                                        const dataView = new DataView(this.wavBuffer);
                                                        // 获取通道数，通道数位于22字节
                                                        this.channels = dataView.getUint16(22, true);
                                                        // 获取采样率，采样率位于第24到28字节
                                                        this.sampleRate = dataView.getUint32(24, true);
                                                        // 获取位深，位深位于第34字节
                                                        this.bitsPerSample = dataView.getUint16(34, true);
                                                        // 格式类别 int 还是 float(3)
                                                        const formatCategory = dataView.getUint16(20, true);
                                                        Logger.info(TAG, `formatCategory: ${formatCategory}`);
                                                        // 获取音频的长度
                                                        const fmtSize = dataView.getUint32(16, true); // fmt chunk size
                                                        let offset = 20 + fmtSize;
                                                        let pcmLength = 0;
                                                        while (offset <= this.wavBuffer.byteLength) {
                                                            const chunkId = dataView.getUint32(offset, true);
                                                            offset += 4;
                                                            const chunkSize = dataView.getUint32(offset, true);
                                                            offset += 4;
                                                            // 'data' 在 ASCII 小端字节
                                                            if (chunkId === 0x61746164) {
                                                                // 找到了 data 块， 返回其大小
                                                                pcmLength = chunkSize;
                                                                break;
                                                            }                    
                                                            offset += chunkSize;                                        
                                                        }
                                                        Logger.info(TAG, `pcmLength: ${pcmLength}`);
                                                        // 设置音频参数
                                                        tmpInfo.channels = this.channels;
                                                        tmpInfo.sampleRate = this.sampleRate;
                                                        tmpInfo.bitsPerSample = this.bitsPerSample;

                                                        audioNapi.audioInAndOutInit(inputId, this.outputNode.id, this.mixerNode.id, this.channels, this.sampleRate, this.bitsPerSample, formatCategory,
                                                            pcmLength, this.wavBuffer.slice(offset, offset + pcmLength));
                                                    }
                                                    this.nodeList.set(inputId, tmpInfo);
                                                }
                                            }

                                            Logger.error(TAG, `Node list size: ${this.nodeList.size}`);
                                            Logger.error(TAG, `input list size: ${this.inputList.length}`);
                                            // 超过一个输入就增加混音节点
                                            if (this.nodeList.size > 1 && this.mixerNode.id === '') {
                                                Logger.info(TAG, 'LEFT ADD');
                                                const mixerNodeId = util.generateRandomUUID(true);
                                                this.mixerNode.id = mixerNodeId;
                                            }
                                        });
                                    })
                                    .margin({top: $r('app.float.margin_5')})
                                }
                            }
                        }
                        .height('20%')
                        .width('100%')
                    })
                }
                .height('100%')
                .width('60%')
                .backgroundColor(Color.Black)
                .padding({left: $r('app.float.margin_10'), right: $r('app.float.margin_10')})

                // 右侧
                Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                    Row() {
                        if (this.nodeList.size > 1) {
                            Column() {
                                this.EffectNode(false, NodeType.MIXER, this.mixerNode.id);
                            }
                        }
                        ForEach(this.mixerList, (node: Node) => {
                            Column() {
                                this.EffectNode(false, node.type, node.id);
                            }
                        })
                        if (this.nodeList.size > 0) {
                            Column() {
                                this.EffectNode(false, NodeType.OUTPUT, this.outputNode.id);
                            }
                        }

                        if (this.outputNode.id === '') {
                            Column() {
                                this.Node(false);
                            }.onClick(() => {
                                if (this.nodeList.size > 0) {
                                    this.addOutputDialogController.open();
                                } else {
                                    this.getUIContext().showAlertDialog({
                                        title: '提示',
                                        message: '添加输入节点后才可以添加输出节点'
                                    });
                                }
                            })
                        }
                    }
                }
                .height('100%')
                .width('38%')
                .backgroundColor(Color.Black)
            }
            .height('90%')
            .width(StyleConst.FULL_WIDTH)
            .backgroundColor(Color.Gray)
        }
        .onReady((context: NavDestinationContext) => {
            this.pageInfos = context.pathStack;
            this.param = context.pathInfo.param as number;
            Logger.info(TAG, `this param is ${this.param}`);
            // 工作模式
            if (this.param !== 0) {
                try {
                    let result = audioNapi.audioEditNodeInit(this.param);
                    Logger.info(TAG, `audioEditNodeInit result: ${result}`);
                } catch (e) {
                    Logger.error(TAG, `audioEditNodeInit error: ${JSON.stringify(e)}`);
                }
            } else {
                // 弹出小弹窗
               Logger.info(TAG, `this param is invalid: ${this.param}`); 
            }
        })
        .title('混音台')
        .height('100%')
        .width('100%')
    }

    @Builder 
    EffectNode(isLeft : boolean, type : NodeType, nodeId : string, info ?: SongInfo) {
        Column() {
            this.Node(isLeft, info, type);
        }.onClick(() => {
            this.isLeft = isLeft;
            this.selectedNode = nodeId;
            this.nodeType = type;
            Logger.info(TAG, `EffectNode nodeId: ${nodeId} ${isLeft}`);
            if (type !== NodeType.INPUT) {
                this.checkNodeDialogController?.open();
            } else {
                //提前copy一份nodes数据
                let nodesArray = this.nodeList.get(nodeId)?.nodes ?? [];
                importSingleSong(this.nodeList, nodeId).then((songMap: Map<string, SongInfo>) => {
                    Logger.error(TAG, `---- nodesArray length is ${nodesArray.length}`);
                    let songInfo = songMap.get(nodeId)
                    if(songInfo){
                        let inputInfo: Node = {
                            id: nodeId,
                            type: NodeType.INPUT,
                            color: colorMap.get(NodeType.INPUT)
                        }
                        if(nodesArray.length > 0){
                            nodesArray[0] = inputInfo
                        } else {
                            nodesArray = [inputInfo]
                        }
                        Logger.error(TAG, `nodesArray length is ${nodesArray.length}`);
                        songInfo.nodes = nodesArray;
                        //获取音频的相关参数
                        this.getPcmFileBuffer(songInfo);
                        if(this.wavBuffer.byteLength !== 0) {
                            const dataView = new DataView(this.wavBuffer);
                            // 获取通道数，通道数位于22字节
                            this.channels = dataView.getUint16(22, true);
                            // 获取采样率，采样率位于24到28字节
                            this.sampleRate = dataView.getUint32(24, true);
                            // 获取位深，位深位于34字节
                            this.bitsPerSample = dataView.getUint16(34, true);
                            // 格式类别 int 还是 float（3） ...
                            const formatCategory = dataView.getUint16(20, true);
                            Logger.info(TAG, `formatCategory: ${formatCategory}`);
                            // 获取音频的长度
                            const fmtSize = dataView.getUint32(16,true); // fmt chunk size
                            let offset = 20 + fmtSize;
                            let pcmLength = 0;
                            while (offset <= this.wavBuffer.byteLength) {
                                const chunkId = dataView.getUint32(offset, true);
                                offset += 4;
                                const chunkSize = dataView.getUint32(offset, true);
                                offset += 4;
                                // 'data' 在 ASCII 小端字节
                                if(chunkId === 0x61746164){
                                    // 找到了 data 块， 返回其大小
                                    pcmLength = chunkSize;
                                    break;
                                }
                                offset += chunkSize;
                            }
                            Logger.info(TAG, `pcmLength: ${pcmLength}`);
                            //设置音频参数
                            songInfo.channels = this.channels;
                            songInfo.sampleRate = this.sampleRate;
                            songInfo.bitsPerSample  = this.bitsPerSample;
                            audioNapi.audioInAndOutInit(nodeId, this.outputNode.id, this.mixerNode.id, this.channels, this.sampleRate, this.bitsPerSample, formatCategory,
                                pcmLength, this.wavBuffer.slice(offset, offset + pcmLength));
                        }
                        this.nodeList.set(nodeId, songInfo)
                        const index = this.inputList.findIndex(song => song.nodeId === nodeId);
                        if(index !== -1 && songInfo.uri !== undefined){
                            this.inputList[index] = songInfo
                        }
                    }
                });
                Logger.error(TAG,`nodeList size is ${this.nodeList.size}`)
            }
        })
        .gesture(
            LongPressGesture()
                .onAction((event: GestureEvent) => {
                    this.isLeft = isLeft;
                    this.selectedNode = nodeId;
                    this.nodeType = type;
                    this.checkNodeDialogController?.open();
                })
        )
    }

    @Builder
    Node(isLeft: boolean, info?:SongInfo | null, type?:NodeType) {
        if ( info && info.songName && type === NodeType.INPUT){
            Column(){
                Text(info.songName)
                    .height('60%')
                    .width("15%")
                    .margin({right: $r('app.float.margin_5')})
                    .textAlign(TextAlign.Center)
                    .backgroundColor(colorMap.get(type))
                    .fontSize($r('app.float.font_size_10'))
                    .maxLines(1)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .borderRadius(5)
            }
        } else if (type) {
            Text(type)
                .height(isLeft ? '60%' : '20%')
                .width("10%")
                .margin({right: $r('app.float.margin_5')})
                .textAlign(TextAlign.Center)
                .backgroundColor(colorMap.get(type))
                .borderRadius(5)
        }   else{
            Text("+")
                .height(isLeft ? '80%' : '20%')
                .width("10%")
                .margin({right: $r('app.float.margin_5')})
                .textAlign(TextAlign.Center)
                .backgroundColor(Color.Brown)
        }
    }

    getPcmFileBuffer(info: SongInfo): boolean{
        if(info.songType === 'pcm'){
            return false;
        }
        try {
            let path: string = new fileUri.FileUri(info.uri).path;
            Logger.info(TAG, `getPcmFileBuffer path: ${JSON.stringify(path)}`);
            let file  = fs.openSync(path, fs.OpenMode.READ_ONLY | fs.OpenMode.CREATE);
            let fsStat  = fs.statSync(path);
            Logger.info(TAG, `failSize : ${fsStat.size}`);
            // 不要删 --- let buffer = audioNapi.getFileBuffer(file.fd, fsStat.size);
            let buffer = new ArrayBuffer(fsStat.size);
            let readOption: ReadOptions = {
                offset: 0, // 期望读取文件的位置。可选，默认从当前位置开始读
                length: fsStat.size // 每次期望读取数据的长度。可选，默认缓冲区长度
            }
            fs.readSync(file.fd, buffer, readOption);
            Logger.info(TAG, `getPcmFileBuffer buffer length: ${buffer.byteLength}`);
            const wavView = newUint8Array(buffer);
            let dataBuffer = wavView.slice(44).buffer;
            Logger.info(TAG, `wavBuffer length: ${buffer.byteLength}`);
            this.wavBuffer = buffer;
            fs.closeSync(file.fd);
            return true;
        } catch (e) {
            Logger.error(TAG, `getPcmFileBuffer error: ${JSON.stringify(e)}`);
            return false;
        }
    }

    async SaveBuffer(value: ArrayBuffer) {
        try {
            Logger.info(TAG,
                `SaveBuffer start, audioFormate: ${this.audioFormate}, sampleRate: ${this.sampleRate}, channels:${this.channels}, bitsPerSample: ${this.bitsPerSample}`);
            let pcmBuffer = value;
            Logger.info(TAG, `SaveBuffer pcmBuffer length : ${pcmBuffer.byteLength}`);
            if(pcmBuffer.byteLength === 0) {
                Logger.info(TAG, 'SaveBuffer error');
            }
            // picker选择器，保存PCM文件
            let documentSaveOptions = new picker.DocumentSaveOptions();
            if(this.newAudioFileName ===  ''){
                this.newAudioFileName = getDateStringWithTimeStamp(new Date().getTime());
            } else {
                this.newAudioFileName = this.newAudioFileName + ' ' + getDateStringWithTimeStamp(new Date().getTime());   
            }
            let newFileNames = `${this.newAudioFileName}${this.audioFormate}`;
            Logger.info(TAG, `SaveBuffer newFileNames is: ${newFileNames}`);
            documentSaveOptions.newFileNames = [newFileNames];
            let context = getContext() as common.Context; // 请确保 getContext(this) 返回结果为UIAbilityContext
            let documentPicker = new picker.DocumentViewPicker(context);
            documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string>) => {
                if(err){
                    Logger.error(TAG,
                        `DocumentViewPicker.save failed with err, code is: ${err.code}, message is: ${err.message}`);
                    return;
                }
                Logger.info(TAG, 
                    'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
                let filePath = documentSaveResult;
                // cdocumentSaveResult数组中只有一个路径
                let file = fs.openSync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
                Logger.info(TAG, `SaveBuffer filePath : ${filePath}`);
                if(this.audioFormate === '.wav') {
                    // 处理wav
                    let wavBuffer = this.writeWavFileHeader(pcmBuffer);
                    let writeLen = fs.writeSync(file.fd, wavBuffer);
                    Logger.info(TAG, `SaveWavBuffer writeLen : ${writeLen}`);
                    fs.closeSync(file);
                } else {
                    // 处理pcm
                    let writeLen = fs.writeSync(file.fd, pcmBuffer);
                    Logger.info(TAG, `SavePcmBuffer writeLen : ${writeLen}`);
                    fs.closeSync(file);
                }
            }) 
        } catch(e) {
            Logger.error(TAG, `SaveBuffer catch: ${JSON.stringify(e)}`)
        }
    }

    writeString(dv: DataView, offset: number, str: string) {
        for (let i = 0; i < str.length; i++){
            dv.setUint8(offset+i, str.charCodeAt(i));
        }
    }

    concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
        const viewWavHeader = new Uint8Array(wavHeaderBuffer);
        const viewPcm = new Uint8Array(pcmBuffer);

        // 创建一个新的 ArrayBuffer， 大小为两个原始缓冲区之和
        const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
        const resultView = new Uint8Array(resultBuffer);

        // 将两个视图的数据复制到新的视图中
        resultView.set(viewWavHeader);
        resultView.set(viewPcm, viewWavHeader.length);

        return resultBuffer;
    }

    // 定义写 wav 头文件信息的方法
    writeWavFileHeader(pcmBuffer: ArrayBuffer): ArrayBuffer {
        Logger.info(TAG,
            `writeWavFileHeader sampleRate: ${this.sampleRate} --- channels: ${this.channels} --- bitsPerSample: ${this.bitsPerSample}`);
        const header = new ArrayBuffer(44);
        const dv = new DataView(header);
        // 写入 RIFF 块
        this.writeString(dv, 0, 'RIFF');
        let totalDataLen = pcmBuffer.byteLength + 44;
        dv.setUint32(4, totalDataLen, true);
        this.writeString(dv, 8, 'WAVE');
        // 写入 fmt 块
        this.writeString(dv, 12, 'fmt');
        // fmt块大小
        dv.setUint32(16, 16, true);
        // 格式类别
        if (this.bitsPerSample === 32) {
            dv.setUint16(20, 3, true);
        } else {
            dv.setUint16(20, 1, true);
        }
        // 声道数
        dv.setUint16(22, this.channels, true);
        // 采样率
        dv.setUint32(24, this.sampleRate, true);
        // 比特率
        let byteRate = this.bitsPerSample * this.sampleRate * this.channels / 8;
        dv.setUint32(28, byteRate, true);
        // 每个采样点的字节数
        dv.setUint16(32, this.channels * this.bitsPerSample / 8, true);
        // 位深
        dv.setUint16(34, this.bitsPerSample, true);
        // 写入data块
        this.writeString(dv, 36, 'data');
        // 数据库大小 
        dv.setUint32(40, pcmBuffer.byteLength, true);

        let resultBuffer = this.concatArrayBuffer(header, pcmBuffer);
        return resultBuffer;
    }
}