#pragma once
#include <unordered_map>
#include "core/audio/musicTrack.h"

struct MusicLayer
{
	std::string id = "";
	float time = 0;
};

static float masterMusicGain = 1.0f;

class MusicTrackManager
{
public:
	MusicTrackManager();
	~MusicTrackManager();

	/// <summary>
	/// filename or path is ONLY used for loading, id is used for Get()'ing a track
	/// </summary>
	/// <param name="filename"></param>
	/// <param name="id"></param>
	void AddMusicTrackStandardFolder(std::string filename, std::string id);
	void AddMusicTrack(std::string path, std::string id);

	void Play(std::string id);
	void Stop(std::string id);
	void FadeInPlay(std::string id, float startGain, float seconds);
	void FadeOutStop(std::string id, float seconds);
	void FadeBetween(std::string id1, std::string id2, float seconds);

	void GetMusicTrackSourceState(std::string id, ALint& sourceState);
	void SetGain(std::string id, float gain);
	void SetPitch(std::string id, float pitch);
	MusicTrack* GetMusicTrack(std::string id);

	/// <summary>
	/// Loops a music track specified nr of times. 0 or less = loops until stopped or destroyed
	/// </summary>
	/// <param name="id"></param>
	/// <param name="nrOfTimes"></param>
	void LoopMusicTrack(std::string id, bool shouldLoop);

	void Tick();

private:
	std::unordered_map<std::string, MusicTrack*> musicTracks;
	std::vector<MusicTrack*> activeTracks;
	std::string pathToMusicFolder = "";
};