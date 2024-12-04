FROM ubuntu:18.04
ENV LANG C.UTF-8

ARG JOBS
ARG CTNG_VERSION=1.26.0
#ARG DEBIAN_FRONTEND=noninteractive

# Non-interactive configuration of tzdata
ENV DEBIAN_FRONTEND noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN true
RUN { echo 'tzdata tzdata/Areas select Etc'; echo 'tzdata tzdata/Zones/Etc select UTC'; } | debconf-set-selections

# Create unprivileged user to build toolchains and plugins
RUN groupadd -g 1000 build
RUN useradd --create-home --uid 1000 --gid 1000 --shell /bin/bash build

# and install it
RUN apt-get update 
RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
# Options added from <https://askubuntu.com/questions/1129188/how-can-i-fix-ppa-failed-to-fetch-errors-while-running-apt-get-update-on-ubuntu>.
RUN apt-get update --option Acquire::ForceIPv4=true --option Acquire::Retries=100 --option Acquire::http::Timeout=60
RUN apt-get install -y gcc-11 g++-11 git ninja-build flex

# Then install the prerequistes for a juce build
RUN apt-get install -y git vim 
RUN apt-get install -y cmake ninja-build 
RUN apt-get install -y \
	devscripts \
	libx11-dev \
	libxcb-cursor-dev \
	libxcb-keysyms1-dev \
	libxcb-util-dev \
	libxkbcommon-dev \
	libxkbcommon-x11-dev \
	xcb \
	libgtk-3-dev \
	libwebkit2gtk-4.0 \
	libwebkit2gtk-4.0-dev \
	libcurl4-openssl-dev \
	alsa \
	alsa-tools \
	libasound2-dev \
	libjack-dev \
	libfreetype6-dev \
	libxinerama-dev \
	libxcb-xinerama0 \
	libxinerama1 \
	x11proto-xinerama-dev \
	libxrandr-dev \
	libgl1-mesa-dev \
	libxcursor-dev \
	libxcursor1 \
	libxcb-cursor-dev \
	libxcb-cursor0

# cmake is not needed for sfzq
# RUN mkdir /home/build/cmake-install
# WORKDIR /home/build/cmake-install
# RUN apt-get install -y wget
# RUN wget https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.sh
# RUN chmod 755 ./cmake-3.28.1-linux-x86_64.sh
# RUN ./cmake-3.28.1-linux-x86_64.sh --skip-license --prefix=/usr

RUN apt-get install -y rpm

# Finally, pick gcc11
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11

# Switch back go the user space user and build the toolchain
USER build
# RUN mkdir /home/build/dev
# WORKDIR /home/build/dev
WORKDIR /home/build

