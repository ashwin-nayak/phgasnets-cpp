# Dockerfile for development environment
# Primary purpose of this file is for use with VSCode DevContainer Extension.
#
# To build a docker image named "phgasnets-dev" from this file run the commands,
#
# docker build . --tag phgasnets-dev
#

FROM debian:bookworm

ARG USERNAME=user
ARG USER_UID=1001
ARG USER_GID=${USER_UID}
ARG HIGHFIVE_PATH=/opt/highfive

RUN addgroup --gid ${USER_GID} ${USERNAME} \
    && adduser --disabled-password --gecos '' --uid ${USER_UID} --gid ${USER_GID} ${USERNAME} \
    && usermod --uid ${USER_UID} --gid ${USER_GID} ${USERNAME} && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata

RUN apt-get update && apt-get install -y \
    build-essential cmake git \
    libeigen3-dev libceres-dev nlohmann-json3-dev libhdf5-dev catch2 doxygen && \
    git clone --recursive https://github.com/BlueBrain/HighFive.git ${HIGHFIVE_PATH}-src && \
    cd ${HIGHFIVE_PATH}-src && \
    git checkout --quiet v2.9.0 && \
    cmake -B build -S . -DHIGHFIVE_USE_EIGEN=ON -DCMAKE_INSTALL_PREFIX=${HIGHFIVE_PATH} -DHIGHFIVE_USE_BOOST=OFF && \
    cmake --build build --parallel && \
    cmake --install build && \
    rm -rf ${HIGHFIVE_PATH}-src && \
    apt-get install -y python3 python3-pip python3-sphinx python3-numpy python3-scipy python3-matplotlib fonts-lmodern fontconfig \
        python3-h5py python3-tqdm python3-xarray && \
    mkdir -p /usr/local/share/fonts/otf && cd /usr/local/share/fonts/otf && \
    ln -s /usr/share/texmf/fonts/opentype/public/lm-math/latinmodern-math.otf && fc-cache

USER ${USERNAME}

ENV SHELL /bin/bash