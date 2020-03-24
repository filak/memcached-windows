#!/bin/sh

# Copyright 2017-2020 Viktor Szakats <https://vsz.me/>
# See LICENSE.md

cat /etc/*-release

dpkg --add-architecture i386
apt-get -qq -o=Dpkg::Use-Pty=0 update
# shellcheck disable=SC2086
apt-get -qq -o=Dpkg::Use-Pty=0 install \
  autoconf automake libtool \
  curl git gpg python3-pip make \
  libssl-dev \
  gcc-mingw-w64 ${_optpkg} \
  zip zstd time jq dos2unix \
  wine64 wine32 libmemcached-tools

cd mingw/build
./_build.sh
