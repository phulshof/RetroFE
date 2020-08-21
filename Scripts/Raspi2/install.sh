#!/bin/bash-

# --------------------------------------------------------
# Installs RetroFE and SDL2 from source on the Raspberry Pi 2 (Raspbian)
#
# Run from the web:
#   bash <(curl -s https://bitbucket.org/phulshof/retrofe/raw/default/Scripts/Raspi2/install.sh)
# --------------------------------------------------------

bold="\033[1m"
normal="\033[0m"
black='\033[0;30m'
darkgray='\033[1;30m'
lightgray='\033[0;37m'-
blue='\033[0;34m'
green='\033[0;32m'
cyan='\033[0;36m'
red='\033[0;31m'
purple='\033[0;35m'
orange='\033[0;33m'
yellow='\033[1;33m'
white='\033[1;37m'
NC='\033[0m' # No Color

# Welcome message
echo -e "\nThis script will download and install ${red}RetroFE${NC} including ${red}SDL2${NC} from source."
echo -e "It will take ${yellow}15-60 mins${NC} to complete on the Raspberry Pi 2 depending on number of updates and upgrades required."

# Prompt to continue
read -p "  Continue? (y/n) " ans
  if [[ $ans != "y" ]]; then
    echo -e "\nQuitting...\n"
    exit
  fi
  echo -e "\n\nWould you like to ${red}Update${NC} and ${red}Upgrade${NC} Raspbian?"
  read -p " (y/n) " upans
  echo -e "Compile and install ${yellow}SDL2${NC}?$"
  read -p " (y/n) " sdlans
  
# Time the install process
START_TIME=$SECONDS

  if [[ $upans = "y" ]]; then

    # Update Rasbian
    echo -e "\n\n${green}Updating...${NC}"
    sudo apt-get update -y

    # Upgrade Rasbian
    echo -e "\n\n${green}Upgrading...${NC}"
    sudo apt-get upgrade -y
  
  fi

# Install basic dependencies not needed for compiling SDL2
echo -e "\n\n${green}Installing Round 1 Dependencies...${NC}"
sudo apt-get install -y tortoisehg g++ cmake dos2unix

# *********************************************************************
  if [[ $sdlans = "y" ]]; then

# Compile SDL2 Libs-
  echo -e "\n\n${green}Preparing SDL2 Libraries...${NC}"

# Setting up SDL2 variables
  url="http://www.libsdl.org"

  # sdl="SDL2-2.0.3"
  # sdl_url="${url}/release/${sdl}.tar.gz"
  # sdl_config='--host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl'

  image="SDL2_image-2.0.0"
  image_url="${url}/projects/SDL_image/release/${image}.tar.gz"
  image_config=""

  mixer="SDL2_mixer-2.0.0"
  mixer_url="${url}/projects/SDL_mixer/release/${mixer}.tar.gz"
  mixer_config=""

  ttf="SDL2_ttf-2.0.12"
  ttf_url="${url}/projects/SDL_ttf/release/${ttf}.tar.gz"
  ttf_config=""

  get () {
    cd /tmp
    wget -N $1
    tar -xzvf $2.tar.gz
    cd $2
    ./configure && make -j 4 && sudo make install
    rm /tmp/$2.tar.gz
    rm -rf /tmp/$2
  }

  echo -e "\n\n${green}Installing Round 2 Dependencies...${NC}"
  sudo apt-get install -y libglib2.0-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libsdl2* automake build-essential libasound2-dev libdbus-1-dev libfreeimage-dev libfreetype6-dev libjpeg8-dev libmpeg3-dev libopenal-dev libpango1.0-dev libsndfile-dev libtiff4-dev libudev-dev libvorbis-dev libwebp-dev 
  echo -e "\n\n${green}Beginning SDL2 Core build...${NC}"
# get ${sdl_url} ${sdl} ${sdl_config}
# Needs to be fixed so its part of the loop
    cd /tmp
    wget -N http://www.libsdl.org/release/SDL2-2.0.3.tar.gz
    tar -xzvf SDL2-2.0.3.tar.gz
    cd SDL2-2.0.3
    ./configure --host=armv7l-raspberry-linux-gnueabihf --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl
    make -j 4
    sudo make install
    rm /tmp/SDL2-2.0.3.tar.gz
    rm -rf /tmp/SDL2-2.0.3
  echo -e "\n\n${green}Beginning SDL2 Image build...${NC}"
  get ${image_url} ${image} ${image_config}
  echo -e "\n\n${green}Beginning SDL2 Mixer build...${NC}"
  get ${mixer_url} ${mixer} ${mixer_config}
  echo -e "\n\n${green}Beginning SDL2 TTF build...${NC}"
  get ${ttf_url} ${ttf} ${ttf_config}
  echo -e "\n\n${green}SDL2 Libraries Complete....${NC}"
  
  # Make sure Lib locations are updated 
  sudo ldconfig 
# ****************************************************************************************
 fi
cd ~
 
# Download RetroFE source
echo -e "\n\n${green}Downloading RetroFE...${NC}"
hg clone --debug -v https://bitbucket.org/phulshof/retrofe

cd retrofe

# Generate Make files for RetroFE
  echo -e "\n\n${green}Generating make files...${NC}"
  cmake RetroFE/Source -BRetroFE/Build -DVERSION_MAJOR=0 -DVERSION_MINOR=0 -DVERSION_BUILD=0

# Compile RetroFE
  echo -e "\n\n${green}Compiling RetroFE and creating a full environment...${NC}"
  cmake --build RetroFE/Build

# Install Basic folders and menu
  python Scripts/Package.py --os=linux --build=full

# Print the time elapsed
  ELAPSED_TIME=$(($SECONDS - $START_TIME))
  echo -e "\n${bold}Finished in $(($ELAPSED_TIME/60/60)) hr, $(($ELAPSED_TIME/60%60)) min, and $(($ELAPSED_TIME%60)) sec${normal}\n"

#Final Message
  echo -e "Copy ${bold}./retrofe/Artifacts/linux/RetroFE/${normal} to wherever you like or"
  echo -e "Type ${bold}./retrofe/Artifacts/linux/RetroFE/retrofe${normal} to start..."