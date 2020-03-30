#!/bin/sh

# Copyright 2017-2020 Viktor Szakats <https://vsz.me/>
# See LICENSE.md

cat /etc/*-release

dpkg --add-architecture i386
apt-get -qq -o=Dpkg::Use-Pty=0 update
# shellcheck disable=SC2086
apt-get -qq -o=Dpkg::Use-Pty=0 install -y \
  autoconf automake libtool \
  curl git gpg python3-pip make cmake \
  libssl-dev libio-socket-ssl-perl \
  gcc-mingw-w64 g++-mingw-w64 \
  zip zstd time jq dos2unix \
  wine64 wine32 perl nasm golang

cd mingw/build
./_build.sh
