#!/bin/bash

set -eu

OPENMEAT_ROOT="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"
WORKDIR=$(mktemp -d)

trap 'rm -rf ${WORKDIR}' EXIT

cd "${WORKDIR}"

cmake -DBUILD_TESTS=ON "${OPENMEAT_ROOT}"
make "-j$(nproc)"

ctest -V --test-dir tests/
