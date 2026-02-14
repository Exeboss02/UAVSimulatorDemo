#pragma once
#include "al.h"
#include "alc.h"
#include "sndfile.h"
#include <iostream>
#include "utilities/logger.h"
#include "core/audio/musicTrackManager.h"
#include "utilities/masterVolume.h"

//ALuint's can be safely copied according to AI

/// <summary>
/// Initializes sound system and manages resources for playing music
/// </summary>
class AudioManager
{
public:
	AudioManager();
	~AudioManager();

	void Tick();

	//MusicTrackManager Functions

	/// <summary>
	/// Takes path and sets as default music folder. Later you only need to put a filename when you load a music file. Always end path with /
	/// </summary>
	/// <param name="pathToMusicFolder"></param>
	void InitializeMusicTrackManager(std::string pathToMusicFolder);

	/// <summary>
	/// Loads and adds music track to bank from your standard music folder. Only put filename and an id for later identification
	/// </summary>
	/// <param name="filename"></param>
	/// <param name="id"></param>
	void AddMusicTrackStandardFolder(std::string filename, std::string id);

	/// <summary>
	/// Loads and adds music track to bank from your path, therefore put the whole path to the music file and an id for later identification
	/// </summary>
	/// <param name="path"></param>
	/// <param name="id"></param>
	void AddMusicTrack(std::string path, std::string id);

	/// <summary>
	/// Starts playing a music track that has already been added, is a trigger and should be called once when it should start
	/// </summary>
	/// <param name="id"></param>
	void Play(std::string id);

	/// <summary>
	/// Stops playing a music track that is currently playing, is a trigger and should be called once when it should stop
	/// </summary>
	/// <param name="id"></param>
	void Stop(std::string id);

	/// <summary>
	/// Starts playing a music track with a fade in of specified time in seconds, startGain is the starting volume. Is a trigger and should be called once when it should start fading in
	/// </summary>
	/// <param name="id"></param>
	/// <param name="startGain"></param>
	/// <param name="seconds"></param>
	void FadeInPlay(std::string id, float startGain, float seconds);

	/// <summary>
	/// Stops playing a music track with a fade out of specified time in seconds. Is a trigger and should be called once when it should start fading out
	/// </summary>
	/// <param name="id"></param>
	/// <param name="seconds"></param>
	void FadeOutStop(std::string id, float seconds);

	/// <summary>
	/// Retrieves source state for a music track, only for debugging
	/// </summary>
	/// <param name="id"></param>
	/// <param name="sourceState"></param>
	void GetMusicTrackSourceState(std::string id, ALint& sourceState);

	/// <summary>
	/// Sets volume for an individual music track
	/// </summary>
	/// <param name="id"></param>
	/// <param name="gain"></param>
	void SetGain(std::string id, float gain);

	/// <summary>
	/// Sets pitch for an individual music track
	/// </summary>
	/// <param name="id"></param>
	/// <param name="pitch"></param>
	void SetPitch(std::string id, float pitch);

	/// <summary>
	/// Retrieve a music track
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	MusicTrack* GetMusicTrack(std::string id);

private:
	ALCdevice* ALCDevice;
	ALCcontext* ALCContext;

	MusicTrackManager musicTrackManager;

};