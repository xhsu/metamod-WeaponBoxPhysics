import std;

import eiface;
import engine_api;
import meta_api;

// Hook.cpp
extern META_RES fw_PM_Move(playermove_s* ppmove, qboolean server) noexcept;
extern void fw_PM_Move_Post(playermove_s* ppmove, qboolean server) noexcept;
extern META_RES fw_PlayerPostThink(edict_t*) noexcept;
extern qboolean fw_AddToFullPack_Post(entity_state_t* pState, int iEntIndex, edict_t* pEdict, edict_t* pClientSendTo, qboolean cl_lw, qboolean bIsPlayer, unsigned char* pSet) noexcept;
extern void fw_Touch_Post(edict_t* pentTouched, edict_t* pentOther) noexcept;
//

// Description of plugin
inline constexpr plugin_info_t gPluginInfo =
{
	.ifvers		= META_INTERFACE_VERSION,
	.name		= "Weapon Box Physics",
	.version	= "1.0.6",
	.date		= __DATE__,
	.author		= "Luna the Reborn",
	.url		= "http://www.metamod.org/",
	.logtag		= "WBP",
	.loadable	= PT_ANYTIME,
	.unloadable	= PT_ANYPAUSE,
};

inline constexpr auto PLID = &gPluginInfo;

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we do some setup operations here.
void __stdcall GiveFnptrsToDll(enginefuncs_t *pengfuncsFromEngine, globalvars_t *pGlobals) noexcept
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

static int HookGameDLLExportedFn(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	static constexpr DLL_FUNCTIONS gFunctionTable =
	{
		.pfnGameInit	= nullptr,
		.pfnSpawn		= nullptr,
		.pfnThink		= nullptr,
		.pfnUse			= nullptr,
		.pfnTouch		= nullptr,
		.pfnBlocked		= nullptr,
		.pfnKeyValue	= nullptr,
		.pfnSave		= nullptr,
		.pfnRestore		= nullptr,
		.pfnSetAbsBox	= nullptr,

		.pfnSaveWriteFields	= nullptr,
		.pfnSaveReadFields	= nullptr,

		.pfnSaveGlobalState		= nullptr,
		.pfnRestoreGlobalState	= nullptr,
		.pfnResetGlobalState	= nullptr,

		.pfnClientConnect		= nullptr,
		.pfnClientDisconnect	= nullptr,
		.pfnClientKill			= nullptr,
		.pfnClientPutInServer	= nullptr,
		.pfnClientCommand		= nullptr,
		.pfnClientUserInfoChanged	= nullptr,
		.pfnServerActivate		= nullptr,
		.pfnServerDeactivate	= nullptr,

		.pfnPlayerPreThink	= nullptr,
		.pfnPlayerPostThink	= [](edict_t* pPlayer) noexcept { gpMetaGlobals->mres = fw_PlayerPostThink(pPlayer); },

		.pfnStartFrame		= nullptr,
		.pfnParmsNewLevel	= nullptr,
		.pfnParmsChangeLevel= nullptr,

		.pfnGetGameDescription	= nullptr,
		.pfnPlayerCustomization	= nullptr,

		.pfnSpectatorConnect	= nullptr,
		.pfnSpectatorDisconnect	= nullptr,
		.pfnSpectatorThink		= nullptr,

		.pfnSys_Error	= nullptr,

		.pfnPM_Move				= [](playermove_s* ppmove, qboolean server) noexcept { gpMetaGlobals->mres = fw_PM_Move(ppmove, server); },
		.pfnPM_Init				= nullptr,
		.pfnPM_FindTextureType	= nullptr,

		.pfnSetupVisibility	= nullptr,
		.pfnUpdateClientData= nullptr,
		.pfnAddToFullPack	= nullptr,
		.pfnCreateBaseline	= nullptr,
		.pfnRegisterEncoders= nullptr,
		.pfnGetWeaponData	= nullptr,
		.pfnCmdStart		= nullptr,
		.pfnCmdEnd			= nullptr,
		.pfnConnectionlessPacket	= nullptr,
		.pfnGetHullBounds			= nullptr,
		.pfnCreateInstancedBaselines= nullptr,
		.pfnInconsistentFile		= nullptr,
		.pfnAllowLagCompensation	= nullptr,
	};

	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return true;
}

static int HookGameDLLExportedFn_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	static constexpr DLL_FUNCTIONS gFunctionTable =
	{
		.pfnGameInit	= nullptr,
		.pfnSpawn		= nullptr,
		.pfnThink		= nullptr,
		.pfnUse			= nullptr,
		.pfnTouch		= &fw_Touch_Post,
		.pfnBlocked		= nullptr,
		.pfnKeyValue	= nullptr,
		.pfnSave		= nullptr,
		.pfnRestore		= nullptr,
		.pfnSetAbsBox	= nullptr,

		.pfnSaveWriteFields	= nullptr,
		.pfnSaveReadFields	= nullptr,

		.pfnSaveGlobalState		= nullptr,
		.pfnRestoreGlobalState	= nullptr,
		.pfnResetGlobalState	= nullptr,

		.pfnClientConnect		= nullptr,
		.pfnClientDisconnect	= nullptr,
		.pfnClientKill			= nullptr,
		.pfnClientPutInServer	= nullptr,
		.pfnClientCommand		= nullptr,
		.pfnClientUserInfoChanged	= nullptr,
		.pfnServerActivate		= nullptr,
		.pfnServerDeactivate	= nullptr,

		.pfnPlayerPreThink	= nullptr,
		.pfnPlayerPostThink	= nullptr,

		.pfnStartFrame		= nullptr,
		.pfnParmsNewLevel	= nullptr,
		.pfnParmsChangeLevel= nullptr,

		.pfnGetGameDescription	= nullptr,
		.pfnPlayerCustomization	= nullptr,

		.pfnSpectatorConnect	= nullptr,
		.pfnSpectatorDisconnect	= nullptr,
		.pfnSpectatorThink		= nullptr,

		.pfnSys_Error	= nullptr,

		.pfnPM_Move				= &fw_PM_Move_Post,
		.pfnPM_Init				= nullptr,
		.pfnPM_FindTextureType	= nullptr,

		.pfnSetupVisibility	= nullptr,
		.pfnUpdateClientData= nullptr,
		.pfnAddToFullPack	= &fw_AddToFullPack_Post,
		.pfnCreateBaseline	= nullptr,
		.pfnRegisterEncoders= nullptr,
		.pfnGetWeaponData	= nullptr,
		.pfnCmdStart		= nullptr,
		.pfnCmdEnd			= nullptr,
		.pfnConnectionlessPacket	= nullptr,
		.pfnGetHullBounds			= nullptr,
		.pfnCreateInstancedBaselines= nullptr,
		.pfnInconsistentFile		= nullptr,
		.pfnAllowLagCompensation	= nullptr,
	};

	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn_Post' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn_Post' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return true;
}

static int HookEngineAPI(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion) noexcept
{
	static constexpr enginefuncs_t meta_engfuncs = 
	{
		.pfnPrecacheModel	= nullptr,
		.pfnPrecacheSound	= nullptr,
		.pfnSetModel		= nullptr,
		.pfnModelIndex		= nullptr,
		.pfnModelFrames		= nullptr,

		.pfnSetSize			= nullptr,
		.pfnChangeLevel		= nullptr,
		.pfnGetSpawnParms	= nullptr,
		.pfnSaveSpawnParms	= nullptr,

		.pfnVecToYaw		= nullptr,
		.pfnVecToAngles		= nullptr,
		.pfnMoveToOrigin	= nullptr,
		.pfnChangeYaw		= nullptr,
		.pfnChangePitch		= nullptr,

		.pfnFindEntityByString	= nullptr,
		.pfnGetEntityIllum		= nullptr,
		.pfnFindEntityInSphere	= nullptr,
		.pfnFindClientInPVS		= nullptr,
		.pfnEntitiesInPVS		= nullptr,

		.pfnMakeVectors		= nullptr,
		.pfnAngleVectors	= nullptr,

		.pfnCreateEntity		= nullptr,
		.pfnRemoveEntity		= nullptr,
		.pfnCreateNamedEntity	= nullptr,

		.pfnMakeStatic		= nullptr,
		.pfnEntIsOnFloor	= nullptr,
		.pfnDropToFloor		= nullptr,

		.pfnWalkMove		= nullptr,
		.pfnSetOrigin		= nullptr,

		.pfnEmitSound		= nullptr,
		.pfnEmitAmbientSound= nullptr,

		.pfnTraceLine		= nullptr,
		.pfnTraceToss		= nullptr,
		.pfnTraceMonsterHull	= nullptr,
		.pfnTraceHull		= nullptr,
		.pfnTraceModel		= nullptr,
		.pfnTraceTexture	= nullptr,
		.pfnTraceSphere		= nullptr,
		.pfnGetAimVector	= nullptr,

		.pfnServerCommand	= nullptr,
		.pfnServerExecute	= nullptr,
		.pfnClientCommand	= nullptr,

		.pfnParticleEffect	= nullptr,
		.pfnLightStyle		= nullptr,
		.pfnDecalIndex		= nullptr,
		.pfnPointContents	= nullptr,

		.pfnMessageBegin	= nullptr,
		.pfnMessageEnd		= nullptr,

		.pfnWriteByte	= nullptr,
		.pfnWriteChar	= nullptr,
		.pfnWriteShort	= nullptr,
		.pfnWriteLong	= nullptr,
		.pfnWriteAngle	= nullptr,
		.pfnWriteCoord	= nullptr,
		.pfnWriteString	= nullptr,
		.pfnWriteEntity	= nullptr,

		.pfnCVarRegister	= nullptr,
		.pfnCVarGetFloat	= nullptr,
		.pfnCVarGetString	= nullptr,
		.pfnCVarSetFloat	= nullptr,
		.pfnCVarSetString	= nullptr,

		.pfnAlertMessage	= nullptr,
		.pfnEngineFprintf	= nullptr,

		.pfnPvAllocEntPrivateData	= nullptr,
		.pfnPvEntPrivateData		= nullptr,
		.pfnFreeEntPrivateData		= nullptr,

		.pfnSzFromIndex		= nullptr,
		.pfnAllocString		= nullptr,

		.pfnGetVarsOfEnt		= nullptr,
		.pfnPEntityOfEntOffset	= nullptr,
		.pfnEntOffsetOfPEntity	= nullptr,
		.pfnIndexOfEdict		= nullptr,
		.pfnPEntityOfEntIndex	= nullptr,
		.pfnFindEntityByVars	= nullptr,
		.pfnGetModelPtr			= nullptr,

		.pfnRegUserMsg		= nullptr,

		.pfnAnimationAutomove	= nullptr,
		.pfnGetBonePosition		= nullptr,

		.pfnFunctionFromName	= nullptr,
		.pfnNameForFunction		= nullptr,

		.pfnClientPrintf	= nullptr,
		.pfnServerPrint		= nullptr,

		.pfnCmd_Args	= nullptr,
		.pfnCmd_Argv	= nullptr,
		.pfnCmd_Argc	= nullptr,

		.pfnGetAttachment	= nullptr,

		.pfnCRC32_Init			= nullptr,
		.pfnCRC32_ProcessBuffer	= nullptr,
		.pfnCRC32_ProcessByte	= nullptr,
		.pfnCRC32_Final			= nullptr,

		.pfnRandomLong	= nullptr,
		.pfnRandomFloat	= nullptr,

		.pfnSetView			= nullptr,
		.pfnTime			= nullptr,
		.pfnCrosshairAngle	= nullptr,

		.pfnLoadFileForMe	= nullptr,
		.pfnFreeFile		= nullptr,

		.pfnEndSection		= nullptr,
		.pfnCompareFileTime	= nullptr,
		.pfnGetGameDir		= nullptr,
		.pfnCvar_RegisterVariable	= nullptr,
		.pfnFadeClientVolume	= nullptr,
		.pfnSetClientMaxspeed	= nullptr,
		.pfnCreateFakeClient	= nullptr,
		.pfnRunPlayerMove		= nullptr,
		.pfnNumberOfEntities	= nullptr,

		.pfnGetInfoKeyBuffer	= nullptr,
		.pfnInfoKeyValue		= nullptr,
		.pfnSetKeyValue			= nullptr,
		.pfnSetClientKeyValue	= nullptr,

		.pfnIsMapValid		= nullptr,
		.pfnStaticDecal		= nullptr,
		.pfnPrecacheGeneric	= nullptr,
		.pfnGetPlayerUserId	= nullptr,
		.pfnBuildSoundMsg	= nullptr,
		.pfnIsDedicatedServer	= nullptr,
		.pfnCVarGetPointer	= nullptr,
		.pfnGetPlayerWONId	= nullptr,

		.pfnInfo_RemoveKey		= nullptr,
		.pfnGetPhysicsKeyValue	= nullptr,
		.pfnSetPhysicsKeyValue	= nullptr,
		.pfnGetPhysicsInfoString= nullptr,
		.pfnPrecacheEvent		= nullptr,
		.pfnPlaybackEvent		= nullptr,

		.pfnSetFatPVS		= nullptr,
		.pfnSetFatPAS		= nullptr,

		.pfnCheckVisibility	= nullptr,

		.pfnDeltaSetField			= nullptr,
		.pfnDeltaUnsetField			= nullptr,
		.pfnDeltaAddEncoder			= nullptr,
		.pfnGetCurrentPlayer		= nullptr,
		.pfnCanSkipPlayer			= nullptr,
		.pfnDeltaFindField			= nullptr,
		.pfnDeltaSetFieldByIndex	= nullptr,
		.pfnDeltaUnsetFieldByIndex	= nullptr,

		.pfnSetGroupMask			= nullptr,

		.pfnCreateInstancedBaseline	= nullptr,
		.pfnCvar_DirectSet			= nullptr,

		.pfnForceUnmodified			= nullptr,

		.pfnGetPlayerStats			= nullptr,

		.pfnAddServerCommand		= nullptr,

		// Added in SDK 2.2:
		.pfnVoice_GetClientListening	= nullptr,
		.pfnVoice_SetClientListening	= nullptr,

		// Added for HL 1109 (no SDK update):
		.pfnGetPlayerAuthId	= nullptr,

		// Added 2003/11/10 (no SDK update):
		.pfnSequenceGet							= nullptr,
		.pfnSequencePickSentence				= nullptr,
		.pfnGetFileSize							= nullptr,
		.pfnGetApproxWavePlayLen				= nullptr,
		.pfnIsCareerMatch						= nullptr,
		.pfnGetLocalizedStringLength			= nullptr,
		.pfnRegisterTutorMessageShown			= nullptr,
		.pfnGetTimesTutorMessageShown			= nullptr,
		.pfnProcessTutorMessageDecayBuffer		= nullptr,
		.pfnConstructTutorMessageDecayBuffer	= nullptr,
		.pfnResetTutorMessageDecayData			= nullptr,

		// Added Added 2005-08-11 (no SDK update)
		.pfnQueryClientCvarValue	= nullptr,
		// Added Added 2005-11-22 (no SDK update)
		.pfnQueryClientCvarValue2	= nullptr,
		// Added 2009-06-17 (no SDK update)
		.pfnEngCheckParm			= nullptr,
	};

	if (!pengfuncsFromEngine) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookEngineAPI' called with null 'pengfuncsFromEngine' parameter.");
		return false;
	}
	else if (*interfaceVersion != ENGINE_INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookEngineAPI' called with version mismatch. Expected: %d, receiving: %d.", ENGINE_INTERFACE_VERSION, *interfaceVersion);

		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return false;
	}

	memcpy(pengfuncsFromEngine, &meta_engfuncs, sizeof(enginefuncs_t));
	return true;
}

// Must provide at least one of these..
inline constexpr META_FUNCTIONS gMetaFunctionTable =
{
	.pfnGetEntityAPI			= nullptr,						// HL SDK; called before game DLL
	.pfnGetEntityAPI_Post		= nullptr,						// META; called after game DLL
	.pfnGetEntityAPI2			= &HookGameDLLExportedFn,		// HL SDK2; called before game DLL
	.pfnGetEntityAPI2_Post		= &HookGameDLLExportedFn_Post,	// META; called after game DLL
	.pfnGetNewDLLFunctions		= nullptr,						// HL SDK2; called before game DLL
	.pfnGetNewDLLFunctions_Post	= nullptr,						// META; called after game DLL
	.pfnGetEngineFunctions		= &HookEngineAPI,				// META; called before HL engine
	.pfnGetEngineFunctions_Post	= nullptr,						// META; called after HL engine
};

// Metamod requesting info about this plugin:
//  ifvers			(given) interface_version metamod is using
//  pPlugInfo		(requested) struct with info about plugin
//  pMetaUtilFuncs	(given) table of utility functions provided by metamod
int Meta_Query(const char *pszInterfaceVersion, plugin_info_t const **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs) noexcept
{
	*pPlugInfo = PLID;
	gpMetaUtilFuncs = pMetaUtilFuncs;

	return true;
}
static_assert(std::same_as<decltype(&Meta_Query), META_QUERY_FN>);

// Metamod attaching plugin to the server.
//  now				(given) current phase, ie during map, during changelevel, or at startup
//  pFunctionTable	(requested) table of function tables this plugin catches
//  pMGlobals		(given) global vars from metamod
//  pGamedllFuncs	(given) copy of function tables from game dll
int Meta_Attach(PLUG_LOADTIME iCurrentPhase, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs) noexcept
{
	if (!pMGlobals) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'Meta_Attach' called with null 'pMGlobals' parameter.");
		return false;
	}

	gpMetaGlobals = pMGlobals;

	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'Meta_Attach' called with null 'pFunctionTable' parameter.");
		return false;
	}

	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
	gpGamedllFuncs = pGamedllFuncs;
	return true;
}
static_assert(std::same_as<decltype(&Meta_Attach), META_ATTACH_FN>);

// Metamod detaching plugin from the server.
// now		(given) current phase, ie during map, etc
// reason	(given) why detaching (refresh, console unload, forced unload, etc)
int Meta_Detach(PLUG_LOADTIME iCurrentPhase, PL_UNLOAD_REASON iReason) noexcept
{
	return true;
}
static_assert(std::same_as<decltype(&Meta_Detach), META_DETACH_FN>);
