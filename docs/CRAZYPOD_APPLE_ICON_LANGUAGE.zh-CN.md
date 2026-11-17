# CrazyPod · Apple 原生比例图标语言（2026）

目标：视觉上接近 iPod / iOS 原生 UI 的清爽、克制与明确语义，同时让 320×240 iPod Classic 6/6.5/7G 上的 Click Wheel 保持跟手。**这是设计参考，不代表使用了 Apple 官方图标素材**。

## 视觉参考与边界

官方参考：
- Apple Human Interface Guidelines / SF Symbols：https://developer.apple.com/design/human-interface-guidelines/sf-symbols
- Apple Design Resources：https://developer.apple.com/design/resources/

SF Symbols 与部分 Apple Design Resources 有明确使用许可和限制，不将其官方 SVG、字体、位图或 App 图标直接打包到本 Rockbox 设备固件。CrazyPod 自行绘制基于普通几何语义的细线图标，供开源分发。与苹果平台相同的**视觉原则**不是复制任何专有路径、品牌标识或系统文件。

## 产品入口

| CrazyPod 入口 | 图形语义 | 不能混用 |
|---|---|---|
| Now Playing | 圆形播放按钮 | 不能用其他应用的 Play Store 风格三角 |
| Music | 单音符与音符头 | 不能用安卓 Material 音乐列表符号 |
| Podcasts | 同心广播弧、中心信号与短柄 | 绝不能用音量喇叭符号 |
| Settings | 克制、同心的齿轮及中央轴心 | 不用粗壮实心 Android Material 齿轮 |
| Playback Queue | 三行序列 + 播放符号 | 不用应用网格 |
| Brightness | 中心圆盘和八条日光射线 | 不能用音量图标 |
| Volume | 喇叭和两级音波 | 不用麦克风 |

所有符号使用 24×24 光学基准网格，最终在屏幕上绘制为 **14×14 A8**。线帽圆润、视觉中心一致、留白足够且统一使用系统前景色；大背景、胶囊高光和图标语义分层，不把 320×240 UI 做成一排安卓 App 磁贴。Home 保留彩色小徽标，但图标本体统一为白色单色线形。

## 实现

- `apps/crazypod/ui/presentation/crazypod_menu_icon_assets.c`：43 类原创几何符号，涵盖 116 个原有语义图标 ID，沿用旧 ABI。只有第一次请求某一图标时才执行整数式软件栅格化，并把 14×14 A8 结果留在固定缓冲中；同一类符号共享缓冲，无文件、网络和动态内存分配。当前基础缓冲上限：`43 × 14 × 14 = 8,428` B（不含 LVGL 描述符和其他小状态）。
- `apps/crazypod/ui/shell/crazypod_desktop.c`：Home 的 Music、Podcasts、Now Playing、Settings 直接取对应语义图标，以白色单色模式居中放入现有 19px 小徽标；旧的 `LV_SYMBOL_VOLUME_MID` 播客替身已删除。
- 整体菜单及 Home 快捷面板从相同的 `crazypod_menu_icon_asset` 获取图标，避免一部分苹果风、一部分 Material 风。
- 已删除以前的 `tools/generate-crazypod-menu-icons.py`（通过网络下载 Material Symbols）与 `crazypod_menu_icon_data.inc`（原 Material 图集），避免以后不经意把 Android 图标重新引入构建。

## 质量门槛

1. C99 / 静态 ID 映射测试、ARM 构建及 macOS 模拟器都必须通过，PR 始终 Draft。
2. 在 320×240 模拟器重点看 14px 线条是否过细/过粗、Podcast 的识别性、文字与图标基线、菜单项对比度；必要时只调整图形而不改变 Click Wheel 逻辑。
3. 真机确认首次开启菜单的图标延迟、后续列表滚动的 P95 延迟、播放时音频下溢、启动 RAM 和闪存大小；上述指标尚未实机测量。
4. 保留无障碍文字标签，不用图标取代菜单文字；不增加音频/USB/储存/屏幕 TE 路径的复杂度。
