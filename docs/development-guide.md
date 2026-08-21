# YoloLabel 开发指南 —— 多目录功能实战记录与维护手册

> 本文记录 2026-08 多目录管理功能的完整开发过程:架构决策、踩过的坑、
> 验证方法,以及后续维护所需的一切信息。
> 设计阶段的原型文档见 [multi-dir-plan.md](multi-dir-plan.md)(已全部落地)。

---

## 一、项目概况

| 项 | 内容 |
|---|---|
| 语言/框架 | C++17 / **Qt 6**(Widgets),qmake 构建 |
| 规模 | 约 4000 行(5 个模块) |
| 平台 | Windows / Ubuntu(macOS 有文档支持) |
| 本机验证环境 | Qt 6.11.2 (mingw_64) + gcc 13.1.0,Windows 10 |

模块一览:

| 模块 | 行数 | 职责 |
|---|---|---|
| `mainwindow.{h,cpp}` | ~1600 | 主窗口、目录/类别管理、热键、会话恢复 |
| `label_img.{h,cpp}` | ~870 | 标注画布:画框/拖拽/缩放/撤销重做(未改动) |
| `cloud_labeler.{h,cpp}` | ~830 | 云端 AI 标注 HTTP 客户端(未改动) |
| `yolo_detector.{h,cpp}` | ~750 | ONNX 本地推理(可选编译,未改动) |
| `dir_manager.{hpp,cpp}` | ~230 | **新增**:多目录状态核心(纯 C++,零 Qt 依赖) |

**注意**:本项目虽然放在 `rust_dev` 目录下,但它是 C++/Qt 项目,与 Rust 无关。
动手前先看 `.pro` 文件确认构建体系,不要凭目录名假设。

---

## 二、构建环境与命令

### 2.1 本机工具链

| 用途 | 路径 |
|---|---|
| Qt 6.11.2 + qmake | `F:/Qt-dev/6.11.2/mingw_64/bin/` |
| 编译器(gcc 13.1 + make) | `F:/Qt-dev/Tools/mingw1310_64/bin/` |
| 备用 g++(跑单测,无 Qt 也能用) | `D:/mingw64/bin/g++.exe` |

### 2.2 完整构建(Git Bash)

```bash
export PATH="/f/Qt-dev/6.11.2/mingw_64/bin:/f/Qt-dev/Tools/mingw1310_64/bin:$PATH"
cd /f/zcode_proj/rust_dev/Yolo_Label-master
mkdir -p build && cd build       # shadow build,不污染源码目录
qmake ../YoloLabel.pro
mingw32-make -j8 release
# 产物: build/release/YoloLabel.exe (~270 KB)
```

### 2.3 单元测试(dir_manager 核心)

```bash
cd /f/zcode_proj/rust_dev/Yolo_Label-master
/d/mingw64/bin/g++.exe -std=c++17 -Wall -Wextra -I. \
    tests/test_dir_manager.cpp dir_manager.cpp -o /tmp/test_dir_manager.exe
/tmp/test_dir_manager.exe
# 期望输出: 62 checks, 0 failures
```

任何 C++17 编译器都行(Qt 的 g++、Linux 的 g++/clang 均可),因为核心模块零 Qt 依赖。

### 2.4 分发部署

exe 运行需要 Qt DLL。**直接双击运行或拷给别人前必须部署**:

```bash
/f/Qt-dev/6.11.2/mingw_64/bin/windeployqt.exe --compiler-runtime build/release/YoloLabel.exe
```

部署后 `build/release/` 自包含(Qt6*.dll、MinGW 运行时、`platforms/qwindows.dll`
等都在 exe 旁),当前构建**已执行过此步**,可直接双击。

注意:
- 重新 `mingw32-make` 只覆盖 exe,DLL 保留,无需每次重跑 windeployqt;
- 只有 Qt 升级版本后才需重新部署;
- 分发时打包**整个 release 目录**,不要只发单个 exe(会报缺 Qt6Gui.dll
  或"找不到 Qt 平台插件")。

### 2.5 ONNX 推理(可选)

默认**不编译**。要启用:把 onnxruntime 发行包解压到项目根目录 `onnxruntime/`
(含 `include/` 和 `lib/`),`.pro` 会自动检测并定义 `ONNXRUNTIME_AVAILABLE`。

---

## 三、多目录功能设计

### 3.1 需求

原来一次只能开一个目录,切换数据集要重新走文件对话框。改进目标:
热键循环切换 + 侧边栏可视化列表,目录按名字数字排序,进度/类别可恢复。

### 3.2 架构分层(最重要的决策)

```
┌──────────────────────────────────────────────┐
│ Qt 层: MainWindow (mainwindow.cpp)           │  UI 粘合、文件 IO、热键
├──────────────────────────────────────────────┤
│ 核心层: DirManager (dir_manager.{hpp,cpp})   │  纯 C++17,零 Qt 依赖
└──────────────────────────────────────────────┘
```

**为什么分层**:开发本功能时本机没有 Qt 环境(后来才装)。
把"排序/去重/切换/进度"这些纯逻辑抽出来,用裸 g++ 就能写单元测试。
事后验证了这个决策的价值:Qt 环境装好后接入,只暴露了 3 个琐碎的编译错误,
核心逻辑一个 bug 都没有。

`DirManager` API 速查:

| 方法 | 说明 |
|---|---|
| `addDir(path)` | 追加目录,去重 + 数字排序,返回排序后索引 |
| `switchToDir(i)` | 循环取模切换,已是当前则返回 false |
| `nextDir()/prevDir()` | 循环相邻索引,少于 2 个目录返回 -1 |
| `setProgress(dir, i)` / `progressOf(dir)` | 每目录浏览进度 |
| `setClassesFile(dir, f)` / `classesFileOf(dir)` | 每目录类别文件绑定 |
| `snapshot()/restore()` | 整体导出/导入(持久化用) |

### 3.3 数据模型与持久化

```cpp
struct DirEntry {
    std::string path;         // 绝对路径(尾部斜杠已归一)
    int         progress;     // 上次浏览到的图片索引
    std::string classesFile;  // 绑定的类别文件(空 = 沿用全局)
};
```

Windows 上 QSettings 落在注册表 `HKCU\Software\YoloLabel\Session`:

| 键 | 类型 | 含义 |
|---|---|---|
| `dirList` | REG_MULTI_SZ | 目录绝对路径列表 |
| `dirProg` | REG_BINARY(QVariantMap) | 目录 → 进度索引 |
| `dirCls` | REG_BINARY(QVariantMap) | 目录 → 类别文件路径 |
| `dirCur` | REG_DWORD | 当前目录索引 |
| `imgDir`/`objFile` | REG_SZ | **旧版单目录字段**,保留兼容 |

兼容迁移:`restoreLastSession()` 优先走 `dirList` 分支;若为空但旧字段
`imgDir` 存在,自动迁移进列表。老用户升级无感。

调试技巧:目录没恢复/列表不对时,先 `reg query HKCU\Software\YoloLabel\Session`
看持久化内容,能立刻区分"没存"还是"没读"。

### 3.4 切换流程(`switch_to_dir(int)`)

1. 云端批量标注进行中 → 拒绝切换(防止图片列表中途被换);
2. 目标 == 当前目录 → 只恢复进度 `goto_img(progress)`;
3. 记录当前目录进度 + `save_label_data()` 保存标注;
4. 目标目录有绑定类别文件 → `load_label_list_data()` 加载,否则沿用全局;
5. `get_files()` 装载图片,`init()` 重置 UI;
6. `goto_img(min(progress, count-1))` 恢复进度;
7. 刷新 Folders 列表、`save_dir_state()` 持久化、状态栏提示。

### 3.5 热键与 UI

| 热键 | 动作 |
|---|---|
| `Ctrl+Shift+D` | 下一个目录(循环) |
| `Ctrl+Shift+A` | 上一个目录(循环) |

选这组键是因为单键已全部占用:`A/D/Q/W/S/R/Space/0-9/反引号/方向键`,
以及 `Ctrl+S/D/C/V/Z/Y/0/Delete`。加新热键前先查构造函数里的 QShortcut 区
(mainwindow.cpp 开头)和 README 快捷键表,避免冲突。

Folders tab(右侧面板第三个 tab):
- `QListWidget` 显示目录名,当前目录带 `(进度/总数)`;
- 底部 "+ Open New Folder…" 按钮走现有 `open_img_dir()` 流程(内部自动注册目录);
- 点击列表项 → `switch_to_dir()`;热键切换后列表选中项同步刷新。

---

## 四、经验教训(按踩坑顺序)

### 4.1 自然排序:拼排序 key 是陷阱,成对数值比较才对

**第一版做法**(错误):把目录名转成"排序 key 字符串"(数字段去前导零后拼接),
再用 `key < key` 比较。

**为什么错**:`train2` → key `train2`,`train10` → key `train10`,
字典序 `"train10" < "train2"` 依然成立(`'1' < '2'`)。把数字折叠进字符串
并不能改变字符串比较的字典序本质。

**正确做法**:逐字符成对比较(`numericCompare`),遇到数字段就提取两边的
完整数字串,**按数值比较**(先比去前导零后的长度,再逐位比),字母段按
小写字典序。数字段和字母段交替推进。这正是 `QCollator::setNumericMode(true)`
的语义,只是自己实现了可单测的版本。

### 4.2 一个初始化 bug 让所有数字比较静默失效

数值比较里的前导零剥离:

```cpp
size_t za = ia;                       // 错!ia 是数字段结尾
while (za < ia && a[za] == '0') ++za; // 条件永假,za 不动
size_t lenA = ia - za;                // 恒等于 0!

size_t za = i;                        // 对!i 是数字段开头
while (za < ia && a[za] == '0') ++za;
```

`za` 初始化成段尾 `ia` 导致 `lenA` 恒为 0,所有数字段比较都返回"相等",
排序退化成纯字典序——**不崩溃、不报错,只是结果不对**。
定位手段:当测试失败但代码"看着没问题"时,把可疑函数复制出来加 printf
插桩(打印段边界和长度),单独编译运行,和正式模块二分对照。

### 4.3 测试断言自己也会错:分清"代码 bug"和"测试 bug"

调试后期有一半失败是**测试预期写错**,不是实现错。典型:
`switchToDir(-1)` 的循环取模语义(`((-1 % n) + n) % n`)在纸面上算错过两次。
教训:
- 测试失败先问"实现和断言哪个更可能错",别默认实现有罪;
- 循环取模、边界 wrap 这类逻辑,预期值要在纸上逐步推,别心算。

### 4.4 索引跟踪脆弱,路径跟踪可靠

往有序列表插入元素后维护"当前索引"(`if (inserted < m_current) ++m_current`)
在插入点等于当前位置等边界下出错。**可靠做法**:排序前记下当前条目的
**路径**,排序后按路径重新查找索引。以 O(n) 查找换确定性,值得。

### 4.5 无 Qt 环境也能开发核心逻辑

Qt-free 核心模块 + 裸 g++ 单测的组合,让"环境没就绪"没有阻塞开发。
等 Qt 装好后,集成层暴露的只有 3 个编译错误,十分钟修完:

1. `open_img_dir(ok)` 返回 `void`,误放进 `if` 条件 → 先调用再查出参;
2. `QVariant` 没有 `toVariantMap()`,正确的是 `toMap()`;
3. 按钮 lambda 里重复注册目录(`open_img_dir` 内部已做)→ 删冗余。

### 4.6 GUI 程序的自动化验证策略(本机没法截图)

没有截图手段时,用三层验证替代:

1. **进程存活**:启动 6 秒后 `tasklist` 仍在 → 没有启动即崩;
2. **持久化检查**:`reg query` 注册表,确认 `dirList/dirProg/dirCur` 写入正确;
3. **会话恢复**:第二次不带参数启动,仍稳定存活 → restore 路径没崩。

交互细节(热键手感、列表高亮)留给人工验收。自动化证明"不崩 + 状态对",
人工证明"好用"。

### 4.7 引用失效隐患:快照优先于持有引用

Qt 层曾写 `const DirEntry &target = *m_dirs.currentEntry();`,随后调用
`get_files()/init()` 可能间接改动 `m_dirs` 使引用悬空。改为**立即拷贝**出
`QString newDir/newCls/int savedProg` 再往下走。对会变化的容器,
取值即拷贝,别长期持有引用。

---

## 五、已知限制与后续工作

| # | 限制 | 说明 / 建议 |
|---|---|---|
| 1 | **类别绑定没有 UI 入口** | `DirManager::setClassesFile` 机制完备,切换/恢复都会遵守,但没有任何界面能设置它。建议:Folders 列表项右键菜单 "Bind classes file…" |
| 2 | **没有移除目录的 UI** | 核心 `removeDir()` 已实现并测试,未接界面。建议:右键菜单 "Remove from list"(只移出列表,不删磁盘文件) |
| 3 | 进度总数只在当前目录显示 | 其他目录的 `(n/total)` 需要 `entryList` 扫描才知道 total,未做缓存,列表里只显示名字 |
| 4 | 目录内容变化不自动刷新 | 新增/删除图片后需重新打开该目录才刷新列表 |
| 5 | 仅 Windows 回归过 | 代码是 Qt6 跨平台写法,Ubuntu/macOS 未重新编译验证;Linux 上 QSettings 落 `~/.config/YoloLabel/Session.conf` 而非注册表 |
| 6 | **项目没有版本控制** | 强烈建议 `git init` + 首次提交(记得 `.gitignore` 掉 `build/`),再继续改动 |

---

## 六、维护手册

### 6.1 改动后的验证清单(按序执行)

1. 单测:`62 checks, 0 failures`(见 §2.3);
2. 编译:`mingw32-make release` 零 error(§2.2);
3. 启动存活:带参启动 + 无参启动各一次,进程 6 秒不退(§4.6);
4. 注册表抽查:`dirList` 内容与预期一致;
5. 人工:开 2-3 个目录,验证 `Ctrl+Shift+D/A` 循环切换、进度恢复、重启后会话恢复。

### 6.2 常见改动锚点

| 想做什么 | 去哪里改 |
|---|---|
| 加/改热键 | `MainWindow` 构造函数 QShortcut 区(mainwindow.cpp 顶部);同步更新 README 快捷键表 |
| 加侧边栏 tab | `initSideTabWidget()`(样式变量 `tabStyle/btnStyle` 已备好) |
| 改切换逻辑 | `switch_to_dir()`(注意 §4.7,取值即拷贝) |
| 改持久化字段 | `save_dir_state()` / `restore_dir_state()` 成对改;新键记得处理旧数据兼容 |
| 改排序/去重规则 | `DirManager::lessThan` / `addDir`(dir_manager.cpp),必须先补单测 |
| 加源文件 | 同步更新 `YoloLabel.pro` 的 SOURCES/HEADERS |

### 6.3 排障速查

| 症状 | 首查 |
|---|---|
| 双击 exe 报缺 DLL | 没 deploy:PATH 加 Qt bin,或跑 windeployqt(§2.4) |
| 重启后目录列表空 | 注册表 `dirList` 是否写入;路径是否还存在(目录被删则跳过恢复) |
| 切换目录后类别不对 | 该目录的 `dirCls` 绑定(目前只能手改注册表,见限制 #1) |
| 目录顺序"不对" | §4.1:数字感知排序,`train2 < train10` 是**预期行为** |
| 单测失败 | 先分清代码/断言谁错(§4.3),再 printf 插桩(§4.2) |

---

*文档版本:2026-08-20 · 对应提交状态:多目录功能已实现并通过 Windows 编译验证*
