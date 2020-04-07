#!/bin/sh -ex

export _NAM
export _VER
export _BAS
export _DST

_NAM="$(basename "$0")"
_NAM="$(echo "${_NAM}" | cut -f 1 -d '.')"
_VER="$1"
_cpu="$2"

(
  cd "${_NAM}" || exit

  # Cross-tasks

  # Detect host OS
  case "$(uname)" in
    *_NT*)   os='win';;
    Linux*)  os='linux';;
    Darwin*) os='mac';;
    *BSD)    os='bsd';;
  esac

  # Build

  rm -fr pkg CMakeFiles CMakeCache.txt cmake_install.cmake

  find . -name '*.o'   -type f -delete
  find . -name '*.obj' -type f -delete
  find . -name '*.a'   -type f -delete
  find . -name '*.lo'  -type f -delete
  find . -name '*.la'  -type f -delete
  find . -name '*.lai' -type f -delete
  find . -name '*.Plo' -type f -delete
  find . -name '*.pc'  -type f -delete

  _CFLAGS="-m${_cpu} -fno-ident -DNDEBUG"
  [ "${_cpu}" = '32' ] && _CFLAGS="${_CFLAGS} -fno-asynchronous-unwind-tables"

  options=''
  options="${options} -DCMAKE_SYSTEM_PROCESSOR=${_ARCH}"
  options="${options} -DCMAKE_SYSTEM_NAME=Windows"
  options="${options} -DCMAKE_BUILD_TYPE=Release"
  options="${options} -DCMAKE_C_COMPILER=${_CCPREFIX}gcc"
  options="${options} -DCMAKE_CXX_COMPILER=${_CCPREFIX}g++"
  options="${options} -DCMAKE_INSTALL_PREFIX=/usr/local"

  unset CC
  cmake . ${options} \
    "-DCMAKE_C_FLAGS=-static-libgcc ${_CFLAGS}" \
    "-DCMAKE_CXX_FLAGS=-static-libgcc -static-libstdc++ ${_CFLAGS}"

  make -j 2 crypto
  make -j 2 ssl
  DESTDIR="${PWD}/pkg/usr/local"
  mkdir -p "${DESTDIR}/lib"
  cp -a include "${DESTDIR}"
  cp -a crypto/libcrypto.a "${DESTDIR}/lib"
  cp -a ssl/libssl.a "${DESTDIR}/lib"

  _pkg="${DESTDIR}"

  # Make steps for determinism

  # BoringSSL version to be used is the %Y%m%d%H%M date format of the
  # https://github.com/google/boringssl/tree/chromium-stable's latest commit
  readonly _ref='VERSION'
  echo "chromium-stable-${_VER}" > "${_ref}"
  touch -c -t "${_VER}" "${_ref}"

  "${_CCPREFIX}strip" -p --enable-deterministic-archives -g ${_pkg}/lib/*.a

  touch -c -r "${_ref}" ${_pkg}/include/openssl/*.h
  touch -c -r "${_ref}" ${_pkg}/lib/*.a

  # Create package

  _BAS="${_NAM}-${_VER}-win${_cpu}-mingw"
  _DST="$(mktemp -d)/${_BAS}"

  mkdir -p "${_DST}/include/openssl"
  mkdir -p "${_DST}/lib"

  cp -a ${_pkg}/include     "${_DST}"
  cp -a ${_pkg}/lib         "${_DST}"
  cp -a VERSION             "${_DST}"

  unix2dos -q -k "${_DST}"/LICENSE
  unix2dos -q -k "${_DST}"/VERSION

  ../_pack.sh "$(pwd)/${_ref}"
  _NAM="${_NAM}-windows" ../_ul.sh
)
