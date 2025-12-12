/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { Logger } from '../../utils/Logger';
import { JSON, taskpool, util } from '@kit.ArkTS';
import fs from '@ohos.file.fs';
import { SceneInfo, NodeInfo, PipelineInfo, JsonFileInfo } from './Interface'
import audioNapi from 'libentry.so';

// Initialize the message class of the subthread of the pipeline task to synchronize the message to the main thread.
export class MultiInitPipelineSonMsg {
  sceneInfoId: string = "";
  pipelineId: string = "";
  outputId: string = "";
  mixerId: string = "";
}

@Concurrent
export async function initAllInputNode(jsonFileInfo:JsonFileInfo, sceneInfoIdToSceneInfoMap: Map<string, SceneInfo>, pipelineIdToPipelineInfoMap:Map<string, PipelineInfo>): Promise<number> {
  const TAG: string = 'AudioEditTestApp_AudioEdit';
  const inputDir = `/storage/Users/currentUser/Download/src.main.audiodemo/input_files`;
  const SUCCESS: number = 0;
  const FAILED: number = 1;
  Logger.info(TAG, `initAllInputNode start`);

  for (const sceneInfoId of jsonFileInfo.sceneInfos) {
    const sceneInfo = sceneInfoIdToSceneInfoMap.get(sceneInfoId);
    const pipelineId = sceneInfo?.pipelineId;
    if (sceneInfo == undefined || pipelineId == undefined) {
      Logger.error(TAG, `initAllInputNode get null sceneInfo`);
      return FAILED;
    }
    const pipelineInfo = pipelineIdToPipelineInfoMap.get(pipelineId);
    if (pipelineInfo == undefined) {
      Logger.error(TAG, `initAllInputNode get null pipelineInfo`);
      return FAILED;
    }
    let status = audioNapi.multiPipelineEnvPrepare(pipelineId);
    if (status != SUCCESS) {
      Logger.error(TAG, `multiPipelineEnvPrepare ERROR while initAllInputNode`);
      return FAILED;
    }
    for (const inputFile of sceneInfo.inputFiles) {
      // Obtaining the File Content
      let inputPath = `${inputDir}/${inputFile.path}`;
      let file = fs.openSync(inputPath, fs.OpenMode.READ_WRITE);
      let fsStat = fs.statSync(inputPath);
      let buffer = new ArrayBuffer(fsStat.size);
      fs.readSync(file.fd, buffer);
      let inputNodeStatus =
        audioNapi.multiAudioInAndOutInit(inputFile.inputId, pipelineInfo.outputId, pipelineInfo.mixerId, inputPath);
      fs.closeSync(file.fd);
      if (inputNodeStatus != SUCCESS) {
        Logger.error(TAG, `input node init ERROR, inputId:${inputFile.inputId}, pipelineId :${pipelineId}`);
        return FAILED;
      }
      Logger.info(TAG, `input node init done, inputId:${inputFile.inputId}, pipelineId :${pipelineId}`);
    }
    Logger.info(TAG, `all input node had inited , pipelineId:${pipelineId}`);
  }
  return SUCCESS;
}

@Concurrent
export async function multiInitPipeline(sceneInfoId: string, pipelineId: string): Promise<number> {
  const TAG: string = 'AudioEditTestApp_AudioEdit';
  Logger.info(TAG, ' AutoTest multiInitPipeline start');
  let ret = audioNapi.audioEditNodeInitMultiPipeline(pipelineId);
  let msg: MultiInitPipelineSonMsg = {
    sceneInfoId: sceneInfoId,
    pipelineId: pipelineId,
    outputId: util.generateRandomUUID(true),
    mixerId: util.generateRandomUUID(true)
  }
  taskpool.Task.sendData(msg);
  Logger.info(TAG, ' AutoTest multiInitPipeline end');
  return ret;
}

@Concurrent
export async function multiInitInput(inputId: string, outputId: string, mixerId: string, pipelineId: string, audioFile: string,
  multiPipelineFlag: boolean): Promise<number> {
  const TAG: string = 'AudioEditTestApp_AudioEdit';
  Logger.info(TAG, ` AutoTest multiInitInput start, inputId:${inputId}, pipelineId:${pipelineId}`);
  const inputDir = `/storage/Users/currentUser/Download/src.main.audiodemo/input_files`;

  // Obtaining the File Content
  let inputPath = `${inputDir}/${audioFile}`;
  let file = fs.openSync(inputPath, fs.OpenMode.READ_WRITE);
  let fsStat = fs.statSync(inputPath);
  let buffer = new ArrayBuffer(fsStat.size);
  fs.readSync(file.fd, buffer);

  Logger.info(TAG, `input FIle: ${inputPath}, buffer.length: ${buffer.byteLength}`);

  const wavView = new Uint8Array(buffer);
  let wavHeaderBuffer = wavView.slice(0, 44).buffer;
  const dataView = new DataView(wavHeaderBuffer);
  // Get the number of channels; the number of channels is located at byte 22.
  const channels: number = dataView.getUint16(22, true);
  // Get the sampling rate, which is located at bytes 24 to 28.
  const sampleRate: number = dataView.getUint32(24, true);
  // Get bit depth, bit depth is located at byte 34.
  const bitsPerSample: number = dataView.getUint16(34, true);
  // Get the length of the audio.
  const fmtSize = dataView.getUint32(16, true); // fmt chunk size
  let offset = 20 + fmtSize;
  let pcmLength = 0;
  while (offset <= buffer.byteLength) {
    const chunkId = dataView.getUint32(offset, true);
    offset += 4;
    const chunkSize = dataView.getUint32(offset, true);
    offset += 4;
    // 'data' In ASCII little-endian byte order
    if (chunkId === 0x61746164) {
      // Found the data block, returned its size
      pcmLength = chunkSize;
      break;
    }
    offset += chunkSize;
  }
  Logger.info(TAG, `pcmLength: ${pcmLength}`);
  Logger.info(TAG,
    `audioEffectNodeTestSuccess01 sampleRate: ${sampleRate}, bitsPerSample: ${bitsPerSample}, channels: ${channels}`);
  if (multiPipelineFlag) {
    audioNapi.multiPipelineEnvPrepare(pipelineId);
  }
  // let ret = audioNapi.multiAudioInAndOutInit(inputId, outputId, mixerId, file.fd, buffer.byteLength);
  fs.closeSync(file.fd);
  Logger.info(TAG, ` AutoTest multiInitInput end, inputId:${inputId}, pipelineId:${pipelineId}`);
  return 0;
}