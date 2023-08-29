# About collections
[Back](README.md)

  
A collection defines a set of games or menu level that generally have a
common purpose, e.g. "Nintendo Entertainment System" or "Consoles".
RetroFE generally supports 3 types of collections:

1.  (Sub-)Menu collections
2.  ItemGame collections
3.  ItemSub/Merged collections

These types can be combined in one collection though, by simply having
the required elements of more types in one collection.

  

## Default collections

The basic RetroFE installation leaves you with 2 main folders but you
will be able to add many other as you wish later on :  
  

| Folder   | Description                                                                                                                                                                                  |
|----------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| \_common | Shared icons and other artworks used for the categories, manufacturers, etc                                                                                                                  |
| Main     | Default collection used for the main menu to work properly and containing the key file to be edited once you have created extra collections so they can be enabled on your screen (menu.txt) |

  
**PS**: In the installation you downloaded, some pre configured sets may
have been added for your convenience, they will not contain the roms of
course, this is something you need to sort out on your own.  
  
### How to Add/Create Extra collections? 

To create a new collection, depite the specific kind required, simply
run the following commands at a commmand prompt. (assuming you are in
the root of your RetroFE directory)

**For Windows (example):**

      core\RetroFE.exe -createcollection "Nintendo Entertainment System"

**For Linux (example):**

      ./RetroFE -createcollection "Nintendo Entertainment System"

Replace "Nintendo Entertainment System" with the collection name of your
choosing.

This will create a "Nintendo Entertainment System Folder", create the
rom folder, artwork folders and default configuration files. More
details on the other steps in the [detailed setup
section](getting_started#adding_collections)

  
**PS:** To enable your new collection in the interface menu, you will
need to add the name of the collection in the menu.txt file stored in
the Main folder

  

## Directory Structure of a collection

| File / Folder                  | Description                                                                                                                                                                                                                                                                                                                                                                                                                    |
|--------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| info.conf                      | This file can be used to add additional system information, such as manufacturer, year, generation, etc. that can be displayed from the theme layouts. This same information can also be defined within settings.conf, but this extra file allows for a clean separation of settings and system information.                                                                                                                   |
| include.txt                    | If not empty, include only those files to show on the list, if empty, all files in ROM folder will be included. This file must contain one game per line (without the file extension).                                                                                                                                                                                                                                         |
| exclude.txt                    | List of ROMs to exclude from showing up on the menu (if you want this collection to be a menu of collections). This file must contain one game per line (without the file extension).                                                                                                                                                                                                                                          |
| exclude_all.txt                | List of ROMs to exclude from the all playlist. This file must contain one game per line (without the file extension).                                                                                                                                                                                                                                                                                                          |
| [settings.conf](SETTINGS.md) | Set ROM file extensions, launcher (emulator to execute), override default media paths, etc                                                                                                                                                                                                                                                                                                                                     |
| \<collectionname>.sub          | Imports a list of games from another collection. (i.e. Your collection is named "Mario". You would have an "Nintendo Entertainment System.sub" and "Super Nintendo Entertainment System.sub". Each sub file would contain a list of all mario games for that system. The .sub file contains one game per line (without the file extension).If the file is blank, the list settings defined for the subcollection will be used. |
| roms                           | Default location to search for ROM files (can be different if modified in the settings.conf file)                                                                                                                                                                                                                                                                                                                              |
| playlists                      | Default location for the favorites.txt file allowing you to manage your favorites                                                                                                                                                                                                                                                                                                                                              |
| medium_artwork/                | Artwork for individual games                                                                                                                                                                                                                                                                                                                                                                                                   |
| medium_artwork/artwork_back    | Default location to search for flyer, box and case backs                                                                                                                                                                                                                                                                                                                                                                       |
| medium_artwork/artwork_front   | Default location to search for flyer, box and case fronts                                                                                                                                                                                                                                                                                                                                                                      |
| medium_artwork/medium_back     | Default location to search for disc and cartridge backs                                                                                                                                                                                                                                                                                                                                                                        |
| medium_artwork/medium_front    | Default location to search for disc and cartridge fronts                                                                                                                                                                                                                                                                                                                                                                       |
| medium_artwork/screenshot      | Default location to search for screenshots for each individual game                                                                                                                                                                                                                                                                                                                                                            |
| medium_artwork/screentitle     | Default location to search for screentitles for each individual game                                                                                                                                                                                                                                                                                                                                                           |
| medium_artwork/logo            | Default location to search for logos for each individual game                                                                                                                                                                                                                                                                                                                                                                  |
| medium_artwork/video           | Default location to search for videos for each individual game                                                                                                                                                                                                                                                                                                                                                                 |
| medium_artwork/story           | Default location to search for game information story txt files for scrolling text display.                                                                                                                                                                                                                                                                                                                                    |
| system_artwork/                | Artwork for the system collection. i.e. a picture of a Nintendo Entertainment System, its logo, or a "best of" video.                                                                                                                                                                                                                                                                                                          |
| system_artwork/device.png      | Picture of the system device                                                                                                                                                                                                                                                                                                                                                                                                   |
| system_artwork/logo.png        | Picture of the systems logo                                                                                                                                                                                                                                                                                                                                                                                                    |
| system_artwork/video.mp4       | Video to play for this particular system                                                                                                                                                                                                                                                                                                                                                                                       |
| system_artwork/story.txt       | Text file containing system information for scrolling text display.                                                                                                                                                                                                                                                                                                                                                            |

  
  
# The 3 kinds of Collections Explained 

  
## (Sub)menu 

A menu contains a list of other collections, and can be defined via the
menu.txt file. When you select that item in the frontend, it will load
that collection in a submenu. RetroFE allows you to combine games and
collections in a single menu, by using both a menu.txt file and a games
collection. A (sub-)menu collection is used to create a hierarchical
level within your menu structure, e.g. Main->Consoles->Nintendo
Entertainment System.

Below is an example for a basic collections/\<collection name>/menu.txt
file:

    Atari 2600
    Atari 5200
    Atari 7800
    MAME
    Nintendo Entertainment System
    Super Nintendo Entertainment System

Note that for each item specified, a directory with an identical name
(case sensitive) must exist in your collections folder, so e.g.
/collections/Super Nintendo Entertainment System/.

Older versions of RetroFE used a menu.xml file in stead of a menu.txt
file. For legacy support, such files are still supported. Below is an
example for a basic collections/\<collection name>/menu.xml file:

    <menu>
       <collection name="Atari 2600" />
       <collection name="Atari 5200" />
       <collection name="Atari 7800" />
       <collection name="MAME" />
       <collection name="Nintendo Entertainment System" />
       <collection name="Super Nintendo Entertainment System" />
    </menu>

As you can see: using the menu.txt file requires far less typing. :)

  

## Games collection

Games collections are collections consisting of a list of games, e.g.
for a certain system like "Nintendo Entertainment System". It uses the
contents of the rom directory in combination with the include.txt and
exclude.txt file in the following manner:

If list.includeMissingItems in the [settings.conf](SETTINGS.md) is
false:

        - Read the contents of the roms directory.
        - Keep only the names defined in include.txt.
        - Remove the names defined in exclude.txt.

If list.includeMissingItems in the [settings.conf](SETTINGS.md) is
true:

        - Use the names defined in include.txt.
        - Remove the names defined in exclude.txt.

  

## Sub/Merged collection

Sub collections contain a subset of other collections, e.g. Capcom Play
System I can be set up as a sub-set of Arcade. The sub-collection is
defined by a \<collection>.sub file, e.g. "Capcom Play System I.sub".

Also, by using multiple .sub files you can also create a merged
collection, e.g. all Contra games from different systems. You could have
a "Nintendo Entertainment System.sub" file containing all the Contra
games from the NES, and a "Super Nintendo Entertainment System.sub" file
containing all the Contra games from the SNES. RetroFE will merge these
collections into one.

A .sub file is simply a text file containing a list of game names,
similar to the include.txt file, but rom file, art files, etc. will be
pulled from the collection it refers to.

Example: You have a MAME or Arcade collection, and wish to add a "Capcom
Play System III" sub-collection.

        - Create the "collections/Capcom Play System III" directory.
        - Add a settings.conf file; this file can be empty or just contain some system information.
        - Add your system_artwork.
        - Create a MAME.sub text file containing:

      jojo
      jojoba
      redearth
      sfiii
      sfiii2
      sfiii3

       5. Add the "Capcom Play System III" to your Main's menu.txt like
you would with any other collection. Enjoy your new sub collection. :)

  
**PS**: If a .sub file is empty, it will use merge that entire
collection according to the rules defined above for Games collections.  
  

# Playlists

RetroFE supports playlists, a sort of sub collection within a
collection. You can step through them using the previous/next playlist
keys defined in your [controls.conf](CONTROLS.md) file. You can use this
feature to e.g. show a list of all games from a certain manufacturer or
a certain genre.

Playlists are defined via txt files in the playlists directory of your
collection, and like include.txt files simply contain a list of the
games you want in that playlist. If a game is pulled from another
collection via a sub/merged collection (.sub file), use \_\<collection
name>:\<game name>. The asterisk (\*) can be used in stead of the game
name to include an entire collection.

Example: playlists/example.txt

    _Super Nintendo Entertainment System:Super Mario World
    _Nintendo Entertainment System:Super Mario Bros
    Super Turrican

This defines a playlist called example, containing the Super Mario World
game from the Super Nintendo Entertainment System collection, Super
Mario Bros from the Nintendo Entertainment System collection, and Super
Turrican from this collection (available in the roms section and/or
defined via the include.txt file).

A special type of playlist is the favorites playlist
(playlists/favorites.txt). Games can be added/removed from this playlist
directly from RetroFE using the addPlaylist and removePlaylist controls,
and a separate key favPlaylist allows you to switch between the
favorites list and the full game list. You can even use the
autoFavorites = yes parameter setting in your [global
settings.conf](GLOBAL_SETTINGS.md) file to automatically switch to
your favorite games when you enter a collection.

[Back](README.md)
