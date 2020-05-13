#!/bin/sh -x

export MEMCACHED_VER_='1.6.6'
export MEMCACHED_DATE_VER_='' # Dynamically resolved based on MEMCACHED_VER_
export MC_CRUSHER_VER_='master'
export LIBEVENT_VER_='' # Dynamically resolved based on latest release
export OPENSSL_VER_='' # Dynamically resolved based on latest release
export OSSLSIGNCODE_VER_='1.7.1'
export OSSLSIGNCODE_HASH=f9a8cdb38b9c309326764ebc937cba1523a3a751a7ab05df3ecc99d18ae466c9

# Create revision string
# NOTE: Set _REV to empty after bumping CURL_VER_, and
#       set it to 1 then increment by 1 each time bumping a dependency
#       version or pushing a CI rebuild for the master branch.
export _REV=''

[ -z "${_REV}" ] || _REV="_${_REV}"

echo "Build: REV(${_REV})"

# Quit if any of the lines fail
set -e

# Detect host OS
case "$(uname)" in
  *_NT*)   os='win';;
  Linux*)  os='linux';;
  Darwin*) os='mac';;
  *BSD)    os='bsd';;
esac

# Install required component
# TODO: add `--progress-bar off` when pip 10.0.0 is available
if [ "${os}" != 'win' ]; then
  pip3 --version
  pip3 --disable-pip-version-check install --user pefile
fi

alias curl='curl -fsSR --connect-timeout 15 -m 20 --retry 3'
alias wget='wget -nv --timeout=15 --tries=3 --https-only'

gpg_recv_key() {
  # https://keys.openpgp.org/about/api
  req="pks/lookup?op=get&options=mr&exact=on&search=0x$1"
# curl "https://keys.openpgp.org/${req}"     | gpg --import --status-fd 1 || \
  curl "https://pgpkeys.eu/${req}"           | gpg --import --status-fd 1 || \
  curl "https://keyserver.ubuntu.com/${req}" | gpg --import --status-fd 1
}

if [ "${_BRANCH#*dev*}" != "${_BRANCH}" ]; then
  _patsuf='.dev'
elif [ "${_BRANCH#*master*}" = "${_BRANCH}" ]; then
  _patsuf='.test'
else
  _patsuf=''
fi

# libevent
LIBEVENT_LATEST_URL=$(curl -Ls -o /dev/null -w %{url_effective} https://github.com/libevent/libevent/releases/latest)
LIBEVENT_VER_=$(basename "${LIBEVENT_LATEST_URL}" | sed -e "s/^release-//")
curl -o pack.bin -L --proto-redir =https "https://github.com/libevent/libevent/releases/download/release-${LIBEVENT_VER_}/libevent-${LIBEVENT_VER_}.tar.gz" || exit 1
openssl dgst -sha256 pack.bin | grep -q "${LIBEVENT_HASH}" || exit 1
tar -xvf pack.bin >/dev/null 2>&1 || exit 1
rm pack.bin
rm -f -r libevent && mv libevent-* libevent
[ -f "libevent${_patsuf}.patch" ] && dos2unix < "libevent${_patsuf}.patch" | patch --batch -N -p1 -d libevent

# MinGW-w64 binaries from https://bintray.com/vszakats/generic/openssl are
# recommended by OpenSSL (see https://wiki.openssl.org/index.php/Binaries)
dl_openssl_bin() {
  OPENSSL_CPU=$1
  OPENSSL_ARCHIVE_NAME="openssl-${OPENSSL_VER_}-win${OPENSSL_CPU}-mingw"
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

# BoringSSL version to be used is the %Y%m%d%H%M date format of the
# https://github.com/google/boringssl/tree/chromium-stable's latest commit
if [ -n "${TLS_BORINGSSL}" ]; then
    rm -rf boringssl
    git clone --branch chromium-stable --depth=1 https://github.com/google/boringssl.git
    [ -f "boringssl${_patsuf}.patch" ] && dos2unix < "boringssl${_patsuf}.patch" | patch --batch -N -p1 -d boringssl
    cd boringssl
    export BORINGSSL_VER_="$(git log --date=format:'%Y%m%d%H%M' -1 | sed '3q;d' | awk -F ' ' '{print $2}')"
    cd ..
else
  # OpenSSL
  OPENSSL_LATEST_URL=$(curl -Ls -o /dev/null -w %{url_effective} https://bintray.com/vszakats/generic/openssl/_latestVersion)
  OPENSSL_VER_=$(basename "${OPENSSL_LATEST_URL}")
  if [ -n "$CPU" ]; then
    dl_openssl_bin "${CPU}"
  else
    dl_openssl_bin 64
    dl_openssl_bin 32
  fi
fi

# Official memcached to be used in timestamping since it has no Changelog that can be used as reference
UPSTREAM_DIR="memcached-${MEMCACHED_VER_}"
rm -rf "${UPSTREAM_DIR}"
git clone --branch ${MEMCACHED_VER_} --depth=1 https://github.com/memcached/memcached.git "${UPSTREAM_DIR}"
cd "${UPSTREAM_DIR}"
export MEMCACHED_DATE_VER_="$(git log --date=format:'%Y%m%d%H%M' -1 | sed '3q;d' | awk -F ' ' '{print $2}')"
cd ..
echo "memcached upstream version: ${MEMCACHED_VER_} date: ${MEMCACHED_DATE_VER_}"

# Download mc-crusher if enabled
if [ -n "${CRUSHER_TEST}" ]; then
  curl -o mc-crusher-${MC_CRUSHER_VER_}.zip -L --proto-redir =https "https://github.com/memcached/mc-crusher/archive/${MC_CRUSHER_VER_}.zip" || exit 1
fi

# Download Coverity Scan Self-Build Tool
if [ -n "${COVERITY_SCAN}" ]; then
  wget https://scan.coverity.com/download/linux64 --post-data "token=${COVERITY_TOKEN}&project=${COVERITY_PROJECT}" -O coverity_tool.tgz
  tar -xvf coverity_tool.tgz >/dev/null 2>&1 || exit 1
  rm coverity_tool.tgz
  rm -f -r cov-analysis && mv cov-analysis-* cov-analysis
fi

# Download coverage uploader
if [ -z "${CODECOV_DISABLE}" ]; then
  curl -o codecov.sh -L --proto-redir =https "https://codecov.io/bash" || exit 1
fi

# osslsigncode
# NOTE: "https://github.com/mtrojnar/osslsigncode/archive/${OSSLSIGNCODE_VER_}.tar.gz"
curl -o pack.bin -L --proto-redir =https "https://deb.debian.org/debian/pool/main/o/osslsigncode/osslsigncode_${OSSLSIGNCODE_VER_}.orig.tar.gz" || exit 1
openssl dgst -sha256 pack.bin | grep -q "${OSSLSIGNCODE_HASH}" || exit 1
tar -xvf pack.bin >/dev/null 2>&1 || exit 1
rm pack.bin
rm -f -r osslsigncode && mv osslsigncode-${OSSLSIGNCODE_VER_} osslsigncode
[ -f 'osslsigncode.patch' ] && dos2unix < 'osslsigncode.patch' | patch --batch -N -p1 -d osslsigncode

set +e

rm -f pack.bin pack.sig
