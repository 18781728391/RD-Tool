// Basic reactive drop src not included.
// @AutoGavy 2020.3.4

#define CAMERA_DELAY <TIME>
#define CAMERA_DELAY_BUFFER CAMERA_DELAY * 2

static short g_iDelayScreenW[CAMERA_DELAY_BUFFER], g_iDelayScreenH[CAMERA_DELAY_BUFFER];
static short g_iDelayCameraX[CAMERA_DELAY_BUFFER], g_iDelayCameraY[CAMERA_DELAY_BUFFER];
static int iDelayCount = 0;

void ...()
{
	int mx, my;
	ASWInput()->GetFullscreenMousePos(&mx, &my);
	if (iDelayCount < CAMERA_DELAY)
	{
		g_iDelayScreenW[iDelayCount] = (short)ScreenWidth();
		g_iDelayScreenH[iDelayCount] = (short)ScreenHeight();
		cmd->screenw = (short)ScreenWidth();
		cmd->screenh = (short)ScreenHeight();

		g_iDelayCameraX[iDelayCount] = (short)mx;
		g_iDelayCameraY[iDelayCount] = (short)my;
		cmd->mousex = (short)mx;
		cmd->mousey = (short)my;
	}
	else if (iDelayCount < CAMERA_DELAY_BUFFER)
	{
		cmd->screenw = g_iDelayScreenW[iDelayCount - CAMERA_DELAY];
		cmd->screenh = g_iDelayScreenH[iDelayCount - CAMERA_DELAY];
		g_iDelayScreenW[iDelayCount - CAMERA_DELAY] = (short)ScreenWidth();
		g_iDelayScreenH[iDelayCount - CAMERA_DELAY] = (short)ScreenHeight();

		cmd->mousex = g_iDelayCameraX[iDelayCount - CAMERA_DELAY];
		cmd->mousey = g_iDelayCameraY[iDelayCount - CAMERA_DELAY];
		g_iDelayCameraX[iDelayCount - CAMERA_DELAY] = (short)mx;
		g_iDelayCameraY[iDelayCount - CAMERA_DELAY] = (short)my;
	}
	if (iDelayCount > CAMERA_DELAY_BUFFER - 2)
		iDelayCount = CAMERA_DELAY;
	iDelayCount++;
}
