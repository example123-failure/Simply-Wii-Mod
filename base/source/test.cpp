#include "../include/base.h"
#include <gccore.h>
#include <stdio.h>

int main()
{
	SWM_Config config;
	config.EnableConsole = true;
	config.EnableGX = false;
	config.UseDoubleBuffering = false;


	if (!SWM_Init(config))
	{
		return 1;
	}

	if (SWM_GetState() != SWM_State::ConsoleReady)
	{
		return 2;
	}


	printf("Simply Wii Mod is working!\n");

	while (true)
	{
		VIDEO_WaitVSync();
	}
	return 0;
}