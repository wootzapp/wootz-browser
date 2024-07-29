# <img src="https://www.wootzapp.com/image/logo2.png" alt="Logo" width="200" height="150"> WootzApp

## WootzApp

Check out the official website: https://www.wootzapp.com/

Please join our [discord](https://discord.gg/n9dqrRzJ8V)

### System requirements for Build
A 64-bit Intel machine running Linux with at least 8GB of RAM. More than 32GB is highly recommended.
At least 100GB of free disk space.
You must have Git and Python installed already.
Most development is done on Ubuntu. Other distros may or may not work; see the Linux instructions for some suggestions.

Building the Android client on Windows or Mac is not supported and doesn't work.
### Warning:
Developers should **NOT** work in an existing Chromium directory to avoid potential conflicts.


### Installation Steps (building chromium):
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
Create a chromium directory for the checkout and change to it (you can call this whatever you like and put it wherever you like, as long as the full path has no spaces):
```bash
mkdir ~/chromium && cd ~/chromium
fetch --nohooks --no-history android
gclient sync --nohooks --revision src@refs/tags/127.0.6489.0 --reset --upstream -D --force --no-history
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

#### Build Options:

##### Option 1: Generate the default Chromium APK
Run following command in ```chromium/src``` directory 
```
gn args out/Default
```

and add following arguments:


```bash
target_os = "android"
target_cpu = "arm64"
```
##### Option 2: Generate an optimized APK (approximately 131 MB)

Run following command in ```chromium/src``` directory 
```
gn args out/Default
```
 and add following arguments:


```bash
target_os = "android"
target_cpu = "arm"
is_official_build = true
is_debug = false
symbol_level = 0
enable_nacl = false
proprietary_codecs = true
ffmpeg_branding = "Chrome"
remove_webcore_debug_symbols = true
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
git clone --depth 1 https://github.com/wootzapp/wootz-browser.git
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
