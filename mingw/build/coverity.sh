#!/bin/sh

# Standalone script to build current dir with coverity and upload automatically
# the result for analysis.
# Notes before calling this script:
#   cov-analysis/bin must be in PATH
#   current dir must be ready for make or already configured
#   COMPILER_TYPE defaults to gcc but not the COMPILER_CMD which must be passed
#   COVERITY_TOKEN env must be set correctly
#   COVERITY_PROJECT env must be set correctly
#   SRC_VERSION defaults to current date in %Y-%m-%d_%H.%M format unless set
#   SRC_DESC defaults to current date in %Y-%m-%d_%H.%M format unless set

COMPILER_CMD=$1
COMPILER_TYPE=$2
COVERITY_DIR="cov-int"
COVERITY_TGZ="${COVERITY_DIR}.tar.gz"

print_usage() {
  echo "$0 COMPILER_CMD COMPILER_TYPE(default:gcc)"
  echo "Example: $0 x86_64-w64-mingw32-gcc gcc"
}

if [ -z "${COMPILER_CMD}" ]; then
  print_usage
  exit 1
fi

CUR_DATE="$(date +%Y-%m-%d_%H.%M)"
if [ -z "${SRC_VERSION}" ]; then
  echo "SRC_VERSION env not set. Defaulting to ${CUR_DATE}."
  SRC_VERSION="${CUR_DATE}"
fi
if [ -z "${SRC_DESC}" ]; then
  echo "SRC_DESC env not set. Defaulting to ${CUR_DATE}."
  SRC_DESC="${CUR_DATE}"
fi

# Clean the build
make clean
rm -rf "${COVERITY_DIR}"*

# Configure coverity
cov-configure --template --compiler "${COMPILER_CMD}" --comptype gcc

# Build with coverity
cov-build --dir "${COVERITY_DIR}" make

# Create the coverity tarball
tar -czf "${COVERITY_TGZ}" "${COVERITY_DIR}"

# Upload for analysis
curl --form token="${COVERITY_TOKEN}" \
--form email="${COVERITY_MAIL}" \
--form file=@"${COVERITY_TGZ}" \
--form version="${SRC_VERSION}" \
--form description="${SRC_DESC}" \
https://scan.coverity.com/builds?project="${COVERITY_PROJECT}"

# Clean the build
make clean
