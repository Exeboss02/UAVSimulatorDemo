#include "core/audio/audioManager.h"

AudioManager::AudioManager()
{
	this->ALCDevice = alcOpenDevice(nullptr);
	if (!this->ALCDevice) Logger::Error("failed to get sound device!");

	this->ALCContext = alcCreateContext(this->ALCDevice, nullptr);
	if (!this->ALCContext) Logger::Error("failed to create context!");

	if (!alcMakeContextCurrent(this->ALCContext)) Logger::Error("failed to set this as current context!");

	const ALCchar* name = nullptr;
	if (alcIsExtensionPresent(this->ALCDevice, "ALC_ENUMERATE_ALL_EXT"))
	{
		name = alcGetString(this->ALCDevice, ALC_ALL_DEVICES_SPECIFIER);
	}
	if (!name || alcGetError(this->ALCDevice) != AL_NO_ERROR)
	{
		name = alcGetString(this->ALCDevice, ALC_DEVICE_SPECIFIER);
	}

	std::string n = name;
	Logger::Log("opened " + n);

	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); //AL_LINEAR_DISTANCE would also work

	ALfloat listenerOrient[] = { 0.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f }; // Forward: Z, Up: Y
	alListenerfv(AL_ORIENTATION, listenerOrient);
}

AudioManager::~AudioManager()
{
	if (!alcMakeContextCurrent(nullptr)) Logger::Error("failed to unset context during termination!");

	alcDestroyContext(this->ALCContext);
	ALenum err = alcGetError(this->ALCDevice);

	if (err != ALC_NO_ERROR)
	{
		Logger::Error("error when destroying AL-context!");
	}

	if (!alcCloseDevice(this->ALCDevice)) Logger::Error("failed to close AL-device!");
}

AudioManager& AudioManager::GetInstance()
{
	static AudioManager instance;
	return instance;
}

void AudioManager::AddMusicTrackStandardFolder(std::string filename, std::string id)
{
	this->musicTrackManager.AddMusicTrackStandardFolder(filename, id);
}

void AudioManager::AddMusicTrack(std::string path, std::string id)
{
	this->musicTrackManager.AddMusicTrack(path, id);
}

void AudioManager::Play(std::string id)
{
	this->musicTrackManager.Play(id);
}

void AudioManager::Stop(std::string id)
{
	this->musicTrackManager.Stop(id);
}

void AudioManager::FadeInPlay(std::string id, float startGain, float seconds)
{
	this->musicTrackManager.FadeInPlay(id, startGain, seconds);
}

void AudioManager::FadeOutStop(std::string id, float seconds)
{
	this->musicTrackManager.FadeOutStop(id, seconds);
}

void AudioManager::GetMusicTrackSourceState(std::string id, ALint& sourceState)
{
	this->musicTrackManager.GetMusicTrackSourceState(id, sourceState);
}

float AudioManager::GetMusicTrackGain(std::string id)
{
	MusicTrack* track = this->musicTrackManager.GetMusicTrack(id);
	if(track) return track->GetGain();
	else return -1;
}

void AudioManager::SetGain(std::string id, float gain)
{
	this->musicTrackManager.SetGain(id, gain);
}

void AudioManager::SetPitch(std::string id, float pitch)
{
	this->musicTrackManager.SetPitch(id, pitch);
}

void AudioManager::LoopMusicTrack(std::string id, bool shouldLoop)
{
	this->musicTrackManager.LoopMusicTrack(id, shouldLoop);
}

MusicTrack* AudioManager::GetMusicTrack(std::string id) { return this->musicTrackManager.GetMusicTrack(id); }

void AudioManager::Tick() { this->musicTrackManager.Tick(); }

void AudioManager::SetMasterMusicVolume(float musicMaster)
{
	this->masterVolume.SetMusicGain(musicMaster);
}

void AudioManager::SetListenerPosition(float x, float y, float z)
{
	alListener3f(AL_POSITION, x, y, z);

	ALfloat listenerOrient[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	alListenerfv(AL_ORIENTATION, listenerOrient);
}

void AudioManager::SetListenerOrientation(ALfloat listenerOrientation[6])
{
	alListenerfv(AL_ORIENTATION, listenerOrientation);
}

float AudioManager::GetMasterMusicVolume() const
{
	return this->masterVolume.GetMusicGain();
}

void AudioManager::SetMasterSoundEffectsVolume(float soundEffectsMaster)
{
	this->masterVolume.SetSoundEffectsGain(soundEffectsMaster);
}

float AudioManager::GetMasterSoundEffectsVolume() const
{
	return this->masterVolume.GetSoundEffectsGain();
}
