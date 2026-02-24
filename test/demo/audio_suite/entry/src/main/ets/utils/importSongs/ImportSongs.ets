/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { common } from '@kit.AbilityKit';
import { AudioAsset, SongInfo } from '../InterfaceInfo';
import { picker } from '@kit.CoreFileKit';
import { METADATA_SERVICE, MusicMetadata } from './MusicMetadataService';
import { Logger } from '../Logger';

const TAG: string = 'AudioEditTestApp_ImportSongs';

// Import Song
export async function importSongs(playList: Map<string, SongInfo>): Promise<Map<string, SongInfo>> {
  let newMap: Map<string, SongInfo> = new Map();
  try {
    let context =
      getContext() as common.Context; // Please ensure that getContext(this) returns a result of UIAbilityContext.
    let documentPicker = new picker.DocumentViewPicker(context);
    let selectResult: string[] =
      await documentPicker.select({ fileSuffixFilters: ['.wav'], maxSelectNumber: 500 });
    Logger.info(TAG,
      `DocumentViewPicker.select successfully, documentSelectResult uri: ${JSON.stringify(selectResult)}`);
    let tempMap: Map<string, SongInfo> = new Map();
    playList.forEach((song: SongInfo, key: string) => {
      tempMap.set(key, song);
    });
    for (let i = 0; i < selectResult.length; i++) {
      let uri = selectResult[i];
      let index = uri.lastIndexOf('/') + 1;
      let fileName = decodeURIComponent(uri.slice(index));
      let fileType = fileName.substring(fileName.lastIndexOf('.') + 1);
      let info: SongInfo = {
        uri,
        duration: 0,
        songName: fileName,
        songType: fileType,
      };
      if (!tempMap.has(info.uri)) {
        await addToList(uri, info, tempMap);
      }
    }
    newMap = tempMap;
  } catch (error) {
    Logger.error(TAG, `DocumentViewPicker failed with err: ${JSON.stringify(error)}`);
  }
  return newMap;
}

// Import Songs (Modified to Single File Import)
export async function importSingleSong(nodeList: Map<string, SongInfo>,
  nodeId?: string): Promise<Map<string, SongInfo>> {
  let newMap: Map<string, SongInfo> = new Map();
  try {
    let context = getContext() as common.Context;
    let documentPicker = new picker.DocumentViewPicker(context);

    // Modification: Set maxSelectNumber to 1 to restrict selecting only one file.
    let selectResult: string[] =
      await documentPicker.select({
        fileSuffixFilters: ['.pcm', '.wav'],
        maxSelectNumber: 1 // Limitation: Only one file can be selected.
      });
    Logger.info(TAG,
      `DocumentViewPicker.select successfully, documentSelectResult uri: ${JSON.stringify(selectResult)}`);

    // Copy original playlist
    let tempMap: Map<string, SongInfo> = new Map(nodeList);

    // Process only the first file (if any)
    if (selectResult.length > 0) {
      let uri = selectResult[0]; // Take only the first file
      let index = uri.lastIndexOf('/') + 1;
      let fileName = decodeURIComponent(uri.slice(index));
      let fileType = fileName.substring(fileName.lastIndexOf('.') + 1);

      let info: SongInfo = {
        uri,
        duration: 0,
        songName: fileName,
        songType: fileType,
        nodeId: nodeId,
        nodes: []
      };
      // Check if it already exists
      await addToList(uri, info, tempMap, true);
    }
    Logger.warn(TAG, `File ${tempMap.size} ***************`);
    newMap = tempMap;
  } catch (error) {
    Logger.error(TAG, `DocumentViewPicker failed with err: ${JSON.stringify(error)}`);
  }

  return newMap;
}

/**
 * Import non-repetitive songs
 * @param uri      URI of the imported file
 * @param info     Existing song information
 * @param tempMap  Collection of all songs map
 */
export async function addToList(
  uri: string,
  info: SongInfo,
  tempMap: Map<string, SongInfo>,
  isSingle?: boolean
) {
  try {
    const data: MusicMetadata = await METADATA_SERVICE.fetchMetadata(uri);
    if (data.metadata?.title) {
      info.songName = data.metadata.title;
    }
    if (data.metadata?.duration) {
      info.duration = Number(data.metadata.duration);
    }
    if (data.metadata?.artist) {
      info.singerName = data.metadata.artist;
    }
    info.albumName = data.metadata?.album;
    info.albumCover = data.pixelMap;
    if (isSingle && info?.nodeId) {
      tempMap.set(info.nodeId, info);
    } else {
      tempMap.set(info.uri, info);
    }
    return true;
  } catch (error) {
    Logger.error(TAG, `Failed to fetch metadata for ${uri}: ${JSON.stringify(error)}`);
    return false;
  }
}

export function assembleAudioAsset(audioAsset: AudioAsset, value: SongInfo | AudioAsset) {
  audioAsset.uri = value.uri;
  audioAsset.songName = value.songName;
  audioAsset.songType = value.songType;
  audioAsset.nodes = value.nodes;
  audioAsset.singerName = value.singerName;
  audioAsset.albumName = value.albumName;
  audioAsset.albumCover = value.albumCover;
  audioAsset.fd = value.fd;
  audioAsset.duration = value.duration;
}