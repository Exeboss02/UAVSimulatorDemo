#pragma once
#include "al.h"
#include "alc.h"
#include <string>

struct SoundClip
{
	SoundClip();
	~SoundClip();

    std::string id = "";
	std::string filepath = "";
	ALuint buffer = 0;

    //delete copy constructor to avoid copying
	SoundClip(const SoundClip&) = delete;
	SoundClip& operator=(const SoundClip&) = delete;
};