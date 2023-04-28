# ThingSet node library in C

**IMPORTANT:** This library is still **work-in-progress** and **NOT useable**, yet. It is published already to keep everyone interested updated.

[ThingSet](https://thingset.io) is a transport-agnostic and self-explanatory remote API for embedded devices and humans.

This library implements the node side in C for v0.6 of the specification. It was re-implemented almost from scratch to overcome shortcomings of the previous [ThingSet device library](https://github.com/ThingSet/thingset-device-library).

The main goal is to provide a full-featured ThingSet module for Zephyr RTOS. The Zephyr environment is currently required, but the library can also be used in other embedded C environments with minor changes.

## Features

Below tables show the implementation status of the different ThingSet functions:

| Request/Response | Text mode | Binary mode |
| ---------------- | --------- | ----------- |
| GET              | no        | no          |
| FETCH            | no        | no          |
| UPDATE           | no        | no          |
| EXEC             | no        | no          |
| CREATE           | no        | no          |
| DELETE           | no        | no          |
| RESPONSE         | yes       | yes         |

| Publish/Subscribe | Text mode | Binary mode |
| ----------------- | --------- | ----------- |
| DESIRE            | no        | no          |
| REPORT            | no        | no          |

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
