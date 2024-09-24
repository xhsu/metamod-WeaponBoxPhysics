import std;

import eiface;
import engine_api;
import meta_api;

/*

Nagi:

pfnCmdStart
SV_PlayerRunPreThink
SV_PlayerRunThink
SV_AddLinksToPM
pfnPM_Move Post 修改成SOLID_TRIGGER，SetOrigin - 因为在这里触发的玩家touch
pfnPlayerPostThink Pre只恢复SOLID
pfnCmdEnd

*/

META_RES fw_PM_Move(playermove_s* ppmove, qboolean server) noexcept
{
	if (ppmove->spectator)
		return MRES_IGNORED;

	static std::vector<size_t> rgiPhysEntsIndex{};
	rgiPhysEntsIndex.clear();
	//rgiPhysEntsIndex.reserve(MAX_PHYSENTS);	// 'reserve' have a good use only in the very first call

	for (size_t i = 0; i < (size_t)ppmove->numphysent; ++i)
	{
		[[unlikely]]
		if (ppmove->physents[i].info == 0)	// entindex == 0 is the WORLD, mate.
		{
			rgiPhysEntsIndex.emplace_back(i);
			continue;
		}

		if (ppmove->physents[i].fuser4 == 9527.f)
			continue;

		rgiPhysEntsIndex.emplace_back(i);
	}

	for (size_t i = 0; i < rgiPhysEntsIndex.size(); ++i)
	{
		if (i != rgiPhysEntsIndex[i])
			ppmove->physents[i] = ppmove->physents[rgiPhysEntsIndex[i]];
	}

	ppmove->numphysent = std::ssize(rgiPhysEntsIndex);
	return MRES_HANDLED;
}

void fw_PM_Move_Post(playermove_s* ppmove, qboolean server) noexcept
{
	std::span all_entities{ g_engfuncs.pfnPEntityOfEntIndex(0), (size_t)gpGlobals->maxEntities };

	for (auto&& ent : all_entities)
	{
		if (ent.v.fuser4 == 9527.f)
		{
			ent.v.solid = SOLID_TRIGGER;
			g_engfuncs.pfnSetOrigin(&ent, ent.v.origin);	// Purpose: Calling SV_LinkEdict(e, false)
		}
	}
}

META_RES fw_PlayerPostThink(edict_t*) noexcept
{
	std::span all_entities{ g_engfuncs.pfnPEntityOfEntIndex(0), (size_t)gpGlobals->maxEntities };

	for (auto&& ent : all_entities)
	{
		if (ent.v.fuser4 == 9527.f)
		{
			ent.v.solid = SOLID_BBOX;
			g_engfuncs.pfnSetOrigin(&ent, ent.v.origin);
		}
	}

	return MRES_HANDLED;
}

qboolean fw_AddToFullPack_Post(entity_state_t* pState, int iEntIndex, edict_t* pEdict, edict_t* pClientSendTo, qboolean cl_lw, qboolean bIsPlayer, unsigned char* pSet) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	if (pClientSendTo->v.deadflag != DEAD_NO)
		return false;

	if (pEdict->v.fuser4 != 9527.f)
		return false;

	pState->solid = SOLID_NOT;

	return false;
}

void fw_Touch_Post(edict_t* pentTouched, edict_t* pentOther) noexcept
{
	// Purpose: resolve the 'stuck in the air' situation.

	if (pentTouched->v.classname != pentOther->v.classname)
		return;

	if (strcmp(STRING(pentTouched->v.classname), "weaponbox"))
		return;

	if ((pentTouched->v.flags & FL_ONGROUND) && (pentOther->v.flags & FL_ONGROUND))
		return;

	Vector vecVel{
		g_engfuncs.pfnRandomFloat(-1.f, 1.f),
		g_engfuncs.pfnRandomFloat(-1.f, 1.f),
		g_engfuncs.pfnRandomFloat(-0.5f, 1.f),
	};
	vecVel = vecVel.Normalize() * g_engfuncs.pfnRandomFloat(50.f, 60.f);

	pentTouched->v.velocity = vecVel;
	pentOther->v.velocity = -vecVel;
}
