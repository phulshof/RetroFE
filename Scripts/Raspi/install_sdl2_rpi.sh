#!/bin/bash

# --------------------------------------------------------
# Installs SDL2 from source on the Raspberry Pi (Raspbian)
#
# Run from the web:
#   bash <(curl -s raw_script_url_here)
# --------------------------------------------------------

bold="\033[1m"
normal="\033[0m"

# Welcome message
echo -e "\n${bold}This will install SDL2 from source."
echo -e "It will take about an hour to compile on the Raspberry Pi.${normal}"

# Prompt to continue
read -p "  Continue? (y/n) " ans
if [[ $ans != "y" ]]; then
  echo -e "\nQuitting...\n"
  exit
fi
echo

# Time the install process
START_TIME=$SECONDS

# Setting up SDL2 variables
url="http://www.libsdl.org"

sdl="SDL2-2.0.3"
sdl_url="${url}/release/${sdl}.tar.gz"

image="SDL2_image-2.0.0"
image_url="${url}/projects/SDL_image/release/${image}.tar.gz"

mixer="SDL2_mixer-2.0.0"
mixer_url="${url}/projects/SDL_mixer/release/${mixer}.tar.gz"

ttf="SDL2_ttf-2.0.12"
ttf_url="${url}/projects/SDL_ttf/release/${ttf}.tar.gz"

get () {
  cd /tmp
  wget -N $1
  tar -xzf $2.tar.gz
  cd $2
  ./configure && make && sudo make install
  rm /tmp/$2.tar.gz
  rm -rf /tmp/$2
}

get ${sdl_url} ${sdl}
get ${image_url} ${image}
get ${mixer_url} ${mixer}

sudo apt-get install -y libfreetype6-dev libudev-dev libasound2-dev libdbus-1-dev libvorbis-dev libmpeg3-dev 
get ${ttf_url} ${ttf}

# Print the time elapsed
ELAPSED_TIME=$(($SECONDS - $START_TIME))
echo -e "\n${bold}Finished in $(($ELAPSED_TIME/60/60)) hr, $(($ELAPSED_TIME/60%60)) min, and $(($ELAPSED_TIME%60)) sec${normal}\n"
