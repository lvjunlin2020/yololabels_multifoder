# 多目录管理改进计划(Multi-Directory Support)

> **状态:已全部落地**(2026-08-20,Windows Qt 6.11.2 编译验证通过,62 项单测全过)。
> 实施记录、经验教训与维护手册见 [development-guide.md](development-guide.md)。

## 背景

YoloLabel 目前一次只能打开一个图片目录;要标注另一个目录的数据集,必须重新打开
文件对话框(甚至重开程序)。对于"多个小数据集交替标注"的工作流很不方便。

目标:
1. 维护一个**目录列表**(按目录名数字排序),支持热键快速切换;
2. 侧边栏增加 **Folders** tab,可视化地查看/切换目录,显示每个目录的标注进度;
3. 切换目录时自动**保存当前标注**、**恢复上次进度**(每个目录记住看到第几张)、
   **按目录缓存类别表**(不同数据集可以用不同的 classes 文件);
4. 目录列表、进度、类别文件路径持久化到 QSettings,重启后保留。

## 架构

```
dir_manager.hpp / dir_manager.cpp        ← 纯 C++ 核心逻辑(零 Qt 依赖,可单测)
  DirManager
    - addDir(path)               追加目录(去重、排序)
    - switchToDir(index)         切换并返回是否变化
    - removeDir(path)            移除(当前目录被移除时落到相邻目录)
    - setProgress(dir, idx)      记录每目录浏览进度
    - setClasses(dir, path)      绑定目录的类别文件
    - load/save(QSettings 由调用方处理)

mainwindow.h / mainwindow.cpp           ← Qt 层
  - 持有 DirManager m_dirs
  - switch_to_dir(int)   保存当前标注 → 加载新目录 → 恢复进度/类别
  - Folders tab:QListWidget + "Open New Folder…" 按钮
  - 热键:Ctrl+Shift+D 下一目录 / Ctrl+Shift+A 上一目录(循环)
  - saveSession/restoreLastSession 升级为目录列表 + 进度 + 类别路径
```

选择纯 C++ 核心的原因:本机没有 Qt 开发环境(无法 qmake 全量编译),
把排序/去重/切换/进度逻辑抽出来后,可以用 mingw64 g++ 直接跑单元测试,
保证核心逻辑正确;Qt 层只做 UI 粘合。

## 数据模型

```cpp
struct DirEntry {
    QString path;        // 绝对路径
    int     progress = 0;// 上次在该目录看到的图片索引
    QString classesFile; // 该目录绑定的类别文件(可为空 = 沿用全局)
};
```

持久化(QSettings "YoloLabel"/"Session"):
- `dirList`  : QStringList(目录绝对路径)
- `dirProg`  : QVariantMap(dir → int)
- `dirCls`   : QVariantMap(dir → QString)
- 兼容旧字段 `imgDir`/`objFile`:首次运行时若 `dirList` 为空但 `imgDir` 有值,
  自动迁移进列表。

## 切换流程 switch_to_dir(index)

1. `m_cloudLabeler` 忙则拒绝(与现有 next_img 一致);
2. 若目标目录 == 当前目录:只恢复进度 `goto_img(progress)`,直接返回;
3. 保存当前目录标注 `save_label_data()`,记录当前进度;
4. 类别处理:
   - 目标目录有绑定类别文件 → `load_label_list_data()` 加载;
   - 无绑定但全局已有类别 → 沿用(不重载);
   - 全局无类别 → 弹文件对话框选择(用户取消则仍切换,类别留空)。
5. `get_files(dir)` 加载图片;`init()` 初始化 UI;
6. `goto_img(min(progress, imgCount-1))` 恢复进度;
7. 更新 Folders 列表选中项、状态栏提示 `Switched to: <dir> (i/n)`。

## 热键

| 热键 | 动作 |
|---|---|
| Ctrl+Shift+D | 下一个目录(循环) |
| Ctrl+Shift+A | 上一个目录(循环) |

避开已占用按键:A/D/Q/W/R/S/数字/方向键/Ctrl+S/Ctrl+D/Ctrl+C/V/0/Del。

## 测试

`tests/test_dir_manager.cpp` — 纯 g++ 单元测试(无 Qt):
- addDir 去重 + 数字排序(2 < 10)
- switchToDir 越界/循环/当前目录
- removeDir 当前目录回落
- progress 记录与读取
- classes 绑定与查询

编译:`g++ -std=c++17 -I<root> tests/test_dir_manager.cpp <root>/dir_manager.cpp`

## 实施顺序

1. 本文档
2. dir_manager.{hpp,cpp} + 单元测试 → 本地跑通
3. mainwindow 接入(成员、切换逻辑、持久化)
4. 热键 + Folders tab UI
5. README 快捷键表更新

## 不做的事

- 不改 label_img / yolo_detector / cloud_labeler;
- 不做目录树(只支持平级列表;递归子目录以后再说);
- 类别文件不做自动猜测(目录旁自动找 *.names),只支持手动绑定 + 沿用全局。
