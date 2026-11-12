Unit Tests
==========

The library contains an extensive set of unit tests to validate that the specification is
implemented correctly.

The unit tests are written using the
`Zephyr Ztest environment <https://docs.zephyrproject.org/latest/develop/test/ztest.html>`_.

Build and run the tests with the following command from the root directory of the library:

.. code-block:: bash

    ../zephyr/scripts/twister -T ./tests --integration -v

See also `tests folder <https://github.com/ThingSet/thingset-node-c/tree/main/tests>`_ for
further information regarding the unit tests.
