// Basic reactive drop src not included.
// @AutoGavy 2020.3.3

#ifdef CLIENT_DLL
	int iAutoReloadCounter = 0;
#endif

void ...()
{
#ifdef CLIENT_DLL
	switch (this->Classify())
	{
	case CLASS_ASW_PDW:	case CLASS_ASW_SHOTGUN:	case CLASS_ASW_RAILGUN:
	case CLASS_ASW_DEAGLE:	case CLASS_ASW_PISTOL:	case CLASS_ASW_SNIPER_RIFLE:
		if (iAutoReloadCounter < 2 && UsesClipsForAmmo1() && m_bInReload)
		{
			if (asw_fast_reload_enabled.GetBool() && gpGlobals->curtime >= m_fFastReloadStart && gpGlobals->curtime <= m_fFastReloadEnd)
			{
				engine->ClientCmd("+reload");
				iAutoReloadCounter++;
			}
		}
		else
		{
			engine->ClientCmd("-reload");
			iAutoReloadCounter = 0;
		}
		break;
	}
#endif
}
