# Quest port (Beat Saber 1.40.8)

The repository root contains the native Quest C++ project. The original PCVR C# project remains under `AudioLink/`.

## Build prerequisites

- Current QPM CLI (the `qpm` command)
- Android NDK
- CMake 3.22+
- Ninja
- PowerShell 7
- Python 3

## Build

Open PowerShell in the repository folder and run:

```powershell
qpm restore
pwsh -ExecutionPolicy Bypass -File ./scripts/build.ps1 -clean
pwsh -ExecutionPolicy Bypass -File ./scripts/createqmod.ps1 AudioLink -clean
```

The `-ExecutionPolicy Bypass` option applies only to that PowerShell process and avoids changing the machine-wide execution policy. The native code resolves SongCore's capability-registration symbol dynamically, avoiding a direct native link to SongCore. The current `qpm.json` pins the Beat Saber 1.40.8 bindings (`bs-cordl` 4008.0.0), `libil2cpp` 0.4.0 headers, beatsaber-hook 6.4.2, Custom Types 0.18.4, and Lapiz 0.2.23. Keep UI additions compatible with this dependency set; older QuestUI/BSML packages resolve incompatible Custom Types and hook versions.

The native library name is `libbs-audiolink.so` and the QPM mod ID is `bs-audiolink`. The root `mod.json` is used by the PowerShell packager; the legacy `qpm qmod build` command is not needed. The Unity asset bundle is stored as base64 text so it can be committed through the repository API; the build pipeline decodes it before compiling.

## GitHub Actions build

Run **Build Quest QMOD** from the Actions tab. **Quest source ref** defaults to
`quest/1.40.8-port`; change it to another Quest branch, tag, or commit if
needed. This is separate from the branch containing the workflow file (usually
`master`). Enter a version (for example, `1.2.3`) in **Mod version** to override
`mod.json` for this build only; two-part versions such as `1.0` are normalized
to `1.0.0`. Leave it blank to use the version from the Quest source ref. The
workflow summary displays the workflow ref, Quest source ref and commit,
packaged version, and tracked source tree before compilation. A successful run
uploads a version-named QMOD artifact. **Publish GitHub Release** defaults to
off; when enabled, the workflow creates the versioned `v<mod version>` tag and
release from the exact Quest source commit it built. It will not overwrite an
existing release.

## In-game settings

Open Beat Saber **Settings > AudioLink** to configure the mod. Test plane is
off by default because creating Unity primitives can be unstable on some Quest
setups. Gain, bass, treble, and fade length can also be adjusted there; each
change is saved immediately and applied to the active AudioLink instance.

## SongCore capability

The native plugin registers the exact `AudioLink` capability in `late_load`,
resolving the SongCore API symbol dynamically. This avoids a hard native link
dependency and logs an error if SongCore's capability registration API cannot
be found. Runtime settings are stored in the Scotland2 mod configuration.

**Verification status:** compilation and QMOD assembly can be verified on the
build runner, but the settings page and live audio behavior still need
verification on a Quest device. Check the Beat Saber log for
`Registered SongCore capability: AudioLink` after installing the QMOD.
