# YoloLabel — Multi-Folder Edition

> **This is a modified fork of [Yolo_Label](https://github.com/developer0hye/Yolo_Label)
> by Yonghye Kwon, published under the same MIT License.**
> Full credit for the original tool goes to the original author — the bulk of this README
> is his original documentation, kept for completeness.

![multi-folder labeling](docs/screenshot_multidir.png)

## What's new in this fork / 本 fork 的新增内容

> **The headline improvement is multi-folder continuous labeling.**
> The original tool forces you to reopen a dialog every time you switch dataset folders —
> this fork removes that friction so you can keep a whole labeling session running across
> as many folders as you need.
>
> **本 fork 最核心的改进是：支持多目录连续标注。**
> 原版切换数据集目录必须重新打开文件对话框——本 fork 去掉了这个麻烦，整个标注会话可以在任意多个目录之间无缝进行。

- **Hotkeys / 热键切换**: `Ctrl + Shift + D` / `Ctrl + Shift + A` cycle to the next / previous
  folder (ordered naturally, so `train2` comes before `train10`).
  按 `Ctrl + Shift + D` / `Ctrl + Shift + A` 在目录之间循环切换(按自然顺序排序，`train2` 排在 `train10` 之前)。

- **Folders tab / 侧边栏目录列表**: a new tab in the right panel lists every opened folder,
  shows the active one with its progress (`images (1/11)` in the screenshot above), and switches
  on click. The **+ Open New Folder…** button adds more folders.
  右侧面板新增 “Folders” 标签页，列出所有已打开目录，当前目录带进度提示(如截图中的 `images (1/11)`)，点击即可切换；底部 “+ Open New Folder…” 按钮可继续追加目录。

- **Progress memory / 进度记忆**: each folder remembers the last image you were on — switch
  away and back, and you continue where you left off. Progress also survives app restarts.
  每个目录独立记忆浏览到的图片位置——切走再切回，仍在原处继续。进度也跨程序重启保留。

- **Per-folder class files / 每目录类别文件**: each folder can be bound to its own class list;
  switching to it loads that list automatically (infrastructure in place, UI to bind is on the
  roadmap).
  每个目录可以绑定自己的类别文件，切换时自动加载对应类别(底层机制已完备，绑定 UI 在路线图中)。

- **Legacy migration / 旧会话兼容**: old single-folder sessions are picked up automatically.
  旧版单目录的 QSettings 会话会自动迁移到目录列表里，老用户升级无感。

Design notes, build instructions, lessons learned and a maintenance guide are documented
in [docs/development-guide.md](docs/development-guide.md). The folder bookkeeping lives in
a Qt-free `DirManager` core covered by 62 unit tests (`tests/test_dir_manager.cpp`).

设计思路、踩坑记录与维护手册见 [docs/development-guide.md](docs/development-guide.md)。
目录管理逻辑抽离在零 Qt 依赖的 `DirManager` 核心中，含 62 个单元测试
(`tests/test_dir_manager.cpp`)。

Everything below is the original YOLO-Label documentation.

---

## Sponsors

- AIM(https://www.aimdefence.com/)
<a href="https://www.aimdefence.com/">
 <img src="https://user-images.githubusercontent.com/35001605/143895639-79b0b6b1-365f-4132-b272-f161d3ef5cb4.png" width="200">
</a>


## WHAT IS THIS?!
 Reinventing The Wheel?!!!!
 
 ![1_hfyjxxcfingbcyzcgksaiq](https://user-images.githubusercontent.com/35001605/47629997-b47aa200-db81-11e8-8873-71ae653563e0.png)

 In the world, there are many good image-labeling tools for object detection. -e.g. , ([Yolo_mark](https://github.com/AlexeyAB/Yolo_mark), [BBox-Label-Tool](https://github.com/puzzledqs/BBox-Label-Tool), [labelImg](https://github.com/tzutalin/labelImg)). 
 
But... I've reinvented one...
 
## WHY DID YOU REINVENT THE WHEEL? ARE YOU STUPID?

 When I used the pre-existing programs to annotate a training set for YOLO V3, I was sooooooooooo bored...
 
 So I thought why it is so boring??

 And I found an answer.
 
 The answer is that pre-existing programs are not **sensitive**.
 
 So I decided to make a **sensitive** image-labeling tool for object detection.
 
 ## SHOW ME YOUR SENSITIVE IMAGE-LABELING TOOL!!

 It's the **SENSITIVE** image-labeling tool for object detection!
 
![image](docs/screenshot_main.png)

https://user-images.githubusercontent.com/35001605/211560039-367f27d7-63ab-4342-824e-9f47f2afbc35.mp4

![cut (2)](https://user-images.githubusercontent.com/35001605/143729909-b2da3669-020a-4769-ab1d-2646dd7bbb6b.gif)

 ## HMM... I SAW THIS DESIGN SOMEWHERE
  I refer to [the website of Joseph Redmon](https://pjreddie.com/darknet/
) who invented the [YOLO](https://www.youtube.com/watch?v=MPU2HistivI).

  ![redmon2](https://user-images.githubusercontent.com/35001605/47635529-a1270100-db98-11e8-8c03-1dcea7c77d1d.PNG)
# TUTORIAL / USAGE

## Download

Pre-built binaries are available on the [Releases](https://github.com/developer0hye/Yolo_Label/releases) page.

| OS | Download | Note |
|---|---|---|
| **Windows (x64)** | [YoloLabel-Windows-x64.zip](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-Windows-x64.zip) | Unzip and run `YoloLabel.exe` |
| **Linux (x64)** | [YoloLabel-Linux-x64.AppImage](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-Linux-x64.AppImage) | `chmod +x` and run |
| **macOS (Apple Silicon)** | [YoloLabel-macOS.dmg](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-macOS.dmg) | Open DMG and drag to Applications |

## Install and Run

### For Windows

1. Download [YoloLabel-Windows-x64.zip](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-Windows-x64.zip)

2. Unzip

3. Run YoloLabel.exe

![image](https://user-images.githubusercontent.com/35001605/111152300-e74b5680-85d3-11eb-8df7-178148548c12.png)

### For Linux

1. Download [YoloLabel-Linux-x64.AppImage](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-Linux-x64.AppImage)

2. Make executable and run
```
chmod +x YoloLabel-Linux-x64.AppImage
./YoloLabel-Linux-x64.AppImage
```

### For macOS

1. Download [YoloLabel-macOS.dmg](https://github.com/developer0hye/Yolo_Label/releases/latest/download/YoloLabel-macOS.dmg)

2. Open the DMG and drag `YoloLabel.app` to Applications

3. Launch YoloLabel from Applications

> **Build from source:** If you prefer, install Qt 6 (`brew install qt@6`), clone this repo, then run `qmake && make`. See [Build with ONNX Runtime](#build-with-onnx-runtime) for auto-label support.

## Prepare Custom Dataset and Load

1. Put your .jpg, .png -images into a directory
(In this tutorial I will use the Kangarooo and the Raccoon Images. These images are in the 'Samples' folder.)

![dataset](https://user-images.githubusercontent.com/35001605/47704306-8e7afd80-dc66-11e8-9f28-13010bd2d825.PNG)

2. Put the names of the objects, each name on a separate line and save the file( .txt, .names).

![objnames](https://user-images.githubusercontent.com/35001605/47704259-75724c80-dc66-11e8-9ed1-2f84d0240ebc.PNG)

3. Run Yolo Label!

![image](https://user-images.githubusercontent.com/35001605/143729836-b2ee1188-f829-473f-aff0-d13569b3fc39.png)

4. Click the button 'Open Files' and open the folder with the images and the file('*'.names or '*'.txt) with the names of the objects.

![image](https://user-images.githubusercontent.com/35001605/211560758-f119f562-9462-4ebe-86fa-a9c169b18993.png)

5. And... Label!... Welcome to Hell... I really hate this work in the world.

This program has adopted a different labeling method from other programs that adopt **"drag and drop"** method.

To minimize wrist strain when labeling, I adopted the method **"twice left button click"** method more convenient than 

**"drag and drop"** method.

**drag and drop**

![draganddrop](https://user-images.githubusercontent.com/35001605/48674135-6fe49400-eb8c-11e8-963c-c343867b7565.gif)


**twice left button click**

![twiceleftbuttonclickmethod](https://user-images.githubusercontent.com/35001605/48674136-71ae5780-eb8c-11e8-8d8f-8cb511009491.gif)


![ezgif-5-805073516651](https://user-images.githubusercontent.com/35001605/47698872-5bc80980-dc54-11e8-8984-e3e1230eccaf.gif)

6. End

![endimage](https://user-images.githubusercontent.com/35001605/47704336-a6528180-dc66-11e8-8551-3ecb612b7353.PNG)

## USAGE AND OPTIONS
```
./YoloLabel [dataset dir] [class file] [model.onnx]
# Examples
./YoloLabel ../project/dataset/objects/frames ../project/dataset/objects/obj_names.txt
./YoloLabel ../project/dataset/objects/frames ../project/dataset/objects/obj_names.txt yolov8n.onnx
./YoloLabel ../project/dataset/objects/frames yolov8n.onnx
```

Arguments are detected by file extension — `.onnx` files are loaded as YOLO models, all other files are loaded as class name lists. When a model with embedded class names is loaded without a class file, class names are populated from the model automatically.


## SHORTCUTS

| Key | Action |
|---|:---:|
| `A` | Save and Prev Image  |
| `D,  Space` | Save and Next Image |
| `S` | Next Label <br> ![ezgif-5-f7ee77cd24c3](https://user-images.githubusercontent.com/35001605/47703190-d3049a00-dc62-11e8-846f-5bd91e98bdbc.gif)  |
| `W` | Prev Label <br> ![ezgif-5-ee915c66dad8](https://user-images.githubusercontent.com/35001605/47703191-d39d3080-dc62-11e8-800b-986ec214b80c.gif)  |
| `O` | Open Files |
| `V` | Visualize Class Name |
| `Ctrl + S` | Save |
| `Ctrl + Delete` (Windows/Linux) / `Cmd + Delete` (macOS) | Delete all existing bounding boxes in the image |
| `Ctrl + D`, `Delete` | Delete current image |
| `` ` `` (Backtick) | Select first class (class 0) |
| `0-9` | Quick select class by number |
| `Arrow Keys` | Nudge the bounding box under the cursor (~1-2px step) |
| `Shift + Arrow Keys` | Nudge the bounding box under the cursor (~5px step) |
| `Ctrl + Arrow Keys` (Windows/Linux) / `Cmd + Arrow Keys` (macOS) | Resize the bounding box under the cursor (~1-2px step, top-left corner fixed) |
| `Ctrl + Shift + Arrow Keys` (Windows/Linux) / `Cmd + Shift + Arrow Keys` (macOS) | Resize the bounding box under the cursor (~5px step, top-left corner fixed) |
| `Ctrl + C` (Windows/Linux) / `Cmd + C` (macOS) | Copy bounding boxes to clipboard |
| `Ctrl + V` (Windows/Linux) / `Cmd + V` (macOS) | Paste bounding boxes from clipboard |
| `Ctrl + Z` (Windows/Linux) / `Cmd + Z` (macOS) | Undo last action (add, remove, or clear all) |
| `Ctrl + Y` (Windows) / `Ctrl + Shift + Z` (Linux) / `Cmd + Shift + Z` (macOS) | Redo last undone action |
| `R` | Auto Label current image (requires loaded ONNX model) |
| `Ctrl + 0` (Windows/Linux) / `Cmd + 0` (macOS) | Reset zoom to 100% |
| `Ctrl + Shift + D` (Windows/Linux) / `Cmd + Shift + D` (macOS) | Switch to next folder (cyclic) |
| `Ctrl + Shift + A` (Windows/Linux) / `Cmd + Shift + A` (macOS) | Switch to previous folder (cyclic) |

| Mouse | Action |
|---|:---:|
| `Right Click` | Delete Focused Bounding Box in the image <br> ![ezgif-5-8d0fb51bec75](https://user-images.githubusercontent.com/35001605/47706913-c20d5600-dc6d-11e8-8a5c-47065f6a6416.gif) |
| `Left Click + Drag` on existing box | Move/reposition the bounding box |
| `Option + Left Click` (macOS) / `Alt + Left Click` (Win/Linux) | Change class of focused bounding box to the currently selected label (no need to remove and redraw) |
| `Double Click` on color column in label table | Change label color |
| `Ctrl + Scroll` (Windows/Linux) / `Cmd + Scroll` (macOS) | Zoom in/out (centered on cursor, up to 10x) |
| `Ctrl + Left Drag` (Windows/Linux) / `Cmd + Left Drag` (macOS) or `Middle Mouse Drag` | Pan while zoomed in |
| `Wheel Down` (when cursor is over image) | Save and Next Image  |
| `Wheel Up` (when cursor is over image) | Save and Prev Image |

## Auto-Label (Pseudo Labeling)

Tired of drawing every single bounding box by hand? Load a pre-trained YOLO model and let it do the boring work for you.

YOLO-Label supports **local ONNX inference** — just export any [Ultralytics](https://github.com/ultralytics/ultralytics) detection model to `.onnx` and load it. Class names, input size, and model configuration are all read from the ONNX metadata automatically. No separate config files needed.

### Supported Models

Any Ultralytics detection model exported with `model.export(format="onnx")`:

| Model | Note |
|---|---|
| YOLOv5 | Anchor-based, objectness score |
| YOLOv8, YOLO11, YOLO12, YOLOv26 | Anchor-free |
| End-to-end models | NMS baked in |

### How to Use

1. Open your dataset as usual (images + class file)
2. Click **Load Model** and select a `.onnx` file
3. Adjust the **confidence threshold** with the slider (default 25%)
4. Click **Auto Label** (or press `R`) to detect objects on the current image
5. Click **Auto Label All** to batch-process all images
6. Review and correct results using the existing manual annotation tools — auto-labeled boxes can be moved, deleted, or undone (`Ctrl+Z`)

> **Tip:** You can skip the class file entirely. If the model has embedded class names (all Ultralytics exports do), they will be loaded automatically.

### Build with ONNX Runtime

Pre-built releases include ONNX Runtime. To build from source with auto-label support:

```bash
# Download ONNX Runtime (one-time setup)
./scripts/download_onnxruntime.sh

# Build
qmake YoloLabel.pro "ONNXRUNTIME_DIR=$PWD/onnxruntime"
make -j$(nproc)
```

Without ONNX Runtime, the app builds and works normally — just without the auto-label feature.

## Cloud Auto-Label (yololabel.com)

No local GPU? No problem. YOLO-Label integrates with **[yololabel.com](https://yololabel.com)** — a cloud inference service that runs open-vocabulary object detection on your images without requiring any local model or GPU.

### Create an Account and Get an API Key

1. Go to [yololabel.com](https://yololabel.com) and sign up with **email + password** or **Sign in with Google**
2. If you registered with email, check your inbox and click the verification link
3. Log in and navigate to **API Keys**, then click **Create Key**
4. Give the key a name and copy it immediately — it is shown only once

### How to Use

1. Open your dataset as usual
2. In the **⚙ AI Settings** tab on the right panel, paste your API key and (optionally) a custom detection prompt
   - The prompt is a semicolon-separated list of labels, e.g. `car; person; bicycle`
   - Leave blank to use the loaded class names automatically
3. Click **☁ Auto Label AI** to detect objects in the current image
4. Click **☁ Auto Label All AI** to process the entire dataset

### Batch Processing

When **☁ Auto Label All AI** is clicked with multiple images, images are submitted in batches of up to 20 per request using the `/v1/jobs/batch` endpoint. Progress is shown live in the button label.

## Contrast Adjustment

Use the **Contrast slider** at the top of the window to adjust image brightness/contrast in real-time. This is useful when labeling dark or overexposed images. The slider ranges from 0% to 100% (default 50%).

## Usage Timer

A timer in the status bar counts how many hours (and minutes/seconds) you have been using the program. It runs **only while the window is focused** (switches to another app to pause). Use the **Reset** button in the status bar to zero the timer at any time.

## Button Events

### Remove

It was replaced by the shortcut **Ctrl + D**.

![ezgif-2-90fb8205437e](https://user-images.githubusercontent.com/35001605/49983945-fbddb600-ffa8-11e8-9672-f7b71e4e603b.gif)

## ETC

You can access all image by moving horizontal slider bar. But when you control horizontal slider bar, the last processed image will not be saved automatically. So if you want not to lose your work, you should save before moving the horizontal slider bar.

![ezgif-5-53abf38b3387](https://user-images.githubusercontent.com/35001605/47708528-97bd9780-dc71-11e8-94f1-5ee23776d5fe.gif)

# CONCLUSIONS

I've reinvented the wheel.

![dont-reinvent-the-wheel](https://user-images.githubusercontent.com/35001605/47709289-46160c80-dc73-11e8-8ef6-6af3a3c52403.jpg)

# TO DO LISTS

- [x] ~~Upload binary file for easy usage for windows and ubuntu~~
- [x] ~~deployment for ubuntu~~
- [x] ~~macOS Developer signing for Gatekeeper~~
