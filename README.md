# WaspEngine
Game engine and tech demos.

A small native Win32 + OpenGL application/engine prototype, built with C++ and MSVC.

## What it does

- Creates a Win32 window and runs a real-time game loop (`Input` → `Update` → `Render`) driven by `QueryPerformanceCounter`-based delta time.
- Renders a rotating, colored 3D pyramid using a hand-rolled OpenGL 3.3 core-profile renderer (no external GL loader or math library — see [`WaspEngine/Renderer.h`](WaspEngine/Renderer.h) / [`WaspEngine/Renderer.cpp`](WaspEngine/Renderer.cpp)).
- Supports moving the pyramid with the arrow keys.
- Opens a console window alongside the main window for diagnostic logging (window/HWND/input device creation status).

## Requirements

- Windows 10/11
- Visual Studio 2022+ (or Visual Studio 2026) with the **Desktop development with C++** workload
- A GPU/driver supporting OpenGL 3.3 core profile

## Building

### From Visual Studio

Open `WaspEngine.sln` and build (`Ctrl+Shift+B`).

### From the command line

Using `make` (see [Makefile](Makefile)):

```powershell
make build            # Debug|x64 (default)
make build CONFIGURATION=Release
make run              # build then launch the app
make rebuild          # force a full rebuild
make clean            # remove build outputs
make stop             # stop a running instance
```

Or directly with MSBuild (requires a Developer PowerShell/Command Prompt, or `msbuild.exe` on PATH):

```powershell
msbuild WaspEngine.sln /p:Configuration=Debug /p:Platform=x64
```

The built executable lands at `x64\<Configuration>\WaspEngine.exe`.

## Controls

| Key         | Action                    |
|-------------|---------------------------|
| Arrow keys  | Move the pyramid in-frame |

## Continuous Integration

Every push and pull request to `main` triggers the [Build workflow](.github/workflows/build.yml), which builds both `Debug` and `Release` (`x64`) on a Windows GitHub Actions runner. See [CONTRIBUTING.md](CONTRIBUTING.md) for the branching/PR workflow and branch protection details.

## Project structure

```
WaspEngine.sln                   Solution file
WaspEngine/
  main.cpp                      WinMain, window creation, game loop, WinProc
  Renderer.h / Renderer.cpp      OpenGL 3.3 renderer (context, shaders, pyramid geometry)
  stdafx.h / stdafx.cpp          Precompiled header stub
  WaspEngine.vcxproj              Project file
Makefile                         Local build/run/clean commands
.github/workflows/build.yml      CI build workflow
CONTRIBUTING.md                  Branching/PR conventions
```
