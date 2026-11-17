# CrazyPod · iPhone 原生图标内部参考与私有构建（2026）

## 当前策略

用户希望优先使用 iPhone 的官方 App 图标，以及苹果系统符号风格；此阶段**只用于用户本地内部原型构建**，后续统一替换为具有明确分发授权的素材。

**GitHub 开发分支和公共 CI 不再嵌入此前上传的四张 Apple 风格 PNG 像素数据。** 早期试验中曾把它们编码进 `crazypod_iphone_app_icons.c`，现已移除。正式构建退回自行绘制的 14px 菜单符号及 21px 彩色圆角应用缩略图。保留 116 个菜单图标语义 ID，相关 UI、滚轮和音频代码不受此变更影响。

## 参考图与用途

- `home/music.png`：用户上传的白底黑色音符，Music。
- `home/podcasts.png`：用户上传的紫色圆形播客图案，Podcasts。
- `home/settings.png`：用户上传的银灰齿轮，Settings。
- `home/voice_memos.png`：用户上传的语音备忘录波形。**不能用作 Now Playing**。
- Now Playing 不是 iOS 上独立 App。默认使用原创播放符号；也可以在本地 `menu/now_playing.png` 替换其单色导航标识。

对音乐、播客、Now Playing、设置与控制中心中出现的菜单项，可在本地提供 `menu/<语义ID小写>.png`（如 `menu/playback.png`、`menu/queue.png`、`menu/brightness.png`、`menu/speaker.png`、`menu/shuffle.png`、`menu/repeat.png`）。本地构建器按照 `apps/crazypod/ui/crazypod_menu_icon.h` 中的 ID 识别文件名，并在严格模式检查活跃产品界面所需图标。历史上已经移除的应用仍有兼容 ID，但无需为此恢复那些应用。

## 本地私有构建

用户可以从聊天提供的本地 ZIP 中提取四张用户素材，或在自己拥有使用权的前提下从 Apple 开发设计工具导出透明底 PNG。**本工具不自动下载、扫描 Apple 系统或重分发任何官方资产。**

目录示意：

```text
CrazyPod-private-icons/
├── home/
│   ├── music.png
│   ├── podcasts.png
│   ├── settings.png
│   └── voice_memos.png
└── menu/
    ├── now_playing.png
    ├── music_library.png
    ├── playlist.png
    ├── queue.png
    ├── brightness.png
    ├── speaker.png
    └── ...其余活跃系统图标...
```

在 macOS/Linux/WSL/Git Bash 下（Windows 可按自身路径调整）：

```sh
python3 -m pip install Pillow
export CRAZYPOD_PRIVATE_ICONS_DIR=/绝对路径/CrazyPod-private-icons
# 仅已收到的 4 张素材时：显式允许其余项目使用原创回退图标
export CRAZYPOD_PRIVATE_ICONS_ALLOW_PARTIAL=1
./build-hw.sh
# 或：./build-sim.sh
```

当所有所需的 `home/*.png` 和 `menu/*.png` 准备齐后，**取消** `CRAZYPOD_PRIVATE_ICONS_ALLOW_PARTIAL`；默认严格模式会列出缺少的文件并直接拒绝构建，避免“界面中还混有非苹果图标但被误认为已完成”。

- 私有 ARM 输出目录：`build-hw-ipod6g-private/`（诊断构建会继续隔离），ZIP 名称：`CrazyPod-6G-INTERNAL-ONLY.zip`。
- 私有模拟器：`build-sim-private/`。
- 转换器：`tools/prepare-crazypod-private-icons.py`。
- 生成的 C 像素数据：`apps/crazypod/ui/presentation/crazypod_private_icon_assets.inc`，被 `.gitignore` 忽略。
- 启用宏：`CRAZYPOD_PRIVATE_ICONS`；标准编译根本不加载生成的像素表。
- 私有图片处理：Home App 21×21 RGB565A8（彩色、保留透明圆角）；Menu 14×14 A8（要求透明背景单色符号）。运行时无 PNG 解码与磁盘访问，且图标数组为只读编译数据。点击转盘仅复用描述符。

## 授权边界

“仅内部使用”不自动授予使用苹果官方图标或 SF Symbols 的许可。Apple 的设计资源及符号在使用范围、平台、标识和再分发方面存在明确限制：
- https://developer.apple.com/design/resources/
- https://developer.apple.com/support/downloads/terms/apple-design-resources/Apple-Design-Resources-License-20230621-English.pdf
- https://www.apple.com/legal/intellectual-property/guidelinesfor3rdparties.html

本地构建工具仅处理**用户自己提供并自行确认有权使用**的 PNG。该功能不代表 CrazyPod 获得 Apple 许可，也不是允许在非苹果平台使用官方 SF Symbols 的结论。任何包含受限制素材的私有编译结果都不得公开分发；准备正式版本时必须替换/授权并重新构建。

## 测试与出货要求

公共 CI 测试无私有像素的默认回退和 UI/导航/声学构建；私有集成测试使用本地目录验证 21×21 A8 透明角、14×14 A8 可读性、图标本体及每项语义映射。任何内部构建仍需检查声音播放、Click Wheel、Hold、USB、LCD TE 和音频下溢；未经真机验证不宣称可安全刷机。PR 保持 Draft。
