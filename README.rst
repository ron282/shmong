===============================================================================
ShmoNG - Shmoose Next Generattion - A XMPP Client for Sailfish OS
===============================================================================

Fork of `Shmoose <https://github.com/geobra/harbour-shmoose>`_ to be build on `qxmpp <https://github.com/qxmpp-project/qxmpp/>`_ instead of `Swiften <https://swift.im/swiften.html>`_.

Very early alpha stage.


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
 git checkout 1.5
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
 sfdk deploy --sdk  "-*-devel"
 cd ../qca
 sfdk deploy --sdk  "-*-devel"
 cd ../qxmpp
 sfdk deploy --sdk  "-*-devel"
 cd ../shmong
 sfdk deploy --sdk  "-*-devel"

To recompile for another target using the same directories
Remove the build directory for each library and object files for app::

cd libomemo-c
rm -rf build
cd ../qca
rm -rf build
cd ../qxmpp
rm -rf build
cd ../shmong
rm *.o

Restart from select a SDK::
