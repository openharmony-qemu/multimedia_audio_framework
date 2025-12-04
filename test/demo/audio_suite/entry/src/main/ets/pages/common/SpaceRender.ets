import { SpacePosition, SpaceRender, SpaceRenderMode } from '../../utils/importSongs/InterfaceInfo'
import { Logger } from '../../utils/Logger';

const TAG: string = 'AudioEditTestApp_SpaceRender';

// SpaceRenderPositionView.ets
@Component
export struct SpaceRenderPositionView {
  // ==================== 属性定义 ====================

  // 外部传入的参数
  @Link spaceRender: SpaceRender;

  // 是否激活（高亮）
  @State isActive: boolean = false;

  // 用于记录当前拖拽的点位置
  @State currentX: number = 0;
  @State currentY: number = 0;

  // 画布尺寸
  @State viewSize: number = 450;

  // 图标资源（可替换为实际图标）
  @Prop centerBitmapId: number = -1;

  // 点半径、线宽
  private pointRadius: number = 8;
  private lineWidth: number = 1.5;

  // 最大空间范围（5.0f）
  private MAX_DEST: number = 5.0;

  // 网格数量
  private AREA_COUNT: number = 4;

  private settings: RenderingContextSettings = new RenderingContextSettings(true);
  private contextFront: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings);
  private contextTop: CanvasRenderingContext2D = new CanvasRenderingContext2D(this.settings);

  // ==================== 事件处理 ====================

  // 将像素坐标 (0 ~ canvasSize) 映射到 [-5, 5]
  private pixelToReal(pixel: number, canvasSize: number): number {
    const usableWidth = canvasSize - 2 * this.pointRadius;
    const normalizedPixel = (pixel - this.pointRadius) / usableWidth; // [0, 1]
    // 映射到 [-5, 5]
    return normalizedPixel * 10 - 5;
  }

  // 将真实坐标 [-5,5] 映射为像素坐标 [0, canvasSize]
  private realToPixel(real: number, canvasSize: number): number {
    const usableWidth = canvasSize - 2 * this.pointRadius;
    const normalized = (real + 5) / 10; // [-5,5] → [0,1]
    // 映射到 [margin, canvasSize - margin]
    return normalized * usableWidth + this.pointRadius;
  }


  // 正视图修改
  onTouchOfSpaceRenderFront(event: TouchEvent) {
    if (event.touches.length === 1) {
      const touch = event.touches[0];

      if (event.type === TouchType.Down || event.type === TouchType.Move) {
        const canvasWidth = this.contextFront.width;
        const canvasHeight = this.contextFront.height;

        //限制touch坐标，让点的中心不越界
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

    // 防止父容器拦截事件
    // event.preventDefault();
  }
  // 俯视图修改
  onTouchOfSpaceRenderTop(event: TouchEvent) {
    if (event.touches.length === 1) {
      const touch = event.touches[0];

      if (event.type === TouchType.Down || event.type === TouchType.Move) {
        const canvasWidth = this.contextFront.width;
        const canvasHeight = this.contextFront.height;

        //限制touch坐标，让点的中心不越界
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

  // ==================== 逻辑方法 ====================

  // 限制点不能超出边界
  private limitPointPosition() {
    const min = this.pointRadius + this.lineWidth;
    const max = this.viewSize - this.pointRadius - this.lineWidth;

    this.currentX = Math.max(min, Math.min(max, this.currentX));
    this.currentY = Math.max(min, Math.min(max, this.currentY));
  }

  // 绘画视图
  private drawCanvas(ctx: CanvasRenderingContext2D): void {
    const width = ctx.width;
    const height = ctx.height;
    Logger.error(TAG, `width is ${width} height is ${height}`)

    // 计算每条线的间距（5条线 → 4段）
    const lineSpacingX = width / 4; // 竖线间距
    const lineSpacingY = height / 4; // 横线间距

    // 颜色定义
    const borderAndMiddleColor = '#000';     // 深色：边框 + 中线
    const regularLineColor = '#999';           // 浅色：普通网格线

    // 线宽
    const lineWidth = 2;

    // 绘制竖线（共5条）
    for (let i = 0; i <= this.AREA_COUNT; i++) {
      const x = i * lineSpacingX;

      // 判断是否为边框线（i=0 或 i=4）或中线（i=2）
      const isBorderOrMiddle = i === 0 || i === 2 || i === 4;

      ctx.strokeStyle = isBorderOrMiddle ? borderAndMiddleColor : regularLineColor;
      ctx.lineWidth = isBorderOrMiddle ? lineWidth : 1; // 中线/边框可稍粗

      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, height);
      ctx.stroke();
    }

    // 绘制横线（共5条）
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

    // 绘制中心图标（如果存在）
    if (this.centerBitmapId > 0) {
      // 在真实项目中，这里应加载图片资源
      // 本例中用一个白色圆形代替
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

  // ==================== Canvas 绘制 ====================
  private drawCanvasFront(ctx: CanvasRenderingContext2D): void {
    this.drawCanvas(ctx);

    // 绘制可拖动的点
    const pointColor = '#ff076ab1';
    ctx.fillStyle = pointColor;
    ctx.beginPath();

    let pixelX: number, pixelY: number;

    if (this.spaceRender.mode === SpaceRenderMode.FIXED_POSITION) {
      const realX = this.spaceRender.fixedPosition?.x ?? 0;
      const realZ = this.spaceRender.fixedPosition?.z ?? 0;

      // 注意：我们不改 this.spaceRender，只确保映射值在 [-5,5] 内
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

    // 绘制可拖动的点
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

  // ==================== 模板渲染 ====================

  build() {
    Row() {
      Column() {
        // 正视图
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
        // 俯视图
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