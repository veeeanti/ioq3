/*
============================================================================
    Copyright (C) 2026 veeλnti (jk idfaf about this lol)
============================================================================
*/
// cl_discord.c -- Discord Rich Presence integration

#include "client.h"

#ifdef USE_DISCORD

#include <time.h>

#include "../thirdparty/discord-rpc/include/discord_rpc.h"

static qboolean discordInitialized = qfalse;
static int64_t discordStartTime = 0;
static connstate_t lastState = CA_DISCONNECTED;

// Forward declarations
static void Discord_Init_RPC(void);
static void Discord_Shutdown_RPC(void);
static void Discord_Update_RPC(void);

/*
 =================
 Discord_ReadyCallback
 
 Called when Discord is ready
 =================
 */
static void Discord_ReadyCallback(const DiscordUser* user)
{
    Com_Printf("[Discord] Connected to Discord: %s#%s\n", user->username, user->discriminator);
}

/*
 =================
 Discord_ErrorCallback
 
 Called when Discord encounters an error
 =================
 */
static void Discord_ErrorCallback(int errorCode, const char *message)
{
    Com_Printf("[Discord] Error (%d): %s\n", errorCode, message);
}

/*
 =================
 Discord_JoinCallback
 
 Called when a user joins through Discord
 =================
 */
static void Discord_JoinCallback(const char *joinSecret)
{
    Com_Printf("[Discord] Join request: %s\n", joinSecret);
}

/*
 =================
 Discord_SpectateCallback
 
 Called when a user spectates through Discord
 =================
 */
static void Discord_SpectateCallback(const char *spectateSecret)
{
    Com_Printf("[Discord] Spectate request: %s\n", spectateSecret);
}

/*
 =================
 Discord_JoinRequestCallback
 
 Called when someone requests to join
 =================
 */
static void Discord_JoinRequestCallback(const DiscordUser* request)
{
    Com_Printf("[Discord] Join request from %s#%s\n", request->username, request->discriminator);
}

/*
 =================
 Discord_Init_RPC
 
 Initialize Discord RPC
 =================
 */
static void Discord_Init_RPC(void)
{
    DiscordEventHandlers handlers;
    
    if (discordInitialized)
        return;
    
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = Discord_ReadyCallback;
    handlers.errored = Discord_ErrorCallback;
    handlers.joinGame = Discord_JoinCallback;
    handlers.spectateGame = Discord_SpectateCallback;
    handlers.joinRequest = Discord_JoinRequestCallback;
    
    // Use a default Discord App ID - users can change this
    // This is a generic "Quake III Arena" style ID - you should create your own
    // at https://discord.com/developers/applications
    Discord_Initialize("1482831199217909790", &handlers, 1, NULL);
    
    discordStartTime = time(NULL);
    discordInitialized = qtrue;
    
    Com_Printf("[Discord] Rich Presence initialized\n");
}

/*
 =================
 Discord_Shutdown_RPC
 
 Shutdown Discord RPC
 =================
 */
static void Discord_Shutdown_RPC(void)
{
    if (!discordInitialized)
        return;
    
    Discord_Shutdown();
    discordInitialized = qfalse;
    
    Com_Printf("[Discord] Rich Presence shutdown\n");
}

/*
 =================
 Discord_UpdatePresenceState
 
 Update presence based on current game state
 =================
 */
static void Discord_UpdatePresenceState(void)
{
    DiscordRichPresence presence;
    static char mapName[MAX_QPATH];
    static char serverInfo[MAX_STRING_CHARS];
    
    if (!discordInitialized)
        return;
    
    memset(&presence, 0, sizeof(presence));
    
    // Set the start time if we're in-game
    if (clc.state == CA_ACTIVE)
    {
        presence.startTimestamp = discordStartTime;
    }
    
    switch (clc.state)
    {
        case CA_DISCONNECTED:
        case CA_UNINITIALIZED:
            presence.state = "Main Menu";
            presence.details = "At Main Menu";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_CONNECTING:
            presence.state = "Connecting";
            presence.details = "Connecting to server...";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_CHALLENGING:
            presence.state = "Connecting";
            presence.details = "Authenticating...";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_CONNECTED:
            presence.state = "Connected";
            presence.details = "Loading map...";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_LOADING:
            presence.state = "Loading";
            presence.details = "Loading game...";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_PRIMED:
            presence.state = "Ready";
            presence.details = "Waiting for game start...";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_ACTIVE:
            // Get current map name
            if (cl.mapname[0])
            {
                Q_strncpyz(mapName, cl.mapname, sizeof(mapName));
            }
            else
            {
                Q_strncpyz(mapName, "Unknown", sizeof(mapName));
            }
            
            presence.state = "Playing";
            Com_sprintf(serverInfo, sizeof(serverInfo), "on %s", mapName);
            presence.details = serverInfo;
            
            // Add player count if available
            if (clc.serverMessage[0])
            {
                // Could add player count info here from server info
            }
            
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_CINEMATIC:
            presence.state = "Cinematic";
            presence.details = "Watching cinematic";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
            
        case CA_AUTHORIZING:
        default:
            presence.state = "In-Game";
            presence.details = "Quake III Arena";
            presence.largeImageKey = "q3logo";
            presence.largeImageText = "Quake III Arena";
            presence.instance = 1;
            break;
    }
    
    // Update the presence
    Discord_UpdatePresence(&presence);
    
    lastState = clc.state;
}

/*
 =================
 Discord_Update_RPC
 
 Called every frame to update Discord presence
 =================
 */
static void Discord_Update_RPC(void)
{
    if (!discordInitialized)
        return;
    
    // Update presence if state changed
    if (clc.state != lastState)
    {
        Discord_UpdatePresenceState();
    }
    
    // Run Discord callbacks
    Discord_RunCallbacks();
}

/*
 =================
 CL_InitDiscord
 
 Called from CL_Init
 =================
 */
void CL_InitDiscord(void)
{
#ifdef USE_DISCORD
    Discord_Init_RPC();
#endif
}

/*
 =================
 CL_ShutdownDiscord
 
 Called from CL_Shutdown
 =================
 */
void CL_ShutdownDiscord(void)
{
#ifdef USE_DISCORD
    Discord_Shutdown_RPC();
#endif
}

/*
 =================
 CL_DiscordFrame
 
 Called every frame from CL_Frame
 =================
 */
void CL_DiscordFrame(void)
{
#ifdef USE_DISCORD
    Discord_Update_RPC();
#endif
}

#else /* USE_DISCORD - stubs for when Discord is not enabled */

/*
 =================
 CL_InitDiscord - stub
 =================
 */
void CL_InitDiscord(void)
{
}

/*
 =================
 CL_ShutdownDiscord - stub
 =================
 */
void CL_ShutdownDiscord(void)
{
}

/*
 =================
 CL_DiscordFrame - stub
 =================
 */
void CL_DiscordFrame(void)
{
}

#endif /* USE_DISCORD */
