# Simply-Wii-Mod
Simply Wii Mod is a High level wrapper for GX and Wii Video initilization. I really just was annoyed at the amount of settings/code i had to do, so I automated it--while still providing access to low level control.                  AS OF 10/15/2026 THIS IS NOT FINISHED



# Finished Product
The Finished version is meant to include simplified coding of the most of the <GCCORE> libary, I'm still new at C++ so this is gonna take me a while (:<

The goal is to be able to write something like this:

<sub>
#include <simplywiimod/base.h>
#include <simplywiimod/gx.h>
#include <simplywiimod/input.h>

int main()
{
    SWM.Init();

   while (SWM.Running())
    {
        SWM.PollEvents();

        SWM.Begin();
        SWM.DrawCube(0.0f, 0.0f, -5.0f);
        SWM.End();

        SWM.SwapBuffer();
    }

    SWM.Shutdown();
}
 </sub>
 
Instead of this.  ):

<sub>
// adapted from	the	original acube demo	by tkcne.

// enjoy

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

GXRModeObj	*screenMode;
static void	*frameBuffer;
static vu8	readyForCopy;
#define	FIFO_SIZE (256*1024)

s16	vertices[] ATTRIBUTE_ALIGN(32) = {
	0, 15, 0,
	-15, -15, 0,
	15,	-15, 0};

u8 colors[]	ATTRIBUTE_ALIGN(32)	= {
	255, 0,	0, 255,		// red
	0, 255,	0, 255,		// green
	0, 0, 255, 255};	// blue

void update_screen(Mtx viewMatrix);
static void	copy_buffers(u32 unused);

int	main(void) {
	Mtx	view;
	Mtx44	projection;
	GXColor	backgroundColor	= {0, 0, 0,	255};
	void *fifoBuffer = NULL;

	VIDEO_Init();
	WPAD_Init();

	screenMode = VIDEO_GetPreferredMode(NULL);

	frameBuffer	= MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));

	VIDEO_Configure(screenMode);
	VIDEO_SetNextFramebuffer(frameBuffer);
	VIDEO_SetPostRetraceCallback(copy_buffers);
	VIDEO_SetBlack(false);
	VIDEO_Flush();



	fifoBuffer = MEM_K0_TO_K1(memalign(32,FIFO_SIZE));
	memset(fifoBuffer,	0, FIFO_SIZE);

	GX_Init(fifoBuffer, FIFO_SIZE);
	GX_SetCopyClear(backgroundColor, 0x00ffffff);
	GX_SetViewport(0,0,screenMode->fbWidth,screenMode->efbHeight,0,1);
	GX_SetDispCopyYScale((f32)screenMode->xfbHeight/(f32)screenMode->efbHeight);
	GX_SetScissor(0,0,screenMode->fbWidth,screenMode->efbHeight);
	GX_SetDispCopySrc(0,0,screenMode->fbWidth,screenMode->efbHeight);
	GX_SetDispCopyDst(screenMode->fbWidth,screenMode->xfbHeight);
	GX_SetCopyFilter(screenMode->aa,screenMode->sample_pattern,
					 GX_TRUE,screenMode->vfilter);
	GX_SetFieldMode(screenMode->field_rendering,
					((screenMode->viHeight==2*screenMode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer,GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	guVector camera =	{0.0F, 0.0F, 0.0F};
	guVector up =	{0.0F, 1.0F, 0.0F};
	guVector look	= {0.0F, 0.0F, -1.0F};

	guPerspective(projection, 60, (CONF_GetAspectRatio() == CONF_ASPECT_16_9) ? 16.0F/9.0F : 4.0F/3.0F, 10.0F,	300.0F);
	GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
	GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX8);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,	GX_POS_XYZ,	GX_S16,	0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8,	0);
	GX_SetArray(GX_VA_POS, vertices, 3*sizeof(s16));
	GX_SetArray(GX_VA_CLR0,	colors,	4*sizeof(u8));
	GX_SetNumChans(1);
	GX_SetNumTexGens(0);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

	while(SYS_MainLoop())
	{
		guLookAt(view, &camera,	&up, &look);
		GX_SetViewport(0,0,screenMode->fbWidth,screenMode->efbHeight,0,1);
		GX_InvVtxCache();
		GX_InvalidateTexAll();
		update_screen(view);

		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(0);
	}
	return 0;
}

void update_screen(	Mtx	viewMatrix )
{
	Mtx	modelView;

	guMtxIdentity(modelView);
	guMtxTransApply(modelView, modelView, 0.0F,	0.0F, -50.0F);
	guMtxConcat(viewMatrix,modelView,modelView);

	GX_LoadPosMtxImm(modelView,	GX_PNMTX0);

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

static void	copy_buffers(u32 count __attribute__ ((unused)))
{
	if (readyForCopy==GX_TRUE) {
		GX_SetZMode(GX_TRUE, GX_LEQUAL,	GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer,GX_TRUE);
		GX_Flush();
		readyForCopy = GX_FALSE;
	}
}




 </sub>





Artificial Intelligence Disclaimer:

Some Artificial Intelligence was used in this project for the following purposes:

Debugging (WITHOUT EDITING)
Structure ideas (like how it should be structured)

a small note on AI:

(Summary) a small amount of AI was used in this project, but not for directly editing it, only for LEARNING purposes. Read more below.

I personally have a generally negative option on AI, and sometimes feel guilty when using it, however, I try to at least code
the crap myself instead of letting it do it all for me, I want to learn ts, not have AI do it for me. I'm sorry to those who decide not to use this because they heard AI was used in it at all. I'm from Python, Unity C#, and learning C++ wasn't too different than C#, but I do apologize for any small code sections, I try to make sure AI has the least amount of work in my project, but as a standard for this project, I never let AI directly edit it, while using it, I did type out what it did say (rarely), BUT!! I would argue that Typing it out myself is really more than just copying and pasting it, (a quick note, most sections used the little amount of tutorials from this website: http://www.codemii.com/category/wii-programming-tutorials/) I used AI for certain parts because of the little amount of documentation on GX, and Wii modding coding. And hey techincally I wrote it myself because I WASN'T copy and pasting, it's almost like a tutorial but personalized. 

As a final note, I'm sorry I just suck. and kinda had to use ai (smal amount). sorry bros.

I really wish I didn't have to use any AI, I hate it. But at the same time I depend on it too much.   

