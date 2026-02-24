/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. All rights reserved.
 */

import { importSongs } from '../utils/importSongs/ImportSongs';
import { SongInfo } from '../utils/InterfaceInfo';
import audioNapi from 'libentry.so';
import fileUri from '@ohos.file.fileuri';
import fs, { ReadOptions } from '@ohos.file.fs';
import { Logger } from '../utils/Logger';
import { BusinessError } from '@kit.BasicServicesKit';
import { common } from '@kit.AbilityKit';
import fileio from '@ohos.fileio';
import { tag } from '@kit.ConnectivityKit';
import { util } from '@kit.ArkTS';

const TAG: string = 'AudioEditTestApp_SongBox';

@Builder
export function SongBoxBuilder() {
  SongBox()
}

@Component
export struct SongBox {
  pageInfos: NavPathStack = new NavPathStack();
  @StorageLink('playList') playList: Map<string, SongInfo> = new Map(); // Playlist, using Map for deduplication
  @State fileSize: number = 0;
  @State fileBuffer: ArrayBuffer = new ArrayBuffer(0);

  build() {
    NavDestination() {
      Column() {
        List({ initialIndex: 0 }) {
          if (this.playList.size != 0) {
            ForEach(Array.from(this.playList?.values()), (info: SongInfo) => {
              if (info.uri !== 'format' && info.uri !== 'resultPcmBuffer') {
                ListItem() {
                  Column() {
                    this.songItem(info);
                  }
                }
                .padding($r('app.float.padding_10'))
                .margin({ top: '10vp' })
                .backgroundColor(Color.Black)
                .border({
                  width: 3,
                  color: Color.Black,
                  radius: 10,
                  style: BorderStyle.Solid
                })
              }
            }, (info: SongInfo) => JSON.stringify(info))
          }
        }
      }
      .margin({ top: $r('app.float.margin_35') })
    }
    .onReady((context: NavDestinationContext) => {
      this.pageInfos = context.pathStack;
    })
    .title('音乐Box')
    .menus(this.NavigationMenus())
  }

  @Builder
  NavigationMenus() {
    Row() {
      Button($r('app.string.import'))
        .onClick((event: ClickEvent) => {
          importSongs(this.playList).then((songMap: Map<string, SongInfo>) => {
            this.playList = songMap;
          });
        })
        .width('60vp')
        .height('30vp')
    }
    .margin({ top: '10vp' })
  }

  /**
   * Song Bar
   * @param info  Song Information
   */
  @Builder
  songItem(info: SongInfo) {
    Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
      Image($r('app.media.icon_home_withdraw'))
        .width($r('app.float.width_50'))
        .height($r('app.float.height_50'))
        .margin({ right: $r('app.float.margin_35') })
      Text(info.songName).margin({ right: $r('app.float.margin_35') }).fontColor(Color.White)
    }.margin({ bottom: $r('app.float.margin_10') })

    Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
      Button($r('app.string.addSong')).onClick((event: ClickEvent) => {
        const result = this.getPcmFileBuffer(info.uri, info.songType);
        if (!result) {
          this.getUIContext().showAlertDialog({
            message: '只支持wav格式中44100/48000Hz的音频',
            confirm: {
              value: '确定',
              action: () => {
              }
            },
            cancel: () => {
              Logger.info(TAG, 'cancel');
            }
          });
        } else {
          // Add audio, create input and output nodes, and set the audio file format
          this.pageInfos.pop(this.playList.get(info.uri));
        }
      })
      Button($r('app.string.delete'))
        .backgroundColor(Color.Red)
        .fontColor(Color.White)
        .onClick(() => {
          const songInfo = this.playList.get(info.uri);
          if (songInfo && songInfo.fd) {
            try {
              fs.closeSync(songInfo.fd);
              Logger.info(TAG, `File fd: ${songInfo.fd} closed successfully`);
            } catch (e) {
              Logger.error(TAG, `Failed to close file fd: ${songInfo.fd}, error: ${JSON.stringify(e)}`);
            }
          }
          this.playList.delete(info.uri);
        })
    }
  }

  getPcmFileBuffer(uri: string, songType?: string): boolean {
    let file: fs.File;
    try {
      let path: string = new fileUri.FileUri(uri).path;
      Logger.info(TAG, `getPcmFileBuffer path: ${JSON.stringify(path)}`);
      file = fs.openSync(path, fs.OpenMode.READ_ONLY | fs.OpenMode.CREATE);
      let fsStat = fs.statSync(path);
      Logger.info(TAG, `fileSize : ${fsStat.size}`);
      let buffer = new ArrayBuffer(fsStat.size);
      let readOption: ReadOptions = {
        offset: 0, // The expected position to read the file from. Optional, defaults to reading from the current position
        length: fsStat.size // The length of data expected to be read each time. Optional, default buffer length
      }
      fs.readSync(file.fd, buffer, readOption);
      Logger.info(TAG, `getPcmFileBuffer buffer length: ${buffer.byteLength}`);
      let songName = this.playList.get(uri)?.songName ?? '';
      this.playList.set(uri, {
        uri: uri,
        songName: songName,
        songType: songType,
        wavBuffer: buffer as ArrayBuffer,
        fd: file.fd,
      });
      return true;
    } catch (e) {
      Logger.error(TAG, `getPcmFileBuffer error: ${JSON.stringify(e)}`);
      return false;
    }
  }
}