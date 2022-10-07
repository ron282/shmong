===============================================================================
ShmoNG - Shmoose Next Generattion - A XMPP Client for Sailfish OS
===============================================================================

Fork of `Shmoose <https://github.com/geobra/harbour-shmoose>`_ to be build on `qxmpp <https://github.com/qxmpp-project/qxmpp/>`_ instead of `Swiften <https://swift.im/swiften.html>`_.

Very early alpha stage.

-------------------------------------------------------------------------------
Install build dependencies on SFOS
-------------------------------------------------------------------------------

Ssh into mersdk::

* mkdir xmpp && cd xmpp

Build Qca::

* wget https://github.com/KDE/qca/archive/refs/tags/v2.2.1.tar.gz
* tar -xvf ...
* cd qca-2.2.1
* mkdir build_arm && cd build_arm
* set TESTS to off in CMakeLists.txt
* sb2 -t SailfishOS-4.2.0.21-aarch64 cmake ..
* sb2 -t SailfishOS-4.2.0.21-aarch64 make
* cd ..

Build libomemo-c::

* https://github.com/dino/libomemo-c
* cd libomemo-c
* add 'set(CMAKE_POSITION_INDEPENDENT_CODE ON)' to CMakeLists.txt
* mkdir build_arm && cd build_arm
* sb2 -t SailfishOS-4.2.0.21-aarch64 cmake ..
* sb2 -t SailfishOS-4.2.0.21-aarch64 make
* cd ..

Build Qxmpp::

* git clone https://github.com/geobra/qxmpp-sfos
* cd qxmpp-sfos
* patch -p1 < sfos.diff
* mkdir build_arm && cd build_arm
* sb2 -t SailfishOS-4.2.0.21-aarch64 cmake -DBUILD_EXAMPLES=false -DBUILD_TESTS=false -DBUILD_OMEMO=true ..
* sb2 -t SailfishOS-4.2.0.21-aarch64 make

-------------------------------------------------------------------------------
Install build dependencies on host
-------------------------------------------------------------------------------

create and build in each of the dependencies packets a build dir and do the same build steps::

* mkdir build && cd build
* cmake ..
* make


-------------------------------------------------------------------------------
Build ShmoNG
-------------------------------------------------------------------------------

on SFOS::

* mb2 -t SailfishOS-4.2.0.21-armv7hl build

on Host::

* qmake
* make

