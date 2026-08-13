# Android Nobs Hello World

A minimal native Android C project built directly with Nobs, the Android NDK
LLVM toolchain, and Android SDK command-line tools. No Gradle, Java, Kotlin, or
CMake is involved.

The project produces two Android arm64 artifacts from the same `core` unit:

- `android_binary::binary`: a regular ELF executable with `main()`; it prints
  `Hello World` to standard output.
- `anrdoid_package::package`: an APK containing a NativeActivity shared
  library; it renders `HELLO WORLD` over a coffee-black background and also
  prints `Hello World` to logcat under the `NobsHelloWorld` tag.

The `anrdoid_debug`, `anrdoid_release`, and `anrdoid_package` spellings are
intentional and retained for compatibility with the original project.

## Requirements

- Windows and PowerShell (the commands below were tested on Windows).
- A built `nobs.exe`.
- A JDK. The standalone SDK manager needs Java, and `keytool` is used to create
  the debug signing key.
- Android SDK Platform 36.
- Android SDK Build Tools 36.0.0 (Nobs requires 35.0.0 or newer for 16 KiB APK
  alignment).
- Android SDK Platform Tools (`adb`).
- Android NDK 29.0.14206865, whose LLVM is used by the example registry entry.
- An `arm64-v8a` Android device running API 21 or newer.

Set the path to your Nobs executable once per PowerShell session:

```powershell
$nobs = 'C:\path\to\nobs.exe'
```

## Set up the Android toolchain

There are two supported setups. Use the first one when Android Studio already
manages your SDK. Use the second one for a standalone command-line SDK.

### Option A: existing Android Studio SDK and `--find-compilers`

In Android Studio's SDK Manager, install these packages:

- Android SDK Platform 36
- Android SDK Build Tools 36.0.0
- Android SDK Platform Tools
- NDK (Side by side) 29.0.14206865

The default Windows SDK root is usually
`$env:LOCALAPPDATA\Android\Sdk`. Point `ANDROID_HOME` at the actual SDK root and
let Nobs discover the SDK tools:

```powershell
$env:ANDROID_HOME = "$env:LOCALAPPDATA\Android\Sdk"
& $nobs --find-compilers
```

`--find-compilers` detects the Android SDK from `ANDROID_HOME`,
`ANDROID_SDK_ROOT`, or the standard Android Studio location. On Windows, Nobs
does not automatically search Android NDK installations because they have no
universal install root. Add the NDK LLVM toolchain manually to the existing
`tools` block in:

```text
%APPDATA%\nobs\.nobs_registry.txt
```

Use the real SDK path for your account:

```make
tools {
  android_arm64_llvm {
    tool_type = "llvm-clang"
    toolchain_path = R"(C:\Users\<username>\AppData\Local\Android\Sdk\ndk\29.0.14206865\toolchains\llvm\prebuilt\windows-x86_64)"
    TOOLCHAIN_TRIPLET = "aarch64-linux-android21"
    toolchain_min_version = "21"
  }
}
```

If the registry already contains `tools { ... }`, add only the
`android_arm64_llvm { ... }` entry inside it; do not create a second `tools`
block.

### Option B: standalone SDK without Android Studio

Install a JDK, then download the
[Android SDK Command-Line Tools](https://developer.android.com/tools). Create
`C:\Android\cmdline-tools\latest` and place the archive's `bin`, `lib`,
`NOTICE.txt`, and `source.properties` contents inside `latest`, so that
`sdkmanager.bat` has this layout:

```text
C:\Android\cmdline-tools\latest\bin\sdkmanager.bat
```

Install the packages needed by this project and accept their licenses:

```powershell
$sdkRoot = 'C:\Android'
$sdkmanager = "$sdkRoot\cmdline-tools\latest\bin\sdkmanager.bat"

& $sdkmanager --sdk_root=$sdkRoot `
    'platform-tools' `
    'platforms;android-36' `
    'build-tools;36.0.0' `
    'ndk;29.0.14206865'

& $sdkmanager --sdk_root=$sdkRoot --licenses
```

The package syntax and `--sdk_root` option are documented in the official
[`sdkmanager` guide](https://developer.android.com/tools/sdkmanager).

You may now set `ANDROID_HOME` and let Nobs discover the SDK packager:

```powershell
$env:ANDROID_HOME = 'C:\Android'
& $nobs --find-compilers
```

Then add the `android_arm64_llvm` entry shown below. Alternatively, register
both the SDK packager and NDK compiler manually by merging these entries into
the existing `%APPDATA%\nobs\.nobs_registry.txt` `tools` block:

```make
tools {
  android_sdk_36 {
    tool_type = "android-sdk"
    android_sdk_path = R"(C:\Android)"
    android_build_tools_version = "36.0.0"
  }

  android_arm64_llvm {
    tool_type = "llvm-clang"
    toolchain_path = R"(C:\Android\ndk\29.0.14206865\toolchains\llvm\prebuilt\windows-x86_64)"
    TOOLCHAIN_TRIPLET = "aarch64-linux-android21"
    toolchain_min_version = "21"
  }
}
```

Again, merge the entries into an existing `tools` block instead of creating a
duplicate block.

Confirm that Nobs can see the project configuration:

```powershell
& $nobs -u
& $nobs -p
```

Expected units include `android_binary`, `anrdoid_package`, and `core`.
Expected profiles are `anrdoid_debug` and `anrdoid_release`.

## Create the debug signing key

The debug profile asks the Nobs Android SDK builder to sign and verify the APK.
The private key is intentionally excluded by `.gitignore`, so create it once
after cloning the repository:

```powershell
& keytool -genkeypair `
    -keystore '.\debug.keystore' `
    -storepass android `
    -keypass android `
    -alias androiddebugkey `
    -keyalg RSA `
    -keysize 2048 `
    -validity 10000 `
    -dname 'CN=Android Debug,O=Android,C=US' `
    -noprompt
```

Set the signing passwords in the environment before a debug APK build:

```powershell
$env:NOBS_DEBUG_STORE_PASSWORD = 'android'
$env:NOBS_DEBUG_KEY_PASSWORD = 'android'
```

Nobs passes only the environment variable names to `apksigner`; password
values are not written into the Nobs context or build cache. This key is for
local development only and must not be used to publish an application. The
release profile stays unsigned until a real release signing configuration is
provided.

## Build

Run all commands from the repository root.

### Debug

```powershell
& $nobs android_binary::binary anrdoid_debug
& $nobs anrdoid_package::package anrdoid_debug
```

Artifacts:

```text
out\aarch64-unknown-linux-android21\anrdoid_debug\android_binary\binary\hello_world
out\android-36\anrdoid_debug\anrdoid_package\package\hello_world.apk
```

The APK is already aligned, signed, and verified by Nobs.

### Release

```powershell
& $nobs android_binary::binary anrdoid_release
& $nobs anrdoid_package::package anrdoid_release
```

Artifacts:

```text
out\aarch64-unknown-linux-android21\anrdoid_release\android_binary\binary\hello_world
out\android-36\anrdoid_release\anrdoid_package\package\hello_world.apk
```

The release executable is ready to run. The release APK is aligned but
unsigned and must be signed with a release key before installation or
distribution.

## Deploy to a device

Enable Developer options and USB debugging on the Android device, connect it
over USB, unlock it, and accept the RSA authorization prompt. `adb` is part of
the Android SDK Platform Tools package; see the official
[ADB documentation](https://developer.android.com/tools/adb) for connection
and troubleshooting details.

Set its path and confirm that the device state is `device`:

```powershell
$adb = 'C:\Android\platform-tools\adb.exe'
& $adb devices -l
```

If Android Studio installed the SDK in its default location, use:

```powershell
$adb = "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe"
```

### Install and run the APK

Use the signed debug APK:

```powershell
$apk = Join-Path $PWD 'out\android-36\anrdoid_debug\anrdoid_package\package\hello_world.apk'

& $adb install -r $apk
& $adb shell am start -n 'com.example.nobshelloworld/android.app.NativeActivity'
```

The NativeActivity displays `HELLO WORLD` over a coffee-black background. It
also writes the message to logcat:

```powershell
& $adb logcat -d -s 'NobsHelloWorld:I' '*:S'
```

To follow the tag continuously, omit `-d` and stop the command with `Ctrl+C`:

```powershell
& $adb logcat -s 'NobsHelloWorld:I' '*:S'
```

Remove the installed app with:

```powershell
& $adb uninstall com.example.nobshelloworld
```

### Push and run the standalone executable

First verify that the device supports the artifact's `arm64-v8a` ABI:

```powershell
& $adb shell getprop ro.product.cpu.abilist
```

Push the executable to the device's temporary directory, make it executable,
and run it:

```powershell
$binary = Join-Path $PWD 'out\aarch64-unknown-linux-android21\anrdoid_debug\android_binary\binary\hello_world'

& $adb push $binary /data/local/tmp/hello_world
& $adb shell chmod 755 /data/local/tmp/hello_world
& $adb shell /data/local/tmp/hello_world
```

Expected output:

```text
Hello World
```

Clean up the uploaded executable when finished:

```powershell
& $adb shell rm /data/local/tmp/hello_world
```

## Project structure

```text
.
|-- .nobs_project.txt
|-- android_binary/       # main() entry point
|-- anrdoid_package/      # NativeActivity, manifest, and APK target
`-- core/                 # shared header and PIC static library
```
