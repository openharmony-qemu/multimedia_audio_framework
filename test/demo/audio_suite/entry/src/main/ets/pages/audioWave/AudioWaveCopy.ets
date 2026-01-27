/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

import { AudioAsset, AudioTrack, Node } from "../../utils/InterfaceInfo";
import { Logger } from "../../utils/Logger";
import audioNapi from 'libentry.so';

const TAG: string = 'AudioEditTestApp_AudioWaveCopy';

export interface AudioAssetContext {
  trkIdx: number;
  audioTrackList: AudioTrack[];
  audioWave: [string, string[]];
  audioWaves: Array<[string, string[]]> | undefined;
  contextMap: Map<string, CanvasRenderingContext2D>;
  canvasStartXMap: Map<string, number>;
  canvasWidthMap: Map<string, number>;
  settings: RenderingContextSettings;
}

export interface AudioAssetPanContext extends AudioAssetContext {
  offsetTime: number;
}

export function copyOneAudioAsset(
  ctx: AudioAssetContext
): boolean {
  const trkIdx = ctx.trkIdx;
  const audioTrackList = ctx.audioTrackList;
  const audioWave = ctx.audioWave;
  const audioWaves = ctx.audioWaves;
  const contextMap = ctx.contextMap;
  const canvasStartXMap = ctx.canvasStartXMap;
  const canvasWidthMap = ctx.canvasWidthMap;
  const settings = ctx.settings;
  if (
    trkIdx < 0 ||
      trkIdx >= audioTrackList.length
  ) {
    return false;
  }
  const audioTrack = audioTrackList[trkIdx];
  const audioAssetArray = audioTrack.audioAssetArray;
  if (audioAssetArray === undefined) {
    return false;
  }
  if (audioWaves === undefined) {
    return false;
  }
  //Define an acceptable error range (e.g., 1e-10)
  const EPSILON = 1e-10;
  //Obtains the current audio index and the last audio start time.
  let astIdx = audioAssetArray.findIndex(item => Math.abs(Number(audioWave[0]) -  (item.startTime ?? 0)) < EPSILON);
  if (astIdx === -1) {
    return false;
  }
  const lastStartTime = audioAssetArray[audioAssetArray.length-1].startTime;
  Logger.error(TAG,
    ` copyOneAudioAsset lastStartTime is :${lastStartTime}`);
  const audioAsset = audioAssetArray[astIdx];
  let duration = audioAsset.duration ?? 0;
  //Obtains the start time (in ms) of the copied audio.
  const copyStartTime = Math.ceil((lastStartTime ?? 0) + (audioAssetArray[audioAssetArray.length-1].duration ?? 0));
  const newAudioAsset: AudioAsset =
    {
      assetId: copyStartTime,
      uri: audioAsset.uri,
      songName: audioAsset.songName,
      songType: audioAsset.songType,
      nodes: audioAsset.nodes?.slice(),
      singerName: audioAsset.singerName,
      albumCover: audioAsset.albumCover,
      albumName: audioAsset.albumName,
      wavBuffer: audioAsset.wavBuffer,
      channels: audioAsset.channels,
      sampleRate: audioAsset.sampleRate,
      bitsPerSample: audioAsset.bitsPerSample,
      fd: audioAsset.fd,
      startTime: copyStartTime,
      duration: duration
    }
  audioTrack.audioAssetArray?.push(newAudioAsset);
  let lastIndex = audioWaves.findIndex(item => Math.abs(Number(item[0]) -  (lastStartTime ?? 0)) < EPSILON );
  if (lastIndex === -1) {
    return false;
  }
  let audioWaveLast = audioWaves[lastIndex];
  //Obtains the last canvas.
  const lastCanvasStartX = canvasStartXMap.get(audioWaveLast[0]) ?? 0;
  const lastAudioWavesWidth = canvasWidthMap.get(audioWaveLast[0]) ?? 0;
  //Obtains the total length of the currently clicked audio waveform.
  const currentAudioWavesWidth = canvasWidthMap.get(audioWave[0]) ?? 0;
  //Starting coordinates of the canvas after the copy.
  const copyCanvasStartX = lastCanvasStartX + lastAudioWavesWidth;
  //Obtains the currently clicked audio waveform.
  const currentAudioWaves = audioWave[1].slice();
  //Reassign a value to the map.
  contextMap.set(copyStartTime.toString(), new CanvasRenderingContext2D(settings));
  //Assign a value to audioWaves
  audioWaves?.push([copyStartTime.toString(), currentAudioWaves]);
  //Assign a value to canvasStartXMap.
  canvasStartXMap.set(copyStartTime.toString(), copyCanvasStartX);
  //Assign a value to canvasWidthMap.
  canvasWidthMap.set(copyStartTime.toString(), currentAudioWavesWidth);

  //Add audio to the corresponding audio track
  let newStartIndex = 0;
  let newEndIndex = (newAudioAsset.duration ?? 0) * audioAsset.sampleRate * audioAsset.channels *
    audioAsset.bitsPerSample / 8 / 1000;
  let indexs: number[] = [newStartIndex,newEndIndex];
  let res =
    audioNapi.addAudioAsset(audioTrack.audioTrackId, audioAsset.startTime, newAudioAsset.startTime, indexs, false);
  if (!res) {
    Logger.error(TAG,
      `add AudioAsset failed, trackId is :${audioTrack.audioTrackId} ,AudioAsset startTime is :${newAudioAsset.startTime}`);
    return false;
  }
  Logger.info(TAG, `add AudioAsset success,AudioAsset startTime is :${newAudioAsset.startTime},AudioAsset endTime is :${(newAudioAsset.startTime ?? 0) + (newAudioAsset.duration ?? 0)}`);
  return true;
}

export function copyMultipleAudioAsset(
  ctx: AudioAssetContext
): boolean {
  const trkIdx = ctx.trkIdx;
  const audioTrackList = ctx.audioTrackList;
  const audioWave = ctx.audioWave;
  const audioWaves = ctx.audioWaves;
  const contextMap = ctx.contextMap;
  const canvasStartXMap = ctx.canvasStartXMap;
  const canvasWidthMap = ctx.canvasWidthMap;
  const settings = ctx.settings;
  const track = audioTrackList[trkIdx];
  if (!track || !track.audioAssetArray) {
    Logger.info(TAG, `无效的音轨或音频索引`);
    return false;
  }
  const audioTrack = audioTrackList[trkIdx];
  const audioAssetArray = audioTrack.audioAssetArray;
  if (undefined === audioAssetArray) {
    return false;
  }
  if (undefined === audioWaves) {
    return false;
  }
  //Define an acceptable error range (e.g., 1e-10)
  const EPSILON = 1e-10;
  let astIdx = audioAssetArray.findIndex(item => Math.abs(Number(audioWave[0]) - (item.startTime ?? 0)) < EPSILON);
  if (astIdx === -1) {
    return false;
  }
  const audioAsset = audioAssetArray[astIdx];
  //Obtains the length of the clicked audio.
  let currentAudioWavesTime = audioAsset.duration ?? 0;
  let traDuration: number = 0;
  let longestTrackDuration: number = 0;
  //Obtains the current audio track duration and the maximum audio track duration.
  for (let i = 0; i < audioTrackList.length; i++) {
    let audioAssets = audioTrackList[i].audioAssetArray;
    if (audioAssets && audioAssets.length > 0) {
      let asset = audioAssets[audioAssets.length-1];
      let durationTemp: number = asset.duration ?? 0;
      if (longestTrackDuration < ((asset.startTime ?? 0) + durationTemp)) {
        longestTrackDuration = (asset.startTime ?? 0) + durationTemp;
      }
      if (i === trkIdx) {
        traDuration = (asset.startTime ?? 0) + durationTemp;
      }
    }
  }
  if (traDuration === 0) {
    Logger.info(TAG, 'The audio track length is invalid.');
    return false;
  }
  if (traDuration >= longestTrackDuration) {
    Logger.info(TAG, 'The current audio track is already the longest; there is no need to duplicate it.');
    return true;
  }
  if (audioAsset.duration === undefined) {
    Logger.info(TAG, 'The selected audio does not have a length.');
    return true;
  }
  //The difference between the current track and the longest track in length.
  traDuration = Math.ceil(traDuration);
  const neededDuration = Math.ceil(longestTrackDuration - traDuration);
  const newAudioAsset: AudioAsset =
    {
      assetId: traDuration,
      uri: audioAsset.uri,
      songName: audioAsset.songName,
      songType: audioAsset.songType,
      nodes: audioAsset.nodes?.slice(),
      singerName: audioAsset.singerName,
      albumCover: audioAsset.albumCover,
      albumName: audioAsset.albumName,
      wavBuffer: audioAsset.wavBuffer,
      channels: audioAsset.channels,
      sampleRate: audioAsset.sampleRate,
      bitsPerSample: audioAsset.bitsPerSample,
      fd: audioAsset.fd,
      startTime: traDuration, //Current audio track length
      duration: neededDuration //The difference between the current track and the longest track in length.
    };
  track.audioAssetArray.push(newAudioAsset);
  //Obtains the start time (in ms) of the copied audio.
  const copyStartTime = traDuration;
  //Final Soundwave
  let lastStartTime = 0;
  let lastIndex = 0;
  for (let i = 0; i < (audioWaves?.length ?? 0); i++) {
    if (!Number.isNaN(audioWaves[i][0]) && lastStartTime <= Number(audioWaves[i][0])) {
      lastStartTime = Number(audioWaves[i][0]);
      lastIndex = i;
    }
  }
  let audioWaveLast = audioWaves[lastIndex];
  //Obtains the last canvas.
  const lastCanvasStartX = canvasStartXMap.get(audioWaveLast[0]) ?? 0;
  const lastAudioWavesWidth = canvasWidthMap.get(audioWaveLast[0]) ?? 0;
  //Obtains the total length of the currently clicked audio waveform.
  const currentAudioWavesWidth = canvasWidthMap.get(audioWave[0]) ?? 0;
  //Starting coordinates of the canvas after the copy.
  const copyCanvasStartX = lastCanvasStartX + lastAudioWavesWidth;
  //Obtains the total length of the copied audio waveform.
  const copyAudioWavesWidth = currentAudioWavesWidth * (neededDuration / currentAudioWavesTime);
  //Obtain the audio waveform after copying
  const copyAudioWaves: string[] = [];
  //Floor down
  let floorNum = Math.floor(neededDuration / currentAudioWavesTime);
  //Ceiling rounding
  let ceilNum = Math.ceil(neededDuration / currentAudioWavesTime);
  for (let i = 0; i < floorNum; i++) {
    copyAudioWaves.push(...audioWave[1].slice());
  }
  //Add the remaining part.
  if (floorNum != ceilNum) {
    copyAudioWaves.push(...audioWave[1].slice(Math.floor(audioWave[1].length *
      (neededDuration / currentAudioWavesTime - floorNum))));
  }
  //Re-assign a value to the map.
  contextMap.set(copyStartTime.toString(), new CanvasRenderingContext2D(settings));
  //Assign a value to audioWaves
  audioWaves?.push([copyStartTime.toString(), copyAudioWaves]);
  //Assign a value to canvasStartXMap.
  canvasStartXMap.set(copyStartTime.toString(), copyCanvasStartX);
  //Assign a value to canvasWidthMap.
  canvasWidthMap.set(copyStartTime.toString(), copyAudioWavesWidth);

  //Add audio to the current audio track
  let newStartIndex = 0;
  let newEndIndex = neededDuration * audioAsset.sampleRate * audioAsset.channels *
    audioAsset.bitsPerSample / 8 / 1000;
  let indexs: number[] = [newStartIndex,newEndIndex];
  let res =
    audioNapi.addAudioAsset(audioTrack.audioTrackId, audioAsset.startTime, newAudioAsset.startTime, indexs, true);
  if (!res) {
    Logger.error(TAG,
      `add AudioAsset failed, trackId is :${audioTrack.audioTrackId} ,AudioAsset startTime is :${newAudioAsset.startTime}`);
    return false;
  }

  Logger.info(TAG, `An audio copy of ${neededDuration} has been added to make the audio track reach the maximum length. The new audio's start time is ${copyStartTime}, and the end time is ${copyStartTime + neededDuration}.`);
  return true;
}

export function deleteAudioAsset(ctx: AudioAssetContext): boolean {
  const trkIdx = ctx.trkIdx;
  const audioTrackList = ctx.audioTrackList;
  const audioWave = ctx.audioWave;
  const audioWaves = ctx.audioWaves;
  const contextMap = ctx.contextMap;
  const canvasStartXMap = ctx.canvasStartXMap;
  const canvasWidthMap = ctx.canvasWidthMap;
  if (
    trkIdx < 0 ||
      trkIdx >= audioTrackList.length
  ) {
    return false;
  }
  const audioTrack = audioTrackList[trkIdx];
  const audioAssetArray = audioTrack.audioAssetArray;
  if (undefined === audioAssetArray) {
    return false;
  }
  if (undefined === audioWaves) {
    return false;
  }
  //Define an acceptable error range (e.g., 1e-10)
  const EPSILON = 1e-10;
  let astIdx = audioAssetArray.findIndex(item => Math.abs(Number(audioWave[0]) - (item.startTime ?? 0)) < EPSILON);
  if (astIdx === -1) {
    return false;
  }
  const audioAsset = audioAssetArray[astIdx];
  if(audioAssetArray.length == 1){
    audioAsset.nodes?.forEach((node: Node) => {
      audioNapi.deleteNode(node.id);
    });
  }
  audioAsset.nodes?.splice(0, audioAsset.nodes?.length);
  //Obtain the start time
  const startTime = audioAsset.assetId;
  audioAssetArray.splice(astIdx, 1)
  //Delete sound waves
  let waveIndex = audioWaves.findIndex(item => Math.abs(Number(item[0]) - startTime) < EPSILON);
  if (waveIndex === -1) {
    return false;
  }
  audioWaves.splice(waveIndex, 1);
  //Delete a map
  contextMap.delete(startTime.toString());
  //Delete the canvasStartXMap.
  canvasStartXMap.delete(startTime.toString());
  canvasWidthMap.delete(startTime.toString());
  //Delete the audio from the corresponding audio track
  let res = audioNapi.deleteAudioAsset(audioTrack.audioTrackId, startTime);
  if (!res) {
    Logger.error(TAG,
      `delete AudioAsset failed, trackId is :${audioTrack.audioTrackId} ,AudioAsset startTime is :${startTime}`);
    return false;
  }
  return true;
}

export function panGestureAudioAsset(ctx: AudioAssetPanContext): boolean {
  const trkIdx = ctx.trkIdx;
  const audioTrackList = ctx.audioTrackList;
  const audioWave = ctx.audioWave;
  const audioWaves = ctx.audioWaves;
  const contextMap = ctx.contextMap;
  const canvasStartXMap = ctx.canvasStartXMap;
  const canvasWidthMap = ctx.canvasWidthMap;
  const offsetTime = ctx.offsetTime;
  if (
    trkIdx < 0 ||
      trkIdx >= audioTrackList.length
  ) {
    return false;
  }
  const audioTrack = audioTrackList[trkIdx];
  const audioAssetArray = audioTrack.audioAssetArray;
  if (undefined === audioAssetArray) {
    return false;
  }
  if (undefined === audioWaves) {
    return false;
  }
  //Define an acceptable error range (e.g., 1e-10)
  const EPSILON = 1e-10;
  let astIdx = audioAssetArray.findIndex(item => Math.abs(Number(audioWave[0]) - (item.startTime ?? 0)) < EPSILON);
  if (astIdx == -1) {
    return false;
  }
  const audioAsset = audioAssetArray[astIdx];
  //Obtain the start time
  const oldStartTime = audioAsset.assetId;
  const newStartTime = Math.max(0,audioAsset.assetId + offsetTime);
  Logger.info(TAG,`Old time: ${oldStartTime}, New time: ${newStartTime}`);
  audioAsset.assetId = newStartTime;
  audioAsset.startTime = newStartTime;

  //Processing sound waves
  let waveIndex = audioWaves.findIndex(item => Math.abs(Number(item[0]) - oldStartTime) < EPSILON);

  if (waveIndex === -1) {
    Logger.error(TAG,"No corresponding sound wave found.");
    return false;
  }

  audioWaves[waveIndex][0] = newStartTime.toString();

  //New start time
  let contextMapValue = contextMap.get(oldStartTime.toString());
  if (contextMapValue != undefined) {
    contextMap.delete(oldStartTime.toString());
    contextMap.set(newStartTime.toString(), contextMapValue);
  }

  let canvasStartXMapValue = canvasStartXMap.get(oldStartTime.toString());
  if (canvasStartXMapValue != undefined) {
    canvasStartXMap.delete(oldStartTime.toString());
    canvasStartXMap.set(newStartTime.toString(), canvasStartXMapValue);
  }

  let canvasWidthMapValue = canvasWidthMap.get(oldStartTime.toString());
  if (canvasWidthMapValue != undefined) {
    canvasWidthMap.delete(oldStartTime.toString());
    canvasWidthMap.set(newStartTime.toString(), canvasWidthMapValue);
  }
  //Modify the audio start time
  let res = audioNapi.setAudioAssetStartTime(audioTrack.audioTrackId, oldStartTime, newStartTime);
  if (!res) {
    Logger.error(TAG,
      `Set AudioAsset StartTime failed, trackId is :${audioTrack.audioTrackId} ,AudioAsset old startTime is :${oldStartTime} ,AudioAsset new startTime is :${newStartTime}`);
    return false;
  }
  return true;
}