/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

export const audioCapturerInit: (isRealPlay: boolean, inputId: string, mixerId: string, outputId: string, startTime: number, isPure: boolean) => void;
export const audioCapturerStart: () => void;
export const audioCapturerStop: () => void;
export const audioCapturerRelease: () => void;
export const getAudioFrames: () => ArrayBuffer;
export const mixRecordBuffer: (inputId: string, mixerId: string, outputId: string) => void;
export const mixPlayInitBuffer: (inputId: string, mixerId: string, outputId: string, startTime: number) => void;
export const audioCapturerPause: () => void;
export const clearRecordBuffer: () => void;
export const realPlayRecordBuffer: (inputId: string) => ArrayBuffer;