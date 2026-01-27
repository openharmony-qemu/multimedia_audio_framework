/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

import { Logger } from "../../utils/Logger";
import {
  TimeMsUnit,
  ScaleAlignment,
  TimeScaleOption,
  TickRange,
  ScaleMode,
  MiddleIndicatorOption,
  TimePickedCallback,
  PlayStatus
} from "./TimeLineUtils";
import { displaySync } from "@kit.ArkGraphics2D";

const TAG: string = 'AudioEditTestApp_TimeLine';

@Component
export struct TimeLine {
  // @Require model: TimeBarModel
  @Prop viewWidth: number = 300;
  @Prop viewHeight: number = 50;
  private settings: RenderingContextSettings = new RenderingContextSettings(true);
  // Canvas 2D context bound to the host Canvas view.
  private context: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings);
  // Whether a frame drawing is in progress (prevents re-entrant zoom).
  private isDrawing: boolean = false;
  // Distance (px) between small ticks on the time bar (10px => 10 minutes)
  @Consume intervalWidth: number = 2;
  @Consume audioWavePositionX: number = 0;
  // Current ruler mode (hour/minute).
  @Consume divisorMode: ScaleMode = ScaleMode.MODE_TWENTY_MILLISECONDS;
  private zoomSize: number = 0;
  // Pinch zoom state
  private _inPinch: boolean = false;
  // Batch multiple requests and redraw only once in the next frame
  private _pendingRedraw: boolean = false;
  private _displaySyncRunning: boolean = false;
  private _displaySync: displaySync.DisplaySync | undefined = undefined;
  // HarmonyOS DisplaySync state (replaces rAF scheduling).
  private readonly FRAME_60: number = 60;
  private readonly FRAME_120: number = 120;
  private _dsCallback: ((info: displaySync.IntervalInfo) => void) | null = null;
  // Stop DisplaySync automatically after N idle frames (N >= 2)
  private _idleFrameCount: number = 0;
  private LABEL_PADDING: number = 4;
  // Whether the finger/mouse is currently down on the canvas.
  private _touchDownFlag: boolean = false;
  // Last pointer x-position for delta computation.
  private _touchDownPosition: number = 0;
  // Accumulate fractional pan deltas between gesture frames.
  private _panResidual: number = 0;
  // Listener for notifying time change while moving/auto-scrolling.
  private _onTimeBarMoveListener: TimePickedCallback | null = null;
  // Movement sensitivity (in px); too small causes frequent refresh.
  private MOVE_SENSITIVE = 1; // Reduced threshold for smoother low-speed pan tracking.
  // Current middle-line time (epoch ms).
  @Link @Watch('scheduleRedraw') _currentTime: number;
  @Consume xPosition: number = 0;
  @Consume currentFirstTick: number = 0;
  // Epoch ms corresponding to the far left edge of the canvas.
  private leftTime: number = 0;
  // Drag bounds (epoch ms). Set after segments are prepared.
  private _minTime = Number.MIN_SAFE_INTEGER;
  private _maxTime = Number.MAX_SAFE_INTEGER;
  @Prop @Watch('onPlayStatusChange') isPlay: boolean = false;
  private intervalID: number | null = null;
  // Middle indicator options
  @State middleIndicatorOption: MiddleIndicatorOption = {
    length: 500,
    fillColor: '#8161c4',
  }

  @State @Watch('onTimeScaleOptionChange') timeScaleOption: TimeScaleOption = {
    scaleLineWidth: 1,
    scaleLineHeight: 8,
    scaleTextColor: '#8161c4',
    scaleLineColor: '#8161c4',
    scaleAlignment: ScaleAlignment.TOP
  }

  private onPlayStatusChange() {
    if (this.isPlay) {
      let startTime = Date.now();
      this.intervalID = setInterval(() => {
        let endTime = Date.now();
        this._currentTime = endTime - startTime;
        this.scheduleRedraw();
      }, 2)
    } else {
      clearInterval(this.intervalID);
    }
  }

  build() {
    Column() {
      Canvas(this.context)
        .width(this.viewWidth)
        .height(this.viewHeight)
        .onReady(() => {
          this.redrawTimeBar()
        })
        .gesture(GestureGroup(GestureMode.Parallel,
          PanGesture()
            .onActionStart((event: GestureEvent) => {
              this.onPanActionStart(event);
            })
            .onActionUpdate((event: GestureEvent) => {
              this.onPanActionUpdate(event);
            })
            .onActionEnd((event: GestureEvent) => {
              this.onPanActionEnd(event);
            })
        ))
    }
    .height(this.viewHeight)
    .width(this.viewWidth)

  }

  /** Pinch start. */
  private onPinchActionStart(_event: GestureEvent) {
    this._inPinch = true;
  }

  /** Pinch update (zoom). */
  private onPinchActionUpdate(event: GestureEvent) {
    if (this.isDrawing) {
      return;
    }
    const scaleChange = Math.abs(event.scale - this.zoomSize);
    if (scaleChange <= 0.1) {
      return;
    }

    this.zoomSize = event.scale;

    Logger.info(TAG, `zoomSize: ${this.zoomSize}`);

    // [Start zoom_scale]
    // Update per-division width
    if (this.zoomSize > 1) {
      this.intervalWidth += 10;
    } else {
      this.intervalWidth -= 10;
    }
    Logger.info(TAG, `intervalWidth: ${this.intervalWidth}`);
    // Switch modes and enforce bounds
    if (this.intervalWidth < 40) {
      this.divisorMode = ScaleMode.MODE_TWO_HUNDRED_MILLISECONDS;
      if (this.intervalWidth < 10) {
        this.intervalWidth = 10;
      }
    } else if (this.intervalWidth < 80) {
      this.divisorMode = ScaleMode.MODE_TWENTY_MILLISECONDS;
    } else {
      this.divisorMode = ScaleMode.MODE_TWENTY_MILLISECONDS;
      this.intervalWidth = 80;
    }
    // [End zoom_scale]
    this.scheduleRedraw(); // Coalesce pinch redraw with requestAnimationFrame for stability.
  }

  /** Pinch end. */
  private onPinchActionEnd(_event: GestureEvent) {
    this._inPinch = false;
  }

  /**
   * Called when timeline scale option change.
   */
  onTimeScaleOptionChange() {
    this.redrawTimeBar();
  }

  private redrawTimeBar() {
    if (!this.context) {
      return;
    }
    this.draw();
  }

  private draw() {
    this.drawScales();
    this.drawMiddleLine();
  }

  // [Start draw_middleLine]
  /** Draw middle indicator line at canvas center. */
  private drawMiddleLine() {
    this.context.beginPath();
    this.context.moveTo(this.context.width / 2 + 1, 0);
    this.context.lineTo(this.context.width / 2 + 1, this.middleIndicatorOption.length);
    this.context.strokeStyle = this.middleIndicatorOption.fillColor;
    this.context.lineWidth = 2;
    this.context.stroke();
  }

  private drawScales() {
    if (!this.context) {
      return;
    }

    this.isDrawing = true;
    // Clear the entire canvas once at the beginning of a frame
    this.context.clearRect(0, 0, this.viewWidth, 200);
    this.intervalWidth = 2;
    // Number of small divisions visible
    const scaleNum = Math.floor(this.viewWidth / this.intervalWidth) + 2;
    // middleLineDuration是左侧多少个20ms   (TimeMsUnit.ONE_SECOND / this.intervalWidth)  是 1 毫秒的长度
    const middleLineDuration = (this.viewWidth / 2) * (TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth);
    // 表示画布最左侧边缘对应的实际时间戳（毫秒）
    this.leftTime = this._currentTime - middleLineDuration;
    // let leftTime = 0;
    // [Start draw_scale_2]
    // Find the first tick (aligned to 20 ms)
    let firstTick = Math.floor(this.leftTime / TimeMsUnit.TWENTY_MILLISECONDS);
    this.currentFirstTick = firstTick;
    // xPosition 表示的是当前时间相对于最近的前一个 20 毫秒刻度的位置偏移量, 故xPosition<=0
    this.xPosition =
      (firstTick * TimeMsUnit.TWENTY_MILLISECONDS - this.leftTime) * (this.intervalWidth / TimeMsUnit.TWENTY_MILLISECONDS);
    Logger.info(TAG, `scaleNum: ${scaleNum}， viewWidth： ${this.viewWidth}, intervalWidth: ${this.intervalWidth}, middleLineDuration: ${middleLineDuration}, leftTime: ${this.leftTime}, firstTick: ${firstTick}, xPosition: ${this.xPosition}`);

    for (let i = 0; i < scaleNum; i++) {
      const currentX = this.xPosition + i * this.intervalWidth * 10;
      if (this.divisorMode === ScaleMode.MODE_TWO_HUNDRED_MILLISECONDS) {
        const isMajorTick = firstTick % 50 === 0;
        const tickLength =
          isMajorTick ? this.timeScaleOption.scaleLineHeight * 2 : this.timeScaleOption.scaleLineHeight;
        const tickRange = this.drawTickLine(currentX, tickLength);
        if (isMajorTick) {
          this.drawTimeText(currentX, firstTick * 200, tickRange);
        }
        firstTick += 10;
      } else if (this.divisorMode === ScaleMode.MODE_TWENTY_MILLISECONDS) {
        for (let j = 0; j < 10; j++) {
          const positionX = j === 0 ? currentX : currentX + j * this.intervalWidth;
          const isMajorTick = (firstTick % 10) === 0;
          const isTimeTick = (firstTick % 50) === 0;
          const tickLength =
            isMajorTick ? this.timeScaleOption.scaleLineHeight * 2 : this.timeScaleOption.scaleLineHeight;
          const tickRange = this.drawTickLine(positionX, tickLength);
          if (isTimeTick) {
            this.drawTimeText(positionX, firstTick * 20, tickRange);
          }
          firstTick += 1;
        }
      }

      this.isDrawing = false;
    }
  }

  /**
   * Draw 'HH:mm' label near currentX.
   * @param currentX X coordinate.
   * @param firstTick 10-minute tick index.
   * @param tickRange Vertical range occupied by the associated tick line.
   */
  private drawTimeText(currentX: number, firstTick: number, tickRange: TickRange) {
    if (!this.context || firstTick < 0) {
      return;
    }
    this.context.font = '36px';
    this.context.textBaseline = 'alphabetic';
    // Logger.info(TAG, `firstTick: ${firstTick}`);
    const hourMinute = this.getHourMinute(firstTick);
    const measureResult = this.context.measureText(hourMinute);
    const timeStrHeight = measureResult.height;

    this.context.fillStyle = this.timeScaleOption.scaleTextColor;
    const textY = this.resolveLabelBaseline(tickRange, timeStrHeight);
    this.context.fillText(
      hourMinute,
      currentX - measureResult.width / 2,
      textY
    );
  }

  /**
   * Convert time-index to formatted 'HH:mm'.
   * Input is '10-minute tick index'.
   * @param timeIndex 10-minute tick index.
   * @returns Formatted time string 'HH:mm'.
   */
  private getHourMinute(timeIndex: number): string {
    // Logger.info(TAG, `getHourMinute timeIndex: ${timeIndex}`);
    const milliseconds = Math.floor(timeIndex % 1000);
    const totalSeconds = Math.floor(timeIndex / 1000); // 将毫秒转换为秒
    const hours = Math.floor(totalSeconds / 3600) % 24; // 计算小时数，并确保不超过24小时
    const remainingSecondsAfterHours = totalSeconds % 3600; // 剩余的秒数
    const minutes = Math.floor(remainingSecondsAfterHours / 60); // 计算剩余的分钟数
    const seconds = remainingSecondsAfterHours % 60; // 计算剩余的秒数

    return `${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
  }

  /**
   * Resolve the baseline Y coordinate for tick labels based on alignment mode.
   * @param tickRange Vertical range of the associated tick line.
   * @param textHeight Height of the text to render.
   * @returns Baseline Y coordinate for the text.
   */
  private resolveLabelBaseline(tickRange: TickRange, textHeight: number): number {
    const totalHeight = this.context?.height ?? 0;
    switch (this.timeScaleOption?.scaleAlignment) {
      case ScaleAlignment.BOTTOM: {
        const candidate = tickRange.startY - this.LABEL_PADDING;
        return Math.max(textHeight, candidate);
      }
      case ScaleAlignment.CENTER: {
        const below = tickRange.endY + textHeight + this.LABEL_PADDING;
        if (below + this.LABEL_PADDING <= totalHeight) {
          return below;
        }
        const above = tickRange.startY - this.LABEL_PADDING;
        return Math.max(textHeight, above);
      }
      case ScaleAlignment.TOP:
      default: {
        const candidate = tickRange.endY + textHeight;
        return Math.min(totalHeight - this.LABEL_PADDING, candidate);
      }
    }
    return 10;
  }

  /**
   * Draw a single tick line at the specified position with consistent styling.
   * @param positionX - The x-coordinate position for drawing the tick line
   * @param length - The length of the tick line
   * @returns The vertical range used for the tick line
   */
  private drawTickLine(positionX: number, length: number): TickRange {
    this.audioWavePositionX = positionX;
    const tickRange = this.resolveTickLineRange(length);
    this.context.beginPath();
    this.context.moveTo(positionX, tickRange.startY);
    this.context.lineTo(positionX, tickRange.endY);
    this.context.strokeStyle = this.timeScaleOption.scaleLineColor;
    this.context.lineWidth = this.timeScaleOption.scaleLineWidth;
    this.context.stroke();
    return tickRange;
  }

  /**
   * Compute the vertical range of a tick line based on alignment.
   * @param length Desired tick length.
   * @returns Start and end Y coordinates of the tick line.
   */
  private resolveTickLineRange(length: number): TickRange {
    const totalHeight = this.context?.height ?? 0;
    // const clampedLength = Math.max(0, Math.min(length, totalHeight));
    const clampedLength = length;
    switch (this.timeScaleOption?.scaleAlignment) {
      case ScaleAlignment.CENTER: {
        const center = totalHeight / 2;
        const half = clampedLength / 2;
        const startY = Math.max(0, center - half);
        const endY = Math.min(totalHeight, center + half);
        return { startY, endY };
      }
      case ScaleAlignment.BOTTOM: {
        const endY = totalHeight;
        const startY = Math.max(0, endY - clampedLength);
        return { startY, endY };
      }
      case ScaleAlignment.TOP:
      default: {
        const startY = 0;
        const endY = Math.min(totalHeight, clampedLength);
        return { startY, endY };
      }
    }
  }

  /**  Schedule redraw on the next animation frame when supported. */
  private scheduleRedraw(): void {
    this._pendingRedraw = true;
    this.initDisplaySync();
    if (!this._displaySyncRunning) {
      this.startDisplaySync();
    }
  }

  private initDisplaySync() {
    if (this._displaySync) {
      return;
    }
    this._displaySync = displaySync.create();
    // Set the expected frame rate
    const range: ExpectedFrameRateRange = {
      expected: this.FRAME_120,
      min: this.FRAME_60,
      max: this.FRAME_120
    };
    this._displaySync.setExpectedFrameRateRange(range);
    this._dsCallback = (_info: displaySync.IntervalInfo) => {
      if (this._pendingRedraw) {
        this._pendingRedraw = false;
        this._idleFrameCount = 0;
        // Redraw uniformly within the frame callback
        this.redrawTimeBar();
      } else {
        // Idle frame counter to prevent continuous callback triggering during inactivity
        this._idleFrameCount++;
        if (this._idleFrameCount >= 2) {
          this.stopDisplaySync();
        }
      }
    };
    this._displaySync.on('frame', this._dsCallback!);
  }

  /** Start DisplaySync within the UIContext. */
  private startDisplaySync() {
    if (!this._displaySync || this._displaySyncRunning) {
      return;
    }
    const uiContext: UIContext = (this.getUIContext?.() as UIContext);
    if (uiContext) {
      uiContext.runScopedTask(() => {
        this._displaySync?.start();
        this._displaySyncRunning = true;
      });
    } else {
      this._displaySync.start();
      this._displaySyncRunning = true;
    }
  }

  /**
   * Stop the callback but keep the subscription; restart via scheduleRedraw when redraw is needed later.
   */
  private stopDisplaySync() {
    if (this._displaySync && this._displaySyncRunning) {
      this._displaySync.stop();
      this._displaySyncRunning = false;
      this._idleFrameCount = 0;
    }
  }

  /** Pan gesture start. */
  private onPanActionStart(event: GestureEvent) {
    this._touchDownFlag = true;
    this._touchDownPosition = event.offsetX;
    // Reset residual accumulator at the beginning of a pan gesture.
    this._panResidual = 0;
    this._onTimeBarMoveListener?.onMoveScaleCallback(this._currentTime, PlayStatus.START);
  }

  /** Pan gesture update (drag to move timeline). */
  private onPanActionUpdate(event: GestureEvent) {
    Logger.info(TAG, `onPanActionUpdate`);
    if (this._inPinch) {
      return;
    } // Ignore when pinching
    // [Start move_scale]
    // Capture raw delta for accumulation.
    const rawDelta = event.offsetX - this._touchDownPosition;
    this._touchDownPosition = event.offsetX;

    // Accumulate sub-pixel movement for smoother tracking.
    this._panResidual += rawDelta;
    if (Math.abs(this._panResidual) < this.MOVE_SENSITIVE) {
      return;
    }

    const effectiveDelta = this._panResidual * 3;
    this._panResidual = 0;

    // Update middle-line time, then clamp into [min, max]
    const nextTime = this._currentTime - (effectiveDelta * (TimeMsUnit.TWENTY_MILLISECONDS / this.intervalWidth));
    Logger.info(TAG, `nextTime: ${nextTime}`);
    this._currentTime = this.clampToBounds(nextTime);
    Logger.info(TAG, `_currentTime: ${this._currentTime}`);

    // this.model.currentTime = this._currentTime;

    // Redraw and notify
    this.scheduleRedraw(); // Defer redraw to next frame for smoother visuals.
    this._onTimeBarMoveListener?.onMoveScaleCallback(this._currentTime, PlayStatus.PLAYING);
    // [End move_scale]
  }

  /** Pan end -> notify picked time once. */
  private onPanActionEnd(_event: GestureEvent) {
    this._onTimeBarMoveListener?.onMoveScaleCallback(this._currentTime, PlayStatus.STOP);
    this._touchDownFlag = false;
    // Clear residual delta after the gesture ends.
    this._panResidual = 0;
  }
  /**
   * Clamp a time to currently valid bounds.
   * @param t Epoch ms to clamp.
   * @returns Clamped epoch ms.
   */
  private clampToBounds(t: number): number {
    if ( t < 0) {
      return 0;
    }
    // if (t > 60 * 1000) {
    //   return 60 * 1000;
    // }
    return t;
  }
}