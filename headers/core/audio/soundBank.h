#pragma once
#include "al.h"
#include "alc.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include "sndfile.h"
#include "soundClip.h"
#include "utilities/logger.h"

class SoundBank
{
public:
	SoundBank();
	~SoundBank();

	/// <summary>
	/// Sets standard folder for sound effects, always end path with /
	/// </summary>
	/// <param name="pathToSoundFolder"></param>
	void Initialize(std::string pathToSoundFolder);

	/// <summary>
	/// Loads and adds sound clip from the standard folder, only put filename and id for later identification
	/// </summary>
	/// <param name="filename"></param>
	/// <param name="id"></param>
	void AddSoundClipStandardFolder(const std::string filename, const std::string id); //filename without the path to its folder

	/// <summary>
	/// Loads and adds sound clip from path
	/// </summary>
	/// <param name="path"></param>
	/// <param name="id"></param>
	void AddSoundClip(const std::string path, const std::string id); //full/relative path to soundfile

	/// <summary>
	/// Retrieve path to standard folder fro sound effects
	/// </summary>
	/// <returns></returns>
	std::string GetPathToSoundFolder();

	/// <summary>
	/// Retrieve sound clip
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	SoundClip* GetSoundClip(const std::string id);

	/// <summary>
	/// BETA - Removes sound clip
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	bool RemoveSoundClip(const std::string path);

private:
	std::unordered_map<std::string, SoundClip*> soundClips;
	std::string pathToSoundFolder = "";

	void CreateSoundBuffer(std::string filepath, std::string id);
	void DeleteSoundBuffer(std::string id);
};