# CrazyPod 音乐播放器工程结构与迁移门禁

状态：**渐进式模块迁移中**，本文是可执行的架构约束，不是已完成声明。
开发分支：`refactor/classic-2025-menu-navigation`；主线和既有音乐数据不受本轮修改影响。

## 产品边界

唯一运行态入口：音乐库、播客、随机播放、锁屏、设置；播放器内部保留
Now Playing、Cover Flow、歌词、播放队列、专辑/艺人/歌单、EQ、播放模式、
播放控制、音量、Home 快捷页、时间和电量，以及 USB、存储和恢复功能。
`assets/crazypod-icons/basic` 的 16 个原始 BMP 不改动、不重新生成。

不再提供：Books、Notes、Photos/Video、Organizer、Workout、Calendar、
Contacts、Customize（独立应用）、Mini Apps、外部播放页主题和 GB/GBC。
历史功能编号仅用于安全读取旧状态，不能成为可执行产品入口。

## 模块拥有权与依赖方向

| 边界 | 所属代码 | 对外合同 | 允许依赖 |
| --- | --- | --- | --- |
| 产品状态与音频领域 | `apps/crazypod/crazypod_music*`, `crazypod_playlist*`, `crazypod_state*` | 音频/队列/持久化服务 | Rockbox 底层；不得 include UI 私有头 |
| 产品 feature 门面 | `ui/features/music`, `now_playing`, `settings` | 各自的 `*_feature.h` | 本 feature 内部实现及领域服务 |
| 导航 | `ui/navigation` | 路由编号、合法性、栈、查询、分发 | 三个保留 feature 的公共门面；不得依赖旧 feature |
| 应用编排 | `ui/app` | 启动、输入连接、运行时服务、路由动作/绘制 | 导航、feature 门面和 Shell；不自行创建第二份产品允许列表 |
| 外壳与呈现 | `ui/shell`, `ui/presentation` | Home、状态栏、锁屏、菜单组件 | 应用层公开状态与绘制合同 |
| 硬件与平台 | `platform` 和既有 Rockbox HAL | LCD、TE、按键、音频、USB、存储 | 不含产品页面逻辑 |
| 源码构建清单 | `apps/SOURCES` | 生产固件真正链接的模块 | 最终只列保留模块和必要公共依赖 |
| 兼容资料 | 历史 enum ID、v17 菜单存档及只读迁移 | 安全解析、忽略退役 ID | 不得动态重新启用旧应用 |

依赖方向原则：`Shell → app/nav → feature facade → domain → platform`。
反向调用只通过注入的 host/callback 接口，不通过跨目录访问私有对象。
视觉组件不得直接启动扫描或修改音频状态；持久化模块不得依赖 LVGL。
复杂场景优先用业务状态机，而不是各页面自造定时器。

## 当前已建立的导航边界

1. `crazypod_ui_routes.h` 是当前唯一合法 route ID 判定来源。
   `route_registry` 是稀疏的**产品**注册表，退役 ID 无 descriptor，
   旧值仍保留，避免存档编号重排。
2. `route_query` 只查询 Music / Now Playing / Settings 和 Home 快捷页。
3. `feature_input` 只绑定 Music / Now Playing / Settings 输入处理。
4. `app_launcher` 复用导航合法性，不再复制一套单独白名单。
5. `route_renderer` 只绘制音乐、播放页、设置和 Home 页面，
   `app_input` 不再依赖笔记编辑功能。
6. `tests/check-crazypod-navigation-modules.py` 是持续生效的架构门禁；
   `tests/crazypod_ui_pure_host_test.c` 检查保留/退役 feature 的注册状态。

## 已实施的模块迁移

### 02 · 应用编排与渲染

`route_actions`、`choice_coordinator`、`composition`、
`route_renderer`、`feature_input`、`app_input`、
`menu_preview` 和 UI composition root 已集中收敛为音乐、播放、
设置和 Shell 路径；废弃了图书、笔记、照片、联系人、运动等旧应用
的导航激活、选择确认、预览和输入代码，而非仅在首页隐藏按钮。

系统提示和 USB 存储生命周期已不再处理失效的照片/视频/CPK 目录，
并保留音频目录重新扫描、封面失效、字形文件句柄释放及 USB 确认。
原生播放页不再经由外部 CPK 主题/空函数兼容层；
原生专辑封面源尺寸直接由音乐服务初始化。

激活路径遵循“路由归属负责处理”的规则：设置、Home 和不可播放的
菜单不得落入通用播放失败提示；只有歌曲列表、队列、搜索结果等
实际承载音轨的页面可以尝试播放。该约束由导航模块合同测试守护。

### 03 · 固件编译闭包

`apps/SOURCES` 当前有 **117 条 CrazyPod 产品源码记录**，原先约
242 条。Mini Apps/CPK、Photos/Video/MPEG2、Books/EPUB、
Notes、Organizer、Customize 独立应用及外置 Now Playing 主题实现
均已从生产编译清单移除。历史 route/feature ID 与菜单存档编号未重新
编号。仍保留壁纸显示、截图、图片解码等音乐播放器设备 UI 所需服务。

被解除链接的部分源码和历史功能单测**仍保存在仓库**，用于复核迁移和
兼容性；不能将“从编译清单移除”误写成“全部旧源码文件已删除”。
今后若做最终的源文件归档/删除，需要同步收敛测试夹具和文档，并核对
链接符号及可复现构建，不得靠新增空桩过关。

### 04 · 构建/验收

每个模块级变更应通过结构门禁、C99 host 单元测试、macOS 模拟器构建、
iPod 6G 交叉编译，再做音频/中文元数据/转盘/锁屏/USB/首次扫描的实机
冒烟和压力测试。已确认 `bf517681` 是上一轮双构建通过的基线，
**本轮大范围源文件移除仍必须以最新 CI 完成记录为准**。只有全部门禁实证通过后，才能称为可刷机候选。
所有未运行或失败的门禁必须明确报告，Draft PR 不得当作发布包。

## 禁止的实现方式

- 在生产代码里新增“空函数”伪装旧功能已移除。
- 在多个页面重复维护各自的可用功能白名单。
- 将产品 feature ID/历史存档字段直接删掉或重新编号。
- 为了编译通过临时降低编译告警、排除失败测试或拷贝新版假音频实现。
- 只从 Home 隐藏应用，却继续把其运行时编译进固件并宣称已完成卸载。
