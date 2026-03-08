# Ragna-TH Build & Installation Guide

Complete instructions for building the client, dedicated server, and editor on all supported platforms.

---

## Prerequisites

### All Platforms

| Dependency | Version | Notes |
|-----------|---------|-------|
| **Unreal Engine** | 5.3+ | Source build or Epic Games Launcher |
| **Git** | 2.x+ | For cloning the repository |
| **PostgreSQL** | 14+ | Server-side database (development can use SQLite) |

### Windows

| Dependency | How to Install |
|-----------|----------------|
| Visual Studio 2022 | Install with the **Game development with C++** and **Desktop development with C++** workloads |
| .NET 6.0 SDK | Required by UnrealBuildTool |
| Windows 10/11 SDK | Installed via Visual Studio Installer |

### macOS

| Dependency | How to Install |
|-----------|----------------|
| Xcode 14.1+ | `xcode-select --install` or install from the App Store |
| Command Line Tools | Included with Xcode |
| Metal-compatible GPU | Required — macOS is the primary platform |

### Linux

| Dependency | How to Install |
|-----------|----------------|
| clang 16+ | `sudo apt install clang` (Ubuntu/Debian) or `sudo dnf install clang` (Fedora) |
| build-essential | `sudo apt install build-essential cmake` |
| Vulkan SDK | `sudo apt install libvulkan-dev vulkan-tools` |
| X11/Wayland libs | `sudo apt install libx11-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev` |
| Mono / .NET 6.0 | Required by UnrealBuildTool |

---

## 1. Clone the Repository

```bash
git clone https://github.com/juanmarques/ragna-th.git
cd ragna-th
```

---

## 2. Engine Setup

### Option A: Epic Games Launcher (Windows/macOS)

1. Install UE 5.3+ from the Epic Games Launcher
2. Right-click `RagnarokUE/RagnarokUE.uproject` → **Switch Unreal Engine version** → select 5.3+
3. Double-click the `.uproject` to open in the Editor

### Option B: Source Build (All Platforms)

```bash
# Clone UE5 (requires Epic Games GitHub access)
git clone https://github.com/EpicGames/UnrealEngine.git -b 5.3
cd UnrealEngine

# Setup (platform-specific)
# Windows:
Setup.bat && GenerateProjectFiles.bat
# macOS/Linux:
./Setup.sh && ./GenerateProjectFiles.sh

# Build
# Windows:
.\Engine\Build\BatchFiles\RunUAT.bat BuildGraph -Script=Engine/Build/Graph/Examples/BuildEditorAndTools.xml
# macOS/Linux:
./Engine/Build/BatchFiles/RunUAT.sh BuildGraph -Script=Engine/Build/Graph/Examples/BuildEditorAndTools.xml
```

Set the `UE_ROOT` environment variable to your engine install path:

```bash
# bash/zsh
export UE_ROOT="/path/to/UnrealEngine"

# PowerShell
$env:UE_ROOT = "C:\UnrealEngine"
```

---

## 3. Generate Project Files

### Windows
```powershell
cd ragna-th\RagnarokUE
"%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" GenerateProjectFiles -project="%CD%\RagnarokUE.uproject" -game
```
Then open the generated `RagnarokUE.sln` in Visual Studio 2022.

### macOS
```bash
cd ragna-th/RagnarokUE
"$UE_ROOT/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh" \
    -project="$(pwd)/RagnarokUE.uproject" -game
```
Then open the generated Xcode project/workspace.

### Linux
```bash
cd ragna-th/RagnarokUE
"$UE_ROOT/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" \
    -project="$(pwd)/RagnarokUE.uproject" -game
```
Then build from the command line (see below) or open with an IDE that supports Makefiles/CMake.

---

## 4. Build Targets

The project defines three build targets:

| Target | File | Type | Description |
|--------|------|------|-------------|
| `RagnarokUE` | `RagnarokUE.Target.cs` | Game (Client) | Player-facing game client |
| `RagnarokUEServer` | `RagnarokUEServer.Target.cs` | Server | Headless dedicated server |
| `RagnarokUEEditor` | `RagnarokUEEditor.Target.cs` | Editor | Unreal Editor with project loaded |

### Build Configurations

| Configuration | Use Case |
|--------------|----------|
| `Development` | Day-to-day development with debug symbols and logging |
| `DebugGame` | Full debugging — slow but maximum diagnostics |
| `Shipping` | Optimized release build, no debug/logging overhead |
| `Test` | Like Shipping but with console commands and stats enabled |

---

## 5. Command-Line Builds

All commands assume `UE_ROOT` is set and you are in the `ragna-th/RagnarokUE` directory.

### Windows

```powershell
# Client — Development
"%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    RagnarokUE Win64 Development ^
    -project="%CD%\RagnarokUE.uproject"

# Dedicated Server — Development
"%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    RagnarokUEServer Win64 Development ^
    -project="%CD%\RagnarokUE.uproject"

# Client — Shipping (release)
"%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
    RagnarokUE Win64 Shipping ^
    -project="%CD%\RagnarokUE.uproject"
```

### macOS

```bash
# Client — Development
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUE Mac Development \
    -project="$(pwd)/RagnarokUE.uproject"

# Dedicated Server — Development
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUEServer Mac Development \
    -project="$(pwd)/RagnarokUE.uproject"

# Client — Shipping (release)
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUE Mac Shipping \
    -project="$(pwd)/RagnarokUE.uproject"
```

### Linux

```bash
# Client — Development
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUE Linux Development \
    -project="$(pwd)/RagnarokUE.uproject"

# Dedicated Server — Development
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUEServer Linux Development \
    -project="$(pwd)/RagnarokUE.uproject"

# Dedicated Server — Shipping (production)
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUEServer Linux Shipping \
    -project="$(pwd)/RagnarokUE.uproject"
```

---

## 6. Packaging (Cooked Builds)

Use RunUAT (Unreal Automation Tool) to produce distributable packages.

### Client Package

```bash
# Windows
"%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%CD%\RagnarokUE.uproject" ^
    -platform=Win64 -clientconfig=Shipping ^
    -build -cook -stage -package -archive ^
    -archivedirectory="%CD%\Packaged\Client"

# macOS
"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$(pwd)/RagnarokUE.uproject" \
    -platform=Mac -clientconfig=Shipping \
    -build -cook -stage -package -archive \
    -archivedirectory="$(pwd)/Packaged/Client"

# Linux
"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$(pwd)/RagnarokUE.uproject" \
    -platform=Linux -clientconfig=Shipping \
    -build -cook -stage -package -archive \
    -archivedirectory="$(pwd)/Packaged/Client"
```

### Dedicated Server Package

```bash
# Linux (recommended for production servers)
"$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh" BuildCookRun \
    -project="$(pwd)/RagnarokUE.uproject" \
    -platform=Linux -serverconfig=Shipping \
    -server -noclient -build -cook -stage -package -archive \
    -archivedirectory="$(pwd)/Packaged/Server"

# Windows
"%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%CD%\RagnarokUE.uproject" ^
    -platform=Win64 -serverconfig=Shipping ^
    -server -noclient -build -cook -stage -package -archive ^
    -archivedirectory="%CD%\Packaged\Server"
```

---

## 7. Database Setup

The dedicated server requires PostgreSQL. The schema is defined in
`Source/RagnarokUEServer/DatabaseSubsystem/RODatabaseSchema.h`.

### Install PostgreSQL

```bash
# Ubuntu/Debian
sudo apt install postgresql postgresql-contrib

# Fedora/RHEL
sudo dnf install postgresql-server postgresql-contrib
sudo postgresql-setup --initdb

# macOS (Homebrew)
brew install postgresql@16
brew services start postgresql@16

# Windows — download from https://www.postgresql.org/download/windows/
```

### Create the Database

```bash
sudo -u postgres psql
```

```sql
CREATE USER ragnath WITH PASSWORD 'your_secure_password';
CREATE DATABASE ragnath_db OWNER ragnath;
GRANT ALL PRIVILEGES ON DATABASE ragnath_db TO ragnath;
\q
```

### Apply the Schema

The schema includes 12 tables: `accounts`, `characters`, `inventory`, `guilds`, `guild_members`, `storage`, `quests`, `friends`, `skills`, `castles`, `mail`, `anticheat_logs`.

Tables are created automatically on first server startup via `RODatabaseSubsystem`, or you can apply them manually by extracting the SQL from `RODatabaseSchema.h`.

---

## 8. Running

### Dedicated Server

```bash
# From packaged build
./RagnarokUEServer -log -port=7777

# From editor build (command line)
"$UE_ROOT/Engine/Binaries/<Platform>/UnrealEditor" \
    "$(pwd)/RagnarokUE.uproject" \
    /Game/Maps/Prontera/Prontera_Main \
    -server -log -port=7777
```

Server configuration in `Config/DefaultEngine.ini`:
- Default map: `Prontera_Main`
- Net tick rate: 60 Hz
- Max client rate: 15,000 bytes/s
- Replication Graph: enabled

### Client

```bash
# From packaged build
./RagnarokUE -windowed -resx=1280 -resy=720

# Connect to a server
./RagnarokUE <server_ip>:<port> -windowed
```

### Editor (Development)

Open `RagnarokUE.uproject` in the Unreal Editor. Use **Play In Editor** (PIE) with:
- **Net Mode**: Play As Listen Server (solo) or Play As Client (connect to a running server)
- **Number of Players**: Set > 1 for local multiplayer testing

---

## 9. Project Modules

| Module | Build File | Purpose |
|--------|-----------|---------|
| `RagnarokUE` | `RagnarokUE.Build.cs` | Main game module — character, combat, items, skills, UI, social, world systems |
| `RagnarokUEServer` | `RagnarokUEServer.Build.cs` | Server-only module — authentication, database (PostgreSQL), world management |

### Key Plugin Dependencies

| Plugin | Purpose |
|--------|---------|
| `GameplayAbilities` | GAS — all skills and abilities |
| `OnlineSubsystem` | Player sessions and authentication |
| `ReplicationGraph` | Optimized grid-based network replication |
| `NavigationSystem` | Click-to-move pathfinding |
| `AIModule` | Monster AI behavior trees |

---

## 10. Troubleshooting

### Build Errors

| Problem | Solution |
|---------|----------|
| `GameplayAbilities module not found` | Enable the plugin in `.uproject` or install the engine with GAS support |
| `ReplicationGraph not found` | Ensure UE 5.3+ — older versions bundle it differently |
| Linker errors on Linux | Install `libpq-dev` for PostgreSQL client libraries |
| Xcode build fails on macOS | Run `sudo xcode-select -s /Applications/Xcode.app` to set active toolchain |

### Runtime Issues

| Problem | Solution |
|---------|----------|
| Client can't connect to server | Verify port 7777 (UDP) is open; check firewall rules |
| Database connection failed | Confirm PostgreSQL is running and credentials match `RODatabaseSubsystem` config |
| Assets missing / black screen | Run a full cook: `BuildCookRun` with `-cook` flag |
| Low FPS in editor | Disable real-time rendering in viewports you're not using; check `r.DefaultFeature` settings in `DefaultEngine.ini` |

---

## Quick Reference

```bash
# Full development workflow (Linux example)

# 1. Clone
git clone https://github.com/juanmarques/ragna-th.git && cd ragna-th/RagnarokUE

# 2. Generate project files
"$UE_ROOT/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" \
    -project="$(pwd)/RagnarokUE.uproject" -game

# 3. Build client + server
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUE Linux Development -project="$(pwd)/RagnarokUE.uproject"
"$UE_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool" \
    RagnarokUEServer Linux Development -project="$(pwd)/RagnarokUE.uproject"

# 4. Set up database
sudo -u postgres createuser ragnath -P
sudo -u postgres createdb ragnath_db -O ragnath

# 5. Run server
./Binaries/Linux/RagnarokUEServer -server -log -port=7777

# 6. Run client
./Binaries/Linux/RagnarokUE 127.0.0.1:7777
```

---

*Targets: Windows (Win64), macOS (Mac/Metal), Linux (Vulkan). Engine: Unreal Engine 5.3+.*
