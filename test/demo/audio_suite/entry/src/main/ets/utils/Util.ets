/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
import { AudioTrack } from "./InterfaceInfo";

export function getDateStringWithTimeStamp(timestamp: number, mode: number = 1): string {
  let date = new Date(timestamp);
  const year = date.getFullYear();
  const month = ('0' + (date.getMonth() + 1)).slice(-2);
  const day = ('0' + date.getDate()).slice(-2);
  const hour = date.getHours();
  const min = date.getMinutes();
  const sec = date.getSeconds();
  const ms = date.getMilliseconds();
  return mode == 0 ? `${year}-${month}-${day}_${hour}:${min}:${sec}` :
    `${year}-${month}-${day} ${hour}-${min}-${sec}`;
}

export function getTimesBySecond(t: number) {
  let h = Math.floor(t / 60 / 60 % 24);
  let m = Math.floor(t / 60 % 60);
  let s = Math.floor(t % 60);
  let hs = h < 10 ? '0' + h : h;
  let ms = m < 10 ? '0' + m : m;
  let ss = s < 10 ? '0' + s : s;
  return `${hs}:${ms}:${ss}`;
}

export function getDate(mode: number) {
  let date = new Date();
  if (mode === 1) {
    return `${date.getFullYear()}/${formatNumber(date.getMonth() + 1)}/${formatNumber(date.getDate())}`;
  } else {
    return `${date.getFullYear()}${formatNumber(date.getMonth() + 1)}${formatNumber(date.getDate())}`;
  }
}

function formatNumber(num: number) {
  if (num <= 9) {
    return '0' + num;
  } else {
    return '' + num;
  }
}

export function sortAudioAssetsByStartTime(audioTrackList: Array<AudioTrack>): void {
  audioTrackList.forEach(track => {
    if (!track.audioAssetArray || !Array.isArray(track.audioAssetArray)) {
      return;
    }

    track.audioAssetArray.sort((a, b) => {
      const startA = a.startTime ?? 0;
      const startB = b.startTime ?? 0;
      return startA - startB;
    });
  });
}