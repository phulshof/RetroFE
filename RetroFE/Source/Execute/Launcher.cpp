/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Launcher.h"
#include "../Collection/Item.h"
#include "../Utility/Log.h"
#include "../Database/Configuration.h"
#include "../Utility/Utils.h"
#include "../RetroFE.h"
#include "../SDL.h"
#include <cstdlib>
#include <locale>
#include <sstream>
#include <fstream>
#ifdef WIN32
#include <windows.h>
#include <cstring>
#endif

Launcher::Launcher(Configuration &c)
    : config_(c)
{
}

bool Launcher::run(std::string collection, Item *collectionItem)
{
    std::string launcherName = collectionItem->collectionInfo->launcher;
    std::string executablePath;
    std::string selectedItemsDirectory;
    std::string selectedItemsPath;
    std::string extensionstr;
    std::string matchedExtension;
    std::string args;

    std::string launcherFile = Utils::combinePath( Configuration::absolutePath, "collections", collectionItem->collectionInfo->name, "launchers", collectionItem->name + ".conf" );
    std::ifstream launcherStream( launcherFile.c_str( ) );
    if (launcherStream.good( )) // Launcher file found
    {
        std::string line;
        if (std::getline( launcherStream, line)) // Launcher found
        {
            launcherName = line;
        }
    }
    launcherName = Utils::toLower(launcherName);

    if(!launcherExecutable(executablePath, launcherName))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "Failed to find launcher executable (launcher: " + launcherName + " executable: " + executablePath + ")");
        return false;
    }
    if(!extensions(extensionstr, collection))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "No file extensions configured for collection \"" + collection + "\"");
        return false;
    }
    if(!collectionDirectory(selectedItemsDirectory, collection))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "Could not find files in directory \"" + selectedItemsDirectory + "\" for collection \"" + collection + "\"");
        return false;
    }
    if(!launcherArgs(args, launcherName))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "No launcher arguments specified for launcher " + launcherName);
        return false;
    }


    // Overwrite selectedItemsDirectory if already set in the file
    if (collectionItem->filepath != "")
    {
        selectedItemsDirectory = collectionItem->filepath;
    }

    // It is ok to continue if the file could not be found. We could be launching a merged romset
    if (collectionItem->file == "")
        findFile(selectedItemsPath, matchedExtension, selectedItemsDirectory, collectionItem->name, extensionstr);
    else
        findFile(selectedItemsPath, matchedExtension, selectedItemsDirectory, collectionItem->file, extensionstr);

    args = replaceVariables(args,
                            selectedItemsPath,
                            collectionItem->name,
                            Utils::getFileName(selectedItemsPath),
                            selectedItemsDirectory,
                            collection);

    executablePath = replaceVariables(executablePath,
                                      selectedItemsPath,
                                      collectionItem->name,
                                      Utils::getFileName(selectedItemsPath),
                                      selectedItemsDirectory,
                                      collection);

    std::string currentDirectoryKey = "launchers." + launcherName + ".currentDirectory";
    std::string currentDirectory    = Utils::getDirectory(executablePath);

    config_.getProperty(currentDirectoryKey, currentDirectory);

    currentDirectory = replaceVariables(currentDirectory,
                                        selectedItemsPath,
                                        collectionItem->name,
                                        Utils::getFileName(selectedItemsPath),
                                        selectedItemsDirectory,
                                        collection);

    if(!execute(executablePath, args, currentDirectory))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "Failed to launch.");
        return false;
    }

    bool reboot = false;
	config_.getProperty("launchers." + launcherName + ".reboot", reboot);

    return reboot;
}


void Launcher::LEDBlinky( int command, std::string collection, Item *collectionItem )
{
	std::string LEDBlinkyDirectory = "";
	config_.getProperty( "LEDBlinkyDirectory", LEDBlinkyDirectory );
	std::string exe  = LEDBlinkyDirectory + "\\LEDBlinky.exe";
	std::string args = std::to_string( command );
	bool wait = false;
	if ( command == 2 )
		wait = true;
	if ( command == 8 )
	{
		std::string launcherName = collectionItem->collectionInfo->launcher;
		std::string launcherFile = Utils::combinePath( Configuration::absolutePath, "collections", collectionItem->collectionInfo->name, "launchers", collectionItem->name + ".conf" );
		std::ifstream launcherStream( launcherFile.c_str( ) );
		if (launcherStream.good( )) // Launcher file found
		{
			std::string line;
			if (std::getline( launcherStream, line)) // Launcher found
			{
				launcherName = line;
			}
		}
		launcherName = Utils::toLower( launcherName );
		std::string emulator = collection;
		config_.getProperty("launchers." + launcherName + ".LEDBlinkyEmulator", emulator );
		args = args + " \"" + emulator + "\"";
	}
	if ( command == 3 || command == 9 )
	{
		std::string launcherName = collectionItem->collectionInfo->launcher;
		std::string launcherFile = Utils::combinePath( Configuration::absolutePath, "collections", collectionItem->collectionInfo->name, "launchers", collectionItem->name + ".conf" );
		std::ifstream launcherStream( launcherFile.c_str( ) );
		if (launcherStream.good( )) // Launcher file found
		{
			std::string line;
			if (std::getline( launcherStream, line)) // Launcher found
			{
				launcherName = line;
			}
		}
		launcherName = Utils::toLower( launcherName );
		std::string emulator = launcherName;
		config_.getProperty("launchers." + launcherName + ".LEDBlinkyEmulator", emulator );
		args = args + " \"" + collectionItem->name + "\" \"" + emulator + "\"";
		if ( emulator == "" )
			return;
	}
	if ( LEDBlinkyDirectory != "" && !execute( exe, args, LEDBlinkyDirectory, wait ) )
	{
        Logger::write( Logger::ZONE_ERROR, "LEDBlinky", "Failed to launch." );
	}
	return;
}


std::string Launcher::replaceVariables(std::string str,
                                       std::string itemFilePath,
                                       std::string itemName,
                                       std::string itemFilename,
                                       std::string itemDirectory,
                                       std::string itemCollectionName)
{
    str = Utils::replace(str, "%ITEM_FILEPATH%", itemFilePath);
    str = Utils::replace(str, "%ITEM_NAME%", itemName);
    str = Utils::replace(str, "%ITEM_FILENAME%", itemFilename);
    str = Utils::replace(str, "%ITEM_DIRECTORY%", itemDirectory);
    str = Utils::replace(str, "%ITEM_COLLECTION_NAME%", itemCollectionName);
    str = Utils::replace(str, "%RETROFE_PATH%", Configuration::absolutePath);
#ifdef WIN32
    str = Utils::replace(str, "%RETROFE_EXEC_PATH%", Utils::combinePath(Configuration::absolutePath, "core", "RetroFE.exe"));
#else
    str = Utils::replace(str, "%RETROFE_EXEC_PATH%", Utils::combinePath(Configuration::absolutePath, "RetroFE"));
#endif

    return str;
}

bool Launcher::execute(std::string executable, std::string args, std::string currentDirectory, bool wait)
{
    bool retVal = false;
    std::string executionString = "\"" + executable + "\" " + args;

    Logger::write(Logger::ZONE_INFO, "Launcher", "Attempting to launch: " + executionString);
    Logger::write(Logger::ZONE_INFO, "Launcher", "     from within folder: " + currentDirectory);

#ifdef WIN32
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    char applicationName[2048];
    char currDir[2048];
    memset(&applicationName, 0, sizeof(applicationName));
    memset(&startupInfo, 0, sizeof(startupInfo));
    memset(&processInfo, 0, sizeof(processInfo));
    strncpy(applicationName, executionString.c_str(), sizeof(applicationName));
    strncpy(currDir, currentDirectory.c_str(), sizeof(currDir));
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.wShowWindow = SW_SHOWDEFAULT;

    if(!CreateProcess(NULL, applicationName, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, currDir, &startupInfo, &processInfo))
#else
    const std::size_t last_slash_idx = executable.rfind(Utils::pathSeparator);
    if (last_slash_idx != std::string::npos)
    {
        std::string applicationName = executable.substr(last_slash_idx + 1);
        executionString = "cd \"" + currentDirectory + "\" && exec \"./" + applicationName + "\" " + args;
    }
    if(system(executionString.c_str()) != 0)
#endif
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "Failed to run: " + executable);
    }

    else
    {
#ifdef WIN32
		if ( wait )
		{
			while(WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &processInfo.hProcess, FALSE, INFINITE, QS_ALLINPUT))
			{
				MSG msg;
				while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					DispatchMessage(&msg);
				}
			}
        }

        // result = GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hProcess);
#endif
        retVal = true;
    }

    Logger::write(Logger::ZONE_INFO, "Launcher", "Completed");

    return retVal;
}

bool Launcher::launcherName(std::string &launcherName, std::string collection)
{
    std::string launcherKey = "collections." + collection + ".launcher";

    // find the launcher for the particular item
    if(!config_.getProperty(launcherKey, launcherName))
    {
        std::stringstream ss;

        ss << "Launch failed. Could not find a configured launcher for collection \""
           << collection
           << "\" (could not find a property for \""
           << launcherKey
           << "\")";

        Logger::write(Logger::ZONE_ERROR, "Launcher", ss.str());

        return false;
    }

    std::stringstream ss;
    ss        << "collections."
              << collection
              << " is configured to use launchers."
              << launcherName
              << "\"";

    Logger::write(Logger::ZONE_DEBUG, "Launcher", ss.str());

    return true;
}



bool Launcher::launcherExecutable(std::string &executable, std::string launcherName)
{
    std::string executableKey = "launchers." + launcherName + ".executable";

    if(!config_.getProperty(executableKey, executable))
    {
        return false;
    }

    return true;
}

bool Launcher::launcherArgs(std::string &args, std::string launcherName)
{
    std::string argsKey = "launchers." + launcherName + ".arguments";

    if(!config_.getProperty(argsKey, args))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "No arguments specified for: " + argsKey);

        return false;
    }
    return true;
}

bool Launcher::extensions(std::string &extensions, std::string collection)
{
    std::string extensionsKey = "collections." + collection + ".list.extensions";

    if(!config_.getProperty(extensionsKey, extensions))
    {
        Logger::write(Logger::ZONE_ERROR, "Launcher", "No extensions specified for: " + extensionsKey);
        return false;
    }

    extensions = Utils::replace(extensions, " ", "");
    extensions = Utils::replace(extensions, ".", "");

    return true;
}

bool Launcher::collectionDirectory(std::string &directory, std::string collection)
{
    std::string itemsPathValue;
    std::string mergedCollectionName;

    // find the items path folder (i.e. ROM path)
    config_.getCollectionAbsolutePath(collection, itemsPathValue);
    directory += itemsPathValue + Utils::pathSeparator;

    return true;
}

bool Launcher::findFile(std::string &foundFilePath, std::string &foundFilename, std::string directory, std::string filenameWithoutExtension, std::string extensions)
{
    std::string extension;
    bool fileFound = false;
    std::stringstream ss;
    ss << extensions;

    while(!fileFound && std::getline(ss, extension, ',') )
    {
        std::string selectedItemsPath = directory + filenameWithoutExtension + "." + extension;
        std::ifstream f(selectedItemsPath.c_str());

        if (f.good())
        {
            std::stringstream ss;

            ss        <<"Checking to see if \""
                      << selectedItemsPath << "\" exists  [Yes]";

            fileFound = true;

            Logger::write(Logger::ZONE_INFO, "Launcher", ss.str());

            foundFilePath = selectedItemsPath;
            foundFilename = extension;
        }
        else
        {
            std::stringstream ss;

            ss        << "Checking to see if \""
                      << selectedItemsPath << "\" exists  [No]";

            Logger::write(Logger::ZONE_WARNING, "Launcher", ss.str());
        }

        f.close();
    }

    // get the launchers executable

    if(!fileFound)
    {
        std::stringstream ss;
        ss        <<"Could not find any files with the name \""
                  << filenameWithoutExtension << "\" in folder \""
                  << directory;

        Logger::write(Logger::ZONE_WARNING, "Launcher", ss.str());

    }

    return fileFound;
}


