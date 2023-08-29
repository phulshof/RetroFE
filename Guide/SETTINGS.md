# Collection settings.conf
[Back](README.md)

The settings.conf file in a collection defines the settings for that
particular collection. See below for a list of configuration parameters
that can be specified in your collection's settings.conf file.

| Variable                 | Default value                                             | Description                                                                                                                                                                                                                                                                                      |
|--------------------------|-----------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| list.path                | /collections/\<collection name>/roms                      | Location of your roms                                                                                                                                                                                                                                                                            |
| list.includeMissingItems | false                                                     | Include games in the include.txt even if the rom does not exist.                                                                                                                                                                                                                                 |
| list.extensions          |                                                           | Comma separated rom extensions (e.g. zip,nes)                                                                                                                                                                                                                                                    |
| list.menuSort            | false                                                     | Defines whether the items in a collection will be sorted alphabetically                                                                                                                                                                                                                          |
| list.romHierarchy        | false                                                     | Defines whether the roms directory should be searched hierarchically. There is a performance hit by enabling this, so only use it when needed.                                                                                                                                                   |
| list.truRIP              | false                                                     | Defines whether the roms directory should be searched hierarchically based on truRIP format. There is a performance hit by enabling this, so only use it when needed.                                                                                                                            |
| launcher                 | \<collection name>                                        | Specifies which [launcher](LAUNCHERS.md) is used when a game is selected from the menu (reads from launchers/\<launcher name>.conf). This value can be overwritten per game by putting the launcher name in a single line .conf file in collections/\<collection name>/launchers/\<game name>.conf. |
| metadata.type            | \<collection name>                                        | Specifies the meta file used in the meta.db to pull additional information like manufacturer, year, genre, etc. from                                                                                                                                                                             |
| meda.\<filetype>         | collections/\<collection name>/medium_artwork/\<filetype> | Overwrites the path used to pull medium artwork from                                                                                                                                                                                                                                             |
| media.system_artwork     | collections/\<collection name>/system_artwork             | Overwrites the path used to pull system artwork from                                                                                                                                                                                                                                             |

All the settings above can take advantage of the following variables:

| Variable               | Description                                                                                                                     |
|------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| %BASE_MEDIA_PATH%      | The root directory where all artwork resides, defined by baseMediaPath in the [global settings.conf](GLOBAL_SETTINGS.md) file |
| %BASE_ITEM_PATH%       | The root directory where all roms resides, defined by baseMediaPath in the [global settings.conf](GLOBAL_SETTINGS.md) file    |
| %ITEM_COLLECTION_NAME% | The name of the current collection                                                                                              |

[Back](README.md)
