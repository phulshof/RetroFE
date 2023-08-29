# Getting Started
[Back](README.md)
  
For Windows users, and some Linux users, getting started with RetroFE is
as simple as following these steps:

1.  Download RetroFE from <http://retrofe.nl/download>
2.  Edit the [global settings.conf](global settings.conf) file.
3.  Edit the [controls.conf](CONTROLS.md) file.
4.  Run RetroFE to verify if the front-end loads (and exits) correctly.
5.  Edit/add/delete [collections](COLLECTIONS.md).
6.  Re-run RetroFE

  
## RetroFE Root Directory Structure 

| File / Folder                         | Description                                                                      |
|---------------------------------------|----------------------------------------------------------------------------------|
| [controls.conf](CONTROLS.md)             | Controller configuration (up, down, select, back, etc)                           |
| log.txt                               | Log output                                                                       |
| meta.db                               | Game (information database year, manufacturer, genre, #players, etc)             |
| RetroFE.lnk                           | Windows link to core / retrofe.exe                                               |
| [settings.conf](GLOBAL_SETTINGS.md) | Global frontend settings (display options, layout to use, base paths, etc)       |
| /collections/                         | Game lists, menus, artwork and ROMs                                              |
| /core/                                | Windows specific libraries needed for retrofe to run (also includes retrofe.exe) |
| /launchers/                           | Configuration files for launchers (emulators)                                    |
| /layouts/                             | Layouts / themes to use or display for the frontend                              |
| /meta/                                | Files to import into meta.db (for scraping)                                      |

  

# Detailed Setup Guide

  
## Installation 

(Note for linux users: Since Linux comes in many shapes and sizes,
RetroFE users need to compile and install their own RetroFE setup using
bitbucket. The instructions for this can be found
[here](https://bitbucket.org/phulshof/retrofe/overview).)

After copying the RetroFE system to the directory of your choice, you're
set to give your installation a first test by running the retrofe
executable in that directory. RetroFE comes with a pre-installed Sega
Genesis system with one game so you can check if the installation went
according to plan.

  
## Configuration

The first configuration step is editing the RetroFE system configuration
file RetroFE/[settings.conf](GLOBAL_SETTINGS.md). In here you
configure the screen settings, global theme, base paths, etc.

The second configuration step is editing the RetroFE controls file
RetroFE/[controls.conf](CONTROLS.md). In here you configure the keys used
to control the RetroFE front-end. Note that the default select key is
space, and not enter as some people expected.

  
## Adding Collections 

RetroFE starts with two (almost empty) [collections](COLLECTIONS.md), but
more can be added easily. As an example, let's set up the Nintendo
Entertainment System collection. First, enter the RetroFE/collections
directory, and create an empty collection using the following command:
../retrofe -createcollection "Nintendo Entertainment System"

Next we add the roms and artwork:

Download a NES romset from your favourite source, and place the roms in
the RetroFE/collections/Nintendo Entertainment System/roms directory.
Download a device image, logo, and video for the system, and place the
device.png, logo.png, and video.mp4 files in the
RetroFE/collections/Nintendo Entertainment System/system_artwork
directory. Download games artwork (artwork_front, logo's, screenshots,
titleshots, videos, etc.) from your favourite art source, and place them
in the RetroFE/collections/Nintendo Entertainment System/medium_artwork
directory.

Now we configure the new system by editing RetroFE/collections/Nintendo
Entertainment System/settings.conf. If you stick to the default
directory structure, this file can be extremely simple:

    list.extensions = nes

    launcher = NES

The first line defines the ROM file extention as .nes; this should match
the file extentions in your RetroFE/collections/Nintendo Entertainment
System/roms directory. The second line defines the name of the launcher
used for this collection. Before the collection can be used, this
launcher needs to be configured:

Edit the RetroFE/launchers/NES.conf (matching the launcher name defined
in the settings.conf) file. I'm currently using MAME 0.162 for this
purpose, so the launcher can be simple:

    executable = mame

    arguments = nes -cart "%ITEM_FILEPATH%"

As an example: if RetroFE starts the game Willow (USA).nes, this
launcher will execute the command:

    mame nes -cart "collections/Nintendo Entertainment System/roms/Willow (USA).nes".

The last step is to add the newly created collection to the main menu by
editing RetroFE/collections/Main/menu.txt, and add the following line:

    Nintendo Entertainment System

When this is done, your newly added collection is ready for testing by
running the retrofe executable RetroFE/retrofe.

[Back](README.md)
