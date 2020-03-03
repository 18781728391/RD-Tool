// Basic reactive drop src not included.
// @AutoGavy 2020.3.3

#ifdef CLIENT_DLL
	int iAutoReloadCounter = 0;
#endif

void ...()
{
#ifdef CLIENT_DLL
	if (iAutoReloadCounter < 2)
	{
		if (asw_fast_reload_enabled.GetBool() && this->Classify() == CLASS_ASW_RAILGUN && gpGlobals->curtime >= m_fFastReloadStart && gpGlobals->curtime <= m_fFastReloadEnd)
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
#endif
}
