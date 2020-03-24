#!/bin/sh -ex

export _NAM
export _VER
export _BAS
export _DST

_NAM="$(basename "$0")"
_NAM="$(echo "${_NAM}" | cut -f 1 -d '.')"
_VER="$1"
_cpu="$2"

MEMCACHED_IP=127.0.0.1
MEMCACHED_PORT=11211

send_mc_string() {
  echo "$1" | nc -q 1 ${MEMCACHED_IP} ${MEMCACHED_PORT}
}

run_crusher_test() {
  CRUSHER_ARCHIVE_NAME="mc-crusher-${MC_CRUSHER_VER_}"
  CRUSHER_DURATION=$1
  CRUSHER_DURATION_DEFAULT=30
  MEMCACHED_EXE="./memcached.exe"
  CRUSHER_EXE="./mc-crusher"
  CRUSHER_CONF_DIR=./conf

  if [ -z "${CRUSHER_DURATION}" ]; then
    CRUSHER_DURATION=${CRUSHER_DURATION_DEFAULT}
  fi

  # Run memcached server
  wine ${MEMCACHED_EXE} -p ${MEMCACHED_PORT} -A -v &

  # Build mc-crusher
  rm -rf "${CRUSHER_ARCHIVE_NAME}"
  unzip "../${CRUSHER_ARCHIVE_NAME}.zip"
  cd "${CRUSHER_ARCHIVE_NAME}"
  make

  echo "Running ${CRUSHER_EXE} with ${CRUSHER_CONF_DIR}/* for ${CRUSHER_DURATION} seconds each..."

  send_mc_string "stats settings"

  for CRUSHER_CONF in $CRUSHER_CONF_DIR/*
  do
    echo "${CRUSHER_EXE} --conf=${CRUSHER_CONF}..."
    send_mc_string "stats"
    ${CRUSHER_EXE} --conf=${CRUSHER_CONF} &
    CRUSHER_PID=$!
    sleep ${CRUSHER_DURATION}
    kill ${CRUSHER_PID} || true
    send_mc_string "stats"
    echo "${CRUSHER_EXE} --conf=${CRUSHER_CONF} done."
  done

  echo "Running ${CRUSHER_EXE} with ${CRUSHER_CONF_DIR}/* for ${CRUSHER_DURATION} seconds each done."

  # Shutdown memcached server
  send_mc_string "shutdown"

  cd ..
}

(
  rm -rf "${_NAM}"
  mkdir "${_NAM}"
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
  options="${options} --disable-extstore"
  options="${options} --disable-tls"
  options="${options} --disable-seccomp"
  options="${options} --disable-sasl"
  options="${options} --disable-sasl-pwdb"
  options="${options} --disable-coverage"
  options="${options} --disable-docs"
  export ac_cv_libevent_dir="$(realpath "$(dirname $0)/..")/libevent/pkg/usr/local"

  MEMCACHED_SRCDIR="$(realpath ../../../)"
  MEMCACHED_CURDIR="${PWD}"
  cd "${MEMCACHED_SRCDIR}"
  ./autogen.sh
  # memcached's version is generated using git describe (see version.sh) and the
  # value is unknown/empty/error on some environment so just replace to make sure.
  echo "m4_define([VERSION_NUMBER], [${_VER}])" > version.m4
  cd "${MEMCACHED_CURDIR}"
  "${MEMCACHED_SRCDIR}/configure" ${options}
  make -j 2
  _pkg='pkg/usr/local'
  mkdir -p "${_pkg}/bin"
  mkdir -p "${_pkg}/include"
  mkdir -p "${_pkg}/tests"
  cp -a *.exe "${_pkg}/bin"
  cp -a config.h "${_pkg}/include"

  ls -l ${_pkg}/bin/*.exe
  "${_CCPREFIX}strip" -p -s ${_pkg}/bin/memcached.exe

  # Make steps for determinism

  readonly _ref="$(realpath "../memcached-${_VER}.tar.gz")"

  ../_peclean.py "${_ref}" ${_pkg}/bin/*.exe

  ../_sign.sh "${_ref}" ${_pkg}/bin/*.exe

  touch -c -r "${_ref}" ${_pkg}/bin/*.exe
  touch -c -r "${_ref}" ${_pkg}/include/*.h

  # Tests

  "${_CCPREFIX}objdump" -x ${_pkg}/bin/*.exe | grep -E -i "(file format|dll name)"

  make test | tee "${_pkg}/tests/make_test.log"

  if [ -n "${CRUSHER_TEST}" ]; then
    run_crusher_test > "${_pkg}/tests/mc-crusher.log" 2>&1
  fi

  touch -c -r "${_ref}" ${_pkg}/tests/*.log

  # Create package

  _BAS="${_NAM}-${_VER}-win${_cpu}-mingw"
  _DST="$(mktemp -d)/${_BAS}"

  mkdir -p "${_DST}"

  cp -f -a ${_pkg}/bin      "${_DST}/"
  cp -f -a ${_pkg}/include  "${_DST}/"
  cp -f -a ${_pkg}/tests     "${_DST}/"

  unix2dos -q -k "${_DST}"/*.txt
  unix2dos -q -k "${_DST}"/tests/*.log

  ../_pack.sh "${_ref}"
  _NAM="${_NAM}-windows" ../_ul.sh
)
