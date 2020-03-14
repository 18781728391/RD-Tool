// Basic reactive drop src not included.
// @AutoGavy 2020.3.3

#ifdef CLIENT_DLL
	static float g_flRandomTimeScale = 0.3f;
	static int g_iReloadCount = 0;
#endif

void ...()
{
#ifdef CLIENT_DLL
	switch (this->Classify())
	{
	case CLASS_ASW_PDW:	case CLASS_ASW_SHOTGUN:	case CLASS_ASW_RAILGUN:
	case CLASS_ASW_DEAGLE:	case CLASS_ASW_PISTOL:	case CLASS_ASW_SNIPER_RIFLE:
		if (!g_iReloadCount)
		{
			if (UsesClipsForAmmo1() && m_bInReload)
			{
				if (asw_fast_reload_enabled.GetBool() &&
					gpGlobals->curtime >= m_fFastReloadStart + (m_fFastReloadEnd - m_fFastReloadStart) * g_flRandomTimeScale &&
					gpGlobals->curtime <= m_fFastReloadEnd)
				{
					engine->ClientCmd("+reload");
					g_flRandomTimeScale = random->RandomInt(0, 6) * 0.1f;
					g_iReloadCount++;
				}
			}
		}
		else if (g_iReloadCount > 1)
		{
			engine->ClientCmd("-reload");
			g_iReloadCount = 0;
		}
		else
			g_iReloadCount++;
		break;
	}
#endif
}
