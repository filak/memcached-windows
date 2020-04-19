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

alias curl='curl -fsSR --connect-timeout 15 -m 20 --retry 3'
alias gpg='gpg --batch --keyserver-options timeout=15 --keyid-format LONG'

(
  BUILD_SCRIPT_DIR="${PWD}"

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

  # TLS
  if [ -n "${TLS_BORINGSSL}" ]; then
    OPENSSL_DIR="${BUILD_SCRIPT_DIR}/boringssl/pkg/usr/local"
  else
    OPENSSL_DIR="${BUILD_SCRIPT_DIR}/openssl-${OPENSSL_VER_}-win${_cpu}-mingw"
  fi

  options=''
  options="${options} --host=${_TRIPLET}"
  options="${options} --enable-extstore"
  options="${options} --enable-tls"
  options="${options} --disable-seccomp"
  options="${options} --disable-sasl"
  options="${options} --disable-sasl-pwdb"
  options="${options} --disable-docs"
  if [ -z "${CODECOV_ENABLE}" ]; then
    options="${options} --disable-coverage"
  fi
  export ac_cv_c_alignment=none
  export ac_cv_libevent_dir="${BUILD_SCRIPT_DIR}/libevent/pkg/usr/local"
  export ac_cv_libssl_dir="${OPENSSL_DIR}"

  MEMCACHED_SRCDIR="$(realpath ../../../)"
  MEMCACHED_CURDIR="${PWD}"
  _pkg="${MEMCACHED_CURDIR}/pkg/usr/local"
  readonly _ref="${MEMCACHED_CURDIR}/VERSION"

  # Change build directory
  cd "${MEMCACHED_SRCDIR}"
  ./autogen.sh
  make distclean || true
  # memcached's version is generated using git describe (see version.sh) and the
  # value is unknown/empty/error on some environment so just replace to make sure.
  echo "m4_define([VERSION_NUMBER], [${_VER}])" > version.m4
  ./configure ${options}

  # Build for coverity before actual build
  if [ -n "${COVERITY_SCAN}" ]; then
    export PATH=${PATH}:"${BUILD_SCRIPT_DIR}/cov-analysis/bin"
    "${BUILD_SCRIPT_DIR}/coverity.sh"
  fi

  make -j 2
  mkdir -p "${_pkg}/bin"
  mkdir -p "${_pkg}/include"
  cp -a *.exe "${_pkg}/bin"
  cp -a config.h "${_pkg}/include"

  ls -l ${_pkg}/bin/*.exe
  "${_CCPREFIX}strip" -p -s ${_pkg}/bin/memcached.exe

  # Make steps for determinism

  echo "${_VER}" > "${_ref}"
  touch -c -t "${MEMCACHED_DATE_VER_}" "${_ref}"

  "${BUILD_SCRIPT_DIR}/_peclean.py" "${_ref}" ${_pkg}/bin/*.exe

  "${BUILD_SCRIPT_DIR}/_sign.sh" "${_ref}" ${_pkg}/bin/*.exe

  touch -c -r "${_ref}" ${_pkg}/bin/*.exe
  touch -c -r "${_ref}" ${_pkg}/include/*.h

  # Tests

  "${_CCPREFIX}objdump" -x ${_pkg}/bin/*.exe | grep -E -i "(file format|dll name)"

  make test
  if [ -n "${TLS_TEST_FULL}" ]; then
    make test_tls
  elif [ -n "${TLS_TEST_BASIC}" ]; then
    make test_basic_tls
  fi

  if [ -n "${CODECOV_ENABLE}" ]; then
    bash "${BUILD_SCRIPT_DIR}/codecov.sh" -x "${_CCPREFIX}gcov"
  fi

  if [ -n "${CRUSHER_TEST}" ]; then
    run_crusher_test > "${_pkg}/tests/mc-crusher.log" 2>&1
  fi

  # Change to original dir
  cd "${MEMCACHED_CURDIR}"

  # Create package

  _BAS="${_NAM}-${_VER}-win${_cpu}-mingw"
  _DST="$(mktemp -d)/${_BAS}"

  mkdir -p "${_DST}"

  cp -f -a ${_pkg}/bin      "${_DST}/"
  cp -f -a ${_pkg}/include  "${_DST}/"

  unix2dos -q -k "${_DST}"/*.txt

  "${BUILD_SCRIPT_DIR}/_pack.sh" "${PWD}/${_ref}"
  _NAM="${_NAM}-windows" "${BUILD_SCRIPT_DIR}/_ul.sh"
)
