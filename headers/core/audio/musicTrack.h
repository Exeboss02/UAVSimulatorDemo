#pragma once
#include "al.h"
#include "alc.h"
#include <string>
#include <iostream>
#include "sndfile.h"
#include "audioInstruction.h"
#include "utilities/logger.h"
#include "utilities/time.h"
#include "utilities/masterVolume.h"
#include "core/tools.h"

class MusicTrack
{
public:
	MusicTrack();
	~MusicTrack();

	std::string filepath = "";
	std::string id = "";
	
	void Play();
	void Stop();
	void FadeIn(float startGain, float seconds);
	void FadeOut(float seconds);
	void Restart();

	bool Initialize(std::string filepath, std::string id);
	void UpdateBufferStream();
	bool LoadTrack();
	void SetPitch(float pitch);
	float GetGain();
	void SetGain(float gain);
	void SetAudioInstruction(AudioInstruction audioInstruction);
	void GetSourceState(ALint& sourceState);

	void ShouldLoop(bool shouldLoop);

private:
	ALuint source;
	ALint state = 0;
	AudioInstruction audioInstruction;
	float pitch = 1.0f;
	float startGain = 1.0f;
	float targetGain = 1.0f;
	float currentGain = 1.0f;
	float position[3] = { 0, 0, 0 };
	float velocity[3] = { 0, 0, 0 };

	float playTime = 0;
	float fadeInTime = 0;
	float currentFadeInTime = 0;
	float fadeOutTime = 0;
	float currentFadeOutTime = 0;

	bool shouldLoop = false;

	static const int BUFFER_SAMPLES = 8192;
	static const int NUM_BUFFERS = 4;
	ALuint buffers[NUM_BUFFERS];
	SNDFILE* sndfile = nullptr;
	SF_INFO sfInfo;
	short* membuf = nullptr;
	ALenum format;

	float* pMusicGain = nullptr;

	void SetTargetGain(float gain);
};