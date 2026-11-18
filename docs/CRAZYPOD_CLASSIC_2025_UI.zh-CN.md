# CrazyPod · Classic 2025 UI 重构契约

状态：实验分支已有构建通过的基础版本；最新首页响应修复仍需 CI 与实机复测。
设备：iPod Classic 6G/6.5G/7G，Rockbox ipod6g，320×240 RGB565，64 MiB RAM。
设计基准：用户提供的“iPod classic 2025 concept”参考图，结合 Apple iPod classic User Guide 的实体滚轮交互。

## 产品取向

CrazyPod 不是 Android 启动器，也不是纯粹复刻 2007 年像素界面。
用简洁浅色主页、纵向可滚动菜单和深色音乐播放页，配合传统 iPod 的确定性操作。
不要使用悬浮网格动作面板、霓虹渐变、强烈描边、金属材质伪装和弹出式常规设置选择器。

## 每次修改的产品决策顺序

1. 从用户任务出发：一次进入、浏览、确认或返回到底应发生什么。
2. 用经典 iPod 的确定性滚轮操作完成任务；常用选择进入真正的二级页面。
3. 保证首页、系统菜单与每个内置应用的层级、字体、间距、反馈一致。
4. 只有能解释状态变化的动画才保留；减少动态效果时亦必须完整可用。
5. 先守住播放、刷新、内存、存储、中文与恢复链路，再评判视觉效果。

符合 Apple 式的产品清晰度，但不以机械复刻 Apple 界面代替 CrazyPod 自己的设计。
完成条件是**功能可用 + 视觉一致 + 输入可预期 + 证据充分**，而不是“看起来像”。

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

## 唯一首页图标语言

旧版 16 套主题的**切换功能**已移除，15 套额外 BMP 素材已删除，
但用户原有的 `assets/crazypod-icons/basic/` 16 张应用图标**保留**。
Classic Home 在启动时一次性将这套 160×160 BMP 以盒式滤波缩小至
20×20 ARGB 图像；列表优先显示该图标，文件缺失时才使用 LVGL
系统符号兜底。此处保持约 27 KiB 的常驻小图缓存，而不恢复
旧原生轮播缓存、实时图标缩放或 Home 专用 DMA 带。
保留老外观／预设文件中的图标字段以兼容已有磁盘二进制布局，
但不再把它作为一个用户可选项。

**素材声明：** 该实现是面向 iPod Classic 的独立设计，并非
Apple 官方图标资产。Apple 的 SF Symbols、原始系统图标等受
相应许可约束，不能把其资源直接视为可随 Rockbox 固件任意分发。
用户如果提供获授权的图形资源，可另行做一套固定资产替换。

## 首页快捷操作：正式二级导航

短按 MENU 从 Home 进入 `HOME_ROUTE_ACTIONS`，显示纵向菜单：播放队列、亮度、音量。
中央键进入对应子页面；亮度/音量页使用滚轮调整，中央键或 MENU 返回上一级，
保留原菜单选中位置。播放队列进入已有 `MUSIC_ROUTE_QUEUE` 路由，MENU 先返回
快捷操作，再返回 Home。Home 长按 MENU 打开 Now Playing 的既有快捷方式保持不变。

此设计不再复制一份浮层导航状态，避免弹窗遮挡、截图底层、误吞按键和
原生/ LVGL 绘制权冲突。音频播放状态与用户设置仍由原有 Rockbox/领域模块维护。

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
├── Home Actions (MENU short)
│   ├── Playback Queue
│   ├── Brightness
│   └── Volume
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
首页纵向菜单由 LVGL 绘制；Music CoverFlow 的原生 TE 同步仍保持独立。
减少无用动画、避免重复 DMA 和堆栈大对象。

## 第一批代码范围

- 全宽的通用菜单，不再使用左 160 像素列表 + 右侧伪物理预览。
- 设置值选项已迁移到真实路由栈；其余标准 choice 页面继续逐项迁移。
- Now Playing 的 Actions、Playback、Queue（最多五行）和 Progress 改成整页布局。
- Now Playing 的无封面背景和时间线改为克制风格。
- 为纯页面导航取消模态下层截图和玻璃模糊工作。
- 原有按键分发、队列操作、播放栈、音频解码与 Rockbox 驱动没有重写。

## 全页面覆盖规则（用户要求：不得像 Android 模拟器）

这不是只针对音乐模块的换肤。所有 CrazyPod 自带页面都必须接受 UI 审查，
但不得为了统一视觉而牺牲专用应用的必要操作。

| 页面族 | 视觉验收标准 | 当前进度 |
| --- | --- | --- |
| Home / app navigation | 纵向滚轮菜单，选中即反馈，无旧图标轮播 | 已移除无效弹簧动画和触摸刷新阻塞，待最新 CI 与实机复测 |
| 迷你播放器 / 状态栏 | 简洁文本和专辑封面、轻量播放状态 | 迷你播放器已修改，状态栏待统一 |
| 音乐库 / 搜索 / 专辑流 | 标题、列表、封面及选中态一致，搜索独立完整页面 | 通用列表已修改，其余待统一 |
| 正在播放 / 队列 / 控制 | 深色信息页，动作走二级菜单，不出现操作网格 | 第一批已修改，待回归 |
| 设置 / 个性化 / 主题选择 | 选项进路由栈，滚轮移动，中央键确认，MENU 返回 | 设置选项已迁移，个性化待迁移 |
| 锁屏 / 锁屏音乐 | 大时钟、简洁封面和播放信息，无玻璃悬浮面板 | 待重构 |
| 照片 / 视频 | 保留照片沉浸式画布；操作改完整列表 | 待重构 |
| Books / Notes | 保留阅读器排版和编辑能力；设置/操作进二级菜单 | 待重构 |
| Calendar / Contacts / Clock / Workout | 遵循系统排版、标签、选中态；内容画布允许特殊布局 | 待重构 |
| Miniapps / Game Boy | 独立应用可以保持自身的游戏画面；外层应用菜单必须 Classic | 待审查 |
| 错误 / 关机 / 删除确认 | 安全动作可以使用阻塞确认；不能用 Android 式弹窗作为常规设置 | 待统一 |
| 字体 / 本地化 | CJK 字符不缺失、长标题可读；异常必须可定位 | 待验证 |

禁止以“已换主题颜色”代替完成，也禁止把错误屏幕当正常的 UI。
验收不是简单搜索是否使用圆角，而是看**页面结构、控件密度、信息层级、滚轮行为和动画性能**。

**所有子页面均需实机或模拟器截图评审，不能因为公共菜单完成就宣布 UI 重构完成。**

## 仍待完成

1. Home 已采用 LVGL 纵向菜单，触摸不再阻塞选中行刷新；需用截图与实机验证响应速度、菜单清晰度、恢复重绘及对播放的影响。Now Playing capsule 仍需与全系统统一；不得破坏 Music CoverFlow 的独立硬件 TE 同步。
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
