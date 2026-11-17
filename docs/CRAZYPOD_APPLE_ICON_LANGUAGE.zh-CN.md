# CrazyPod · Apple 原生比例图标语言（2026）

目标：视觉上接近 iPod / iOS 原生 UI 的清爽、克制与明确语义，同时让 320×240 iPod Classic 6/6.5/7G 上的 Click Wheel 保持跟手。**Home 的三个应用图标使用用户提供的 PNG 图像，第四张 Voice Memos 仅保留为素材。其版权与公开发行授权尚待确认。**

## 视觉参考与边界

官方参考：
- Apple Human Interface Guidelines / SF Symbols：https://developer.apple.com/design/human-interface-guidelines/sf-symbols
- Apple Design Resources：https://developer.apple.com/design/resources/

SF Symbols 与 Apple Design Resources 有使用限制。**列表中的 14px 单色语义图标仍采用 CrazyPod 自绘矢量基元，未复制 SF Symbols 字体或路径。Music、Podcasts、Settings 的 Home App 图标与之不同：它们是用户在对话中上传的 PNG 图像，被缩小、压成 RGB565+A8 后嵌入本开发分支。公开分发前必须确认这些图像的再分发权限；此分支仍保持 Draft，不作为已授权的发行版。**

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

内部菜单符号使用 24×24 光学基准网格，最终在屏幕上绘制为 **14×14 A8**。线帽圆润、视觉中心一致、留白足够，使用当前菜单文字前景色。

**Home 三个应用入口直接显示用户上传的原始风格图片**：第 1 张白色音符 → Music；第 2 张紫色播客 → Podcasts；第 3 张银灰齿轮 → Settings。第 4 张波形图标属于 Voice Memos/语音备忘录，保留在引用素材中，但 CrazyPod 尚无相应录音应用，**不再错误地用作 Now Playing 图标**。Now Playing 仍采用独立的播放符号和深色圆角方形背景。

原图 512×512 RGBA；采用 Lanczos 缩成 21×21 RGB565+A8，保留透明圆角和光照纹理。每个图标 21×21×3=1,323 字节，四张共 **5,292 字节**（不包括每个图像描述符及 Base64 ROM 字符串）。Home 仍是四条 25px 高的列表，未改成手机桌面；转盘滚动不再缩放、解码或载入图标。

## 实现

- `apps/crazypod/ui/presentation/crazypod_menu_icon_assets.c`：43 类原创几何符号，涵盖 116 个原有语义图标 ID，沿用旧 ABI。只有第一次请求某一图标时才执行整数式软件栅格化，并把 14×14 A8 结果留在固定缓冲中；同一类符号共享缓冲，无文件、网络和动态内存分配。当前基础缓冲上限：`43 × 14 × 14 = 8,428` B（不含 LVGL 描述符和其他小状态）。
- `apps/crazypod/ui/shell/crazypod_desktop.c`：Home 的 Music、Podcasts、Settings 使用 `crazypod_iphone_app_icon()` 返回的三幅用户图片；Now Playing 使用独立的原创播放符号，第四张 Voice Memos 作为后续独立功能素材保留。旧的 `LV_SYMBOL_VOLUME_MID` 播客替身已删除。`apps/crazypod/ui/presentation/crazypod_iphone_app_icons.c` 在首次请求时将只读 Base64 像素串解入静态 RGB565A8 缓冲，之后重复使用 LVGL descriptor。
- 整体菜单及 Home 快捷面板从相同的 `crazypod_menu_icon_asset` 获取图标，避免一部分苹果风、一部分 Material 风。
- 已删除以前的 `tools/generate-crazypod-menu-icons.py`（通过网络下载 Material Symbols）与 `crazypod_menu_icon_data.inc`（原 Material 图集），避免以后不经意把 Android 图标重新引入构建。

## 素材与许可风险

这四张图由用户直接上传用作固件原型；在本开发分支中仅按需要转换为低分辨率像素数据，未上传 512px 原 PNG。**用户提供素材并不等于已经获得了公开再分发许可。** 发布公版安装包或将 PR 合并到公开仓库前，应独立确认其版权/商标授权与 Apple Design Resources 的条款。如果无法确认，就把图片移至用户本地私人构建资产、改成取得相应授权的图标，或恢复原创可分发图标。

## 质量门槛

1. C99 / 静态 ID 映射测试、ARM 构建及 macOS 模拟器都必须通过，PR 始终 Draft。
2. 在 320×240 模拟器检查 21px 真实图像的可辨认度、透明圆角是否有黑边、文字与图像基线以及缩小后 Podcasts/Settings 的细节；菜单列表仍检查 14px 单色图标。
3. 真机确认首次开启菜单的图标延迟、后续列表滚动的 P95 延迟、播放时音频下溢、启动 RAM 和闪存大小；上述指标尚未实机测量。
4. 保留无障碍文字标签，不用图标取代菜单文字；不增加音频/USB/储存/屏幕 TE 路径的复杂度。
