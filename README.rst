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


-------------------------------------------------------------------------------
Alternate way to compile using sfdk command line 
-------------------------------------------------------------------------------

Install regular Sailfish SDK: sfdk 
(see https://docs.sailfishos.org/Tools/Sailfish_SDK/Installation/)

Set a common target output directory in your dev directory::

 mkdir ~/dev/RPMS
 sfdk config --global output-prefix=/home/user1/dev/RPMS

List installed SDKs::

 sfdk tools list

Select a SDK::

 sfdk config --global target=SailfishOS-4.4.0.58-armv7hl

Build qca::

 cd ~/dev
 git clone htpp://github.com/ron282/qca
 cd qca
 sfdk build
 cd ..

Build libomemo-c::

 git clone http://github.com/ron282/libomemo-c
 cd libomemo-c
 sfdk build
 cd ..

Build QXmpp::

 git clone https://github.com/ron282/qxmpp
 cd qxmpp
 git checkout qxmpp-project-1.5
 sfdk build
 cd ..

Build Shmong::

 git clone https://github.com/ron282/shmong 
 cd shmong
 sfdk build 
 cd ..

Testing on a device
Check device is installed (install it from QtCreator)::

 sfdk device list

Configuration device::
 sfdk config device="Xperia10"

Deploy packages to device::

 cd libomemo-c
 sfdk deploy --sdk
 cd qca
 sfdk deploy --sdk
 cd ../qxmpp
 sfdk deploy --sdk
 cd ../shmong
 sfdk deploy --sdk

Deploy packages to device::

 cd libomemo-c
 sfdk deploy --sdk  "-*-devel"
 cd ../qca
 sfdk deploy --sdk  "-*-devel"
 cd ../qxmpp
 sfdk deploy --sdk  "-*-devel"
 cd ../shmong
 sfdk deploy --sdk  "-*-devel"

Deploy packages to the device::

 cd libomemo-c
 sfdk deploy --sdk "-*-devel"
 cd ../qca
 sfdk deploy --sdk "-*-devel"
 cd ../qxmpp
 sfdk deploy --sdk "-*-devel"
 cd ../shmong
 sfdk deploy --sdk "-*-devel"
