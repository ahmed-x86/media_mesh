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

## 🚀 Building from Source

It is recommended to build the project in a separate `build` directory:

```bash
git clone https://github.com/ahmed-x86/media_mesh.git
cd media_mesh
```
and build it
```bash
cmake . && make 
```