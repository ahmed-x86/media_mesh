# MediaMesh 🎞️

**MediaMesh** is a sleek, lightweight, and blazing-fast GUI wrapper for FFmpeg, built with C++ and Qt6. It takes the power of terminal-based FFmpeg commands and wraps them in a beautiful, Catppuccin Mocha-themed graphical interface with real-time progress tracking.

## ✨ Features

- **CLI to GUI:** Trigger conversions directly from the terminal, and watch the magic happen in a beautiful UI.
- **Hardware Acceleration:** Native support for NVIDIA (CUDA/NVENC) and AMD (VAAPI) for lightning-fast encoding.
- **Real-time Metrics:** Displays conversion progress, current speed, elapsed time, processed frames, and a live ETA.
- **Catppuccin Mocha Theme:** A gorgeous, eye-friendly dark mode UI out of the box.
- **Wide Format Support:** Supports converting to `mp4`, `mkv`, `webm`, `av1`, `mov` (Davinci Resolve & ProRes), `mp3`, `aac`, `gif`, `jpg`, `png`, `webp`, and more.

## 🛠️ Dependencies

Ensure you have the following installed on your system:
- `ffmpeg` & `ffprobe`
- `qt6-base` (Qt6 Core, Gui, Widgets)
- `cmake`
- `make` or `ninja`
- A C++17 compatible compiler (GCC/Clang)

## 🚀 Building from Source & install in system

It is recommended to build the project in a separate `build` directory:

```bash
git clone https://github.com/ahmed-x86/media_mesh.git
cd media_mesh
```
and build it
```bash
cmake . && make -j$(nproc) && sudo cp MediaMesh /usr/bin/media_mesh
```
## Add to right-click menu

First, navigate to the output directory:
```bash
cd output
```
### 1. Caja (MATE File Manager)
```bash
mkdir -p ~/.local/share/caja/scripts
cp caja/MediaMeshMenu.py ~/.local/share/caja/scripts/
chmod +x ~/.local/share/caja/scripts/MediaMeshMenu.py
```
### 2. Dolphin (KDE File Manager)
```bash
mkdir -p ~/.local/share/kio/servicemenus
cp dolphin/*.desktop ~/.local/share/kio/servicemenus/
```
### 3. Nautilus (GNOME File Manager)
```bash
mkdir -p ~/.local/share/nautilus/scripts
cp nautilus/MediaMeshMenu.py ~/.local/share/nautilus/scripts/
chmod +x ~/.local/share/nautilus/scripts/MediaMeshMenu.py
```
### 4. Nemo (Cinnamon File Manager)
```bash
mkdir -p ~/.local/share/nemo/actions
cp nemo/MediaMeshMenu.py ~/.local/share/nemo/actions/
chmod +x ~/.local/share/nemo/actions/MediaMeshMenu.py
```
### 5. PCManFM (LXDE / LXQt)
```bash
mkdir -p ~/.local/share/file-manager/actions
cp pcmanfm/*.desktop ~/.local/share/file-manager/actions/
```
### 6. Thunar (XFCE File Manager)
```bash
mkdir -p ~/.config/Thunar/
```