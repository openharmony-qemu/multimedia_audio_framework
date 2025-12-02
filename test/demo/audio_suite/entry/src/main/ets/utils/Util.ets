/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

export function getDateStringWithTimeStamp(timestamp: number, mode: number = 1): string {
  let date = new Date(timestamp);
  const year = date.getFullYear();
  const month = ('0' + (date.getMonth() + 1)).slice(-2);
  const day = ('0' + date.getDate()).slice(-2);
  const hour = date.getHours();
  const min = date.getMinutes();
  const sec = date.getSeconds();
  const ms = date.getMilliseconds();
  return mode == 0 ? `${year}/${month}/${day} ${hour}:${min}:${sec}:${ms}` :
    `${year}-${month}-${day} ${hour}-${min}-${sec}`;
}