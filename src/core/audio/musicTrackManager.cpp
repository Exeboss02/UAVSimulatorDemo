#include "core/audio/musicTrackManager.h"
#include "core/filepathHolder.h"

MusicTrackManager::MusicTrackManager()
{
}

MusicTrackManager::~MusicTrackManager()
{
	for (const auto& it : this->musicTracks)
	{
		if (it.second != nullptr)
		{
			delete it.second;
		}
	}

	this->musicTracks.clear();
}

void MusicTrackManager::AddMusicTrackStandardFolder(std::string filename, std::string id)
{
	if(this->musicTracks.contains(id))
	{
		Logger::Warn("MusicTrack ", id, " is already loaded!");
		return;
	}

	MusicTrack* newTrack = new MusicTrack();

	if (!newTrack->Initialize((FilepathHolder::GetAssetsDirectory() / "audio" / "music" / filename).string(), id))
	{
		Logger::Error("Error loading music track: " + this->pathToMusicFolder + filename);
		delete newTrack;
		return;
	}

	this->musicTracks.insert(std::make_pair(id, newTrack));
}

void MusicTrackManager::Play(std::string id)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end())
	{
		Logger::Log("couldn't play music track " + id);
		return;
	}

	else
	{
		musicTrack->second->Play();
		this->activeTracks.push_back(this->musicTracks[id]);
	}
}

void MusicTrackManager::Stop(std::string id)
{

	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't stop music track " + id);
		return;
	}

	musicTrack->second->Stop();
	
	for (int i = 0; i < this->activeTracks.size(); i++)
	{
		if (this->activeTracks[i]->id == id)
		{
			this->activeTracks.erase(this->activeTracks.begin() + i);
			return;
		}
	}

	Logger::Log("failed to stop music track : " + id + ": it isn't active");
}

void MusicTrackManager::FadeInPlay(std::string id, float startGain, float seconds)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't FadeInPLay music track " + id);
		return;
	}

	musicTrack->second->FadeIn(startGain, seconds);
	this->Play(id);
}

void MusicTrackManager::FadeOutStop(std::string id, float seconds)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't FadeOutStop music track " + id);
		return;
	}

	musicTrack->second->FadeOut(seconds);
}

void MusicTrackManager::GetMusicTrackSourceState(std::string id, ALint& sourceState)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't GetMusicTrackSourceState on music track " + id);
		return;
	}

	musicTrack->second->GetSourceState(sourceState);
}

void MusicTrackManager::SetGain(std::string id, float gain)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't SetGain on music track " + id);
		return;
	}

	musicTrack->second->SetGain(gain);
}

void MusicTrackManager::SetPitch(std::string id, float pitch)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't SetPitch on music track " + id);
		return;
	}

	musicTrack->second->SetPitch(pitch);
}

MusicTrack* MusicTrackManager::GetMusicTrack(std::string id)
{
	auto musicTrack = this->musicTracks.find(id);
	if (musicTrack == this->musicTracks.end()) {
		Logger::Log("couldn't GetMusicTrack " + id);
		return nullptr;
	}

	return musicTrack->second;
}

void MusicTrackManager::LoopMusicTrack(std::string id, bool shouldLoop)
{
	MusicTrack* track;
	auto musicTrack = this->musicTracks.find(id);

	if (musicTrack == musicTracks.end())
	{
		Logger::Warn("Couldn't find music track ", id, " for looping");
		return;
	}
	else 
	{
		musicTrack->second->ShouldLoop(shouldLoop);
	}
}

void MusicTrackManager::Tick() {
	for (int i = this->activeTracks.size() - 1; i >= 0; i--)
	{
		ALint sourceState = 0;
		this->activeTracks[i]->GetSourceState(sourceState);
		this->activeTracks[i]->UpdateBufferStream();

		if (sourceState != AL_PLAYING)
		{
			this->activeTracks.erase(this->activeTracks.begin() + i);
		}
	}
}

void MusicTrackManager::AddMusicTrack(std::string path, std::string id)
{
	if(this->musicTracks.contains(id))
	{
		Logger::Warn("MusicTrack ", id, " is already loaded!");
		return;
	}

	MusicTrack* newTrack = new MusicTrack();

	if (!newTrack->Initialize(path, id))
	{
		Logger::Error("Error loading music track: " + path);
		delete newTrack;
		return;
	}

	this->musicTracks.insert(std::make_pair(id, newTrack));
}
