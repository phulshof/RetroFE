# Meta information database
[Back](README.md)

RetroFE uses the following information to create the item lists for the
menus:

-   menu.txt
-   Files in the roms directory
-   include.txt
-   exclude.txt
-   \<collection name>.sub

More information about this can be found in the
[collections](COLLECTIONS.md) section of this documentation. As can be seen
though: RetroFE does **not** use the meta xml files to create these
lists!

The files in the meta directory are used for one thing only: to fill the
meta.db database. This meta.db in turn is used to fill in the extra
information such as full title, genre, rating, score, etc. in the
created menu item list for games. Extra information for collections is
pulled from the settings.conf and/or info.conf file of the collection.

RetroFE supports three types of meta information:

-   HyperSpin xml files from the directory meta/hyperlist.
-   truRIP (super)dat files from the directory meta/trurip.
-   MAME xml files from the directory meta/mamelist/.

In addition, RetroFE allows you to add information via info files named
as collections/\<collection name>/info/\<item name>.conf. These files
can contain lines of additional/overwriting information, e.g.

manufacturer = Konami

Information in these files will supersede information from the meta
database when the overwriteXML parameter in the [global
settings.conf](GLOBAL_SETTINGS.md) file is set.

## HyperSpin xml files

HyperSpin xml files are xml files as supported by the HyperSpin
front-end. RetroFE supports the following tags (, and will ignore the
rest):

-   \<name>
-   \<description>
-   \<cloneof>
-   \<manufacturer>
-   \<developer>
-   \<year>
-   \<genre>
-   \<rating>
-   \<score>
-   \<players>
-   \<ctrltype>
-   \<buttons>
-   \<joyways>

## emuArc (super)dat files

emuArc (super)dat files are xml files as supported by the emuArc group.
RetroFE supports the following tags (, and will ignore the rest):

-   \<rootdir> This subtag of the header tag defines the collection name
    used for the meta database.
-   \<game>
-   \<description>
-   \<publisher>
-   \<developer>
-   \<year>
-   \<genre>
-   \<ratings>
-   \<score>
-   \<players>
-   \<cloneof>

## MAME xml files

MAME xml files are the output of the -listxml output of MAME.

[Back](README.md)
