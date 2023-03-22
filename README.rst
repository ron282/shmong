===============================================================================
ShmoNG - Shmoose Next Generattion - A XMPP Client for Sailfish OS
===============================================================================

Fork of `Shmoose <https://github.com/geobra/harbour-shmoose>`_ to be build on `qxmpp <https://github.com/qxmpp-project/qxmpp/>`_ instead of `Swiften <https://swift.im/swiften.html>`_.

Very early alpha stage.

-------------------------------------------------------------------------------
Install SFOS development tools and SDK
-------------------------------------------------------------------------------

Install regular Sailfish SDK: sfdk 
(see https://docs.sailfishos.org/Tools/Sailfish_SDK/Installation/)

Set a common target output directory in your dev directory::

* mkdir ~/dev/RPMS
* sfdk config --global output-prefix=/home/user1/dev/RPMS

List installed SDKs::

* sfdk tools list

Select a SDK::

* sfdk config --global target=SailfishOS-4.4.0.58-armv7hl

-------------------------------------------------------------------------------
Install build dependencies on SFOS
-------------------------------------------------------------------------------

Build qca::

* cd ~/dev
* git clone htpp://github.com/ron282/qca
* cd qca
* sfdk build

Build libomemo-c::

* cd ~/dev
* git clone http://github.com/ron282/libomemo-c
* cd libomemo-c
* sfdk build

Build Qxmpp::

* cd ~/dev
* git clone https://github.com/ron282/qxmpp
* cd qxmpp
* git checkout qxmpp-project-1.5
* sfdk build

-------------------------------------------------------------------------------
Build ShmoNG
-------------------------------------------------------------------------------

on SFOS::

* cd ~/dev
* git clone https://github.com/ron282/shmong 
* cd shmong
* sfdk build 

-------------------------------------------------------------------------------
Installing on host
-------------------------------------------------------------------------------

Install a device from QtCreator 

Check device is installed

* sfdk device list

Configuration device

* sfdk config device="Xperia10"

Deploy packages to device

* cd libomemo-c
* sfdk deploy --sdk
* cd ../qxmpp
* sfdk deploy --sdk
* cd ../shmong
* sfdk deploy --sdk
