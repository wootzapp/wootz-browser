# <img src="https://www.wootzapp.com/image/logo2.png" alt="Logo" width="200" height="150"> WootzApp

## WootzApp

Check out the official website: https://www.wootzapp.com/

### System requirements
A 64-bit Intel machine running Linux with at least 8GB of RAM. More than 16GB is highly recommended.
At least 100GB of free disk space.
You must have Git and Python installed already.
Most development is done on Ubuntu. Other distros may or may not work; see the Linux instructions for some suggestions.

Building the Android client on Windows or Mac is not supported and doesn't work.

### Installation Steps (building chromium):
<hr/>
First build the [chromium browser](https://chromium.googlesource.com/chromium/src/+/main/docs/android_build_instructions.md)
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
Create a chromium directory for the checkout and change to it (you can call this whatever you like and put it wherever you like, as long as the full path has no spaces):
```bash
mkdir ~/chromium && cd ~/chromium
fetch --nohooks android
```
When fetch completes, it will have created a hidden .gclient file and a directory called src in the working directory. The remaining instructions assume you have switched to the src directory:
```
cd src
```

#### Install additional build dependencies
Once you have checked out the code, run
```
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
#### Build Chromium
Build Chromium with Ninja using the command:
```
autoninja -C out/Default chrome_public_apk
```
### Installation Steps (building wootzapp):
change path to root directory or parent directory of your chromium build.
```
cd ..
git clone --depth 1 https://github.com/Aankirz/wootz-browser.git
```
Final Step
```
sudo chmod -R u+w chromium/src/
cp -r wootz-browser/src/* chromium/src/

cd ~/chromium/src
autoninja -C out/Default chrome_public_apk
```

Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory
Structure](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).
