@Bulider EffectNode(isLeft : boolen, type : NodeType, NodeId : string, info ?: SongInfo) {
    Column() {
        this.Node(isLeft, info, type);
    }.onClick(() => {
        this.isLeft = isLeft;
        this.selectedNode = nodeId;
        this.nodeType = type;
        Logger.info(TAG, 'EffectNode nodeId: ${nodeId} ${isLeft}}');
        if (type !== NodeType.INPUT) {
            this.checkNodeDialogController?.open();
        } else {
            //提前copy一份nodes数据
            let nodesArray = this.nodeList.get(nodeId)?.nodes ?? [];
            importSingleSong(this.nodeList, nodeId).then((songMap: Map<string, SongInfo>) => {
                Logger.error(Tag,'---- nodesArray length is ${nodesArray.length}')
                let songInfo = songMap.get(nodeId)
                if(songInfo){
                    let inputInfo: Node = {
                        id: nodeId,
                        type: Nodetype.INPUT,
                        color: colorMap.get(NodeType.INPUT)
                    }
                    if(nodesArray.length > 0){
                        nodesArray[0] = inputInfo
                    } else {
                        nodesArray = [inputInfo]
                    }
                    Logger.error(TAG,'---- 222 nodesArray length is ${nodesArray.length}')
                    songInfo.nodes = nodesArray;
                    //获取音频的相关参数
                    this.getPcmFileBuffer(songInfo);
                    if(this.wavBuffer,byteLength !== 0) {
                        const dataView = new DataView(this.wavBuffer);
                        // 获取通道数，通道数位于22字节
                        this.channels = dataView.getUint16(22, true);
                        // 获取采样率，采样率位于24到28字节
                        this.sampleRate = dataView.getUint16(24, true);
                        // 获取位深，位深位于34字节
                        this.bitsPerSample = dataView.getUint16(34, true);
                        // 格式类别 int 还是 float（3） ...
                        const formatCategory = dataView.getUint16(20, true);
                        Logger.info(TAG, 'formatCategory: ${formatCategory}');
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
                            if(chunkId == 0x61746164){
                                // 找到了 data 块， 返回其大小
                                pcmLength = chunkSize;
                                break;
                            }
                            offset += chunkSize;
                        }
                        Logger.info(TAG, 'pcmLength: ${pcmLength}');
                        //设置音频参数
                        songInfo.channels = this.channels;
                        songInfo.sampleRate = this.samplesRate;
                        songInfo.bitsPerSample  = this.bitsPerSample;
                        audioNapi.audioInAndOutInit(nodeId, this.outputNode.id, this.mixerNode.id, this.channels, this.sampleRate, this.bitsPerSample, formatCategory,
                            pcmLength, this.wavBuffer.slice(offset, offset + pcmLength));
                    }
                    this.nodeList.set(nodeId, songInfo)
                    const index = this.inputList.findIndex(song => song.nodeId === nodeId);
                    if(index !=== -1 && songInfo.uri !== underfined){
                        this.inputList[index] = songInfo
                    }
                }
            });
            Logger.error(TAG,'nodeList size is ${this.nodeList.size}')
        }
    })
    .gesture(
        Longpressgesture()
            .onAction((event: GestureEvent) => {
                this.islef = isLeft;
                this.selectedNode = nodeId;
                this.nodeType = type;
                this.checkNodeDialogController?.open();
            })
    )
}

@Builder
Node(isLeft: boolen, info?:SongInfo | null, type?:NodeType) {
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
                .textOverflow({ overflow: textOverflow.Ellipsis })
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
    if(info.sognType === 'pcm'){
        return false;
    }
    try {
        let path: string = new fileUri.FileUri(info.uri).path;
        Logger.info(TAG, 'getPcmFIleBuffer path: ${JSON.stringify(path)}');
        let file  = fs.openSync(path, fs.OpenMode.READ_ONLY | fs.OpenMode.CREATE);
        let fsStat  = fs.openSync(path);
        Logger.info(TAG, 'failSize : ${fsStat.size}');
        // 不要删 --- let buffer = audioNapi.getFileBuffer(file.fd, fsStat.size);
        let buffer = new ArrayBuffer(fsStat.size);
        let readOption: ReadOptions = {
            offset: 0, // 期望读取文件的位置。可选，默认从当前位置开始读
            length: fsStat.size // 每次期望读取数据的长度。可选，默认缓冲区长度
        }
        fs.readSync(file.fd, buffer, readOption);
        Logger.info(TAG, 'getPcmBuffer buffer length: ${buffer.byteLength}');
        const wavView = newUint8Array(buffer);
        let dataBuffer = wavView.slice(44).buffer;
        Logger.info(TAG, 'wavBuffer length: ${buffer.byteLength}');
        this.wavBuffer = buffer;
        fs.closeSync(file.fd);
        return true;
    } catch (e) {
        Logger.error(TAG, 'getPcmFileBuffer error: ${JSON.stringify(e)}');
        return false;
    }
}

async SaveBuffer(value: ArrayBuffer) {
    try {
        Logger.info(TAG,
            'SaveBuffer start, audioFormate: ${this.audioFormate}, sampleRate: ${this.sampleRate}, channels:${this.channels}, bitsPerSample: ${this.bitsPerSample}');
        let pcmBuffer = value;
        Logger.info(TAG, 'SaveBuffer pcmBuffer length : ${pcmBuffer.byteLength}');
        if(pcmBuffer.byteLength === 0) {
            Logger.info(TAG, 'SaveBuffer error');
        }
        // picker选择器，保存PCM文件
        let documentSaveOptions = new picker.documentSaveOptions();
        if(this.newAudioFileName ===  ''){
            this.newAudioFileName = getDataStringWithTimeStamp(new Data().getTime());
        } else {
            this.newAudioFileName = this.newAudioFileName + ' ' + getDataStringWithTimeStamp(new Data().getTime());   
        }
        let newFileNames = '${this.newAudioFileName}${this.audioFormate}';
        Logger.info(TAG, 'SaveBuffer newFileNames is: ${newFileNames}');
        documentSaveOptions.newFileNames = [newFileNames];
        let context = getContext() as common.Context; // 请确保 getContext(this) 返回结果为UIAbilityContext
        let documentPicker = new picker.DocumentViewPicker(context);
        documentPicker.save(documentSaveOptions, (err: BusinessError, documentSaveResult: Array<string) => {
            if(err){
                Logger.error(TAG, 'DocumentViewPicker.save failed eith err, code is: ${err.code}, message is: ${err.message}');
                return;
            }
            Logger.info(TAG, 
                'DocumentViewPicker.save successfully, documentSaveResult uri: ' + JSON.stringify(documentSaveResult));
            let filePath = documentSaveResult;
            // cdocumentSaveResult数组中只有一个路径
            let file = fs.opensync(documentSaveResult[0], fs.OpenMode.CREATE | fs.OpenMode.READ_WRITE);
            Logger.info(TAG, 'SaveBuffer filePath : ${filePath}');
            if(this.audioFormate === ".wav") {
                // 处理wav
                let wavBuffer = this writeWavfileHeader(pcmBuffer);
                let writeLen = fs.writeSync(file.fd, wavBuffer);
                Logger.info(TAG, 'SaveWavBuffer writeLen : ${writeLen}');
                fs.closeSync(file);
            } else {
                // 处理pcm
                let writeLen = fs.writeSync(file.fd, pcmBuffer);
                Logger.info(TAG, 'SavePcmBuffer writeLen : ${writeLen}');
                fs.closeSync(file);
            }
        })  catch(e) {
            Logger.error(TAG, 'SaveBuffer catch: ${JSON.stringify{e}}')
        }
    }


}

writeString(dv: DataView, offset: number, str: string) {
    for (let i = 0; i < str.length; i++){
        dv.setUint8(offset+i, str.charCodeAt(i));
    }
}

concatArrayBuffer(wavHeadBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
    const viewWavHeader = new Uint8Array(wavHeadBuffer);
    const viewPcm = new Uint8Array(pcmBuffer);

    // 创建一个新的 ArrayBuffer， 大小为两个原始缓冲区之和
    const resultBuffer = new ArrayBuffer(viewWavBuffer.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);

    // 将两个视图的数据复制到新的视图中
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);

    return resultBuffer;
}