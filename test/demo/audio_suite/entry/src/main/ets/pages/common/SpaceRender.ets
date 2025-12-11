/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

import { SpacePosition, SpaceRender, SpaceRenderMode } from '../../utils/importSongs/InterfaceInfo'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_SpaceRender';

// SpaceRenderPositionView.ets
@Component
export struct SpaceRenderPositionView {
  // ==================== property definition ====================

  // Parameters passed from external sources
  @Link spaceRender: SpaceRender;

  // Activate (Highlight)
  @State isActive: boolean = false;

  // Used to record the current position of the dragged point
  @State currentX: number = 0;
  @State currentY: number = 0;

  // Canvas Size
  @State viewSize: number = 450;

  // Icon resources (can be replaced with actual icons)
  @Prop centerBitmapId: number = -1;

  // Radius, line width
  private pointRadius: number = 8;
  private lineWidth: number = 1.5;

  // Maximum spatial range (5.0f)
  private MAX_DEST: number = 5.0;

  // Number of grids
  private AREA_COUNT: number = 4;

  private settings: RenderingContextSettings = new RenderingContextSettings(true);
  private contextFront: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings);
  private contextTop: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings);

  // ==================== event handling ====================

  // Map pixel coordinates (0 ~ canvasSize) to [-5, 5]
  private pixelToReal(pixel: number, canvasSize: number): number {
    const usableWidth = canvasSize - 2 * this.pointRadius;
    const normalizedPixel = (pixel - this.pointRadius) / usableWidth; // [0, 1]
    // Mapped to [-5, 5]
    return normalizedPixel * 10 - 5;
  }

  // Map the real coordinates [-5, 5] to pixel coordinates [0, canvasSize]
  private realToPixel(real: number, canvasSize: number): number {
    const usableWidth = canvasSize - 2 * this.pointRadius;
    const normalized = (real + 5) / 10; // [-5,5] → [0,1]
    // Mapped to [Margin, Canvas Size - Margin]
    return normalized * usableWidth + this.pointRadius;
  }


  // Front View Modification
  onTouchOfSpaceRenderFront(event: TouchEvent) {
    if (event.touches.length === 1) {
      const touch = event.touches[0];

      if (event.type === TouchType.Down || event.type === TouchType.Move) {
        const canvasWidth = this.contextFront.width;
        const canvasHeight = this.contextFront.height;

        // Limit the touch coordinates to ensure the center of the point does not go out of bounds
        const constrainedX = Math.max(this.pointRadius, Math.min(canvasWidth - this.pointRadius, touch.x));
        const constrainedY = Math.max(this.pointRadius, Math.min(canvasHeight - this.pointRadius, touch.y));

        const realX = parseFloat(this.pixelToReal(constrainedX, canvasWidth).toFixed(2));
        const realZ = parseFloat(this.pixelToReal(constrainedY, canvasHeight).toFixed(2));

        Logger.error(TAG, `The position is ${realX} ${realZ} ${this.spaceRender.fixedPosition?.y}`)
        if (this.spaceRender.mode === SpaceRenderMode.FIXED_POSITION) {
          let fixedPosition: SpacePosition = {
            x: realX,
            y: this.spaceRender.fixedPosition?.y ?? 0,
            z: realZ
          }
          this.spaceRender.fixedPosition = fixedPosition;
        } else {
          let dynamicPosition: SpacePosition = {
            x: realX,
            y: this.spaceRender.dynamicPosition?.y ?? 0,
            z: realZ
          }
          this.spaceRender.dynamicPosition = dynamicPosition;
        }

        this.contextFront.reset();
        this.drawCanvasFront(this.contextFront);
        this.contextTop.reset();
        this.drawCanvasTop(this.contextTop);
      }
    }

    // Prevent parent container from intercepting events
    // event.preventDefault();
  }
  // Top view modification
  onTouchOfSpaceRenderTop(event: TouchEvent) {
    if (event.touches.length === 1) {
      const touch = event.touches[0];

      if (event.type === TouchType.Down || event.type === TouchType.Move) {
        const canvasWidth = this.contextFront.width;
        const canvasHeight = this.contextFront.height;

        // Limit the touch coordinates to ensure the center of the point does not go out of bounds
        const constrainedX = Math.max(this.pointRadius, Math.min(canvasWidth - this.pointRadius, touch.x));
        const constrainedY = Math.max(this.pointRadius, Math.min(canvasHeight - this.pointRadius, touch.y));

        const realX = parseFloat(this.pixelToReal(constrainedX, canvasWidth).toFixed(2));
        const realY = parseFloat(this.pixelToReal(constrainedY, canvasHeight).toFixed(2));

        Logger.error(TAG, `The position is ${realX} ${realY} ${this.spaceRender.fixedPosition?.y}`)
        if (this.spaceRender.mode === SpaceRenderMode.FIXED_POSITION) {
          let fixedPosition: SpacePosition = {
            x: realX,
            y: realY,
            z: this.spaceRender.fixedPosition?.z ?? 0
          }
          this.spaceRender.fixedPosition = fixedPosition;
        } else {
          let dynamicPosition: SpacePosition = {
            x: realX,
            y: realY,
            z: this.spaceRender.dynamicPosition?.z ?? 0
          }
          this.spaceRender.dynamicPosition = dynamicPosition;
        }

        this.contextFront.reset();
        this.drawCanvasFront(this.contextFront);
        this.contextTop.reset();
        this.drawCanvasTop(this.contextTop);
      }
    }
    // event.stopPropagation();
  }

  // ==================== Logical Method ====================

  // Restriction points must not exceed the boundaries.
  private limitPointPosition() {
    const min = this.pointRadius + this.lineWidth;
    const max = this.viewSize - this.pointRadius - this.lineWidth;

    this.currentX = Math.max(min, Math.min(max, this.currentX));
    this.currentY = Math.max(min, Math.min(max, this.currentY));
  }

  // Painting View
  private drawCanvas(ctx: CanvasRenderingContext2D): void {
    const width = ctx.width;
    const height = ctx.height;
    Logger.error(TAG, `width is ${width} height is ${height}`)

    // Calculate the spacing between each line (5 lines → 4 segments)
    const lineSpacingX = width / 4; // Vertical line spacing
    const lineSpacingY = height / 4; // Line spacing

    // Color Definition
    const borderAndMiddleColor = '#000';     // Dark: Border + Center Line
    const regularLineColor = '#999';           // Light color: normal grid lines

    // line width
    const lineWidth = 2;

    // Draw vertical lines (a total of 5)
    for (let i = 0; i <= this.AREA_COUNT; i++) {
      const x = i * lineSpacingX;

      // Determine whether it is a border line (i=0 or i=4) or a center line (i=2)
      const isBorderOrMiddle = i === 0 || i === 2 || i === 4;

      ctx.strokeStyle = isBorderOrMiddle ? borderAndMiddleColor : regularLineColor;
      ctx.lineWidth = isBorderOrMiddle ? lineWidth : 1; // The centerline/border can be slightly thicker

      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, height);
      ctx.stroke();
    }

    // Draw horizontal lines (a total of 5)
    for (let i = 0; i <= this.AREA_COUNT; i++) {
      const y = i * lineSpacingY;

      const isBorderOrMiddle = i === 0 || i === 2 || i === 4;

      ctx.strokeStyle = isBorderOrMiddle ? borderAndMiddleColor : regularLineColor;
      ctx.lineWidth = isBorderOrMiddle ? lineWidth : 1;

      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(width, y);
      ctx.stroke();
    }

    // Draw the central icon (if present)
    if (this.centerBitmapId > 0) {
      // In a real project, image resources should be loaded here.
      // In this example, a white circle is used as a substitute.
      // ctx.fillStyle = '#FFFFFF';
      // ctx.beginPath();
      // ctx.arc(
      //   (this.viewSize - 40) / 2,
      //   (this.viewSize - 40) / 2,
      //   20,
      //   0,
      //   Math.PI * 2
      // );
      // ctx.fill();
    }

    Logger.error(TAG, `width: ${ctx.width} ${ctx.height}`);
  }

  // ==================== Canvas draw ====================
  private drawCanvasFront(ctx: CanvasRenderingContext2D): void {
    this.drawCanvas(ctx);

    // Draw Dragable Points
    const pointColor = '#ff076ab1';
    ctx.fillStyle = pointColor;
    ctx.beginPath();

    let pixelX: number, pixelY: number;

    if (this.spaceRender.mode === SpaceRenderMode.FIXED_POSITION) {
      const realX = this.spaceRender.fixedPosition?.x ?? 0;
      const realZ = this.spaceRender.fixedPosition?.z ?? 0;

      // Note: We do not modify this.spaceRender; we only ensure that the mapped values are within the range of [-5, 5]
      const clampedRealX = Math.max(-5, Math.min(5, realX));
      const clampedRealZ = Math.max(-5, Math.min(5, realZ));

      pixelX = this.realToPixel(clampedRealX, ctx.width);
      pixelY = this.realToPixel(clampedRealZ, ctx.height);
    } else {
      const realX = this.spaceRender.dynamicPosition?.x ?? 0;
      const realZ = this.spaceRender.dynamicPosition?.z ?? 0;

      const clampedRealX = Math.max(-5, Math.min(5, realX));
      const clampedRealZ = Math.max(-5, Math.min(5, realZ));

      pixelX = this.realToPixel(clampedRealX, ctx.width);
      pixelY = this.realToPixel(clampedRealZ, ctx.height);
    }

    ctx.arc(pixelX, pixelY, this.pointRadius, 0, Math.PI * 2);
    ctx.fill();
  }

  private drawCanvasTop(ctx: CanvasRenderingContext2D): void {
    this.drawCanvas(ctx);

    // Draw Dragable Points
    const pointColor = '#ff076ab1';
    ctx.fillStyle = pointColor;
    ctx.beginPath();

    let pixelX: number, pixelY: number;

    if (this.spaceRender.mode === SpaceRenderMode.FIXED_POSITION) {
      const realX = this.spaceRender.fixedPosition?.x ?? 0;
      const realY = this.spaceRender.fixedPosition?.y ?? 0;

      const clampedRealX = Math.max(-5, Math.min(5, realX));
      const clampedRealY = Math.max(-5, Math.min(5, realY));

      pixelX = this.realToPixel(clampedRealX, ctx.width);
      pixelY = this.realToPixel(clampedRealY, ctx.height);
    } else {
      const realX = this.spaceRender.dynamicPosition?.x ?? 0;
      const realY = this.spaceRender.dynamicPosition?.y ?? 0;

      const clampedRealX = Math.max(-5, Math.min(5, realX));
      const clampedRealY = Math.max(-5, Math.min(5, realY));

      pixelX = this.realToPixel(clampedRealX, ctx.width);
      pixelY = this.realToPixel(clampedRealY, ctx.height);
    }

    ctx.arc(pixelX, pixelY, this.pointRadius, 0, Math.PI * 2);
    ctx.fill();
  }

  // ==================== Template Rendering ====================

  build() {
    Row() {
      Column() {
        // front view
        Canvas(this.contextFront)
          .width(`${this.viewSize}px`)
          .height(`${this.viewSize}px`)
          .onTouch((event: TouchEvent) => this.onTouchOfSpaceRenderFront(event))
          .onReady(() => {
            this.drawCanvasFront(this.contextFront);
          })
      }
      .width('50%')

      Column() {
        // top view
        Canvas(this.contextTop)
          .width(`${this.viewSize}px`)
          .height(`${this.viewSize}px`)
          .onTouch((event: TouchEvent) => this.onTouchOfSpaceRenderTop(event))
          .onReady(() => {
            this.drawCanvasTop(this.contextTop);
          })
      }
      .width('50%')
    }
  }
}