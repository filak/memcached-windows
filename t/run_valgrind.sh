#!/bin/bash

export VALGRIND_TEST="1"

# Prefix TCP tests with tcp_ and TLS tests with tls_
if [ -z "${SSL_TEST}" ] ; then
    export VALGRIND_LOG_PREFIX="${VALGRIND_LOG_PREFIX}tcp_"
else
    export VALGRIND_LOG_PREFIX="${VALGRIND_LOG_PREFIX}tls_"
fi

# testapp is fast so it's ok to use track-origins right away
VALGRIND_EXTRA_ARGS="${VALGRIND_EXTRA_ARGS} --track-origins=yes" ./testapp || true
# Let the caller of this script decide if parallel or not.
# Take note that with valgrind, a process consumes higher memory and CPU usage.
if [ -z "${PARALLEL}" ] ; then
    prove t || true
else
    prove -j "${PARALLEL}" t || true
fi

# For unknown reason yet, sometimes valgrind error log doesn't have 'ERROR SUMMARY:.'
# Include 'If you believe this happened as a result of' filter
VALGRIND_ERROR_SUMMARY=$(grep -e 'ERROR SUMMARY: ' -e 'If you believe this happened as a result of' "${VALGRIND_LOG_PREFIX}"* | grep -v -e 'ERROR SUMMARY: 0' -e error_summary -e track-origins_)
if [ -z "${VALGRIND_ERROR_SUMMARY}" ] ; then
    echo "Valgrind OK!"
else
    FAILING_TEST_CASES=""
    while IFS= read -r VALGRIND_LOG_WITH_PREFIX; do
        VALGRIND_LOG=${VALGRIND_LOG_WITH_PREFIX#"${VALGRIND_LOG_PREFIX}"}
        TEST_FILENAME=$(echo ${VALGRIND_LOG} | awk -F  "." '{printf("%s.%s",$1,$2)}')
        # testapp already use track-origins, no need to rerun
        if [[ "${TEST_FILENAME}" != *"testapp"* ]]; then
            FAILING_TEST_CASES="${FAILING_TEST_CASES} t/${TEST_FILENAME}"
        fi
    done <<< "${VALGRIND_ERROR_SUMMARY}"

    VALGRIND_ORIG_LOG_PREFIX="${VALGRIND_LOG_PREFIX}"
    if [ -n "${FAILING_TEST_CASES}" ] ; then
        # Remove duplicate test cases
        FAILING_TEST_CASES=$(echo "${FAILING_TEST_CASES}" | xargs -n1 | sort -u | xargs)

        # Test again failing cases with --track-origins=yes and track-origins_ log prefix
        echo "Testing again failing cases (${FAILING_TEST_CASES} ) with --track-origins=yes and track-origins_ log prefix..."

        export VALGRIND_LOG_PREFIX="${VALGRIND_LOG_PREFIX}track-origins_"
        export VALGRIND_EXTRA_ARGS="${VALGRIND_EXTRA_ARGS} --track-origins=yes"
        if [ -z "${PARALLEL}" ] ; then
            prove ${FAILING_TEST_CASES} || true
        else
            prove -j "${PARALLEL}" ${FAILING_TEST_CASES} || true
        fi
    fi

    VALGRIND_ERROR_SUMMARY=$(grep -e 'ERROR SUMMARY: ' -e 'If you believe this happened as a result of' "${VALGRIND_ORIG_LOG_PREFIX}"* | grep -v -e 'ERROR SUMMARY: 0' -e error_summary)
    echo "${VALGRIND_ERROR_SUMMARY}" | tee --append "${VALGRIND_ORIG_LOG_PREFIX}error_summary.log"

    echo "Valgrind NG!"
    exit 1
fi
