===============================================================================
ShmoNG - Shmoose Next Generattion - A XMPP Client for Sailfish OS
===============================================================================

Fork of `Shmoose <https://github.com/geobra/harbour-shmoose>`_ to be build on `qxmpp <https://github.com/qxmpp-project/qxmpp/>`_ instead of `Swiften <https://swift.im/swiften.html>`_.

Very early alpha stage.

-------------------------------------------------------------------------------
Install SFOS development tools and SDK
-------------------------------------------------------------------------------

Install regular Sailfish SDK: sfdk
* See https://docs.sailfishos.org/Tools/Sailfish_SDK/Installation/

Set a common target output directory in your dev directory
* sfdk config --global output-prefix=/home/user1/dev/RPMS

List installed SDKs
* sfdk tools list
SailfishOS-4.4.0.58                          sdk-provided,latest
├── SailfishOS-4.4.0.58-aarch64              sdk-provided,latest
│   └── SailfishOS-4.4.0.58-aarch64.default  snapshot
├── SailfishOS-4.4.0.58-armv7hl              sdk-provided,latest
│   └── SailfishOS-4.4.0.58-armv7hl.default  snapshot
└── SailfishOS-4.4.0.58-i486                 sdk-provided,latest

Select a SDK
* sfdk config --global target=SailfishOS-4.4.0.58-armv7hl

-------------------------------------------------------------------------------
Install build dependencies on SFOS
-------------------------------------------------------------------------------

From your dev directory
* cd ~/dev

Build Qca::

* git clone htpp://github.com/ron282/qca
* cd qca
* sfdk build

Build libomemo-c::

* git clone http://github.com/geobra/libomemo-c
* cd libomemo-c
* sfdk build

Build Qxmpp::

* git clone https://github.com/ron282/qxmpp-sfos qxmpp-project-1.5
* cd qxmpp
* sfdk build

-------------------------------------------------------------------------------
Build ShmoNG
-------------------------------------------------------------------------------

on SFOS::

* git clone https://github.com/ron282/shmong 
* cd shmong
* sfdk build 

-------------------------------------------------------------------------------
Installing on host
-------------------------------------------------------------------------------

Install a device from QtCreator 

Check device is installed
* sfdk device list
#0 "Xperia10"
    hardware-device  user-defined  defaultuser@192.168.2.16:22
    private-key:

Configuration device
* sfdk config device="Xperia10"

Deploy packages to device
* cd libomemo-c
* sfdk deploy --sdk

* cd ../qxmpp
* sfdk deploy --sdk

* cd ../shmong
* sfdk deploy --sdk



