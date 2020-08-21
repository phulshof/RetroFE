[TOC]

RetroFE is a cross-platform frontend designed for MAME cabinets/game centers/etc. with a focus on simplicity and customization.

# Installing on MacOS #

The quickest and easiest way to install RetroFE for mac is to use [Homebrew](http://brew.sh) and type:
	
	brew install retrofe 

This will install RetroFE in /usr/local/opt/retrofe. RetroFE can then be started by typing 'retrofe'.


### Run RetroFE as an app from the Applications follder
	
	brew linkapps retrofe 


### Install RetroArch 
(RetroFE requires a backend to load the emulator cores) 
	
	brew cask install retroarch 

RetroArch will be installed in /Applications. The RetroFE's sample configuration is setup to use RetroArch for mac. Before starting RetroFE you need to start up RetroArch to download the Genesis Plus GX core and the MAME 2014 core as an example. In RetroArch use arrow keys to move and select by hitting X on the keyboard "Online Updater" -> "Core Updater" -> "<Core Name>". Also run "Online Updater" -> "<Every Updater choice>" to keep RetroArch up to snuff. Check out RetroArch's documentation to learn more. Especially read about using Shaders to make your games look even better.

### Accessing RetroFE Configurations and installing ROMS 

Brew installs applications in /usr/local/Cellar and symlinks the latest version number to /usr/local/opt. This path will not be visible to Finder.

To expose it open Finder and pin the retrofe folder to the Sidebar: 
	
	In Finder -> Go -> Go to Folder -> /usr/local/opt/retrofe 
	In Finder -> File -> Add to Sidebar  

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

	hg clone https://phulshof@bitbucket.org/phulshof/retrofe

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
	
	sudo apt-get install tortoisehg g++ cmake dos2unix zlib1g-dev libsdl2-2.0 libsdl2-mixer-2.0 libsdl2-image-2.0 libsdl2-ttf-2.0 \
                libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
                libgstreamer-plugins-good1.0-dev gstreamer1.0-libav zlib1g-dev libglib2.0-0 libglib2.0-dev sqlite3

## Download and compile the source code ##
Download the source code:

	hg clone https://phulshof@bitbucket.org/phulshof/retrofe

Generate your gcc make files:

	cd retrofe
	cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

Compile RetroFE and create a full environment: 

	cmake --build RetroFE/Build
	python Scripts/Package.py --os=linux --build=full

Copy your live RetroFE system to any folder of your choosing:
	cp -r Artifacts\linux\RetroFE /your/ideal/retrofe/path



# Compiling and installing on Windows #
** Visit the [RetroFE downloads](http://retrofe.com/download.php) page to download a precompiled version if you do not want to compile your own. **

## Install libraries ##
	Install Python 2.7
	Install sphinx with python
	Install visual studio 2012
	Install Microsoft Windows SDK for Windows 7 and .net Framework 4 http://www.microsoft.com/en-us/download/details.aspx?id=8279
	Install cmake
	Install tortoisehg
	Install 7zip
	Install gstreamer and gstreamer-devel to c:/gstreamer(x86, not 64 bit!) from http://gstreamer.freedesktop/org/data/pkg/windows/1.4.0

## Download and compile the source code ##

Download the source code

	hg clone https://phulshof@bitbucket.org/phulshof/retrofe


Setup Environment (to setup necessary variables and paths to compile in visual studio)

	cd retrofe

Generate visual studio solution files

	cmake RetroFE/Source -BRetroFE/Build -DGSTREAMER_ROOT=C:\gstreamer\1.0\x86
  
Compile RetroFE and create a full environment by running the following commands

	cmake --build RetroFE/Build --config Release
	python Scripts\Package.py --os=windows --build=full

Copy your live RetroFE system to any folder of your choosing. files can be found in Artifacts\windows\RetroFE


# Compiling and Installing on Raspberry Pi 2 (raspbian) #

** Due to performance, the Rasperry Pi 1 is not supported. **

Just run the following on your raspberry pi 2 (assuming you have an internet connection:

	bash <(curl -s https://bitbucket.org/phulshof/retrofe/raw/default/Scripts/Raspi2/install.sh)