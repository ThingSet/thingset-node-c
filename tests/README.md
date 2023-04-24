# ThingSet unit tests

## Tests

### Protocol

Tests protocol functions like request/response and statements (in binary and text mode).

## Run unit tests

With twister:

../zephyr/scripts/twister -T ./tests --integration -v

Manually:

west build -b native_posix tests/protocol -t run
