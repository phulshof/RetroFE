[TOC]

RetroFE is a cross-platform frontend designed for MAME cabinets/game centers/etc. with a focus on simplicity and customization.

## Optional ##

### Fix libpng iCCP warnings about incorrect sRGB profile 

The issue is with the png files that are being used with the Artwork. Libpng is pretty touchy about it. You can get rid of these messages with a handy tool called pngcrush found on sourceforge and github.

** Error message: **
	
	libpng warning: iCCP: known incorrect sRGB profile
	libpng warning: iCCP: known incorrect sRGB profile
	libpng warning: iCCP: known incorrect sRGB profile
	libpng warning: iCCP: known incorrect sRGB profile
	libpng warning: iCCP: known incorrect sRGB profile
	libpng warning: iCCP: known incorrect sRGB profile

** Install pngcrush on Mac: ** (linux use apt-get ?)
	
	brew install pngcrush


** Use pngcrush to Find and repair pngs: **
	
	find /usr/local/opt/retrofe/collections -type f -iname '*.png' -exec pngcrush -ow -rem allb -reduce {} \;


### Set $RETROFE_PATH via Environment variable 

RetroFE will load it's media and configuration files relative to where the binary file is located. This allows the build to be portable. If you want RetroFE to load your configuration from a fixed location regardless of where your install is copy your configuration there and set $RETROFE_PATH. Note this will work if you start RetroFE from the command line.

	vi ~/.bash_profile
	export RETROFE_PATH=/your/new/retrofe


### Set RETROFE_PATH via flat file 

Depending on your version of OS X the GUI will read user defined Environment variables from [another place](http://stackoverflow.com/questions/135688/setting-environment-variables-in-os-x). If you find this dificult to setup you can get around it by creating a text file in your HOME directory: /Users/<you>/.retrofe with one line no spaces: /your/new/retrofe. This will also work in Linux. RetroFE's configuration search order is 1st: ENV, Flat file, and executable location.

** See below for optional flat file example **

	echo /your/new/retrofe > ~/.retrofe


## Mac download and compile RetroFE from source code ##
If you don't use Homebrew you will need to download and install the same dependencies as given in the linux instuctions.
You may need to export the libs with $LIBRARY_PATH and or supply the include folders with $CPATH before building.

	export LIBRARY_PATH=/usr/local/opt/gst-plugins-base/lib:/usr/local/opt/gstreamer/lib:/usr/local/lib:/usr/local/opt/glib/lib:/usr/local/opt/gettext/lib

Download the source code:

	git clone https://github.com/phulshof/RetroFE.git

Generate your gcc make files:

	cd retrofe
	cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

Compile RetroFE and create a full environment (ensure --os=mac !): 

	cmake --build RetroFE/Build
	python Scripts/Package.py --os=mac --build=full

Copy your live RetroFE system to any folder of your choosing:
	cp -r Artifacts/mac/RetroFE /your/ideal/retrofe/path

#  Compiling and installing on Ubuntu Linux (10.04 or newer) #

## Install libraries ##
Install necessary dependencies:
	
	sudo apt-get install git g++ cmake dos2unix zlib1g-dev libsdl2-2.0 libsdl2-mixer-2.0 libsdl2-image-2.0 libsdl2-ttf-2.0 \
                libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
                libgstreamer-plugins-good1.0-dev gstreamer1.0-libav zlib1g-dev libglib2.0-0 libglib2.0-dev sqlite3

## Download and compile the source code ##
Download the source code:

	git clone https://github.com/phulshof/RetroFE.git

Generate your gcc make files:

	cd retrofe
	cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

Compile RetroFE and create a full environment: 

	cmake --build RetroFE/Build
	python Scripts/Package.py --os=linux --build=full

Copy your live RetroFE system to any folder of your choosing:
	cp -r Artifacts\linux\RetroFE /your/ideal/retrofe/path



# Compiling and installing on Windows #
** Visit the [RetroFE downloads](http://http://retrofe.nl/download/) page to download a precompiled version if you do not want to compile your own. **

## Install libraries ##
	Install Python 2.7 (https://www.python.org/downloads/windows/)
	Install sphinx with python (https://www.sphinx-doc.org/en/1.6.5/install.html)
	Install visual studio 2019 (https://visualstudio.microsoft.com/downloads/)
	Install Microsoft Windows SDK for Windows 10 and .net Framework 4 (https://developer.microsoft.com/nl-nl/windows/downloads/windows-10-sdk/)
	Install cmake (https://cmake.org/download/)
	Install git (https://git-scm.com/download/win)
	Install 7zip (https://www.7-zip.org/)
	Install gstreamer and gstreamer-devel to c:/gstreamer(x86, not 64 bit!) (https://gstreamer.freedesktop.org/data/pkg/windows/1.18.3/msvc/)

## Download and compile the source code ##

Download the source code

	git clone https://github.com/phulshof/RetroFE.git

Setup Environment (to setup necessary variables and paths to compile in visual studio)

	cd retrofe

Generate visual studio solution files

	cmake -A Win32 -B .\RetroFE\Build -D GSTREAMER_ROOT=C:\gstreamer\1.0\msvc_x86 -S .\RetroFE\Source
  
Compile RetroFE and create a full environment by running the following commands

	cmake --build RetroFE/Build --config Release
	python Scripts\Package.py --os=windows --build=full

Copy your live RetroFE system to any folder of your choosing. files can be found in Artifacts\windows\RetroFE
