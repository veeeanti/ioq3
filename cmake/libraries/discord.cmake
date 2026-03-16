if(NOT USE_DISCORD)
    return()
endif()

if(NOT BUILD_CLIENT)
    return()
endif()

set(INTERNAL_DISCORD_DIR ${SOURCE_DIR}/thirdparty/discord-rpc)
set(INTERNAL_DISCORD_LIBS_DIR ${SOURCE_DIR}/thirdparty/libs)

# Check for Discord RPC headers
if(EXISTS ${INTERNAL_DISCORD_DIR}/include/discord_rpc.h)
    set(DISCORD_FOUND ON)
    set(DISCORD_INCLUDE_DIR ${INTERNAL_DISCORD_DIR}/include)
else()
    # Try to find system Discord RPC
    find_path(DISCORD_INCLUDE_DIR discord_rpc.h)
    if(DISCORD_INCLUDE_DIR)
        set(DISCORD_FOUND ON)
    endif()
endif()

if(DISCORD_FOUND)
    list(APPEND CLIENT_DEFINITIONS USE_DISCORD)
    list(APPEND CLIENT_INCLUDE_DIRS ${DISCORD_INCLUDE_DIR})
    
    # Find the Discord RPC library based on platform
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            # 64-bit Windows
            set(DISCORD_LIB_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win64/discord-rpc.lib)
            set(DISCORD_DLL_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win64/discord-rpc.dll)
            if(NOT EXISTS ${DISCORD_LIB_PATH})
                message(WARNING "Discord: win64 .lib not found, falling back to win32 (requires 32-bit build)")
                set(DISCORD_LIB_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win32/discord-rpc.lib)
                set(DISCORD_DLL_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win32/discord-rpc.dll)
            endif()
            message(STATUS "Discord: Using win64 library")
        else()
            # 32-bit Windows
            set(DISCORD_LIB_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win32/discord-rpc.lib)
            set(DISCORD_DLL_PATH ${INTERNAL_DISCORD_LIBS_DIR}/win32/discord-rpc.dll)
            message(STATUS "Discord: Using win32 library")
        endif()
        
        if(EXISTS ${DISCORD_LIB_PATH})
            list(APPEND CLIENT_LIBRARIES ${DISCORD_LIB_PATH})
            message(STATUS "Discord: Found library at ${DISCORD_LIB_PATH}")
            
            # Copy DLL to output directory
            if(EXISTS ${DISCORD_DLL_PATH})
                list(APPEND CLIENT_DEPLOY_LIBRARIES ${DISCORD_DLL_PATH})
                message(STATUS "Discord: Found DLL at ${DISCORD_DLL_PATH}")
            endif()
        else()
            message(WARNING "Discord: Library not found at ${DISCORD_LIB_PATH}")
        endif()
    elseif(APPLE)
        set(DISCORD_LIB_PATH ${INTERNAL_DISCORD_LIBS_DIR}/macos/libdiscord-rpc.dylib)
        if(EXISTS ${DISCORD_LIB_PATH})
            list(APPEND CLIENT_LIBRARIES ${DISCORD_LIB_PATH})
            message(STATUS "Discord: Found library at ${DISCORD_LIB_PATH}")
        else()
            message(WARNING "Discord: Library not found at ${DISCORD_LIB_PATH}")
        endif()
    elseif(UNIX)
        # Try system library
        find_library(DISCORD_LIBRARY NAMES discord-rpc)
        if(DISCORD_LIBRARY)
            list(APPEND CLIENT_LIBRARIES ${DISCORD_LIBRARY})
            message(STATUS "Discord: Using system library")
        endif()
    endif()
    
    message(STATUS "Discord Rich Presence: Enabled")
else()
    message(WARNING "Discord RPC headers not found, disabling Discord Rich Presence")
    # Force disable Discord if not found
    set(USE_DISCORD OFF PARENT_SCOPE)
endif()
