# MovPress

[Русская версия](README_RUS.md)

MovPress is a lightweight macOS application for compressing screen recordings and creating optimized GIFs. It provides a Qt 6 desktop interface and uses the system-installed `ffmpeg` and `ffprobe` tools for media processing.

![MovPress application window](docs/screenshot.png)

## Features

- Export to MP4/H.264 or GIF with an adaptive color palette
- Built-in playback of both the source and converted video
- Side-by-side video preview and compression settings
- Visual selection of the clip start and end points
- Preview playback limited to the selected clip range
- Custom start time and duration
- Configurable resolution, frame rate, CRF, and encoding preset
- Optional audio preservation
- Custom video cropping
- Ready-to-use profiles for YouTube, Habr, Telegram, and maximum compression
- Drag-and-drop video selection
- Accurate conversion progress reported by `ffprobe`
- Before-and-after file size comparison and space-saving percentage
- Conversion cancellation
- Open the result or reveal it in Finder
- Persistent settings and last-used directory
- Warnings before overwriting files or when disk space is low

## Requirements

- macOS
- CMake 3.21 or newer
- A C++20 compiler
- Qt 6.5 or newer with Widgets, Multimedia, and MultimediaWidgets
- `ffmpeg` and `ffprobe` available in `PATH`, `/opt/homebrew/bin`, or `/usr/local/bin`

## Build

Install the dependencies with Homebrew:

```bash
brew install qt ffmpeg
```

Configure and build the application:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
open build/MovPress.app
```

When using a Qt installation created by the official Qt installer, invoke its `qt-cmake` directly:

```bash
/path/to/Qt/6.x.x/macos/bin/qt-cmake -S . -B build
cmake --build build
open build/MovPress.app
```

## Usage

1. Drop a video onto the application or select it using the file dialog.
2. Preview the video and select the required fragment.
3. Choose a ready-made profile or adjust the compression settings manually.
4. Select the output path and start the conversion.
5. Preview the converted file, open it, or reveal it in Finder.

## Status

MovPress is under active development. Current version: 0.1.0.
