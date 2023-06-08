# ThingSet node library in C

![build badge](https://github.com/ThingSet/thingset-node-c/actions/workflows/build.yml/badge.svg)

[ThingSet](https://thingset.io) is a transport-agnostic and self-explanatory remote API for embedded devices and humans.

This library implements the node side in C for **v0.6 of the specification**. It was re-implemented almost from scratch to overcome shortcomings of the previous [ThingSet device library](https://github.com/ThingSet/thingset-device-library).

The main goal is to provide a full-featured ThingSet module for Zephyr RTOS. The Zephyr environment is currently required, but the library can also be used in other embedded C environments with minor changes.

## Documentation

Full library API documentation: [thingset.io/thingset-node-c](http://thingset.io/thingset-node-c/)

See [github.com/ThingSet/thingset-zephyr-sdk](https://github.com/ThingSet/thingset-zephyr-sdk) for more examples how the library can be used.

## Features

Below tables show the implementation status of the different ThingSet functions:

| Request/Response | Text mode     | Binary mode (IDs) | Binary mode (names) |
| ---------------- | ------------- | ----------------- | ------------------- |
| GET              | yes           | yes               | yes                 |
| FETCH            | yes           | yes               | yes                 |
| UPDATE           | yes           | yes               | yes                 |
| EXEC             | yes           | yes               | yes                 |
| CREATE           | yes (subsets) | no                | yes (subsets)       |
| DELETE           | yes (subsets) | no                | yes (subsets)       |
| RESPONSE         | yes           | yes               | yes                 |

| Publish/Subscribe | Text mode | Binary mode (IDs) | Binary mode (names) |
| ----------------- | --------- | ----------------- | ------------------- |
| DESIRE            | no        | no                | no                  |
| REPORT            | yes       | yes               | no                  |

## Unit testing

Unit tests are written using the [Zephyr ztest environment](https://docs.zephyrproject.org/latest/develop/test/ztest.html). See [tests folder](tests) for details.

Test coverage report: [thingset.io/thingset-node-c/coverage](https://thingset.io/thingset-node-c/coverage)

## Contributors

The code in this library is partly based on the [ThingSet device library](https://github.com/ThingSet/thingset-device-library). See the git commit history for contributors to the library.

Special thanks:

- Bobby Noelte ([b0661](https://github.com/b0661)) for porting the original library to Zephyr and for several improvements regarding unit testing and coverage reports.
- Tom Owen ([towen](https://github.com/towen)) for the idea to use [Zephyr's iterable sections](https://docs.zephyrproject.org/latest/kernel/iterable_sections/index.html#) to define data objects.

## License

This software is released under the [Apache-2.0 License](LICENSE).

Imported projects in the [lib folder](lib) may have a different permissive license like MIT. See the original files for details.
