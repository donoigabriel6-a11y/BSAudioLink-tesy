# Quest port (Beat Saber 1.40.8)

The repository root contains the native Quest C++ project. The original PCVR C# project remains under `AudioLink/`.

## Build prerequisites

- Current QPM CLI (the `qpm` command)
- Android NDK
- CMake 3.22+
- Ninja
- PowerShell 7

## Build

Open PowerShell in the repository folder and run:

```powershell
qpm restore
pwsh -ExecutionPolicy Bypass -File ./scripts/build.ps1 -clean
qpm qmod build
pwsh -ExecutionPolicy Bypass -File ./scripts/createqmod.ps1 AudioLink -clean
```

The `-ExecutionPolicy Bypass` option applies only to that PowerShell process and avoids changing the machine-wide execution policy. The QPM dependency graph pins SongCore `^1.1.26` and Custom Types `^0.18.3`, which are compatible with each other. SongCore is resolved for headers/API integration but is not bundled into the generated QMOD.

The native library name is `libbs-audiolink.so` and the QPM mod ID is `bs-audiolink`. The Unity asset bundle is stored as base64 text so it can be committed through the repository API; the build script restores it before running CMake.

## SongCore capability

The native plugin registers the exact `AudioLink` capability in `late_load`, resolving the SongCore API symbol dynamically. This avoids a hard native link dependency and logs an error if SongCore's capability registration API cannot be found.

**Verification status:** the source has not yet been compiled or tested on a Quest device. Verify that the generated QMOD contains `libbs-audiolink.so`, then install it and check the Beat Saber log for `Registered SongCore capability: AudioLink`.
