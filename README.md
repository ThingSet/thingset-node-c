# ThingSet node library in C

[ThingSet](https://thingset.io) is a transport-agnostic and self-explanatory remote API for embedded devices and humans.

This library implements the node side in C. It was re-implemented almost from scratch to overcome shortcomings of the previous [ThingSet device library](https://github.com/ThingSet/thingset-device-library).

The main purpose is to provide a module for Zephyr RTOS, but the library can also be used in other embedded C environments.

## Unit testing

Unit tests are written using the [Zephyr ztest environment](https://docs.zephyrproject.org/latest/develop/test/ztest.html). See [tests folder](tests) for details.

Test coverage report: [thingset.io/thingset-node-c/coverage](https://thingset.io/thingset-node-c/coverage)

## Contributors

The code in this library is partly based on the [ThingSet device library](https://github.com/ThingSet/thingset-device-library). See commit history for contributors.

## License

This software is released under the [Apache-2.0 License](LICENSE).
