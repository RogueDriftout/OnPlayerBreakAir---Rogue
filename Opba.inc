/* 
Made by RogueDrifter 2018/2/13 
-=-=-=-=-=-=-=-=-=-=-=-=-=-

Callbacks:
-=-=-=-=-
	OnPlayerBreakAir(playerid, breaktype);

-=-=-=-=Usable Defenitions:-=-=-=-=
USE THIS BEFORE THE INCLUDE IF YOU CANCEL THE ORIGINAL SAMP INTERIORS (INCLUDING MOD GARAGES!) TO AVOID EXTRA CHECKS!
#define OPBA_CUSTOM_INTERIORS 

if you don't, use this to define maximum interiors you use:
#define OPBA_MAX_INTERIORS [number]

If you don't want to use the callback it will auto kick,
If you want it to ban you can use:
#define OPBA_BAN_VERSION
*/

#if defined r_BreakAir_ 
	#endinput
#endif

#define r_BreakAir_

#include <a_samp>

#if !defined FILTERSCRIPT

forward OPBA_SetPlayerPos(playerid, Float:x, Float:y, Float:z);
forward OPBA_SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z);
forward OPBA_TogglePlayerSpectating(playerid, toggle);
forward OPBA_SetVehiclePos(vehicleid, Float:x, Float:y, Float:z);
forward OPBA_PutPlayerInVehicle(playerid, vehicleid, seatid);

#if !defined OPBA_CUSTOM_INTERIORS
	forward OPBA_FixIBug(playerid, newinteriorid);
#endif

forward OPBA_PreventAbuse(playerid);
forward OPBA_RCarFall(playerid);
forward OPBA_FixFallBug(playerid);
forward OPBA_AirbrakeTeleport(playerid);
forward OPBA_ResetPImuunity(playerid);
forward OPBA_SetPPos(playerid);
forward OPBA_SafeReset();

#define OPBA_ONFOOT_DETECTION 20.0
#define OPBA_INVEHI_DETECTION 25.0
#define OPBA_MAX_DETECT_SPEED 40.0
#define OPBA_RESET_TIME 3000

#if !defined IsValidVehicle
	native IsValidVehicle(vehicleid); 
#endif

#if !defined OPBA_CUSTOM_INTERIORS

#if !defined OPBA_MAX_INTERIORS
	#define OPBA_MAX_INTERIORS 255
#endif

#endif

#if !defined OnPlayerBreakAir
	forward OPBA_DelayCall(playerid);
#endif

#if defined OPBA_OnPlayerDisconnect
	forward OPBA_OnPlayerDisconnect(playerid, reason);
#endif

#if defined OPBA_OnPlayerInteriorChange
	forward OPBA_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
#endif

#if defined OPBA_OnPlayerConnect
	forward OPBA_OnPlayerConnect(playerid);
#endif

#if defined OPBA_OnPlayerUpdate
	forward OPBA_OnPlayerUpdate(playerid);
#endif

#if defined OPBA_OnPlayerDeath
	forward OPBA_OnPlayerDeath(playerid, killerid, reason);
#endif

#if defined OPBA_OnPlayerSpawn
	forward OPBA_OnPlayerSpawn(playerid);
#endif

#if defined OnPlayerBreakAir
	forward OnPlayerBreakAir(playerid, breaktype);
#endif

#if defined OPBA_OnPlayerStateChange
	forward OPBA_OnPlayerStateChange(playerid, newstate, oldstate);
#endif

#if defined OPBA_OnPlayerExitVehicle
	forward OPBA_OnPlayerExitVehicle(playerid, vehicleid);
#endif

#if defined OPBA_OnPlayerEnterVehicle
	forward OPBA_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
#endif

#if defined OPBA_OnGameModeInit
	forward OPBA_OnGameModeInit();
#endif

#if defined OPBA_OnGameModeExit
	forward OPBA_OnGameModeExit();
#endif

enum E_OPBA_PLAYER_DATA
{
	bool:RacBreakImmunity,
	bool:RacSImu,
	bool:RacPause,
	bool:RacAbuseP,

	RacPauseTicks,
	RacFallBug,
	RacCarFall,

	Float:RacX,
	Float:RacY,
	Float:RacZ,
	
	RacPATimer,	
	RacFixTimer,	
	RacProTimer,
	RacDetectTimer
};

#if !defined OPBA_CUSTOM_INTERIORS

new 
	OPBA_Player[MAX_PLAYERS][E_OPBA_PLAYER_DATA],
	
	bool:OPBA_PInteriors[MAX_PLAYERS][OPBA_MAX_INTERIORS],

	OPBA_VOwner[MAX_VEHICLES],

	OP_SafeTimer;

#else

new 
	OPBA_Player[MAX_PLAYERS][E_OPBA_PLAYER_DATA],

	OPBA_VOwner[MAX_VEHICLES],

	OP_SafeTimer;

#endif

public OPBA_AirbrakeTeleport(playerid)
{
	#if !defined OPBA_CUSTOM_INTERIORS
	new OpbaTempInt = GetPlayerInterior(playerid);
	#endif
	
	static OPBA_Vehicle; 
	OPBA_Vehicle = GetPlayerVehicleID(playerid);

	static Float:OPBA_X, Float:OPBA_Y, Float:OPBA_Z;
	GetVehiclePos(OPBA_Vehicle, OPBA_X, OPBA_Y, OPBA_Z);

	if(OPBA_Player[playerid][RacZ] > OPBA_Z && OPBA_Player[playerid][RacZ] < 0 && OPBA_Z < 0 && OPBA_Player[playerid][RacCarFall] == 0 && IsPlayerInAnyVehicle(playerid))
	{
		if(OPBA_Z - OPBA_Player[playerid][RacZ] >= -5.0)
		{
			if(!OPBA_Player[playerid][RacAbuseP])
			{
				OPBA_Player[playerid][RacCarFall] = 1;
				OPBA_Player[playerid][RacPATimer] = SetTimerEx("OPBA_PreventAbuse", 10*1000, false, "i", playerid);
			}
			else
			{
				OPBA_Player[playerid][RacAbuseP] = false;
			}
		}
	}

	else if(OPBA_Player[playerid][RacCarFall] == 1)
	{
		switch(GetPlayerState(playerid))
		{
			case 1:
			{
				static Float:pOPBA_X, Float:pOPBA_Y, Float:pOPBA_Z;
				GetPlayerPos(playerid, pOPBA_X, pOPBA_Y, pOPBA_Z);
				if(pOPBA_Z >= 0)
				{
					OPBA_Player[playerid][RacCarFall] = 2;
					SetTimerEx("OPBA_RCarFall", 2000, false, "i", playerid);
				}
			}
			case 2..3:
			{
				if(OPBA_Z >= 0)
				{
					OPBA_Player[playerid][RacCarFall] = 2;
					SetTimerEx("OPBA_RCarFall", 2000, false, "i", playerid);
				}
			}
		}
	}

	else if(OPBA_Player[playerid][RacAbuseP] && OPBA_Player[playerid][RacCarFall] == 0)
	{
		switch(GetPlayerState(playerid))
		{
			case 1:
			{
				static Float:pOPBA_X, Float:pOPBA_Y, Float:pOPBA_Z;
				GetPlayerPos(playerid, pOPBA_X, pOPBA_Y, pOPBA_Z);
				if(pOPBA_Z >= 0)
				{
					OPBA_Player[playerid][RacAbuseP] = false;
				}
			}
			case 2..3:
			{
				if(OPBA_Z >= 0)
				{
					OPBA_Player[playerid][RacAbuseP] = false;
				}
			}
		}
	}

	if(GetTickCount() < (OPBA_Player[playerid][RacPauseTicks]+1500) && OPBA_Player[playerid][RacPause])
	{
		GetPlayerPos(playerid, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]);
		OPBA_Player[playerid][RacPause] = false;
	}
	else if(GetTickCount() > (OPBA_Player[playerid][RacPauseTicks]+1500))
	{
		OPBA_Player[playerid][RacPause] = true;
	}

	if(OPBA_GetPlayerSpeed(playerid) <= OPBA_MAX_DETECT_SPEED && !OPBA_Player[playerid][RacPause] && OPBA_Player[playerid][RacFallBug] == 0
	&& !OPBA_Player[playerid][RacBreakImmunity] && OPBA_Player[playerid][RacSImu] && OPBA_Player[playerid][RacCarFall] == 0)
	{
		#if !defined OnPlayerBreakAir
		new OPBA_CheaterName[MAX_PLAYER_NAME], OPBA_CheatString[60];
		GetPlayerName(playerid, OPBA_CheaterName, sizeof(OPBA_CheaterName));

		#if !defined OPBA_BAN_VERSION
		format(OPBA_CheatString, sizeof(OPBA_CheatString), "Server has kicked %s, reason: Airbreak", OPBA_CheaterName);
		#else
		format(OPBA_CheatString, sizeof(OPBA_CheatString), "Server has banned %s, reason: Airbreak", OPBA_CheaterName);
		#endif

		#endif

		switch(GetPlayerState(playerid))
		{
			case 1:
			{
				if(GetPlayerSurfingVehicleID(playerid) == INVALID_VEHICLE_ID && GetPlayerSpecialAction(playerid) != 2)
				{
					#if !defined OPBA_CUSTOM_INTERIORS
					if(!IsPlayerInRangeOfPoint(playerid, OPBA_ONFOOT_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]) && OPBA_PInteriors[playerid][OpbaTempInt])
					{
						#if defined OnPlayerBreakAir
						OnPlayerBreakAir(playerid, 1);
						#else
						SendClientMessageToAll(-1, OPBA_CheatString);
						SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
						#endif
					}

					#else

					if(!IsPlayerInRangeOfPoint(playerid, OPBA_ONFOOT_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]))
					{
						#if defined OnPlayerBreakAir
						OnPlayerBreakAir(playerid, 1);
						#else
						SendClientMessageToAll(-1, OPBA_CheatString);
						SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
						#endif
					}
					#endif
				}
			}
			case 2:
			{
				#if !defined OPBA_CUSTOM_INTERIORS
				if(!IsPlayerInRangeOfPoint(playerid, OPBA_INVEHI_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]) && OPBA_PInteriors[playerid][OpbaTempInt])
				{
					#if defined OnPlayerBreakAir
					OnPlayerBreakAir(playerid, 2);
					#else
					SendClientMessageToAll(-1, OPBA_CheatString);
					SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
					#endif
				}

				#else

				if(!IsPlayerInRangeOfPoint(playerid, OPBA_INVEHI_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]))
				{
					#if defined OnPlayerBreakAir
					OnPlayerBreakAir(playerid, 2);
					#else
					SendClientMessageToAll(-1, OPBA_CheatString);
					SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
					#endif
				}
				#endif
			}
			case 3:
			{
	 			if(OPBA_VOwner[OPBA_Vehicle] == INVALID_PLAYER_ID)
				{
					#if !defined OPBA_CUSTOM_INTERIORS
					if(!IsPlayerInRangeOfPoint(playerid, OPBA_INVEHI_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]) && OPBA_PInteriors[playerid][OpbaTempInt])
					{
						#if defined OnPlayerBreakAir
						OnPlayerBreakAir(playerid, 2);
						#else
						SendClientMessageToAll(-1, OPBA_CheatString);
						SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
						#endif
					}

					#else

				if(!IsPlayerInRangeOfPoint(playerid, OPBA_INVEHI_DETECTION, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]))
				{
					#if defined OnPlayerBreakAir
					OnPlayerBreakAir(playerid, 2);
					#else
					SendClientMessageToAll(-1, OPBA_CheatString);
					SetTimerEx("OPBA_DelayCall", 250, false, "i", playerid);
					#endif
				}
				#endif
				}
			}
		}
	}
	
	GetPlayerPos(playerid, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]);

	return 1;
}

#if !defined OnPlayerBreakAir
public OPBA_DelayCall(playerid)
{
	#if defined OPBA_BAN_VERSION
	BanEx(playerid, "Airbreak");
	#else
	Kick(playerid);
	#endif
	return 1;
}
#endif

public OPBA_SafeReset() 
{ 
    static const tmp2[sizeof(OPBA_VOwner)] = {INVALID_PLAYER_ID,...}; 
    memcpy(OPBA_VOwner, tmp2, 0, sizeof(OPBA_VOwner) * 4); 
    for(new i, j = GetPlayerPoolSize(); i <= j; i++) 
    { 
        if(!IsPlayerConnected(i)) continue; 
        if(GetPlayerState(i)== PLAYER_STATE_DRIVER) 
        OPBA_VOwner[GetPlayerVehicleID(i)]=  i ; 
    } 

    return 1; 
} 

public OPBA_PreventAbuse(playerid)	 return OPBA_Player[playerid][RacCarFall] 		= 0, OPBA_Player[playerid][RacAbuseP] 	   = true;
public OPBA_RCarFall(playerid) 		 return OPBA_Player[playerid][RacCarFall] 		= 0, KillTimer(OPBA_Player[playerid][RacPATimer]);
public OPBA_FixFallBug(playerid) 	 return OPBA_Player[playerid][RacFallBug] 		= 0;
public OPBA_ResetPImuunity(playerid) return OPBA_Player[playerid][RacBreakImmunity] = false;
public OPBA_SetPPos(playerid) 		 return OPBA_Player[playerid][RacSImu] 			= true;

public OPBA_SetPlayerPos(playerid, Float:x, Float:y, Float:z)
{
	KillTimer(OPBA_Player[playerid][RacProTimer]);
	OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);
	OPBA_Player[playerid][RacBreakImmunity] = true;

	OPBA_Player[playerid][RacX] = x, OPBA_Player[playerid][RacY] = y, OPBA_Player[playerid][RacZ] = z;
	SetPlayerPos(playerid, Float:x, Float:y, Float:z);

	return 1;
}

public OPBA_TogglePlayerSpectating(playerid, toggle)
{
	KillTimer(OPBA_Player[playerid][RacProTimer]);
	OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);

	OPBA_Player[playerid][RacBreakImmunity] = true;
	TogglePlayerSpectating(playerid, toggle);

	return 1;
}

public OPBA_SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z)
{
	KillTimer(OPBA_Player[playerid][RacProTimer]);
	OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);
	OPBA_Player[playerid][RacBreakImmunity] = true;

	OPBA_Player[playerid][RacX] = x, OPBA_Player[playerid][RacY] = y, OPBA_Player[playerid][RacZ] = z;
	SetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z);

	return 1;
}

public OPBA_SetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
{
    if(OPBA_VOwner[vehicleid] != INVALID_PLAYER_ID)
    {
    	KillTimer(OPBA_Player[OPBA_VOwner[vehicleid]][RacProTimer]);
    	OPBA_Player[OPBA_VOwner[vehicleid]][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", OPBA_VOwner[vehicleid]),
    	OPBA_Player[OPBA_VOwner[vehicleid]][RacBreakImmunity] = true;

		OPBA_Player[OPBA_VOwner[vehicleid]][RacX] = x, OPBA_Player[OPBA_VOwner[vehicleid]][RacY] = y,
		OPBA_Player[OPBA_VOwner[vehicleid]][RacZ] = z;
    	SetVehiclePos(vehicleid, Float:x, Float:y, Float:z);

    }

    else SetVehiclePos(vehicleid, Float:x, Float:y, Float:z);
	return 1;
}

public OPBA_PutPlayerInVehicle(playerid, vehicleid, seatid)
{
	KillTimer(OPBA_Player[playerid][RacProTimer]);
	OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);

	OPBA_Player[playerid][RacBreakImmunity] = true;
	new Float:x , Float:y, Float:z;
	GetVehiclePos(vehicleid, x, y, z);

	OPBA_Player[playerid][RacX] = x, OPBA_Player[playerid][RacY] = y, OPBA_Player[playerid][RacZ] = z;
	PutPlayerInVehicle(playerid, vehicleid, seatid);

	return 1;
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
	if(OPBA_VOwner[vehicleid] != INVALID_PLAYER_ID && !ispassenger && !OPBA_Player[playerid][RacBreakImmunity])
	{
		KillTimer(OPBA_Player[playerid][RacProTimer]);
		OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);
		OPBA_Player[playerid][RacBreakImmunity] = true;
	}

		#if defined OPBA_OnPlayerEnterVehicle
	return OPBA_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
		#else
	return 1;
		#endif
}

public OnPlayerExitVehicle(playerid, vehicleid)
{
	KillTimer(OPBA_Player[playerid][RacProTimer]);
	OPBA_Player[playerid][RacProTimer] = SetTimerEx("OPBA_ResetPImuunity", OPBA_RESET_TIME, false, "i", playerid);
	OPBA_Player[playerid][RacBreakImmunity] = true;

		#if defined OPBA_OnPlayerExitVehicle
	return OPBA_OnPlayerExitVehicle(playerid, vehicleid);
		#else
	return 1;
		#endif
}

public OnGameModeInit()
{
	print("     [[--|| OnPlayerBreakAir V2.1 ||--]]     ");
    print("     [[--|| Anti airbreak by RogueDrifter V2.1 ||--]]      ");
    print("     [[--|| Successfully Loaded! ||--]]      ");

    OP_SafeTimer = SetTimer("OPBA_SafeReset", 5000, true);

        #if defined OPBA_OnGameModeInit
    return OPBA_OnGameModeInit();
        #else
    return 1;
            #endif
}

public OnGameModeExit()
{
	print("     [[--|| OnPlayerBreakAir V2.1 ||--]]     ");
    print("     [[--|| Anti airbreak by RogueDrifter V2.1 ||--]]      ");
    print("     [[--|| Successfully Unloaded! ||--]]      ");

	KillTimer(OP_SafeTimer);

        #if defined OPBA_OnGameModeExit
    return OPBA_OnGameModeExit();
        #else
    return 1;
            #endif
}

public OnPlayerConnect(playerid)
{
	OPBA_Player[playerid][RacDetectTimer] = SetTimerEx("OPBA_AirbrakeTeleport", 250, true, "i", playerid);

		#if defined OPBA_OnPlayerConnect
	return OPBA_OnPlayerConnect(playerid);
		#else
	return 1;
		#endif
}

public OnPlayerStateChange(playerid, newstate, oldstate)
{
	new OPBA_TempCar = GetPlayerVehicleID(playerid);

	switch(newstate)
	{
		case 1:
		{
			if(oldstate ==2) OPBA_VOwner[OPBA_TempCar] = INVALID_PLAYER_ID;
		}
		case 2:
		{
			OPBA_VOwner[OPBA_TempCar] = playerid;
		}
	}

			#if defined OPBA_OnPlayerStateChange
	return OPBA_OnPlayerStateChange(playerid, newstate, oldstate);
		#else
	return 1;
		#endif
}

public OnPlayerDisconnect(playerid, reason)
{
	KillTimer(OPBA_Player[playerid][RacDetectTimer]);
	KillTimer(OPBA_Player[playerid][RacProTimer]);

	if(GetPlayerState(playerid) == 2) OPBA_VOwner[GetPlayerVehicleID(playerid)] = INVALID_PLAYER_ID;

	OPBA_Player[playerid][RacFallBug] 		= 0;
	OPBA_Player[playerid][RacAbuseP]		= false;
	OPBA_Player[playerid][RacBreakImmunity] = false;
	OPBA_Player[playerid][RacSImu] 			= false;
	OPBA_Player[playerid][RacPause] 		= false;
	OPBA_Player[playerid][RacCarFall]       = 0;

	#if !defined OPBA_CUSTOM_INTERIORS

	for(new i; i < OPBA_MAX_INTERIORS; i++)
    {
    	OPBA_PInteriors[playerid][i] = false;
    }

    #endif
			#if defined OPBA_OnPlayerDisconnect
	return OPBA_OnPlayerDisconnect(playerid, reason);
		#else
	return 1;
		#endif
}

public OnPlayerSpawn(playerid)
{
	GetPlayerPos(playerid, OPBA_Player[playerid][RacX], OPBA_Player[playerid][RacY], OPBA_Player[playerid][RacZ]);
	SetTimerEx("OPBA_SetPPos", 2500, false, "i", playerid);

	#if !defined OPBA_CUSTOM_INTERIORS

	OPBA_PInteriors[playerid][GetPlayerInterior(playerid)] = true;
	#endif

		#if defined OPBA_OnPlayerSpawn
	return OPBA_OnPlayerSpawn(playerid);
		#else
	return 1;
		#endif
}

#if !defined OPBA_CUSTOM_INTERIORS

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	KillTimer(OPBA_Player[playerid][RacFixTimer]);
    OPBA_PInteriors[playerid][oldinteriorid] = false;
    OPBA_Player[playerid][RacFixTimer] = SetTimerEx("OPBA_FixIBug", 1500, false, "ii", playerid, newinteriorid);

		#if defined OPBA_OnPlayerInteriorChange
	return OPBA_OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid);
		#else
	return 1;
		#endif
}

public OPBA_FixIBug(playerid, newinteriorid) return OPBA_PInteriors[playerid][newinteriorid] = true;

#endif

public OnPlayerDeath(playerid, killerid, reason)
{
	OPBA_Player[playerid][RacSImu] = false;

		#if defined OPBA_OnPlayerDeath	
	return OPBA_OnPlayerDeath(playerid, killerid, reason);
		#else
	return 1;
		#endif
}

public OnPlayerUpdate(playerid)
{
	OPBA_Player[playerid][RacPauseTicks] = GetTickCount();

	if(OPBA_PlayerFalling(playerid) && OPBA_Player[playerid][RacFallBug] == 0) OPBA_Player[playerid][RacFallBug] = 1;
	else if(!OPBA_PlayerFalling(playerid) && OPBA_Player[playerid][RacFallBug] == 1) OPBA_Player[playerid][RacFallBug] = 2, SetTimerEx("OPBA_FixFallBug", 2500, false, "i", playerid);
		
		#if defined OPBA_OnPlayerUpdate	
	return OPBA_OnPlayerUpdate(playerid);
		#else
	return 1;
		#endif
}

static OPBA_GetPlayerSpeed(playerid)
{
    new Float:ST[4];

    if(IsPlayerInAnyVehicle(playerid))
    GetVehicleVelocity(GetPlayerVehicleID(playerid),ST[0],ST[1],ST[2]);

    else GetPlayerVelocity(playerid,ST[0],ST[1],ST[2]);
    ST[3] = floatsqroot(floatpower(floatabs(ST[0]), 2.0) + floatpower(floatabs(ST[1]), 2.0) + floatpower(floatabs(ST[2]), 2.0)) * 179.28625;
    return floatround(ST[3]);
}

static OPBA_PlayerFalling(playerid) 
{
	switch(GetPlayerAnimationIndex(playerid))
	{
		case 1130 , 1133: return 1;
	}
	return 0;
}

#if defined _ALS_OnPlayerConnect
  #undef OnPlayerConnect
#else
    #define _ALS_OnPlayerConnect
#endif

#define OnPlayerConnect OPBA_OnPlayerConnect

#if defined _ALS_OnPlayerDisconnect
  #undef OnPlayerDisconnect
#else
    #define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect OPBA_OnPlayerDisconnect

#if defined _ALS_OnPlayerStateChange
  #undef OnPlayerStateChange
#else
    #define _ALS_OnPlayerStateChange
#endif

#define OnPlayerStateChange OPBA_OnPlayerStateChange

#if defined _ALS_OnPlayerInteriorChange
  #undef OnPlayerInteriorChange
#else
    #define _ALS_OnPlayerInteriorChange
#endif

#define OnPlayerInteriorChange OPBA_OnPlayerInteriorChange

#if defined _ALS_OnPlayerSpawn
  #undef OnPlayerSpawn
#else
    #define _ALS_OnPlayerSpawn
#endif

#define OnPlayerSpawn OPBA_OnPlayerSpawn

#if defined _ALS_OnPlayerUpdate
  #undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif

#define OnPlayerUpdate OPBA_OnPlayerUpdate

#if defined _ALS_OnPlayerExitVehicle
  #undef OnPlayerExitVehicle
#else
    #define _ALS_OnPlayerExitVehicle
#endif

#define OnPlayerExitVehicle OPBA_OnPlayerExitVehicle

#if defined _ALS_OnPlayerEnterVehicle
  #undef OnPlayerEnterVehicle
#else
    #define _ALS_OnPlayerEnterVehicle
#endif

#define OnPlayerEnterVehicle OPBA_OnPlayerEnterVehicle

#if defined _ALS_OnGameModeInit
  #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif

#define OnGameModeInit OPBA_OnGameModeInit

#if defined _ALS_OnGameModeExit
  #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif

#define OnGameModeExit OPBA_OnGameModeExit

#if defined _ALS_OnPlayerDeath
  #undef OnPlayerDeath
#else
    #define _ALS_OnPlayerDeath
#endif

#define OnPlayerDeath OPBA_OnPlayerDeath

#if defined _ALS_SetPlayerPos
  #undef SetPlayerPos
#else
    #define _ALS_SetPlayerPos
#endif

#define SetPlayerPos OPBA_SetPlayerPos

#if defined _ALS_SetVehiclePos
  #undef SetVehiclePos
#else
    #define _ALS_SetVehiclePos
#endif

#define SetVehiclePos OPBA_SetVehiclePos

#if defined _ALS_TogglePlayerSpectating
  #undef TogglePlayerSpectating
#else
    #define _ALS_TogglePlayerSpectating
#endif

#define TogglePlayerSpectating OPBA_TogglePlayerSpectating

#if defined _ALS_SetPlayerPosFindZ
  #undef SetPlayerPosFindZ
#else
    #define _ALS_SetPlayerPosFindZ
#endif

#define SetPlayerPosFindZ OPBA_SetPlayerPosFindZ

#if defined _ALS_PutPlayerInVehicle
  #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif

#define PutPlayerInVehicle OPBA_PutPlayerInVehicle

#else //If its a filterscript.

stock OPBA_FSetPlayerPos(playerid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("OPBA_SetPlayerPos", "ifff", playerid, x, y, z);

stock OPBA_FSetVehiclePos(vehicleid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("OPBA_SetVehiclePos", "ifff", vehicleid, x, y, z);

stock OPBA_FSetPlayerPosFindZ(playerid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("OPBA_SetPlayerPosFindZ", "ifff", playerid, x, y, z);

stock OPBA_FPutPlayerInVehicle(playerid, vehicleid, seatid)
	return CallRemoteFunction("OPBA_PutPlayerInVehicle", "iii", playerid, vehicleid, seatid);

stock OPBA_FTogglePlayerSpectating(playerid, toggle)
	return CallRemoteFunction("OPBA_TogglePlayerSpectating", "ii", playerid, toggle);

#if defined _ALS_SetPlayerPos
  #undef SetPlayerPos
#else
    #define _ALS_SetPlayerPos
#endif

#define SetPlayerPos OPBA_FSetPlayerPos

#if defined _ALS_SetVehiclePos
  #undef SetVehiclePos
#else
    #define _ALS_SetVehiclePos
#endif

#define SetVehiclePos OPBA_FSetVehiclePos

#if defined _ALS_SetPlayerPosFindZ
  #undef SetPlayerPosFindZ
#else
    #define _ALS_SetPlayerPosFindZ
#endif

#define SetPlayerPosFindZ OPBA_FSetPlayerPosFindZ

#if defined _ALS_PutPlayerInVehicle
  #undef PutPlayerInVehicle
#else
    #define _ALS_PutPlayerInVehicle
#endif

#define PutPlayerInVehicle OPBA_FPutPlayerInVehicle

#if defined _ALS_TogglePlayerSpectating
  #undef TogglePlayerSpectating
#else
    #define _ALS_TogglePlayerSpectating
#endif

#define TogglePlayerSpectating OPBA_FTogglePlayerSpectating

#endif
