#include <iostream>
#include "../include/base.h"

#include <stdlib.h>
#include <malloc.h>
#include <gccore.h>
#include <ogc/console.h>




namespace
{
	// Mark it as Uninitialized
	SWM_State current_state = SWM_State::Uninitialized;

	// Da common wii mod crap varables at the start
	GXRModeObj* g_rmode = nullptr;
	void* g_frameBuffers[2] = { nullptr, nullptr };
	unsigned int g_frameBufferCount = 0;
}


// Get da state of Simply Wii Mod libary
SWM_State SWM_GetState()
{
	return current_state;
}


bool SWM_Init(const SWM_Config& config)
{


	// PREVENT from Initializing multiple times

	if (current_state != SWM_State::Uninitialized)
	{
		return false;
	}
		

	// Either Console mode, or GX mode must be enabled. As of now you cannot switch.

	if (config.EnableConsole == config.EnableGX)
	{
		// Console or GX only PROGRAM FAIL!
		return false;
	}




	current_state = SWM_State::Initializing;

	// Shared Video Initialization :)
	VIDEO_Init();

	g_rmode = VIDEO_GetPreferredMode(nullptr);

	if (g_rmode == nullptr)
	{
		current_state = SWM_State::Failed;
		return false;
	}

	g_frameBufferCount = (config.EnableGX && config.UseDoubleBuffering) ? 2 : 1;

	for (unsigned int i = 0; i < g_frameBufferCount; i++)
	{
		void* buffer = SYS_AllocateFramebuffer(g_rmode);

		if (buffer == nullptr)
		{
			current_state = SWM_State::Failed;
			return false;
		}

		g_frameBuffers[i] = MEM_K0_TO_K1(buffer);
	}





	// If console mode is on, ENABLE THE FRICKING CONSOLE DUDE.. GOSH

	if (config.EnableConsole)
	{
		console_init(g_frameBuffers[0], 20, 20, g_rmode->fbWidth, g_rmode->xfbHeight, g_rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	}

	VIDEO_Configure(g_rmode);

	VIDEO_SetNextFramebuffer(g_frameBuffers[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();


	if (g_rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	if (config.EnableConsole)
	{
		current_state = SWM_State::ConsoleReady;
		return true;
	}


	current_state = SWM_State::VideoReadyForGX;
	return true;
}


/*   CODE FOR FUTURE REFERENCE..
	// Set Varables here. These are the High level settings that the user should configure

	bool EnableConsole = false;
	bool EnableGX = true;




	bool UseDoubleBuffering = true;
	SWM_VideoMode VideoMode = SWM_VideoMode::Auto;




	// Real initizlization NOT JUST VARABLES

	if (EnableConsole)
	{

		// Console INIT
		static u32* xfb;
		static GXRModeObj* rmode;


		
		WPAD_Init();

		rmode = VIDEO_GetPreferredMode(NULL);
		xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));


		console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

		VIDEO_Configure(rmode);
		VIDEO_SetNextFramebuffer(xfb);
		VIDEO_SetBlack(FALSE);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
	}


	// GX version init
	else if (EnableConsole == false && EnableGX)
	{

		// Video settings
		GXRModeObj* screenMode;
		static void* frameBuffer;
		static vu8	readyForCopy;
		#define	FIFO_SIZE (256*1024)

		
		WPAD_Init();

		screenMode = VIDEO_GetPreferredMode(NULL);

		frameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));

		VIDEO_Configure(screenMode);
		VIDEO_SetNextFramebuffer(frameBuffer);
		VIDEO_SetPostRetraceCallback(copy_buffers);
		VIDEO_SetBlack(false);
		VIDEO_Flush();


		fifoBuffer = MEM_K0_TO_K1(memalign(32, FIFO_SIZE));
		memset(fifoBuffer, 0, FIFO_SIZE);

		GX_Init(fifoBuffer, FIFO_SIZE);
		GX_SetCopyClear(backgroundColor, 0x00ffffff);
		GX_SetViewport(0, 0, screenMode->fbWidth, screenMode->efbHeight, 0, 1);
		GX_SetDispCopyYScale((f32)screenMode->xfbHeight / (f32)screenMode->efbHeight);
		GX_SetScissor(0, 0, screenMode->fbWidth, screenMode->efbHeight);
		GX_SetDispCopySrc(0, 0, screenMode->fbWidth, screenMode->efbHeight);
		GX_SetDispCopyDst(screenMode->fbWidth, screenMode->xfbHeight);
		GX_SetCopyFilter(screenMode->aa, screenMode->sample_pattern,
			GX_TRUE, screenMode->vfilter);
		GX_SetFieldMode(screenMode->field_rendering,
			((screenMode->viHeight == 2 * screenMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

		GX_SetCullMode(GX_CULL_NONE);
		GX_CopyDisp(frameBuffer, GX_TRUE);
		GX_SetDispCopyGamma(GX_GM_1_0);

		guVector camera = { 0.0F, 0.0F, 0.0F };
		guVector up = { 0.0F, 1.0F, 0.0F };
		guVector look = { 0.0F, 0.0F, -1.0F };

		guPerspective(projection, 60, (CONF_GetAspectRatio() == CONF_ASPECT_16_9) ? 16.0F / 9.0F : 4.0F / 3.0F, 10.0F, 300.0F);
		GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
		GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
		GX_SetArray(GX_VA_POS, vertices, 3 * sizeof(s16));
		GX_SetArray(GX_VA_CLR0, colors, 4 * sizeof(u8));
		GX_SetNumChans(1);
		GX_SetNumTexGens(0);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
		GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	}

	std::cout << "SimplyWiiMod initialized!\n";
}

void update_screen(Mtx	viewMatrix)
{
	Mtx	modelView;

	guMtxIdentity(modelView);
	guMtxTransApply(modelView, modelView, 0.0F, 0.0F, -50.0F);
	guMtxConcat(viewMatrix, modelView, modelView);

	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);

	GX_Position1x8(0);
	GX_Color1x8(0);
	GX_Position1x8(1);
	GX_Color1x8(1);
	GX_Position1x8(2);
	GX_Color1x8(2);

	GX_End();

	GX_DrawDone();
	readyForCopy = GX_TRUE;

	VIDEO_WaitVSync();
	return;
}

static void	copy_buffers(u32 count __attribute__((unused)))
{
	if (readyForCopy == GX_TRUE) {
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer, GX_TRUE);
		GX_Flush();
		readyForCopy = GX_FALSE;
	}
}
*/
