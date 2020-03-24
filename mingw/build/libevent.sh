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

  rm -fr pkg

  find . -name '*.o'   -type f -delete
  find . -name '*.obj' -type f -delete
  find . -name '*.a'   -type f -delete
  find . -name '*.lo'  -type f -delete
  find . -name '*.la'  -type f -delete
  find . -name '*.lai' -type f -delete
  find . -name '*.Plo' -type f -delete
  find . -name '*.pc'  -type f -delete

  options=''
  options="${options} --host=${_TRIPLET}"
  options="${options} --enable-shared=no"
  options="${options} --enable-static=yes"
  options="${options} --disable-debug-mode"
  options="${options} --disable-libevent-regress"
  options="${options} --disable-samples"
  options="${options} --disable-openssl"

  ./configure ${options}
  make -j 2 install "DESTDIR=$(pwd)/pkg"
  _pkg='pkg/usr/local'

  ls -l ${_pkg}/lib/*.a

  # Make steps for determinism

  readonly _ref='ChangeLog'

  "${_CCPREFIX}strip" -p --enable-deterministic-archives -g ${_pkg}/lib/*.a
  touch -c -r "${_ref}" ${_pkg}/include/*.h
  touch -c -r "${_ref}" ${_pkg}/include/event2/*.h
  touch -c -r "${_ref}" ${_pkg}/lib/*.a

  # Tests


  # Create package

  _BAS="${_NAM}-${_VER}-win${_cpu}-mingw"
  _DST="$(mktemp -d)/${_BAS}"

  mkdir -p "${_DST}"

  cp -f -a ${_pkg}/include          "${_DST}/"
  cp -f -a ${_pkg}/lib              "${_DST}/"
  cp -f -a ChangeLog                "${_DST}/ChangeLog.txt"

  unix2dos -q -k "${_DST}"/*.txt

  ../_pack.sh "$(pwd)/${_ref}"
  _NAM="${_NAM}-windows" ../_ul.sh
)
