// Basic reactive drop src not included.
// @AutoGavy 2020.3.3

#ifdef CLIENT_DLL
	bool bAllowForceReload = true;
#endif

void ...()
{
#ifdef CLIENT_DLL
	if (bAllowForceReload && asw_fast_reload_enabled.GetBool() && this->Classify() == CLASS_ASW_RAILGUN)
	{
		if (gpGlobals->curtime >= m_fFastReloadStart && gpGlobals->curtime <= m_fFastReloadEnd)
		{
			engine->ClientCmd("+reload");
			bAllowForceReload = false;
		}
	}
#endif

	(...)
	{
#ifdef CLIENT_DLL
		engine->ClientCmd("-reload");
		bAllowForceReload = true;
#endif
	}
}
