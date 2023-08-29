# Launchers
[Back](README.md)


A launcher config file describes how to launch a program (i.e. emulator,
application, or game) when a launchable menu item is selected.

See below for a list of supported configuration properties. Launcher
options

| Property   | Description                                                   |
|------------|---------------------------------------------------------------|
| executable | Path of where the executable exists                           |
| arguments  | Arguments to pass when executing the launcher (i.e. ROM name) |

    executable = D:/Emulators/Nestopia/nestopia.exe
    arguments  = "%ITEM_FILEPATH%"

%ITEM_FILEPATH% is a reserved variable name. See the variables table
below for other variables that may be used. Also note the quotes around
"%ITEM_FILEPATH%" to help not confuse the executable from thinking that
an item with spaces as multiple arguments.

Assuming that "Super Mario Bros" was the selected item, the frontend
will attempt to execute:

    "D:/Emulators/Nestopia/nestopia.exe" "D:/ROMs/Nintendo/Super Mario Bros.nes".

**PS**: You can also use relative paths (relative to the root folder of
RetroFE)

    executable = ../Emulators/Nestopia/nestopia.exe
    arguments  = "%ITEM_FILEPATH%"

Launcher variables

| Variable               | Description                 | Translated Example                    |
|------------------------|-----------------------------|---------------------------------------|
| %ITEM_FILEPATH%        | Full item path              | D:/ROMs/Nintendo/Super Mario Bros.nes |
| %ITEM_NAME%            | The item name               | Super Mario Bros                      |
| %ITEM_FILENAME%        | Filename without path       | Super Mario Bros.nes                  |
| %ITEM_DIRECTORY%       | Folder where file exists    | D:/ROMs/Nintendo                      |
| %ITEM_COLLECTION_NAME% | Name of collection for item | Nintendo Entertainment System         |
| %RETROFE_PATH%         | Folder location of Frontend | D:/Frontends/RetroFE                  |
| %RETROFE_EXEC_PATH%    | Location of RetroFE         | D:/Frontends/RetroFE/RetroFE.exe      |

More elaborate example:

    # Have fceux load a save state automatically for the ROM when started
    executable = D:/Emulators/fceux/fceux.exe
    arguments  = "%ITEM_FILEPATH%" -loadstate "%ITEM_DIRECTORY%/%ITEM_NAME%.fcs"

[Back](README.md)
