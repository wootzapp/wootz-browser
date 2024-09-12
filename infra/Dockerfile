ARG PLATFORM=linux/amd64
FROM --platform=$PLATFORM ubuntu:24.04
ENV PIP_NO_CACHE_DIR=1
RUN apt-get update
RUN apt-get install --yes --no-install-recommends \
    locales-all apt-utils software-properties-common \
    curl wget git lsb-release python3 sudo ccache \
    htop build-essential net-tools iputils-ping dnsutils rsync \
    procps vim zip unzip openssh-client make \
    gnupg2 less telnet jq gettext-base gperf locales fd-find \
    ripgrep bash ca-certificates file
# copied from install-build-deps.sh --android
RUN apt-get install --yes --no-install-recommends \
    binutils binutils-aarch64-linux-gnu \
    binutils-arm-linux-gnueabihf binutils-mips64el-linux-gnuabi64 \
    binutils-mipsel-linux-gnu bison bzip2 cdbs curl dbus-x11 devscripts \
    dpkg-dev elfutils fakeroot flex git-core gperf \
    lib32gcc-s1 lib32stdc++6 lib32z1 libasound2-dev \
    libatk1.0-0 libatspi2.0-0 libatspi2.0-dev libbluetooth-dev libbrlapi-dev \
    libbrlapi0.8 libbz2-1.0 libbz2-dev libc6 libc6-dev libc6-i386 libcairo2 \
    libcairo2-dev libcap-dev libcap2 libcgi-session-perl libcups2 libcups2-dev \
    libcurl4-gnutls-dev libdrm-dev libdrm2 libegl1 libelf-dev libevdev-dev \
    libevdev2 libexpat1 libffi-dev libffi8 libfontconfig1 libfreetype6 \
    libfuse2 libgbm-dev libgbm1 libgl1 libglib2.0-0 libglib2.0-dev \
    libglu1-mesa-dev libgtk-3-0 libgtk-3-dev libinput-dev libinput10 \
    libjpeg-dev libkrb5-dev libncurses6 libnspr4 libnspr4-dev libnss3 \
    libnss3-dev libpam0g libpam0g-dev libpango-1.0-0 libpangocairo-1.0-0 \
    libpci-dev libpci3 libpcre3 libpixman-1-0 libpulse-dev \
    libpulse0 libsctp-dev libspeechd-dev libspeechd2 libsqlite3-0 libsqlite3-dev \
    libssl-dev libstdc++6 libsystemd-dev libudev-dev libudev1 libuuid1 libva-dev \
    libvulkan-dev libvulkan1 libwayland-egl1 libwayland-egl1-mesa libwww-perl \
    libx11-6 libx11-xcb1 libxau6 libxcb1 libxcomposite1 libxcursor1 libxdamage1 \
    libxdmcp6 libxext6 libxfixes3 libxi6 libxinerama1 libxkbcommon-dev libxrandr2 \
    libxrender1 libxshmfence-dev libxslt1-dev libxss-dev libxt-dev libxtst-dev \
    libxtst6 lighttpd locales mesa-common-dev openbox p7zip patch perl pkg-config \
    rpm ruby subversion uuid-dev wdiff x11-utils x11-xserver-utils xcompmgr \
    xserver-xorg-core xserver-xorg-video-dummy xvfb xz-utils zip zlib1g zstd

ARG UID=1000
ARG GID=1000
RUN groupmod -g ${GID} staff || groupadd -g ${GID} staff
RUN useradd -m -u ${UID} -g ${GID} build

RUN usermod -aG sudo build
RUN echo "build ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER build

RUN sudo git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git /depot_tools
ENV PATH=$PATH:/depot_tools
COPY src/build/install-build-deps.sh /tmp/install-build-deps.sh
COPY src/build/install-build-deps.py /tmp/install-build-deps.py
RUN sudo chmod +x /tmp/*.sh
RUN /tmp/install-build-deps.sh --android

ENTRYPOINT [ "/bin/bash" , "-c"]
CMD ["tail -f /dev/null"]

# build and run preserving dir rights
# docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t wootz .                                                                < 1m
# docker run --user $(id -u):$(id -g) -v $(pwd):/wootz --platform linux/amd64 -it --rm wootz bash

