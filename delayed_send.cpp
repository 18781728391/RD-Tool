// Basic reactive drop src not included.
// @AutoGavy 2020.3.4

#define CAMERA_DELAY <TIME>
#define CAMERA_DELAY_BUFFER CAMERA_DELAY * 2

static short g_iDelayScreenW[CAMERA_DELAY_BUFFER], g_iDelayScreenH[CAMERA_DELAY_BUFFER];
static short g_iDelayCameraX[CAMERA_DELAY_BUFFER], g_iDelayCameraY[CAMERA_DELAY_BUFFER];
static int g_iDelayCount = 0;

void ...()
{
	int mx, my;
	ASWInput()->GetFullscreenMousePos(&mx, &my);
	if (g_iDelayCount < CAMERA_DELAY)
	{
		g_iDelayScreenW[g_iDelayCount] = (short)ScreenWidth();
		g_iDelayScreenH[g_iDelayCount] = (short)ScreenHeight();
		cmd->screenw = (short)ScreenWidth();
		cmd->screenh = (short)ScreenHeight();

		g_iDelayCameraX[g_iDelayCount] = (short)mx;
		g_iDelayCameraY[g_iDelayCount] = (short)my;
		cmd->mousex = (short)mx;
		cmd->mousey = (short)my;
	}
	else if (g_iDelayCount < CAMERA_DELAY_BUFFER)
	{
		cmd->screenw = g_iDelayScreenW[g_iDelayCount - CAMERA_DELAY];
		cmd->screenh = g_iDelayScreenH[g_iDelayCount - CAMERA_DELAY];
		g_iDelayScreenW[g_iDelayCount - CAMERA_DELAY] = (short)ScreenWidth();
		g_iDelayScreenH[g_iDelayCount - CAMERA_DELAY] = (short)ScreenHeight();

		cmd->mousex = g_iDelayCameraX[g_iDelayCount - CAMERA_DELAY];
		cmd->mousey = g_iDelayCameraY[g_iDelayCount - CAMERA_DELAY];
		g_iDelayCameraX[g_iDelayCount - CAMERA_DELAY] = (short)mx;
		g_iDelayCameraY[g_iDelayCount - CAMERA_DELAY] = (short)my;
	}
	if (g_iDelayCount > CAMERA_DELAY_BUFFER - 2)
		g_iDelayCount = CAMERA_DELAY;
	g_iDelayCount++;
}
