*****************************************************
Functional Tests for IEC 61850 south plugin
*****************************************************

Use 'Google Unit Test' framework, for checking requirements during the
tests.

Install with:
::
    sudo apt-get install libgtest-dev libgmock-dev

To build and run the functional tests:
::
    mkdir build
    cd build
    cmake -DFUNCTIONAL_TESTS=on ..
    make
    make iec61850_functional_tests
