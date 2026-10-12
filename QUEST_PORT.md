# Quest port (Beat Saber 1.40.8)

The repository root now contains the native Quest C++ project. The original PCVR C# plugin remains under `AudioLink/`.

## Build prerequisites

- QPM-RS
- Android NDK
- CMake 3.22+
- Ninja
- PowerShell 7

## Build

```powershell
qpm restore
pwsh ./scripts/build.ps1 -clean
qpm-rust qmod build
pwsh ./scripts/createqmod.ps1 AudioLink -clean
```

The native library name is `libbs-audiolink.so` and the QPM mod ID is `bs-audiolink`. The Unity asset bundle is stored as base64 text so it can be committed through the repository API; the build script restores the binary before running CMake.

## SongCore capability

The native plugin registers the exact `AudioLink` capability in `late_load`, resolving the SongCore API symbol dynamically. This avoids a hard native link dependency and logs an error if SongCore's capability registration API cannot be found.

**Verification status:** the source has been imported and wired up, but it has not yet been compiled or tested on a Quest device. The first native build and Beat Saber log must be checked before treating this port as working.
