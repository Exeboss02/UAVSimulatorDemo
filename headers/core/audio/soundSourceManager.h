#pragma once
#include <vector>

#include "soundSourceObject.h"

/// <summary>
/// WARNING! Should probably not be used, GameObject3D's should manage their own sound sources instead
/// </summary>
class SoundSourceManager
{
public:
	SoundSourceManager() = default;
	~SoundSourceManager() = default;

	SoundSourceObject* GetSoundSource(int id);
	void AddSoundSource(SoundSourceObject* speaker);
	void SetListenerPosition(float x, float y, float z);

private:
	int idCounter = 1;
	std::vector<SoundSourceObject*> soundSources;
};