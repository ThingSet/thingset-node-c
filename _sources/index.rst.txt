===================================
ThingSet Node Library Documentation
===================================

`ThingSet <https://thingset.io>`_ is a transport-agnostic and self-explanatory remote API for
embedded devices and humans.

This library implements the node side in C for v0.6 of the specification. It was re-implemented
almost from scratch to overcome shortcomings of the previous
`ThingSet device library <https://github.com/ThingSet/thingset-device-library>`_.

The main goal is to provide a full-featured ThingSet module for `Zephyr RTOS`_. The Zephyr
environment is currently required, but the library can also be used in other embedded C
environments with minor changes.

This library uses the very lightweight `JSMN parser <https://github.com/zserge/jsmn>`_ for JSON
data and the `zcbor <https://github.com/zephyrproject-rtos/zcbor>`_ library that comes with Zephyr
for CBOR data.

This documentation is licensed under the Creative Commons Attribution-ShareAlike 4.0 International
(CC BY-SA 4.0) License.

.. image:: static/images/cc-by-sa-centered.png

The full license text is available at `<https://creativecommons.org/licenses/by-sa/4.0/>`_.

.. _Zephyr RTOS: https://zephyrproject.org
.. _ThingSet: https://thingset.io

.. toctree::
    :caption: Overview
    :hidden:

    src/features

.. toctree::
    :caption: Development
    :hidden:

    src/dev/unit_tests

.. toctree::
    :caption: API Reference
    :hidden:

    src/api/library
    src/api/internal
