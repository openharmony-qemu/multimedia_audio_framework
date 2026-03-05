/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

import { Logger } from "../../utils/Logger";
import { ScaleMode, TimeMsUnit } from "./TimeLineUtils";
import { AudioAsset, AudioTrack, AudioWaveColorMap } from  "../../utils/InterfaceInfo";
import { LengthMetrics, SymbolGlyphModifier } from "@kit.ArkUI";
import audioNapi from 'libentry.so';
import {
  AudioAssetContext,
  AudioAssetPanContext,
  copyMultipleAudioAsset, copyOneAudioAsset, deleteAudioAsset, panGestureAudioAsset } from "./AudioWaveCopy";

const TAG: string = 'AudioEditTestApp_AudioEditWave';

@Component
export struct AudioEditWave {
  @Link selectedAudioTrackId: string;
  @Link selectedAudioAssetStartTime: number;
  @Link forceRenderMusicIconState: boolean;
  private settings: RenderingContextSettings = new RenderingContextSettings(true);
  //The context corresponding to the canvas component in an audio track. The key is the start time (ms).
  @State contextMap: Map<string, CanvasRenderingContext2D> = new Map();
  //Audio waveform for importing audio.
  @Prop @Watch('resetDrawCanvas') importAudioWaves: string[] = [];
  //Audio wave data stored in an audio track, where the key is the start time (ms).
  @State audioWaves: Array<[string, string[]]> | undefined = undefined;
  @Prop @Watch('changeViewWidth') viewWidth: number = 300;
  @Prop viewHeight: number = 100;
  @State canvasWidth: number = 300;
  @Prop canvasHeight: number = 100;
  @Prop isShowAudioWave: boolean = false;
  @Prop index: number = -1;
  @Prop audioTrackId: string = '';
  //Kneading gesture information
  @State scaleValue: number = 1
  @State pinchValue: number = 1
  //Swipe left or right for information
  @State positionX: number = 0
  @State offsetX: number = 0
  @State canvasStartX: number = 0;
  @State offsetEndX: number = 0;
  private audioWaveLineWide: number = 1;
  // Distance (px) between small ticks on the time bar (10px => 10 minutes)
  @Consume @Watch('updateAudioWave') intervalWidth: number = 2;
  @Consume audioWavePositionX: number = 0;
  // Current ruler mode (hour/minute).
  @Consume divisorMode: ScaleMode = ScaleMode.MODE_TWENTY_MILLISECONDS;
  @Consume @Watch('changeCurrentFirstTick') currentFirstTick: number = 0;
  //The start position of the canvas corresponding to the audio track stored in a track. The key is the start time (ms), and the value is the distance value.
  @State canvasStartXMap: Map<string, number> = new Map();
  //Width of the canvas corresponding to the audio track stored in a track. The key is the start time (ms), and the value is the width.
  @State canvasWidthMap: Map<string, number> = new Map();
  @State count: number = 0;
  @Prop lastCurrentFirstTick: number = 0;
  //Add parameters
  @Link audioTrackList: AudioTrack[];
  @Prop currentToucAudioWave: [string, string[]];
  private touchPositionX: number  = 0;
  private dragPositionX: number  = 0;

  @Link _currentTime: number;
  //Drag gesture information
  @Prop panGestureOffsetX: number = 0;
  @Prop panGesturePositionX: number = 0;
  @Prop panGesture_currentTime: number = 0;
  private panGesturePanOption: PanGestureOptions = new PanGestureOptions({ direction: PanDirection.Horizontal });

  private changeViewWidth() {
    Logger.info(TAG, `changeViewWidth: ${this.viewWidth}`);
    this.currentFirstTick = - Math.ceil(this.viewWidth / 2 / this.intervalWidth);
  }
  private changeCurrentFirstTick() {
    Logger.info(TAG, `changeCurrentFirstTick: ${this.currentFirstTick}, lastCurrentFirstTick: ${this.lastCurrentFirstTick}`);
    //Left distance corresponding to the initial state at 0 scale.
    let initialValue = Math.ceil(this.viewWidth/2);
    //Offset Value
    let offsetValue = -(this.currentFirstTick - this.lastCurrentFirstTick) * this.intervalWidth;
    this.canvasStartXMap.forEach((value: number, key: string) => {
      this.canvasStartXMap.set(key, value + offsetValue);
      Logger.info(TAG,
        `initialValue: ${initialValue}, value: ${value}, offsetValue: ${offsetValue},
         currentFirstTick: ${this.currentFirstTick}`)
    })
    this.lastCurrentFirstTick = this.currentFirstTick;
  }

  aboutToAppear(): void {
    Logger.info(TAG, 'aboutToAppear');
    // this.contextMap.set('0', new CanvasRenderingContext2D(this.settings));
  }

  private resetDrawCanvas() {
    Logger.info(TAG, `resetDrawCanvas: isShowAudioWave: ${this.isShowAudioWave}, length: ${this.importAudioWaves.length}, index: ${this.index}`);
    if (this.audioWaves && this.audioWaves.length > 0) {
      return;
    }

    let startTime: number = this.audioTrackList?.[this.index]?.audioAssetArray?.[0]?.startTime ?? 0;
    this.contextMap.set(startTime.toString(), new CanvasRenderingContext2D(this.settings));
    this.canvasStartXMap.set(startTime.toString(),  - this.currentFirstTick * this.intervalWidth + (startTime * (this.intervalWidth / TimeMsUnit.TWENTY_MILLISECONDS)));
    this.canvasWidthMap.set(startTime.toString(), this.importAudioWaves.length * this.audioWaveLineWide +
      (this.importAudioWaves.length - this.audioWaveLineWide) * (this.intervalWidth - this.audioWaveLineWide));
    this.audioWaves = [[startTime.toString(), this.importAudioWaves]];
    if (this.isShowAudioWave) {
      this.drawCanvas();
    }
  }

  private updateAudioWave() {
    this.drawCanvas();
  }

  // Handle wave selection
  private handleWaveSelection(item: [string, string[]]) {
    if (this.selectedAudioTrackId === this.audioTrackId &&
      this.selectedAudioAssetStartTime === Number(item[0])) {
      this.selectedAudioAssetStartTime = -1;
      this.selectedAudioTrackId = '';
    } else {
      this.selectedAudioAssetStartTime = Number(item[0]);
      this.selectedAudioTrackId = this.audioTrackId;
    }
  }

  // Handle crop dragging
  private handleCropDrag(event: TouchEvent, side: 'left' | 'right', item: [string, string[]]) {
    if (event.type === TouchType.Down) {
      this.dragPositionX = event.touches[0].x;
    } else if (event.type === TouchType.Move) {
      this.drawCanvas();
    } else if (event.type === TouchType.Up) {
      const oldStartTime = item[0];
      const finalX = event.touches[0].x;
      const deltaX = finalX - this.dragPositionX;
      const totalDeltaX = Math.abs(finalX - this.dragPositionX);
      if (side === 'left') {
        if (deltaX < 0){
          return;
        }
        const newPosition = (this.canvasStartXMap.get(item[0]) ?? 0) + totalDeltaX;
        const newStarTime = Math.ceil(Number(item[0]) + totalDeltaX * TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth);
        //When you drag an item on the left, the key value of the contextMap needs to be updated.
        this.contextMap.delete(item[0]);
        this.contextMap.set(newStarTime.toString(), new CanvasRenderingContext2D(this.settings));
        //Updated the start time and start rendering position in canvasStartXMap.
        this.canvasStartXMap.delete(item[0]);
        this.canvasStartXMap.set(newStarTime.toString(), newPosition);
        //Start time for updating canvasWidthMap.
        const width = this.canvasWidthMap.get(item[0]) ?? 0;
        this.canvasWidthMap.delete(item[0]);
        this.canvasWidthMap.set(newStarTime.toString(), width - totalDeltaX);
        //Update audioWaves
        const tempWave = this.currentToucAudioWave[1];
        const percentCut = totalDeltaX / width;
        const cutWave = Math.floor(this.currentToucAudioWave[1].length * percentCut);
        this.currentToucAudioWave[1] = tempWave.slice(cutWave, this.currentToucAudioWave[1].length);
        let index = this.audioWaves?.findIndex(wave => wave[0] === item[0]) ?? -1;
        if (index >= 0 && this.audioWaves !== undefined) {
          this.audioWaves[index][0] = newStarTime.toString();
          this.audioWaves[index][1] = this.currentToucAudioWave[1];
        }
        //Called the underlying layer, added data first and then deleted data, and updated startTime, duration, buffer, and assestId.
        // 1. Securely obtaining audioAssetArray
        const audioAssetArray = this.audioTrackList[this.index]?.audioAssetArray;
        if (!audioAssetArray) {
          console.warn('The audioAssetArray does not exist.');
          return;
        }
        let audioAsset = audioAssetArray.find(asset => asset.assetId === Number(oldStartTime));
        let cutIndex = audioAssetArray.findIndex(asset => asset.assetId === Number(oldStartTime));
        if (!audioAsset) {
          Logger.warn(TAG, `No audio asset with assetId ${oldStartTime} is found.`);
          return;
        }
        //Calculate the new duration and insert it back into the audioTrackList.
        if (audioAsset) {
          Logger.warn(TAG,`newStarTime is ${newStarTime}`)
          //Assemble the audioasset updates in ets
          const updatedAudioAsset: AudioAsset = {
            assetId: newStarTime,
            uri: audioAsset.uri,
            songName: audioAsset.songName,
            songType: audioAsset.songType,
            nodes: audioAsset.nodes,
            singerName: audioAsset.singerName,
            albumCover: audioAsset.albumCover,
            albumName: audioAsset.albumName,
            channels: audioAsset.channels,
            sampleRate: audioAsset.sampleRate,
            bitsPerSample: audioAsset.bitsPerSample,
            startTime: newStarTime,
            duration: (audioAsset.duration ?? 0) - (totalDeltaX * TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth)
          };
          audioAssetArray[cutIndex] = updatedAudioAsset;
          let newStartIndex =
            (totalDeltaX * TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth) * updatedAudioAsset.sampleRate * updatedAudioAsset.channels * updatedAudioAsset.bitsPerSample / 8 / 1000;
          //If only the left side changes, the relative displacement of the right side remains unchanged, and endIndex is calculated based on the duration of the previous audio.
          let newEndIndex = (audioAsset.duration ?? 0) * updatedAudioAsset.sampleRate * updatedAudioAsset.channels * updatedAudioAsset.bitsPerSample / 8 / 1000;
          let indexs: number[] = [newStartIndex,newEndIndex];
          audioNapi.addAudioAsset(this.audioTrackList[this.index].audioTrackId, Number(oldStartTime), newStarTime, indexs, false);
          audioNapi.deleteAudioAsset(this.audioTrackList[this.index].audioTrackId, Number(oldStartTime));
        }
      } else if (side === 'right') {
        if (deltaX > 0){
          return;
        }
        const width = this.canvasWidthMap.get(item[0]) ?? 0;
        this.canvasWidthMap.set(item[0], width-totalDeltaX);
        //Update audioWaves
        const tempWave = this.currentToucAudioWave[1];
        const percentCut = totalDeltaX / width;
        const cutWave = Math.floor(this.currentToucAudioWave[1].length * percentCut);
        this.currentToucAudioWave[1] = tempWave.slice(0, this.currentToucAudioWave[1].length - cutWave);
        let index = this.audioWaves?.findIndex(wave => wave[0] === item[0]) ?? -1;
        if (index >= 0 && this.audioWaves !== undefined) {
          this.audioWaves[index][0] = item[0];
          this.audioWaves[index][1] = this.currentToucAudioWave[1];
        }
        //For cropping on the right, only pcmBufferLength needs to be updated.
        const audioAssetArray = this.audioTrackList[this.index]?.audioAssetArray;
        if (!audioAssetArray) {
          console.warn('The AudioAssetArray does not exist.');
          return;
        }
        let audioAsset = audioAssetArray.find(asset => asset.assetId === Number(oldStartTime));
        let cutIndex = audioAssetArray.findIndex(asset => asset.assetId === Number(oldStartTime));
        if (!audioAsset) {
          Logger.warn(TAG, `No audio asset with assetId ${oldStartTime} is found.`);
          return;
        }
        if (audioAsset) {
          //Calculate the new duration and insert it back into the audioTrackList.
          const updatedAudioAsset: AudioAsset = {
            assetId: audioAsset.assetId,
            uri: audioAsset.uri,
            songName: audioAsset.songName,
            songType: audioAsset.songType,
            nodes: audioAsset.nodes,
            singerName: audioAsset.singerName,
            albumCover: audioAsset.albumCover,
            albumName: audioAsset.albumName,
            channels: audioAsset.channels,
            sampleRate: audioAsset.sampleRate,
            bitsPerSample: audioAsset.bitsPerSample,
            startTime: audioAsset.startTime,
            duration: Math.ceil((audioAsset.duration ?? 0) - (totalDeltaX * TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth))
          };
          audioAssetArray[cutIndex] = updatedAudioAsset;
          let startIndex = 0;
          let newEndIndex = (updatedAudioAsset.duration ?? 0) * updatedAudioAsset.sampleRate * updatedAudioAsset.channels * updatedAudioAsset.bitsPerSample / 8 / 1000;
          audioNapi.updateAudioAsset(this.audioTrackList[this.index].audioTrackId, Number(oldStartTime), startIndex, newEndIndex);
        }
      }
    }
  }


  build() {
    Row() {
      // Original text display when audio wave is hidden
      Text($r('app.string.audio_waves'))
        .height(40)
        .width(this.isShowAudioWave ? 0 : this.viewWidth)
        .backgroundColor(AudioWaveColorMap[`MUSIC_${this.index % 5 + 1}`])

      // Audio wave display
      ForEach(this.audioWaves, (item: [string, string[]]) => {
        // Main container for each audio wave segment
        Stack({ alignContent: Alignment.Start }) {
          // Canvas for drawing the audio wave
          Canvas(this.contextMap.get(item[0]))
            .onReady(() => {
              this.drawCanvas();
            })
            .onTouch((event: TouchEvent) => {
              if (event.type === TouchType.Down) {
                this.touchPositionX = event.touches[0].x;
                this.currentToucAudioWave = item;
              }
            })
            .bindContextMenu(this.AudioWaveMenu, ResponseType.LongPress)
            .onClick((event: ClickEvent) => {
              this.handleWaveSelection(item);
            })
            .gesture(
              //Drag gesture event
              PanGesture(this.panGesturePanOption)
                .onActionStart(() => {
                  this.panGestureOffsetX = this.canvasStartXMap.get(item[0]) || 0;
                  this.panGesture_currentTime = this._currentTime;
                })
                .onActionUpdate((event: GestureEvent) => {
                  if (event.fingerList.length > 1) {
                    return; //Ignore multi-finger dragging.
                  }
                  if (this.selectedAudioTrackId === this.audioTrackId &&
                    this.selectedAudioAssetStartTime === Number(item[0])) {
                    return; //Cannot be dragged when selected
                  }
                  if (event) {
                    // let offsetTime =  event.offsetX * ( TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth);
                    // this._currentTime = Math.max(0,this.panGesture_currentTime + offsetTime);
                    this.panGesturePositionX = Math.max(- this.currentFirstTick * this.intervalWidth,this.panGestureOffsetX + event.offsetX);
                    this.canvasStartXMap.set(item[0],this.panGesturePositionX);
                    // 当音波头部接近可视区域边缘时滚动时间轴
                    if ((event.fingerList[0].localX +  this.panGesturePositionX) < this.viewWidth * 0.1 && event.velocityX < 0) {
                      this.scrollTimeline(-50); // 向左滚动
                    } else if ((event.fingerList[0].localX +  this.panGesturePositionX) > this.viewWidth * 0.9 && event.velocityX > 0) {
                      this.scrollTimeline(50); // 向右滚动
                    }

                  }
                })
                .onActionEnd((event: GestureEvent) => {
                  if (event.fingerList.length > 1) {
                    return; //Ignore multi-finger dragging.
                  }
                  if (this.selectedAudioTrackId === this.audioTrackId &&
                    this.selectedAudioAssetStartTime === Number(item[0])) {
                    return; //Cannot be dragged when selected
                  }
                  this.panGesturePositionX = Math.max(- this.currentFirstTick * this.intervalWidth,this.panGestureOffsetX + event.offsetX);
                  this.canvasStartXMap.set(item[0],this.panGesturePositionX);
                  let isOverlap = this.checkOverlap(this.panGesturePositionX,item[0]);
                  if(isOverlap){
                    this.canvasStartXMap.set(item[0],this.panGestureOffsetX);
                    this._currentTime = this.panGesture_currentTime;
                  }else{
                    //Data processing logic
                    this.currentToucAudioWave = item;
                     let offsetTime = Math.ceil((this.panGesturePositionX - this.panGestureOffsetX) * ( TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth) + this._currentTime - this.panGesture_currentTime);
                    Logger.info(TAG,`offsetTime ${offsetTime}`);
                    const ctx: AudioAssetPanContext = {
                      trkIdx : this.index,
                      audioTrackList : this.audioTrackList,
                      audioWave :  this.currentToucAudioWave,
                      audioWaves : this.audioWaves,
                      contextMap : this.contextMap,
                      canvasStartXMap : this.canvasStartXMap,
                      canvasWidthMap : this.canvasWidthMap,
                      settings : this.settings,
                      offsetTime: offsetTime,
                    };
                    panGestureAudioAsset(ctx);
                  }
                })
            )
            .width(this.isShowAudioWave ? this.canvasWidthMap.get(item[0]) : 0)
            .height(this.canvasHeight)
            .backgroundColor(AudioWaveColorMap[`MUSIC_${this.index % 5 + 1}`])
            .borderRadius(this.audioTrackId === this.selectedAudioTrackId &&
              Number(item[0]) === this.selectedAudioAssetStartTime ? 0 : 10)
            .border(this.audioTrackId === this.selectedAudioTrackId &&
              Number(item[0]) === this.selectedAudioAssetStartTime ? {
              width: {
                top: '1vp',
                bottom: '1vp'
              },
              color: '#fff9f5f3'
            } : {})

          // Left crop handle (only visible when selected)
          if(this.selectedAudioTrackId === this.audioTrackId &&
            this.selectedAudioAssetStartTime === Number(item[0])) {
            Text() {
            }
            .width(8)
            .height(this.canvasHeight)
            .position({ x: -8 })
            .zIndex(100)
            .backgroundColor(Color.White)
            .onTouch((event: TouchEvent) => {
              this.handleCropDrag(event, 'left', item);
            })
          }

          // Right crop handle (only visible when selected)
          if(this.selectedAudioTrackId === this.audioTrackId &&
            this.selectedAudioAssetStartTime === Number(item[0])) {
            Text() {
            }
            .width(8)
            .height(this.canvasHeight)
            .zIndex(100)
            .position({ x: this.canvasWidthMap.get(item[0])?? 300})
            .backgroundColor(Color.White)
            .onTouch((event: TouchEvent) => {
              this.handleCropDrag(event, 'right', item);
            })
          }
        }
        .width(this.isShowAudioWave ? this.canvasWidthMap.get(item[0]) : 0)
        .height(this.canvasHeight)
        .position({ top: (this.viewHeight - this.canvasHeight) / 2,
          left: this.canvasStartXMap.get(item[0]) })
        .zIndex(50)
      })
    }
    .width(this.viewWidth)
    .height(this.viewHeight)
  }


  drawCanvas() {
    this.contextMap.forEach((value: CanvasRenderingContext2D, key: string) => {
      let waves: string[] = [];
      this.audioWaves?.forEach((item: [string, string[]]) => {
        if (item[0] === key) {
          waves = item[1];
          return;
        }
      })
      if (waves.length === 0) {
        return;
      }
      value.reset();
      value.lineCap = 'round';
      value.lineWidth = this.audioWaveLineWide;
      value.strokeStyle = AudioWaveColorMap[`MUSIC_AUDIO_WAVES_${this.index % 5 + 1}`];
      setTimeout(() => {
        value.beginPath();
        for (let i = 0; i < waves.length; i++) {
          //The purpose of - this.offsetStartX is to set the left start point of the canvas component to this.canvasStartX.
          const currentX = (this.divisorMode === ScaleMode.MODE_TWENTY_MILLISECONDS) ?
            this.intervalWidth * i :
            this.offsetX + this.intervalWidth * i * 0.1;
          value.moveTo(currentX, this.canvasHeight / 2);
          value.lineTo(currentX, (this.canvasHeight / 2) * (1 + Number(waves[i])));
          value.moveTo(currentX, this.canvasHeight / 2);
          value.lineTo(currentX, (this.canvasHeight / 2) * (1 - Number(waves[i])));
        }
      value.stroke();
      }, 10)
    })
  }

  onAudioWaveClick() {
    //Obtains the start coordinates of the first canvas after the split.
    const firstCanvasStartX = this.canvasStartXMap.get(this.currentToucAudioWave[0]) ?? 0;
    //Obtains the start coordinates of the second canvas after the split.
    const secondCanvasStartX = firstCanvasStartX + this.touchPositionX;
    //Obtains the total length of the currently clicked audio waveform.
    const currentAudioWavesWidth = this.canvasWidthMap.get(this.currentToucAudioWave[0]) ?? 0;
    //Obtains the total time when the audio wave is currently clicked.
    const currentAudioWavesTime = this.currentToucAudioWave[1].length * 20;
    //Obtains the first audio start time (ms).
    const firstStartTime = Number(this.currentToucAudioWave[0]);
    //Obtains the second audio start time (ms).
    const secondStartTime = Math.ceil(firstStartTime + (this.touchPositionX / currentAudioWavesWidth) * currentAudioWavesTime);
    //Obtains the total number of audio waves currently clicked.
    const currentAudioWavesCount = this.currentToucAudioWave[1].length;
    //Obtains the lower bound of the first audio waveform after split.
    const firstEndIndex = Math.floor((this.touchPositionX / currentAudioWavesWidth) * currentAudioWavesCount);
    //Obtains the first audio wave data after split.
    const firstAudioWaves = this.currentToucAudioWave[1].slice(0, firstEndIndex);
    //Obtains the second audio waveform data after split.
    const secondAudioWaves = this.currentToucAudioWave[1].slice(firstEndIndex);
    //Delete the original audio data from audioWaves.
    for (let i = 0; i < (this.audioWaves?.length ?? 0); i++) {
      if (!this.audioWaves) {
        return;
      }
      if (this.audioWaves[i][0] === firstStartTime.toString()) {
        this.audioWaves.splice(i, 1);
        Logger.info(TAG, `delete success`);
      }
    }
    //Delete the original audio wave data that was clicked.
    this.contextMap.delete(this.currentToucAudioWave[0]);
    //Destroys the context of the currently clicked canvas.

    //Re-assign a value to the map.
    this.contextMap.set(firstStartTime.toString(), new CanvasRenderingContext2D(this.settings));
    this.contextMap.set(secondStartTime.toString(), new CanvasRenderingContext2D(this.settings));

    //Assign a new value to audioWaves.
    this.audioWaves?.push([firstStartTime.toString(), firstAudioWaves]);
    this.audioWaves?.push([secondStartTime.toString(), secondAudioWaves]);
    // 给canvasStartXMap重新赋值
    this.canvasStartXMap.set(firstStartTime.toString(), firstCanvasStartX);
    this.canvasStartXMap.set(secondStartTime.toString(), secondCanvasStartX);
    //Re-assign the canvasStartXMap value.
    this.canvasWidthMap.set(firstStartTime.toString(), this.touchPositionX );
    this.canvasWidthMap.set(secondStartTime.toString(), currentAudioWavesWidth - this.touchPositionX );

    //Updating the audioTrackList
    const currentAudioAssetIndex: number =
      this.audioTrackList[this.index].audioAssetArray!.findIndex(asset => asset.assetId === Number(firstStartTime));
    if (currentAudioAssetIndex === -1) {
      Logger.info(TAG, 'failed to split audio');
      return;
    }
    const audioAsset = this.audioTrackList[this.index].audioAssetArray![currentAudioAssetIndex];
    const firstAudioAsset: AudioAsset = {
      assetId: Number(firstStartTime),
      uri: audioAsset.uri,
      songName: audioAsset.songName,
      songType: audioAsset.songType,
      nodes: audioAsset.nodes,
      singerName: audioAsset.singerName,
      albumCover: audioAsset.albumCover,
      albumName: audioAsset.albumName,
      channels: audioAsset.channels,
      sampleRate: audioAsset.sampleRate,
      bitsPerSample: audioAsset.bitsPerSample,
      startTime: Number(firstStartTime),
      duration: 20 * firstAudioWaves.length
    };
    this.audioTrackList[this.index].audioAssetArray![currentAudioAssetIndex] = firstAudioAsset;
    const secondAudioAsset: AudioAsset = {
      assetId: Number(secondStartTime),
      uri: audioAsset.uri,
      songName: audioAsset.songName,
      songType: audioAsset.songType,
      nodes: audioAsset.nodes?.slice(),
      singerName: audioAsset.singerName,
      albumCover: audioAsset.albumCover,
      albumName: audioAsset.albumName,
      channels: audioAsset.channels,
      sampleRate: audioAsset.sampleRate,
      bitsPerSample: audioAsset.bitsPerSample,
      startTime: Number(secondStartTime),
      duration: 20 * secondAudioWaves.length
    };
    this.audioTrackList[this.index].audioAssetArray!.push(secondAudioAsset as AudioAsset);
    //When an audio file is added, add the second audio file first and then update the first audio file, to prevent the C++ layer from deleting the original audio file.
    let startIndex = (Number(secondStartTime) - Number(firstStartTime)) * audioAsset.sampleRate * audioAsset.channels * audioAsset.bitsPerSample / 8 / 1000;
    let endIndex = startIndex + 20 * (secondAudioWaves.length - 2) * audioAsset.sampleRate * audioAsset.channels * audioAsset.bitsPerSample / 8 / 1000;
    Logger.info(TAG,
      `onAudioWaveClick startIndex : ${Math.floor(startIndex)}, endIndex: ${Math.floor(endIndex)}, sampleRate: ${audioAsset.sampleRate}, channels: ${audioAsset.channels}, bitsPerSample: ${audioAsset.bitsPerSample}, secondStartTime: ${Number(secondStartTime)}`);
    let indexs: number[] = [Math.floor(startIndex),Math.floor(endIndex)];
    audioNapi.addAudioAsset(this.audioTrackId, Number(firstStartTime), Number(secondStartTime), indexs, false);
    //Update audio
    startIndex = 0;
    endIndex = startIndex + 20 * firstAudioWaves.length * audioAsset.sampleRate * audioAsset.channels * audioAsset.bitsPerSample / 8 / 1000;
    Logger.info(TAG, `onAudioWaveClick startIndex : ${Math.floor(startIndex)}, endIndex: ${Math.floor(endIndex)}`);
    audioNapi.updateAudioAsset(this.audioTrackId, Number(firstStartTime), Math.floor(startIndex), Math.floor(endIndex));
    Logger.info(TAG,
      `onAudioWaveClick firstCanvasStartX: ${firstCanvasStartX}, firstCanvasWidth: ${this.touchPositionX },
                secondcanvasStartX: ${secondCanvasStartX}, secondcanvasWidth: ${currentAudioWavesWidth - this.touchPositionX},
                firstStartTime: ${firstStartTime}, secondStartTime: ${secondStartTime},
                currentAudioWavesTime: ${currentAudioWavesTime}`);
  }


  @State copyIconModifier: SymbolGlyphModifier = new SymbolGlyphModifier($r('sys.symbol.checkmark_square_on_square')).fontSize('24vp');
  @State splitIconModifier: SymbolGlyphModifier = new SymbolGlyphModifier($r('sys.symbol.rectangle_split_3x1')).fontSize('24vp');
  @State deleteIconModifier: SymbolGlyphModifier = new SymbolGlyphModifier($r('sys.symbol.trash')).fontSize('24vp');
  @Builder
  SubMenu() {
    Menu() {
      MenuItem({ symbolStartIcon: this.copyIconModifier, content: "单次" })
        .onClick(() => {
        //Processing single copy logic
        copyOneAudioAsset(this.createAudioAssetContext());
      });
      MenuItem({ symbolStartIcon: this.copyIconModifier, content: "平铺" })
        .onClick(() => {
        //Processing Multiple copy logic
         copyMultipleAudioAsset(this.createAudioAssetContext());
      });

    }.menuItemDivider({
      strokeWidth: LengthMetrics.vp(1),
      color: '#d5d5d5',
      mode: DividerMode.EMBEDDED_IN_MENU
    })
  }


  @Builder
  AudioWaveMenu() {
    Menu() {
      MenuItem({
        symbolStartIcon: this.copyIconModifier,
        content: "复制音频",
        builder: (): void => this.SubMenu()
      });
      MenuItem({
        symbolStartIcon: this.splitIconModifier,
        content: "分割"
      }).onClick(() => {
        this.onAudioWaveClick();
      });
      MenuItem({
        symbolStartIcon: this.deleteIconModifier,
        content: "删除"
      }).onClick(() => {
        // 处理删除逻辑
        deleteAudioAsset(this.createAudioAssetContext());
        // force render music icon
        this.forceRenderMusicIconState = !this.forceRenderMusicIconState;
      });
    }.menuItemDivider({
      strokeWidth: LengthMetrics.vp(1),
      color: '#d5d5d5',
      mode: DividerMode.EMBEDDED_IN_MENU
    })
  }

  //Core methods for overlap detection
  private checkOverlap(x: number, startTime: string): boolean {
    const currentWidth = this.canvasWidthMap.get(startTime) || 0;
    const currentEnd = x + currentWidth;
    for (const key of this.canvasStartXMap.keys()) {
      const otherStartX = this.canvasStartXMap.get(key);
      //Skip over self and invalid items
      if (startTime === key || otherStartX === undefined) continue;
      const otherWidth = this.canvasWidthMap.get(key);
      if (!otherWidth) continue;
      const otherEnd = otherStartX + otherWidth;
      if (x < otherEnd && currentEnd > otherStartX) {
        return true; //Return immediately if overlap is detected
      }
    }
    return false; //No overlap
  }

  //Scroll timeline
  scrollTimeline(delta: number) {
    //Implements the timeline scroll logic
    this._currentTime += delta;
    //Ensure the time is within a reasonable range
    if (this._currentTime < 0) {
      this._currentTime = 0;
    }
  }

  createAudioAssetContext(): AudioAssetContext {
    return {
      trkIdx : this.index,
      audioTrackList : this.audioTrackList,
      audioWave :  this.currentToucAudioWave,
      audioWaves : this.audioWaves,
      contextMap : this.contextMap,
      canvasStartXMap : this.canvasStartXMap,
      canvasWidthMap : this.canvasWidthMap,
      settings : this.settings,
    };
  }
}