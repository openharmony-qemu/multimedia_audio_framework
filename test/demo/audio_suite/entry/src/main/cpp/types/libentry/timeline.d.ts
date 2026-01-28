/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

export const addAudioTrack: (trackId: string, isSilent: boolean) => boolean;
export const deleteAudioTrack: (trackId: string) => boolean;
export const setAudioTrackSilent: (trackIds: string[], isSilents: boolean[]) => boolean;
export const addAudioAsset: (trackId: string, oldStartTime: number, newStartTime: number, indexs: number[], isCopyMultiple: boolean) => boolean;
export const updateAudioAsset: (trackId: string, startTime: number, startIndex: number, endIndex: number) => boolean;
export const deleteAudioAsset: (trackId: string, startTime: number) => boolean;
export const setAudioAssetStartTime: (trackId: string, originStartTime: number, newStartTime: number) => boolean;
export const setAudioAssetPcmBufferLength: (trackId: string, startTime: number, pcmBufferLength: number) => boolean;
export const addAudioAssetEffectNode: (trackId: string, startTime: number, effectNodeId: string) => boolean;
export const deleteAudioAssetEffectNode: (trackId: string, startTime: number, effectNodeId: string) => boolean;
export const clearTimeline: () => void;