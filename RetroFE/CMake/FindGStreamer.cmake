FILE(TO_CMAKE_PATH "$ENV{GSTREAMER_ROOT}" TRY1_DIR)
FILE(TO_CMAKE_PATH "${GSTREAMER_ROOT}" TRY2_DIR)
FILE(GLOB GSTREAMER_ROOT ${TRY1_DIR} ${TRY2_DIR})

FIND_PATH(GSTREAMER_gst_INCLUDE_DIR gst/gst.h
                                    PATHS /opt/homebrew/Cellar/gstreamer/1.22.6/include/gstreamer-1.0 ${GSTREAMER_ROOT}/include ${GSTREAMER_ROOT}/include/gstreamer-1.0 /usr/local/include/gstreamer-1.0 /usr/include/gstreamer-1.0 /usr/local/opt/gstreamer/include/gstreamer-1.0
                                    ENV INCLUDE DOC "Directory containing gst/gst.h include file")

#/usr/local/opt/gst-plugins-base/lib/gstreamer-1.0
FIND_PATH(GSTREAMER_gstappsink_INCLUDE_DIR gst/app/gstappsink.h
                                    PATHS /opt/homebrew/Cellar/gstreamer/1.22.6/include/gstreamer-1.0 /usr/local/opt/gst-plugins-base/include /usr/local/opt/gst-plugins-base/include/gstreamer-1.0
                                    ENV INCLUDE DOC "Directory containing gst/app/gstappsink.h include file")

FIND_PATH(GSTREAMER_gstconfig_INCLUDE_DIR gst/gstconfig.h
                                          PATHS /opt/homebrew/Cellar/gstreamer/1.22.6/include/gstreamer-1.0 ${GSTREAMER_ROOT}/include ${GSTREAMER_ROOT}/lib/include ${GSTREAMER_ROOT}/include/gstreamer-1.0 /usr/local/include/gstreamer-1.0 /usr/include/gstreamer-1.0 /usr/local/lib/include/gstreamer-1.0 /usr/lib/include/gstreamer-1.0 /usr/local/opt/gstreamer/lib/gstreamer-1.0/include
                                          ENV INCLUDE DOC "Directory containing gst/gstconfig.h include file")

FIND_LIBRARY(GSTREAMER_gstaudio_LIBRARY NAMES gstreamer-audio-1.0 gstaudio-1.0 libgstaudio-1.0 gstaudio
                                        PATHS opt/homebrew/Cellar/gstreamer/1.22.6/lib ${GSTREAMER_ROOT}/bin ${GSTREAMER_ROOT}/win32/bin ${GSTREAMER_ROOT}/bin/bin C:/gstreamer/bin ${GSTREAMER_ROOT}/lib ${GSTREAMER_ROOT}/win32/lib /usr/local/lib /usr/lib
                                        ENV LIB
                                        DOC "gstaudio library to link with"
                                        NO_SYSTEM_ENVIRONMENT_PATH)

FIND_LIBRARY(GSTREAMER_gstreamer_LIBRARY NAMES gstreamer-1.0 libgstreamer-1.0 gstreamer
                                         PATHS opt/homebrew/Cellar/gstreamer/1.22.6/lib ${GSTREAMER_ROOT}/bin ${GSTREAMER_ROOT}/win32/bin ${GSTREAMER_ROOT}/bin/bin C:/gstreamer/bin ${GSTREAMER_ROOT}/lib ${GSTREAMER_ROOT}/win32/lib /usr/local/lib /usr/lib
                                         ENV LIB
                                         DOC "gstreamer library to link with"
                                         NO_SYSTEM_ENVIRONMENT_PATH)
										 
FIND_LIBRARY(GSTREAMER_gstbase_LIBRARY NAMES gstreamer-base-1.0 gstbase-1.0 libgstbase-1.0 gstbase
                                       PATHS opt/homebrew/Cellar/gstreamer/1.22.6/lib ${GSTREAMER_ROOT}/bin ${GSTREAMER_ROOT}/win32/bin ${GSTREAMER_ROOT}/bin/bin C:/gstreamer/bin ${GSTREAMER_ROOT}/lib ${GSTREAMER_ROOT}/win32/lib /usr/local/lib /usr/lib
                                       ENV LIB
                                       DOC "gstbase library to link with"
                                       NO_SYSTEM_ENVIRONMENT_PATH)

FIND_LIBRARY(GSTREAMER_gstvideo_LIBRARY NAMES gstreamer-video-1.0 gstvideo-1.0 libgstvideo-1.0 gstvideo
                                        PATHS opt/homebrew/Cellar/gstreamer/1.22.6/lib ${GSTREAMER_ROOT}/bin ${GSTREAMER_ROOT}/win32/bin ${GSTREAMER_ROOT}/bin/bin C:/gstreamer/bin ${GSTREAMER_ROOT}/lib ${GSTREAMER_ROOT}/win32/lib /usr/local/lib /usr/lib
                                        ENV LIB
                                        DOC "gstvideo library to link with"
                                        NO_SYSTEM_ENVIRONMENT_PATH)
                                        
FIND_LIBRARY(GSTREAMER_gstapp_LIBRARY NAMES gstreamer-app-1.0 gstapp-1.0
                                        PATHS opt/homebrew/Cellar/gstreamer/1.22.6/lib ${GSTREAMER_ROOT}/bin ${GSTREAMER_ROOT}/win32/bin ${GSTREAMER_ROOT}/bin/bin C:/gstreamer/bin ${GSTREAMER_ROOT}/lib ${GSTREAMER_ROOT}/win32/lib /usr/local/lib /usr/lib
                                        ENV LIB
                                        DOC "gstapp library to link with"
                                        NO_SYSTEM_ENVIRONMENT_PATH)
                                        

#IF (GSTREAMER_gst_INCLUDE_DIR AND GSTREAMER_gstconfig_INCLUDE_DIR AND
#    GSTREAMER_gstaudio_LIBRARY AND GSTREAMER_gstbase_LIBRARY AND
#    GSTREAMER_gstreamer_LIBRARY AND GSTREAMER_gstvideo_LIBRARY)
	
  SET(GSTREAMER_INCLUDE_DIRS ${GSTREAMER_gst_INCLUDE_DIR} ${GSTREAMER_gstconfig_INCLUDE_DIR})
  list(REMOVE_DUPLICATES GSTREAMER_INCLUDE_DIRS)
  SET(GSTREAMER_LIBRARIES ${GSTREAMER_gstaudio_LIBRARY} ${GSTREAMER_gstbase_LIBRARY}
                          ${GSTREAMER_gstreamer_LIBRARY} ${GSTREAMER_gstvideo_LIBRARY}
                          ${GSTREAMER_gstapp_LIBRARY})
  list(REMOVE_DUPLICATES GSTREAMER_LIBRARIES)
  SET(GSTREAMER_FOUND TRUE)
#ENDIF (GSTREAMER_gst_INCLUDE_DIR AND GSTREAMER_gstconfig_INCLUDE_DIR AND
#       GSTREAMER_gstaudio_LIBRARY AND GSTREAMER_gstbase_LIBRARY AND 
#		GSTREAMER_gstreamer_LIBRARY AND GSTREAMER_gstvideo_LIBRARY)
