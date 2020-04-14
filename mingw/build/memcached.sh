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

gpg_recv_key() {
  # https://keys.openpgp.org/about/api
  req="pks/lookup?op=get&options=mr&exact=on&search=0x$1"
# curl "https://keys.openpgp.org/${req}"     | gpg --import --status-fd 1 || \
  curl "https://pgpkeys.eu/${req}"           | gpg --import --status-fd 1 || \
  curl "https://keyserver.ubuntu.com/${req}" | gpg --import --status-fd 1
}

# MinGW-w64 binaries from https://bintray.com/vszakats/generic/openssl are
# recommended by OpenSSL (see https://wiki.openssl.org/index.php/Binaries)
dl_openssl_bin() {
  OPENSSL_ARCHIVE_NAME="openssl-${OPENSSL_VER_}-win${_cpu}-mingw"
  OPENSSL_ARCHIVE_FILE="${OPENSSL_ARCHIVE_NAME}.tar.xz"
  OPENSSL_ARCHIVE_SIG="${OPENSSL_ARCHIVE_FILE}.asc"
  # Download tarball
  curl -o "${OPENSSL_ARCHIVE_FILE}" -L --proto-redir =https "https://bintray.com/vszakats/generic/download_file?file_path=${OPENSSL_ARCHIVE_FILE}" || exit 1
  # Download the signature
  curl -o "${OPENSSL_ARCHIVE_SIG}" -L --proto-redir =https "https://bintray.com/vszakats/generic/download_file?file_path=${OPENSSL_ARCHIVE_SIG}" || exit 1
  # Verify with Bintray key
  GPG_PK="379CE192D401AB61"
  gpg_recv_key ${GPG_PK}
  gpg --verify-options show-primary-uid-only --verify "${OPENSSL_ARCHIVE_SIG}" "${OPENSSL_ARCHIVE_FILE}" || exit 1
  # Extract tarball
  rm -rf "${OPENSSL_ARCHIVE_NAME}"
  tar -xvf "${OPENSSL_ARCHIVE_FILE}" >/dev/null 2>&1 || exit 1
}

coverity_scan() {
  COVERITY_DIR="cov-int"
  COVERITY_TGZ="${COVERITY_DIR}.tar.gz"
  CUR_DATE="$(date +%Y%m%d%H%M)"
  COVERITY_VER="${MEMCACHED_VER_}_${CUR_DATE}_win${_cpu}-mingw"
  COVERITY_PATH="$(realpath "$(dirname $0)/..")/cov-analysis/bin"

  export PATH=$PATH:${COVERITY_PATH}
  cov-configure --template --compiler ${_TRIPLET}-gcc --comptype gcc

  # Clean the build
  make clean

  rm -rf "${COVERITY_DIR}"
  cov-build --dir "${COVERITY_DIR}" make

  # Create the coverity tarball
  tar -czf "${COVERITY_TGZ}" "${COVERITY_DIR}"

  curl --form token="${COVERITY_TOKEN}" \
    --form email="${COVERITY_MAIL}" \
    --form file=@"${COVERITY_TGZ}" \
    --form version="${COVERITY_VER}" \
    --form description="${COVERITY_VER}" \
    https://scan.coverity.com/builds?project="${COVERITY_PROJECT}"

  # Clean the build
  make clean
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

  if [ -n "${TLS_BORINGSSL}" ]; then
    export ac_cv_libssl_dir="$(realpath "$(dirname $0)/..")/boringssl/pkg/usr/local"
  else
    # Download the pre-built OpenSSL binaries
    dl_openssl_bin
    export ac_cv_libssl_dir="${PWD}/${OPENSSL_ARCHIVE_NAME}"
  fi

  options=''
  options="${options} --host=${_TRIPLET}"
  options="${options} --enable-extstore"
  options="${options} --enable-tls"
  options="${options} --disable-seccomp"
  options="${options} --disable-sasl"
  options="${options} --disable-sasl-pwdb"
  options="${options} --disable-coverage"
  options="${options} --disable-docs"
  export ac_cv_c_alignment=none
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

  # Build for coverity before actual build
  if [ -n "${COVERITY_SCAN}" ]; then
    coverity_scan
  fi

  make -j 2
  _pkg='pkg/usr/local'
  mkdir -p "${_pkg}/bin"
  mkdir -p "${_pkg}/include"
  cp -a *.exe "${_pkg}/bin"
  cp -a config.h "${_pkg}/include"

  ls -l ${_pkg}/bin/*.exe
  "${_CCPREFIX}strip" -p -s ${_pkg}/bin/memcached.exe

  # Make steps for determinism

  readonly _ref='VERSION'
  echo "${_VER}" > "${_ref}"
  touch -c -t "${MEMCACHED_DATE_VER_}" "${_ref}"

  ../_peclean.py "${_ref}" ${_pkg}/bin/*.exe

  ../_sign.sh "${_ref}" ${_pkg}/bin/*.exe

  touch -c -r "${_ref}" ${_pkg}/bin/*.exe
  touch -c -r "${_ref}" ${_pkg}/include/*.h

  # Tests

  "${_CCPREFIX}objdump" -x ${_pkg}/bin/*.exe | grep -E -i "(file format|dll name)"

  make test
  if [ -n "${TLS_TEST_FULL}" ]; then
    make test_tls
  elif [ -n "${TLS_TEST}" ]; then
    make test_basic_tls
  fi

  if [ -n "${CRUSHER_TEST}" ]; then
    run_crusher_test > "${_pkg}/tests/mc-crusher.log" 2>&1
  fi

  # Create package

  _BAS="${_NAM}-${_VER}-win${_cpu}-mingw"
  _DST="$(mktemp -d)/${_BAS}"

  mkdir -p "${_DST}"

  cp -f -a ${_pkg}/bin      "${_DST}/"
  cp -f -a ${_pkg}/include  "${_DST}/"

  unix2dos -q -k "${_DST}"/*.txt

  ../_pack.sh "${PWD}/${_ref}"
  _NAM="${_NAM}-windows" ../_ul.sh
)
