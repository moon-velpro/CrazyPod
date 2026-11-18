# CrazyPod · Classic 2025 UI 重构契约

状态：第一批导航与正在播放页面已落地；**尚未通过编译与实机验证**。
设备：iPod Classic 6G/6.5G/7G，Rockbox ipod6g，320×240 RGB565，64 MiB RAM。
设计基准：用户提供的“iPod classic 2025 concept”参考图，结合 Apple iPod classic User Guide 的实体滚轮交互。

## 产品取向

CrazyPod 不是 Android 启动器，也不是纯粹复刻 2007 年像素界面。
用简洁浅色主页、纵向可滚动菜单和深色音乐播放页，配合传统 iPod 的确定性操作。
不要使用悬浮网格动作面板、霓虹渐变、强烈描边、金属材质伪装和弹出式常规设置选择器。

## 实体操作契约

| 输入 | 列表与二级菜单 | 正在播放 |
| --- | --- | --- |
| 滚轮顺/逆时针 | 下/上移动选中项 | 常规音量；进入 seek 模式后调整进度 |
| 中央键短按 | 进入子菜单/确认选项 | 按播放页模式进行操作；扩展动作有独立菜单 |
| MENU | 返回上一级，恢复之前选中项 | 退出控制页或返回来源列表 |
| 播放/暂停 | 直接控制播放 | 直接播放/暂停 |
| 左/右 | 保留既有列表/播放上下文语义 | 上一曲/下一曲或当前控制模式对应动作 |
| 中央键长按 | 打开项目的附加操作（如有） | 进入动作二级菜单 |

补充：Apple 原版也会通过长按中央键显示附加命令菜单；本项目采用**完整二级页面**呈现这些命令。
不应将惯用操作（排序、语言、主题、播放模式、队列）做成浮层对话框。

## 页面层级

```text
Home
├── Music
│   ├── Artists → Artist → Songs
│   ├── Albums → Album → Songs
│   ├── Songs
│   ├── Playlists → Playlist → Songs
│   ├── Album Flow
│   └── Search → Results
├── Now Playing
│   ├── Actions
│   │   ├── Queue
│   │   ├── Favorite
│   │   ├── Playback → Ordered / Shuffle / Repeat All / Repeat One
│   │   ├── Lyrics
│   │   └── Progress
├── Settings
│   ├── Sound → Values
│   ├── Display → Values
│   ├── Playback → Values
│   └── Main Menu → Item Actions
└── Other apps → Hierarchical feature pages
```

每次 MENU 返回应保留父菜单选中位置；更换播放模式后，音乐不应中断。
弹窗只用于不可逆删除、关机、必须阻止操作的系统故障等场景。
音量等非阻塞的短时 HUD 可以保留，但不是选项菜单。

## 第一批视觉变量

| 用途 | 色值 |
| --- | --- |
| 页面背景 | `#F6F9FC` |
| 列表行 | `#FFFFFF` |
| 主要文字 | `#1E2938` |
| 次要文字 | `#64748B` |
| 选中行 | `#DDEAF7` |
| 图标和层级指示 | `#447AA9` |
| 播放页深色缺省背景 | `#171C23` |

当前硬件以 RGB565 输出；不依赖 GPU 模糊或大规模逐帧半透明合成。
优先使用原有 LVGL 简单控件与既有原生图标刷新机制，避免堆栈大对象。

## 第一批代码范围

- 全宽的通用菜单，不再使用左 160 像素列表 + 右侧伪物理预览。
- 设置和其他标准 choice 页面显示为完整的次级导航页。
- Now Playing 的 Actions、Playback、Queue（最多五行）和 Progress 改成整页布局。
- Now Playing 的无封面背景和时间线改为克制风格。
- 为纯页面导航取消模态下层截图和玻璃模糊工作。
- 原有按键分发、队列操作、播放栈、音频解码与 Rockbox 驱动没有重写。

## 仍待完成

1. Home 的原生 icon carousel 及 Now Playing capsule：需按概念图设计主屏卡片与控件，但**不得破坏 Home 的硬件 TE 同步 / 原生部分更新契约**。
2. 状态栏、空状态、封面缺失占位、个性化主题和系统提示要整体一致。
3. 将暂时借用 choice coordinator 的整页菜单逐步改为真正的 route stack 页面模型，保留各子系统的功能边界。
4. Original iPod Music 的歌名/专辑解析与首次扫描 `StkOv crazypod lock font` 红屏需另行定位，不能假定 UI 修改已修复。
5. 中文字体、长标题滚动、屏幕边界、缓存和帧率需要真机检查。

## 测试门禁

```sh
sh tests/check-crazypod-ui-architecture.sh
sh tests/run-crazypod-ui-host-tests.sh
sh tests/run-miniapp-host-tests.sh
python3 tools/check-crazypod-l10n.py --strict-bare
./build-sim.sh --incremental
./build-hw.sh --incremental
git diff --check
```

必须覆盖：至少 1,785 首歌曲索引、菜单滚轮快速滚动、子菜单 MENU 返回与选中恢复、当前歌曲切换、五行队列滚动、播放模式切换、字体缺字、首次扫歌、不带封面歌曲、重启，以及音频和 USB 回归。
未经模拟器编译和 iPod6G 实机验证不得视为可刷入正式版。

## 依据

Apple 官方 iPod classic User Guide（160GB）：https://cdsassets.apple.com/live/6GJYWVAV/user/ma1195_ipod_classic_160gb_user_guide.pdf
CrazyPod UI architecture：`apps/crazypod/ARCHITECTURE.md`
