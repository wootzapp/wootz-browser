# <img src="https://www.wootzapp.com/image/logo2.png" alt="Logo" width="200" height="150"> WootzApp

## WootzApp

Check out the official website: https://www.wootzapp.com/

### Installation Steps:
<hr/>

#### Install depot_tools
Clone the depot_tools repository:
```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```

Add depot_tools to the end of your PATH (you will probably want to put this in your ~/.bashrc or ~/.zshrc). Assuming you cloned depot_tools to /path/to/depot_tools:
```bash
export PATH="$PATH:/path/to/depot_tools"
```
#### Get the code
Clone the github repo:
```bash
git clone --depth 1 https://github.com/wootzapp/wootz-browser.git
```
Then execute these steps:
```bash
cd wootz-browser
fetch --nohooks --no-history android
```

#### Install additional build dependencies
Now run:
```bash
cd src
build/install-build-deps.sh --android
```

Once you've run install-build-deps at least once, you can now run the Chromium-specific hooks, which will download additional binaries and other things you might need:
#### Run Hooks
```bash
gclient runhooks
```

#### Setting up the build
```bash
gn args out/Default
```
Edit the args.gn file to contain the following arguments:
```bash
target_os = "android"
target_cpu = "arm64"
```
#### Build WootzApp
Apply the patch file
```bash
cd ..
mv wootzapp_changes.patch ./src
cd src
git apply wootzapp_changes.patch
```
Build WootzApp with Ninja using the command (in src):
```bash
autoninja -C out/Default chrome_public_apk
```

And deploy it to your Android device:
```bash
out/Default/bin/chrome_public_apk install
```






Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory
Structure](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).
