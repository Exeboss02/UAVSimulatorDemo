#pragma once
#include "al.h"
#include "alc.h"
#include <string>
#include <iostream>
#include "sndfile.h"
#include "audioInstruction.h"
#include "soundClip.h"
#include "utilities/logger.h"
#include "utilities/masterVolume.h"
#include "gameObjects/gameObject3D.h"
#include <random>

class SoundSourceObject : public GameObject3D
{
public:
	SoundSourceObject();
	~SoundSourceObject();
	SoundSourceObject(const SoundSourceObject&) = delete;
	SoundSourceObject& operator=(const SoundSourceObject&) = delete;

	virtual void Tick() override;

	/// <summary>
	/// Starts playing a sound clip. Is a trigger and should be called once when it should start
	/// </summary>
	/// <param name="soundClip"></param>
	void Play(SoundClip* soundClip);

	/// <summary>
	/// Sets sound source volume
	/// </summary>
	/// <param name="gain"></param>
	void SetGain(float gain);

	/// <summary>
	/// Sets sound source pitch
	/// </summary>
	/// <param name="pitch"></param>
	void SetPitch(float pitch);

	/// <summary>
	/// BETA - Sets random pitch between min and max pitch
	/// </summary>
	/// <param name="minPitch"></param>
	/// <param name="maxPitch"></param>
	void SetRandomPitch(float minPitch, float maxPitch);

	/// <summary>
	/// Turns up or down the current picth
	/// </summary>
	/// <param name="pitchChange"></param>
	void ChangePitch(float pitchChange);

	/// <summary>
	/// Turns up or down the current volume
	/// </summary>
	/// <param name="gainChange"></param>
	void ChangeGain(float gainChange);

	/// <summary>
	/// WARNING! Should not be used and will be removed
	/// </summary>
	/// <param name="instructionSet"></param>
	void SetAudioInstruction(AudioInstruction instructionSet);

	/// <summary>
	/// Sets id for sound source, should not be used by developer!
	/// </summary>
	/// <param name="newId"></param>
	void SetId(int newId);

	/// <summary>
	/// Retrieves the sound source id
	/// </summary>
	/// <returns></returns>
	int GetId();

	/// <summary>
	/// ONLY FOR DEBUG - Retrieves the sound source state for sound source at INDEX within this sound source object (one sound source object has multiple sound sources)
	/// </summary>
	/// <param name="index"></param>
	/// <param name="sourceState"></param>
	void GetSourceState(int index, ALint& sourceState); //for debug

	/// <summary>
	/// Retrieves sound source position
	/// </summary>
	/// <param name="position"></param>
	void GetCurrentSourcePosition(ALfloat* position); //for debug

private:
	int id = -1;
	int nrOfSources = 6;
	int sourceIndex = 0;
	ALuint* sources;
	AudioInstruction currentInstructionSet;

	float pitch = 1;
	float gain = 1;
	float* masterGain = nullptr;
	float velocity[3] = { 0, 0, 0 };
};