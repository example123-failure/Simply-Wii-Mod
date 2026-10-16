#pragma once


enum class SWM_VideoMode
{
	Auto
};


struct SWM_Config
{
	bool EnableConsole = false;

	bool EnableGX = true;

	bool UseDoubleBuffering = true;

	SWM_VideoMode VideoMode = SWM_VideoMode::Auto;
};

enum class SWM_State
{
    Uninitialized,
    Initializing,
    ConsoleReady,
    VideoReadyForGX,
    GXReady,
    Failed
};

bool SWM_Init(const SWM_Config& config);

SWM_State SWM_GetState();