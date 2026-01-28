/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import audioNapi from 'libentry.so';
import { TextModifier } from '@kit.ArkUI';
import { OutputFile } from './autoTest/Interface';
import { Logger } from '../utils/Logger';
const TAG: string = 'AudioEditTestApp_AudioEdit';


@Concurrent
export async function saveFileBuffer(): Promise<ArrayBuffer> {
    audioNapi.setCurrentTime(0);
    let pcmBuffer = audioNapi.saveFileBuffer();
    return pcmBuffer;
}

@Concurrent
export async function saveMulFileBuffer(pipelineId: string): Promise<ArrayBuffer> {
  audioNapi.multiPipelineEnvPrepare(pipelineId);
  let pcmBuffer = audioNapi.multiSaveFileBuffer();
  return pcmBuffer;
}

@Concurrent
export async function multiPipelineSaveFileBuffer(pipelineId: string, outputInfo:OutputFile, multiRenderFlag:boolean): Promise<ArrayBuffer[]> {
    const TAG: string = 'AudioEditTestApp_AudioEdit';
    Logger.info(TAG, `multiPipelineSaveFileBuffer start---pipelineId:${pipelineId}, multiRenderFlag:${multiRenderFlag}`)
    audioNapi.multiPipelineEnvPrepare(pipelineId);
    audioNapi.multiSetFormat(outputInfo?.channels, outputInfo?.sampleRate, outputInfo?.bitDepth);
    let pcmBuffer = audioNapi.multiSaveFileBuffer();

    let tapBuffer: ArrayBuffer = new ArrayBuffer(0);
    if (multiRenderFlag){
        tapBuffer = audioNapi.multiGetSecondOutputAudio();
    }
    return [pcmBuffer as ArrayBuffer, tapBuffer];
}

@Concurrent
export async function multiPipelineRealTimeSaveFileBuffer(pipelineId: string, outputInfo:OutputFile, multiRenderFlag:boolean): Promise<ArrayBuffer[]> {
  const TAG: string = 'AudioEditTestApp_AudioEdit';
  Logger.info(TAG, `multiPipelineRealTimeSaveFileBuffer start---pipelineId:${pipelineId}, multiRenderFlag:${multiRenderFlag}`)
  audioNapi.multiPipelineEnvPrepare(pipelineId);
  audioNapi.multiSetFormat(outputInfo?.channels, outputInfo?.sampleRate, outputInfo?.bitDepth);
  let pcmBuffer = audioNapi.multiRealTimeSaveFileBuffer();
  return [pcmBuffer as ArrayBuffer];
}

@Concurrent
export async function RealTimeSaveFileBuffer(): Promise<ArrayBuffer> {
    let pcmBuffer = audioNapi.realTimeSaveFileBuffer();
    audioNapi.setCurrentTime(0);
    // Set totalWriteAudioDataSize in the buffer map to 0.
    audioNapi.resetTotalWriteAudioDataSize();
    return pcmBuffer;
}

export class MainTitleTextModifier extends TextModifier {
    userStyle: boolean = true;

    applyNormalAttribute(instance: TextAttribute): void {
        if (this.userStyle) {
            instance.fontColor($r('app.color.index_title_text_color'));
        }
    }
}


export function writeString(dv: DataView, offset: number, str: string) {
    for (let i = 0; i < str.length; i++) {
        dv.setUint8(offset + i, str.charCodeAt(i));
    }
}

export function concatArrayBuffer(wavHeaderBuffer: ArrayBuffer, pcmBuffer: ArrayBuffer) {
    const viewWavHeader = new Uint8Array(wavHeaderBuffer);
    const viewPcm = new Uint8Array(pcmBuffer);

    // Create a new ArrayBuffer with a size equal to the sum of the two raw buffers.
    const resultBuffer = new ArrayBuffer(viewWavHeader.byteLength + viewPcm.byteLength);
    const resultView = new Uint8Array(resultBuffer);

    // Copy the data from two views into a new view
    resultView.set(viewWavHeader);
    resultView.set(viewPcm, viewWavHeader.length);

    return resultBuffer;
}

export function writeWavFileHeader(pcmBuffer: ArrayBuffer, sampleRate: number, channels: number, bitsPerSample: number): ArrayBuffer {
    Logger.info(TAG,
        `writeWavFileHeader sampleRate : ${sampleRate} --- channels : ${channels} --- bitsPerSample : ${bitsPerSample}`);
    const header = new ArrayBuffer(44);
    const dv = new DataView(header);
    // Write to RIFF block
    writeString(dv, 0, 'RIFF');
    let totalDataLen = pcmBuffer.byteLength + 44;
    dv.setUint32(4, totalDataLen, true);
    writeString(dv, 8, 'WAVE');
    // Write to fmt block
    writeString(dv, 12, 'fmt ');
    // fmt block size
    dv.setUint32(16, 16, true);
    // Format Category (1 - PCM, 3 - IEEE Floating Point)
    dv.setUint16(20, 3, true);
    if (bitsPerSample === 32) {
        dv.setUint16(20, 3, true);
    } else {
        dv.setUint16(20, 1, true);
    }
    // Number of audio channels
    dv.setUint16(22, channels, true);
    // Sampling rate
    dv.setUint32(24, sampleRate, true);
    // Bitrate
    let byteRate = bitsPerSample * sampleRate * channels / 8;
    dv.setUint32(28, byteRate, true);
    // Number of bytes per sampling point
    dv.setUint16(32, channels * bitsPerSample / 8, true);
    // Bit depth
    dv.setUint16(34, bitsPerSample, true);
    // Write to data block
    writeString(dv, 36, 'data');
    // Data block size
    dv.setUint32(40, pcmBuffer.byteLength, true);

    let resultBuffer = concatArrayBuffer(header, pcmBuffer);
    return resultBuffer;
}