# ImagoRef

<img width="1920" height="1080" alt="ImagoRef" src="https://github.com/user-attachments/assets/983b1af5-8686-469d-9f02-5d50836c96b6" />

ImagoRef is a desktop reference-board application built with `Qt 6`, `QML`, and `C++`.

It is the main desktop client in the ImagoRef ecosystem:

- `ImagoRef` — desktop editor for boards
- `ImagoRef-mobile` — mobile sync viewer
- `ImagoRef-backend` — cloud sync backend

## Highlights

- infinite-style visual board for reference images
- labels, transforms, crop, opacity, and arrangement tools
- local board files and cloud synchronization
- themed `QML` interface with custom assets
- packaging for macOS, Windows, and Linux

## Repository Layout

- `src/` — application source code, controllers, models, managers, and QML
- `res/` — themes and app assets
- `packs/` — platform packaging metadata and installer resources
- `docs/` — project documentation, license, notices, and contribution guide
- `.github/workflows/` — GitHub Actions pipelines for release automation

## Local Build

```bash
cmake -S . -B build
cmake --build build
```

## Release Automation

Pushes to `main` trigger the desktop release workflow. It builds:

- `macOS` package (`.pkg`)
- `Windows` installer (`.exe`)
- `Linux` AppImage (`.AppImage`)

The workflow then creates a Git tag and a GitHub Release automatically.

## Documentation

Project documentation lives in [docs/README.md](docs/README.md).
