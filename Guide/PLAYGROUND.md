# General Layout Information

[Back](README.md)

Layouts define the look and feel for your frontend. Each layout is a
folder containing at least the following two main files.

  
## Directory Structure

| Path                               | Description                                                                                  |
|------------------------------------|----------------------------------------------------------------------------------------------|
| /layouts/\<layout name>/           | Folder to store all image files for a particular asset. i.e. \<layout name> = "Default 16x9" |
| /layouts/\<layout name>/splash.xml | Splash screen to show at startup                                                             |
| /layouts/\<layout name>/layout.xml | Main layout file to show when frontend is loaded                                             |

[An example of a
layout](https://bitbucket.org/teamretro/retrofe/src/default/Package/Environment/Common/layouts/Default%2016x9)
[An example of a
layout.xml](https://bitbucket.org/teamretro/retrofe/src/default/Package/Environment/Common/layouts/Default%2016x9/layout.xml?at=default)

The global settings.conf file contains the default layout used by
RetroFE. It is however possible to give a collection a completely
different layout by adding the layout.xml and layout artwork to the
/layouts/\<layout name>/collections/\<collection name>/layout/
directory. A splash.xml is not required/used for a collection's layout.
This way you can give each collection a completely different look and
feel.

RetroFE will first search for a layout XML file that matches the screen
ratio, so e.g. layout 16x9.xml or layout 4x3.xml. If that file is not
found, it will default to layout.xml.  
  
## General Layout Structure 

A layout generally consists of the following structure:

    <layout>
     <sound/>
     <menu/>
     <video/>
     <image/>
     <text/>
     <reloadableVideo/>
     <reloadableImage/>
     <reloadableText/>
     <reloadableScrollingText/>
    </layout>

  

| menuIndex      | Activated on                                                    |
|----------------|-----------------------------------------------------------------|
| menuIndex="0"  | Activate the animation on the main menu.                        |
| menuIndex="1"  | Activate the animation on first submenu.                        |
| menuIndex="!0" | Activate the animation on every index except for the main menu. |

[Back](README.md)
