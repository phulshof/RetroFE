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
ratio, so e.g. "layout 16x9.xml" or layout 4x3.xml. If that file is not
found, it will default to "layout.xml". Additionally, it will look for a
layout file per monitor, e.g. "layout 16x9 - 0.xml", "layout - 1.xml",
etc.

When entering a collection, RetroFE will check the /layouts/\<layout
name>/collections/\<collection name>/layout/ directory for the same
layout XML files. If found, it will load that new layout, allowing you
to set a different layout for each collection.  
  
## General Layout Structure 

A layout generally consists of the following structure:

    <layout>
     <sound/>
     <menu/>
     <video/>
     <image/>
     <text/>
     <reloadableAudio/>
     <reloadableVideo/>
     <reloadableImage/>
     <reloadableText/>
     <reloadableScrollingText/>
    </layout>

  
## \<layout> Parameters 

The \<layout> tag can use the following parameters:

|              |                                                                                                                                                                                                        |
|--------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| width        | The virtual width to use for this layout. This will be scaled automatically by the frontend if the screen resolution is different.                                                                     |
| height       | The virtual height to use for this layout. This will be scaled automatically by the frontend if the screen resolution is different.                                                                    |
| loadFontSize | The size (quality) of the font to load. Lower font sizes is more blurred, Higher font sizes are a little more pixelated. It is best to set this to the same value as fontSize most used in the layout. |
| font         | Location of the font (relative to the layout folder).                                                                                                                                                  |
| fontColor    | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                                 |
| fontSize     | Default font size of to display if not specified by a component.                                                                                                                                       |
| minShowTime  | Minimum amount of time (in seconds) to show the current layout (only applicable in splash.xml).                                                                                                        |

Example:

    <layout width="1920" height="1080" font="Roboto-Bold.ttf" loadFontSize="64" fontColor="dedede">

  
## \<sound> Parameters

The \<sound> tag can use the following parameters:

\<sound>tag parameters

| Tag  | Description                                                                                                                          |
|------|--------------------------------------------------------------------------------------------------------------------------------------|
| type | triggers on the following events: "load" (page load), "unload" (page exit), "highlight" (scroll), "select" (entering game/sub-menu). |
| src  | Location of the sound file (relative to the layout folder).                                                                          |

Example:

    <sound type="load" src="load.wav"/>

  

## \<menu> Parameters

The \<menu> tag is used to define a menu structure, and uses the
following structure:

    <menu>
      <itemDefaults/>
      <item/>
    </menu>

The \<menu> tag supports the following parameters:

| \<menu> tag parameters |                                                                                                                                                                                                                                              |
|------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| type                   | The type of menu to display. Set to custom to specify all the points on the screen. Set to vertical to have a vertical scrolling list prebuilt for the layout                                                                                |
| videoType              | If specified, uses a videoto be displayed for each menu item (if it exists). Text will be used if the video as well as an image as specified under imageType could not be found.                                                             |
| imageType              | If specified, uses an image to be displayed for each menu item (if it exists). Text will be used if the image could not be found.                                                                                                            |
| orientation            | Set to "horizontal" to use the Left/Right controls to scroll. set to "vertical" to use Up/Down.                                                                                                                                              |
| scrollTime             | The amount of time (in seconds) it takes for an item to scroll to the next point on the menu (i.e 0.750 = 750 milliseconds)                                                                                                                  |
| scrollAcceleration     | The acceleration rate to scroll by when holding down the up/down/left/right scroll key                                                                                                                                                       |
| minScrollTime          | The minimum amount of time (in seconds) it takes for an item to scroll to the next point in the menu that scrollAcceleration can reduce it to.                                                                                               |
| menuIndex              | The index at which the menu should be inserted in the menu hierarchy. If omitted, the menu will be inserted at the next index. This also allows multiple menus to be inserted at the same index, displaying multiple menus at the same time. |

  
The \<itemDefaults> tag supports the following parameters:

| \<menu> \<itemDefaults> tag parameters |                                                                                                                                                                                                |
|----------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| spacing                                | Used when the menu type is set to vertical. Defines the spacing in pixels for all edges of a menu item.                                                                                        |
| index                                  | Used when the menu type is set to vertical. Specify a options for a particular menu item when in list mode (first=first visible item, last=last visible item, start=first - 1, end - last + 1) |
| font                                   | Location of the font (relative to the layout folder).                                                                                                                                          |
| fontColor                              | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                         |
| fontSize                               | Default font size of to display if not specified by a component.                                                                                                                               |
| loadFontSize                           | The size (quality) of the font to load. For the best results, use the same value as fontsize.                                                                                                  |
| volume                                 | Audio volume of the video: 0 = mute, 1=100% volume                                                                                                                                             |

  
The \<item> tag supports the following parameters:

| \<menu> \<item> tag parameters |                                                                                               |
|--------------------------------|-----------------------------------------------------------------------------------------------|
| font                           | Location of the font (relative to the layout folder).                                         |
| fontColor                      | Default RGB color of the font (in hex, i.e. “6699AA”).                                        |
| fontSize                       | Default font size of to display if not specified by a component.                              |
| loadFontSize                   | The size (quality) of the font to load. For the best results, use the same value as fontsize. |
| volume                         | Audio volume of the video: 0 = mute, 1=100% volume                                            |

For more information, see this
[example](https://bitbucket.org/teamretro/retrofe/src/default/Package/Environment/Common/layouts/Default%2016x9/layout.xml?at=default)  
  
=====\<video> Parameters=====

The \<video> tag supports the following parameter:

| \<image> tag parameters |                                                                                             |
|-------------------------|---------------------------------------------------------------------------------------------|
| src                     | The location of the video to display (relative to the location of the active layout folder) |
| volume                  | Audio volume of the video: 0 = mute, 1=100% volume                                          |

Example:

    <video src="intro.avi" volume="0.5" x="0" y="0" height="stretch" width="stretch" layer="0"/>

  
  
## \<image> Parameters

The \<image> tag supports the following parameter:

| \<image> tag parameters |                                                                                             |
|-------------------------|---------------------------------------------------------------------------------------------|
| src                     | The location of the image to display (relative to the location of the active layout folder) |

Example:

    <image src="bg.png" x="0" y="0" height="stretch" width="stretch" layer="0"/>

  
## \<text> Parameters

The \<text> tag supports the following parameters:

| \<text> tag parameters |                                                                                               |
|------------------------|-----------------------------------------------------------------------------------------------|
| value                  | The text message to display                                                                   |
| font                   | Location of the font (relative to the layout folder).                                         |
| fontColor              | Default RGB color of the font (in hex, i.e. “6699AA”).                                        |
| fontSize               | Default font size of to display if not specified by a component.                              |
| loadFontSize           | The size (quality) of the font to load. For the best results, use the same value as fontsize. |

Example:

    <text value="Year:" x="300" y="850" xOrigin="right" yOrigin="top" fontSize="48" layer="7"/>

  

## \<reloadableVideo> and \<reloadableAudio> Parameters

The \<reloadableVideo> and \<reloadableAudio> tag can be used to display
a video or audio (supported types are avi, mp4, wav, and mp3) of the
selected item, and supports the following parameters:

| \<reloadableVideo> and \<reloadableAudio> tag parameters |                                                                                                                                                                                                                                                                                                                                                           |
|----------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| type                                                     | The type of video to display: “numberButtons”, “numberPlayers”, “ctrlType”, “numberJoyWays”, “rating”, “score”, “year”, “title”, “developer”, “manufacturer”, “genre”, “playlist”, “collectionName”, “collectionSize”, “collectionIndex”, “collectionIndexSize”, "isFavorite", or any type you pick where the title will be used as the name of the file. |
| imageType                                                | The type of image to display if a video cannot be found: "numberButtons", "numberPlayers", "year", "title", "manufacturer", "genre".                                                                                                                                                                                                                      |
| mode                                                     | See mode attribute for more details.                                                                                                                                                                                                                                                                                                                      |
| textFallback                                             | Set to true to have text displayed of a the item title if an image cannot be loaded.                                                                                                                                                                                                                                                                      |
| font                                                     | Location of the font (relative to the layout folder).                                                                                                                                                                                                                                                                                                     |
| fontColor                                                | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                                                                                                                                                                                    |
| fontSize                                                 | Default font size of to display if not specified by a component.                                                                                                                                                                                                                                                                                          |
| loadFontSize                                             | The size (quality) of the font to load. For the best results, use the same value as fontsize.                                                                                                                                                                                                                                                             |
| backgroundColor                                          | Fill the component with a background color.                                                                                                                                                                                                                                                                                                               |
| backgroundAlpha                                          | Background color transparency: 0 = 0% visible, 0.5=50% visible, 1=100% visible                                                                                                                                                                                                                                                                            |
| volume                                                   | Audio volume of the video: 0 = mute, 1=100% volume                                                                                                                                                                                                                                                                                                        |
| jukebox                                                  | yes = enable jukebox mode. This will disable the attract mode for this layout, and in stead start scrolling after this video/audio is done playing. Default is no.                                                                                                                                                                                        |
| jukeboxNumLoops                                          | Number of loops to play the jukebox element before the jukebox mode is activated. Default is 1.                                                                                                                                                                                                                                                           |

Example:

    <reloadableVideo imageType="screenshot" x="400" y="300" xOrigin="center" yOrigin="center" height="480" maxWidth="640" layer="3"/>

  
## \<reloadableImage> Parameters

The \<reloadableImage> tag can be used to display an image of the
selected item, and supports the following parameters:

| \<reloadableImage> tag parameters |                                                                                                                                                                                                                                                                                                                                             |
|-----------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| type                              | The type of image to display: “numberButtons”, “numberPlayers”, “ctrlType”, “numberJoyWays”, “rating”, “score”, “year”, “title”, “developer”, “manufacturer”, “genre”, “playlist”, “collectionName”, “collectionSize”, “collectionIndex”, “collectionIndexSize”, or any type you pick where the title will be used as the name of the file. |
| mode                              | See mode attribute for more details                                                                                                                                                                                                                                                                                                         |
| textFallback                      | Set to true to have text displayed of a the item title if an image cannot be loaded                                                                                                                                                                                                                                                         |
| font                              | Location of the font (relative to the layout folder).                                                                                                                                                                                                                                                                                       |
| fontColor                         | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                                                                                                                                                                      |
| fontSize                          | Default font size of to display if not specified by a component.                                                                                                                                                                                                                                                                            |
| loadFontSize                      | The size (quality) of the font to load. For the best results, use the same value as fontsize.                                                                                                                                                                                                                                               |

Example:

    <reloadableImage type="logo" mode="system" x="1350" y="50" xOrigin="center" yOrigin="top" height="250" maxWidth="1100" layer="7"/>

  

## \<reloadableText> Parameters

The \<reloadableText> tag can be used to display textual information
about the selected item, and supports the following parameters:

| \<reloadableText> tag parameters |                                                                                                                                                                                                                                                                            |
|----------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| type                             | The type of text to display: "time", "numberButtons", "numberPlayers", "ctrlType", "numberJoyWays", "rating", "score", "year", "title", "developer", "manufacturer", "genre", "playlist", "collectionName", "collectionSize", "collectionIndex", or "collectionIndexSize". |
| mode                             | See mode attribute for more details                                                                                                                                                                                                                                        |
| font                             | Location of the font (relative to the layout folder).                                                                                                                                                                                                                      |
| fontColor                        | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                                                                                                     |
| fontSize                         | Default font size of to display if not specified by a component.                                                                                                                                                                                                           |
| loadFontSize                     | The size (quality) of the font to load. For the best results, use the same value as fontsize.                                                                                                                                                                              |

Example:

    <reloadableText type="year" x="310" y="850" xOrigin="left" yOrigin="top" fontSize="48" maxWidth="300" layer="7"/>

The type="time" allows for an additional parameter: timeFormat, which
follows the C++
[strftime](http://en.cppreference.com/w/cpp/chrono/c/strftime)
abilities.

  
## \<reloadableScrollingText> Parameters

The \<reloadableScrollingText> tag can be used to display textual
information about the selected item, loaded from a text file, and
supports the following parameters:

| \<reloadableScrollingText> tag parameters |                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|-------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| type                                      | The type of text file to load. Files are generally placed as \<collection name>/medium_artwork/\<type>/\<game name>.txt or \<collection name>/system_artwork/\<type>.txt. In addition, the following meta fields are supported: “numberButtons”, “numberPlayers”, “ctrlType”, “numberJoyWays”, “rating”, “score”, “year”, “title”, “developer”, “manufacturer”, “genre”, “playlist”, “collectionName”, “collectionSize”, “collectionIndex”, or “collectionIndexSize”. |
| mode                                      | See mode attribute for more details.                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| font                                      | Location of the font (relative to the layout folder).                                                                                                                                                                                                                                                                                                                                                                                                                 |
| fontColor                                 | Default RGB color of the font (in hex, i.e. “6699AA”).                                                                                                                                                                                                                                                                                                                                                                                                                |
| fontSize                                  | Default font size of to display if not specified by a component.                                                                                                                                                                                                                                                                                                                                                                                                      |
| loadFontSize                              | The size (quality) of the font to load. For the best results, use the same value as fontsize.                                                                                                                                                                                                                                                                                                                                                                         |
| direction                                 | The direction in which the text should scroll: horizontal, vertical.                                                                                                                                                                                                                                                                                                                                                                                                  |
| scrollingSpeed                            | The speed at which the text should scroll in pixels per second.                                                                                                                                                                                                                                                                                                                                                                                                       |
| startPosition                             | The x/y offset of the start position of the text.                                                                                                                                                                                                                                                                                                                                                                                                                     |
| startTime                                 | The delay before the text starts scrolling.                                                                                                                                                                                                                                                                                                                                                                                                                           |
| endTime                                   | The delay between the finish of the scroll, and restarting the display of the text.                                                                                                                                                                                                                                                                                                                                                                                   |
| alignment                                 | The text alignment for vertical scrolls: left, right, centered, justified.                                                                                                                                                                                                                                                                                                                                                                                            |

The text will scroll across the screen until it is no longer visible.
Then it will be reset. If the startPosition is 0, and the text fits
within the defined size, the text will not scroll.

Example:

    <reloadableScrollingText type="story" alpha="0" x="145" y="355" xOrigin="left" yOrigin="top" width="940" height="215" font="MC360.ttf" fontSize="35" fontColor="f2f2f2" direction="vertical" alignment="justified" scrollingSpeed="15" startPosition="0" startTime="4" endTime="1" alpha="1" layer="7"/> 

  
## General Parameters

The other tags all support a general set of parameters:

| Tag                | Description                                                                                                                                                                 |
|--------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| x                  | X coordinate of where to place the component                                                                                                                                |
| y                  | Y coordinate of where to place the component                                                                                                                                |
| xOffset            | Relative X offset of how many pixels to shift the object from x (x + xOffset)                                                                                               |
| yOffset            | Relative Y offset of how many pixels to shift the object from y (y + yOffset)                                                                                               |
| xOrigin            | X offset on image to use as the pin point for placement. Supports the special settings "left", "center", and "right".                                                       |
| yOrigin            | Y offset on image to use as the pin point for placement. Supports the special settings "top", "center", and "bottom".                                                       |
| alpha              | 0 = 0% visible, 0.5=50% visible, 1=100% visible                                                                                                                             |
| angle              | Angle to rotate image, in degrees                                                                                                                                           |
| width              | Width of the component. Image will be scaled appropriately if not specified. Supports the special settings "stretch".                                                       |
| height             | Height of the component. Image will be scaled appropriately if not specified. Supports the special settings "stretch".                                                      |
| minWidth           | Minimum width to allow the image to be (if scaling is needed)                                                                                                               |
| minHeight          | Minimum height to allow the image to be (if scaling is needed)                                                                                                              |
| maxWidth           | Maximum width to allow the image to be (if scaling is needed)                                                                                                               |
| maxHeight          | Maximum height to allow the image to be (if scaling is needed)                                                                                                              |
| backgroundColor    | Fill the component with a background color                                                                                                                                  |
| backgroundAlpha    | Background color transparency: 0 = 0% visible, 0.5=50% visible, 1=100% visible                                                                                              |
| reflection         | Location of the reflection compared to the original image: top, bottom, left, right. Can be combined to show multiple reflections, e.g. topleft, leftright, topbottomright. |
| reflectionDistance | Distance between the reflection and the original image in pixels.                                                                                                           |
| reflectionScale    | Scale in vertical (reflection top/bottom) or horizontal (reflection left/right) direction of the reflection compared to the original image.                                 |
| reflectionAlpha    | Transparency of the reflection compared to the original image: 0 = 0% visible, 0.5=50% visible, 1=100% visible                                                              |
| containerX         | X coordinate of the cropping container.                                                                                                                                     |
| containerY         | Y coordinate of the cropping container.                                                                                                                                     |
| containerWidth     | Width of the cropping container.                                                                                                                                            |
| containerHeight    | Height of the cropping container.                                                                                                                                           |
| layer              | Layer on which the item should be displayed: 0 = back, 19 = front                                                                                                           |
| monitor            | Monitor number on which the element should be displayed, starting from monitor 0.                                                                                           |
| menuScrollReload   | Reload the art while the menu is scrolling when set to yes/true                                                                                                             |

RetroFE supports 20 layers of images (0-19) to allow items to overlap in
a controlled fashion.

Usage of the x/y/xOffset/yOffset/xOrigin/yOrigin parameters:

<img src="/coordinates.png" width="600" />

Usage of the xOffset/yOffset parameters is generally reserved for when
you use values like "top\|bottom\|left\|right\|center" for x and/or y,
e.g. x="center" xOffset="10" will place the object 10 pixels right of
the center.

All these paremeters can also be used in animations to create an even
more beautiful layout experience. These animations use the following
syntax:

    <action type>
      <set duration="<time in seconds>"
        <animate type="<animation parameter>" from="<from setting>" to="<to setting>" algorithm="<algorithm_setting>"/>
      </set>
    </on<action_type>

It is possible to put multiple animates in the same set; these
animations will happen at the same time. It is possible to put multiple
sets in the same action type: these animations will happen in sequence.
It is possible to omit the from attribute; this will animate from the
current value. It is possible to use type="nop" (no operation), without
any other attributes (\<animate type="nop"/>. This can e.g. be used to
delay an animation by preceding it with a set containing just a "nop"
animation.

RetroFE supports the following action types:

| action types     |                                                                                                                |
|------------------|----------------------------------------------------------------------------------------------------------------|
| onEnter          | Action happens on entering RetroFE.                                                                            |
| onExit           | Action happens on exiting RetroFE.                                                                             |
| onIdle           | Action happens continuously while no other animations are active for that object.                              |
| onMenuIdle       | Action happens continuously while the menu isn't scrolling and no other animations are active for that object. |
| onMenuScroll     | Action happens when the menu starts scrolling.                                                                 |
| onHighlightEnter | Action happens when the item is being selected.                                                                |
| onHighlightExit  | Action happens when item is no longer selected.                                                                |
| onMenuEnter      | Action happens on entering a menu.                                                                             |
| onMenuExit       | Action happens on exiting a menu.                                                                              |
| onGameEnter      | Action happens on starting a game.                                                                             |
| onGameExit       | Action happens on quitting a game.                                                                             |
| onPlaylistEnter  | Action happens on entering a playlist.                                                                         |
| onPlaylistExit   | Action happens on exiting a playlist.                                                                          |
| onMenuJumpEnter  | Action happens on entering a jump in the menu (next/previous letter/page, random).                             |
| onMenuJumpExit   | Action happens on exiting a jump in the menu (next/previous letter/page, random).                              |
| onAttractEnter   | Action happens on entering attract mode.                                                                       |
| onAttract        | Action happens continuously while in attract mode.                                                             |
| onAttractExit    | Action happens on exiting attract mode.                                                                        |

The action type can be combined with a menuIndex attribute. This will
force RetroFE to only activate the animation for the specified menu
index, with 0 being the main menu. Some examples:

| menuIndex       | Activated on                                                                                                                                    |
|-----------------|-------------------------------------------------------------------------------------------------------------------------------------------------|
| menuIndex="0"   | Activate the animation on the main menu.                                                                                                        |
| menuIndex="1"   | Activate the animation on first submenu.                                                                                                        |
| menuIndex="!0"  | Activate the animation on every index except for the main menu.                                                                                 |
| menuIndex=">1"  | Activate the animation on menu index 2, 3, 4, ...                                                                                               |
| menuIndex="\<2" | Activate the animation on menu index 0 and 1.                                                                                                   |
| menuIndex="i"   | Activate the animation when the index of the menu corresponds with the current menu index. This can only be used for animations for menu items. |

RetroFE supports the following algorithms:

| action algorithms |                    |                      |                  |                 |
|-------------------|--------------------|----------------------|------------------|-----------------|
| easeInquadratic   | easeOutquadratic   | easeInoutquadratic   | easeIncubic      | easeOutcubic    |
| easeInoutcubic    | easeInquartic      | easeOutquartic       | easeInoutquartic | easeInquintic   |
| easeOutquintic    | easeInoutquintic   | easeInsine           | easeOutsine      | easeInoutsine   |
| easeInexponential | easeOutexponential | easeInoutexponential | easeIncircular   | easeOutcircular |
| easeInoutcircular | linear             |                      |                  |                 |

Example:

    <image src="bg.png" x="0" y="0" height="stretch" width="stretch" layer="0">
      <onIdle>
        <set duration="2">
          <animate type="alpha" from="1" to="0.2" algorithm="easeinquadratic"/>
        </set>
        <set duration="1">
          <animate type="alpha" from="0.2" to="1" algorithm="easeinquadratic"/>
        </set>
      </onIdle>
    </image>

  

## Mode Attribute

Several parameters allow for a mode attribute: system, common, layout,
systemlayout, and commonlayout. This attribute can be omitted. This
attribute has the following effects:

Mode system and systemlayout use the information from the collection
you're in rather than the selected item.

Mode layout, systemlayout, and commonlayout use the art in the
layouts/\<layout name>/collections/\<collection name>/ directory rather
than collections/\<collection name>/ directory.

Mode common and commonlayout use the art in the
collections/\_common/medium_artwork directory rather than the
collections/\<collection name>/medium_artwork directory.

The item path also depends on whether the selected item is a collection
or a game; it will select the first found file.

Example:

You're in the Main collection, have the SNES collection selected, and
are displaying a reloadableImage of type "logo":

      No mode used:
          collections/Main/medium_artwork/logo/SNES.png
          collections/SNES/system_artwork/logo.png
          collections/Main/medium_artwork/logo/default.png
      Mode layout:
          layouts/<layout name>/collections/Main/medium_artwork/logo/SNES.png
          layouts/<layout name>/collections/SNES/system_artwork/logo.png
          layouts/<layout name>/collections/Main/medium_artwork/logo/default.png
      Mode system (not very useful in the main menu):
          collections/Main/medium_artwork/logo/Main.png
          collections/Main/system_artwork/logo.png
          collections/Main/medium_artwork/logo/default.png
      Mode systemlayout (not very useful in the main menu):
          layouts/<layout name>/collections/Main/medium_artwork/logo/Main.png
          layouts/<layout name>/collections/Main/system_artwork/logo.png
          layouts/<layout name>/collections/Main/system_artwork/default.png
      Mode common (not very useful for type logo):
          collections/_common/medium_artwork/logo/SNES.png
          collections/_common/medium_artwork/logo/default.png
      Mode commonlayout (not very useful for type logo):
          layouts/<layout name>/collections/_common/medium_artwork/logo/SNES.png
          layouts/<layout name>/collections/_common/medium_artwork/logo/default.png

Example:

You're in the SNES collection, have the 1942 game selected, and are
displaying a reloadableImage of type "logo":

      No mode used:
          collections/SNES/medium_artwork/logo/1942.png
          collections/SNES/medium_artwork/logo/default.png
      Mode layout:
          layouts/<layout name>/collections/SNES/medium_artwork/logo/1942.png
          layouts/<layout name>/collections/SNES/medium_artwork/logo/default.png
      Mode system:
          collections/SNES/system_artwork/logo.png
      Mode systemlayout:
          layouts/<layout name>/collections/SNES/system_artwork/logo.png
      Mode common (not very useful for type logo):
          collections/_common/medium_artwork/logo/1942.png
          collections/_common/medium_artwork/logo/default.png
      Mode commonlayout (not very useful for type logo):
          layouts/<layout name>/collections/_common/medium_artwork/logo/1942.png
          layouts/<layout name>/collections/_common/medium_artwork/logo/default.png

Example:

You're in the SNES collection by Nintendo, have the 1942 game by Capcom
selected, and are displaying a reloadableImage of type "manufacturer":

      No mode used:
          collections/SNES/medium_artwork/manufacturer/Capcom.png
          collections/SNES/medium_artwork/manufacturer/default.png
      Mode layout:
          layouts/<layout name>/collections/SNES/medium_artwork/manufacturer/Capcom.png
          layouts/<layout name>/collections/SNES/medium_artwork/manufacturer/default.png
      Mode system:
          collections/SNES/system_artwork/Nintendo.png
      Mode systemlayout:
          layouts/<layout name>/collections/SNES/system_artwork/Nintendo.png
      Mode common:
          collections/_common/medium_artwork/manufacturer/Capcom.png
          collections/_common/medium_artwork/manufacturer/default.png
      Mode commonlayout:
          layouts/<layout name>/collections/_common/medium_artwork/manufacturer/Capcom.png
          layouts/<layout name>/collections/_common/medium_artwork/manufacturer/default.png

[Back](README.md)
