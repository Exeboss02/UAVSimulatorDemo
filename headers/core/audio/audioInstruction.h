#pragma once
#include "al.h"
#include "alc.h"
#include "sndfile.h"

/// <summary>
/// This should not be used! will be removed soon
/// </summary>
struct AudioInstruction
{
	bool loopSound;
	float loopNrOfTimes; //0 = loops until something else tells it not to

	bool fadeIn;
	float fadeInTime;

	bool fadeOut;
	float fadeOutTime;
};