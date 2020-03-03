// Because of this, no longer using rd_networked_mouse 0 now.
// Basic reactive drop src not included.
// @AutoGavy 2020.2.24

#include "windows.h"
#include "asw_util_shared.h"
#include "asw_trace_filter.h"

#define PI 3.14159265358979

bool bIsKeyPressed = false;
bool bShouldSetCursor = false;

extern ConVar asw_cam_marine_dist;
extern ConVar asw_cam_marine_shift_enable;

void ...()
{
	C_ASW_Player* pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if (!pPlayer)
		return;

	if (GetAsyncKeyState(VK_KEY) & 0x8000)	// set up a hotkey to enable / disable autoaiming
	{
		if (!bIsKeyPressed && pPlayer->GetSteamID().ConvertToUint64() == I64U)
		{
			bShouldSetCursor = bShouldSetCursor ? false : true;
			bIsKeyPressed = true;
			if (bShouldSetCursor)
			{
				// setting new view field
				asw_cam_marine_dist.SetValue(512);
				asw_cam_marine_shift_enable.SetValue(0);
				Msg(">>> AutoAim Enabled <<<\n");
			}
			else
			{
				// reset settings as well
				asw_cam_marine_dist.SetValue(412);
				asw_cam_marine_shift_enable.SetValue(1);
				Msg(">>> AutoAim Disabled <<<\n");
			}
		}
	}
	else
		bIsKeyPressed = false;

	if (bShouldSetCursor)
	{
		// verify that we have input.
		Assert(ASWInput() != NULL);

		C_ASW_Marine* pMarine = pPlayer->GetMarine();
		C_ASW_Weapon *pWeapon = pMarine ? pMarine->GetActiveASWWeapon() : NULL;
		if (!pWeapon)
			return;
		float flWeaponRadiusScale = pWeapon ? pWeapon->GetAutoAimRadiusScale() : 1.0f;

		// our source firing point
		Vector marineScreenPos, alienScreenPos, bestAlienScreenPos;
		Vector vWorldSpaceCameraToCursor;
		IASW_Client_Aim_Target* pAutoAimEnt = NULL;
		IASW_Client_Aim_Target* pBestAlien = NULL;
		IASW_Client_Aim_Target* pHighlightAlien = NULL;  // this the ent our cursor is over
		float flBestAlienRadius = 0;
		bool bPreferFlatAiming = true;
		int omx, omy;
		Vector vCameraLocation;
		QAngle cameraAngle;
		Vector vTraceEnd;
		int nTraceMask = (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE);		// CONTENTS_PLAYERCLIP
		trace_t tr;

		ASWInput()->ASW_GetCameraLocation(pPlayer, vCameraLocation, cameraAngle, omx, omy, false);

		// get screen size
		float mx, my;
		int x, y;
		engine->GetScreenSize(x, y);

		// get current cursor position; bad way, should get central coord of this screen
		/*
		POINT pCursorPos;
		GetCursorPos(&pCursorPos);
		int current_posx = pCursorPos.x;
		int current_posy = pCursorPos.y;
		*/
		int current_posx = x / 2;
		int current_posy = y / 2;

		mx = current_posx - x;
		my = current_posy - y;

		Vector X, Y, Z;
		float mx_ratio = ((float)mx) / ((float)x);
		float my_ratio = ((float)my) / ((float)y);
		float fRatio = float(ScreenHeight()) / float(ScreenWidth());
		float FOVAngle = pPlayer->GetFOV();
		AngleVectors(cameraAngle, &X, &Y, &Z);
		vWorldSpaceCameraToCursor = X
			- tanf(FOVAngle*PI / 180 * 0.5) * 2 * Y * (-mx_ratio * 0.5f)* (0.75f / fRatio)
			+ tanf(FOVAngle*PI / 180 * 0.5) * 2 * Z * (-my_ratio * 0.5f)*  0.75f;

		vWorldSpaceCameraToCursor.NormalizeInPlace();
		vTraceEnd = vCameraLocation + vWorldSpaceCameraToCursor * ASW_MAX_AIM_TRACE;

		// BenLubar(sd2-ceiling-ents): use CASW_Trace_Filter to handle *_asw_fade properly
		CASW_Trace_Filter filter(pMarine, COLLISION_GROUP_NONE);
		// do a trace into the world to see what we've pointing directly at
		UTIL_TraceLine(vCameraLocation, vTraceEnd, nTraceMask, &filter, &tr);
		if (tr.fraction >= 1.0f)
		{
			if (!pMarine)
				return;
			float flFloorZ = (pMarine->GetRenderOrigin() + Vector(0, 0, ASW_MARINE_GUN_OFFSET_Z)).z;
		}


		if (!pMarine)
		{
			// no marine, just use the trace spot
			return;
		}

		Vector vecMarineWeaponPos = pMarine->GetRenderOrigin() + Vector(0, 0, ASW_MARINE_GUN_OFFSET_Z);
		float fFloorZ = vecMarineWeaponPos.z;	// normally aim flat

		// if we're clicking right on something
		if (tr.m_pEnt && tr.m_pEnt != pMarine)
		{
			// store if we're clicking right on an aim target
			IASW_Client_Aim_Target* pPossibleAimEnt = dynamic_cast<IASW_Client_Aim_Target*>(tr.m_pEnt);
			if (pPossibleAimEnt && pPossibleAimEnt->IsAimTarget())
			{
				pBestAlien = pPossibleAimEnt;
				debugoverlay->ScreenPosition(pBestAlien->GetAimTargetPos(vecMarineWeaponPos, bPreferFlatAiming), bestAlienScreenPos);	// tr.m_pEnt->WorldSpaceCenter()			
			}
		}

		// go through possible aim targets and see if we're pointing the cursor in the direction of any	
		float best_d = -1;
		debugoverlay->ScreenPosition(vecMarineWeaponPos, marineScreenPos);	// asw should be hacked gun pos?

		// if we have our cursor over a target (and not using the controller), make sure we have LOS to him before we continue
		if (!ASWInput()->ControllerModeActive() && pBestAlien)
		{
			// check we have LOS to the target
			CTraceFilterLOS traceFilter(pMarine, COLLISION_GROUP_NONE);
			trace_t tr2;
			UTIL_TraceLine(vecMarineWeaponPos, pBestAlien->GetAimTargetRadiusPos(vecMarineWeaponPos), MASK_OPAQUE, &traceFilter, &tr2);
			C_BaseEntity *pEnt = pBestAlien->GetEntity();
			bool bHasLOS = (!tr2.startsolid && (tr2.fraction >= 1.0 || tr2.m_pEnt == pEnt));
			// we can't shoot it, so skip autoaiming to it, but still return it as an entity that we want to highlight
			if (!bHasLOS)
			{
				pHighlightAlien = pBestAlien;
				pBestAlien = NULL;
			}
		}

		if (!pBestAlien && !pHighlightAlien)		// if we don't already have an ideal aim target from above, do the loop
		{
			for (int i = 0; i < IASW_Client_Aim_Target::AutoList().Count(); i++)
			{
				IASW_Client_Aim_Target *pAimTarget = static_cast< IASW_Client_Aim_Target* >(IASW_Client_Aim_Target::AutoList()[i]);
				C_BaseEntity *pEnt = pAimTarget->GetEntity();
				if (!pEnt || !pAimTarget->IsAimTarget())
					continue;
				// check it isn't attached to our marine (infesting parasites)
				if (pEnt->GetMoveParent() == pMarine)
					continue;

				// autoaiming: skip yourself
				if (pEnt == pMarine)
					continue;
				// check he's in range
				Vector vecAlienPos = pAimTarget->GetAimTargetRadiusPos(vecMarineWeaponPos); //pEnt->WorldSpaceCenter();
				if (vecAlienPos.DistToSqr(vecMarineWeaponPos) > ASW_MAX_AUTO_AIM_RANGE)
					continue;

				Vector vDirection = vecAlienPos - vecMarineWeaponPos;
				float fZDist = vDirection.z;
				if (fZDist > 250.0f)
				{
					VectorNormalize(vDirection);
					if (vDirection.z > 0.85f)
					{
						// Don't aim at stuff that's high up and directly above us
						continue;
					}
				}

				debugoverlay->ScreenPosition(vecAlienPos, alienScreenPos);

				Vector AlienEdgeScreenPos;
				float flRadiusScale = ASWInput()->ControllerModeActive() ? 2.0f : 1.0f;
				flRadiusScale *= flWeaponRadiusScale * 2.0f;		// autoaiming: radius of cursor
				debugoverlay->ScreenPosition(vecAlienPos + Vector(pAimTarget->GetRadius() * flRadiusScale, 0, 0), AlienEdgeScreenPos);
				float alien_radius = (alienScreenPos - AlienEdgeScreenPos).Length2D();
				if (alien_radius <= 0)
					continue;

				float intersect1, intersect2;
				Vector2D LineDir(omx - marineScreenPos.x, omy - marineScreenPos.y);
				LineDir.NormalizeInPlace();

				if (ASW_LineCircleIntersection(Vector2D(alienScreenPos.x, alienScreenPos.y),	// center
					alien_radius,															// radius
					Vector2D(marineScreenPos.x, marineScreenPos.y),							// line start
					LineDir,																	// line direction
					&intersect1, &intersect2))
				{
					// midpoint of intersection is closest to circle center
					float midintersect = (intersect1 + intersect2) * 0.5f;
					if (midintersect > 0)
					{
						//Vector2D Midpoint = Vector2D(marineScreenPos.x, marineScreenPos.y) + LineDir * midintersect;
						//Midpoint -= Vector2D(alienScreenPos.x, alienScreenPos.y);
						// use how near we are to the center to prioritise aim target
						//float dist = Midpoint.LengthSqr();

						// we are now prioritizing enemies which are closer to the player that intersect the trace
						float dist = midintersect;
						if (dist < best_d || best_d == -1)
						{
							// check we have LOS to the target
							CTraceFilterLOS traceFilter(pMarine, COLLISION_GROUP_NONE);
							trace_t tr2;
							UTIL_TraceLine(vecMarineWeaponPos, pAimTarget->GetAimTargetRadiusPos(vecMarineWeaponPos), MASK_OPAQUE, &traceFilter, &tr2);
							bool bHasLOS = (!tr2.startsolid && (tr2.fraction >= 1.0 || tr2.m_pEnt == pEnt));
							if (!bHasLOS)
							{
								// just skip aliens that we can't shoot to
								continue;
								//dist += 50.0f;		// bias against aliens that we don't have LOS to - we'll only aim up/down at them if we have no other valid targets
							}

							if (dist < best_d || best_d == -1)
							{
								best_d = dist;
								pBestAlien = pAimTarget;
								bestAlienScreenPos = alienScreenPos;
								flBestAlienRadius = alien_radius;
							}
						}
					}
				}
			}
		}

		// we found something to aim at
		if (pBestAlien)
		{
			fFloorZ = pBestAlien->GetAimTargetPos(vecMarineWeaponPos, bPreferFlatAiming).z;
			pAutoAimEnt = pBestAlien;
		}

		// in controller mode, if we have an alien to autoaim at, then aim directly at it
		if (ASWInput()->ControllerModeActive() && pBestAlien)
			pAutoAimEnt = pBestAlien;

		// calculate where our trace direction intersects with a simulated floor
		if (vWorldSpaceCameraToCursor.z != 0)
		{
			pPlayer->SmoothAimingFloorZ(fFloorZ);

			// just do a little bit of additional x/y auto aim
			if (pBestAlien && pBestAlien->GetEntity() && pBestAlien->GetEntity()->IsNPC())
			{
				Vector vecBestAlienPos = pBestAlien->GetAimTargetRadiusPos(vecMarineWeaponPos);
				if (vecBestAlienPos.DistToSqr(vecMarineWeaponPos) <= ASW_MAX_AUTO_AIM_RANGE)
				{
					Vector vecAlienScreenPos;
					Vector2D finalAlienScreenPos;
					debugoverlay->ScreenPosition(vecBestAlienPos, vecAlienScreenPos);

					finalAlienScreenPos.Init(vecAlienScreenPos.x, vecAlienScreenPos.y);
					if (!finalAlienScreenPos.IsZero())
						SetCursorPos(finalAlienScreenPos.x, finalAlienScreenPos.y);
				}
			}
		}
	}
}
