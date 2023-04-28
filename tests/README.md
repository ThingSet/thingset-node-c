# ThingSet unit tests

## Tests

### Protocol

Tests protocol functions like request/response and statements (in binary and text mode).

## Run unit tests

With twister:

    ../zephyr/scripts/twister -T ./tests --integration -v

Manually:

    west build -b native_posix tests/protocol -t run

## Test Coverage

First build the test using above manual command and go into the build folder.

    cd build

Extract coverage information (excluding files from Zephyr):

    lcov --capture --directory ./modules/thingset-node-c --output-file lcov.info --rc lcov_branch_coverage=1 --exclude '*zephyr*'

Generate HTML output (`--header-title` requires lcov v1.16)

    genhtml lcov.info --output-directory lcov_html --ignore-errors source --branch-coverage --highlight --legend --header-title "Coverage Report for ThingSet node library"

The HTML output file can be found under `lcov_html/index.html`.

    firefox lcov_html/index.html &
