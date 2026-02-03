/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2026. ALL rights reserved.
 */

export  interface TickRange {
  startY: number;
  endY: number;
}

/** Time scale modes */
export enum ScaleMode {
  MODE_HOUR = 1, // each small tick = 10 minutes, show hour labels
  MODE_MINUTE = 2, // each big tick = 1 minute, show minute labels
  MODE_TWENTY_MILLISECONDS = 3, // 20 milliseconds
  MODE_TWO_HUNDRED_MILLISECONDS = 4, // 200 milliseconds
}

/** Time constants */
export enum TimeMsUnit {
  TWENTY_MILLISECONDS = 20, // 20 milliseconds
  TWO_HUNDRED_MILLISECONDS = 200, // 200 milliseconds
  ONE_SECOND = 1 * 1000, // 1 second
  ONE_MINUTE = 60 * 1000, // 1 minute in ms
  TEN_MINUTE = 10 * 60 * 1000, // 10 minutes in ms
  ONE_HOUR = 60 * 60 * 1000, // 1 hour in ms
}

/**
 * Visual options for the time scale (ticks and labels).
 */
export interface TimeScaleOption {
  /** Line width for tick marks. */
  scaleLineWidth: number;

  /** Base tick height (the "big tick" usually doubles this). */
  scaleLineHeight: number;

  /** Color of tick marks. */
  scaleLineColor: string | number | CanvasGradient | CanvasPattern;

  /** Color of time labels. */
  scaleTextColor: string | number | CanvasGradient | CanvasPattern;

  /**
   * The alignment style of the ticks on the time-bar
   */
  scaleAlignment: ScaleAlignment;
}

/**
 * Enum defining the alignment options for time axis tick marks
 * Used to specify the vertical positioning of tick lines on a time scale
 */
export enum ScaleAlignment {

  /**
   * Tick marks are displayed at the top of the time axis
   */
  TOP = 0,

  /**
   * Tick marks are displayed in the center of the time axis
   */
  CENTER = 1,

  /**
   * Tick marks are displayed at the bottom of the time axis
   */
  BOTTOM = 2,
}

/**
 * Visual options for the middle indicator line on the time bar.
 */
export interface MiddleIndicatorOption {
  /** Line length in pixels. */
  length: number;

  /** fill color of the middle indicator. */
  fillColor: string | number | CanvasGradient | CanvasPattern;
}

/** timeBar status */
export enum PlayStatus {
  START = 1,
  PLAYING = 2,
  STOP = 3,
}

/**
 * Callbacks for timeline dragging and auto-scrolling.
 */
export interface TimePickedCallback {
  /**
   * Invoked when the time bar is being dragged or pressed.
   * @param curTime Epoch milliseconds at the middle indicator.
   * @param status Current gesture status (START/PLAYING/STOP).
   */
  onMoveScaleCallback: (curTime: number, status?: PlayStatus) => void;
}