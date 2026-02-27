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
	this->musicTracks[id]->Play();
	this->activeTracks.push_back(this->musicTracks[id]);
}

void MusicTrackManager::Stop(std::string id)
{
	this->musicTracks[id]->Stop();
	
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
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Error("FadeInPlay() couldn't find: " + id);
		return;
	}

	track->FadeIn(startGain, seconds);
	this->Play(id);
}

void MusicTrackManager::FadeOutStop(std::string id, float seconds)
{
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Error("FadeOutStop() couldn't find: " + id);
		return;
	}

	track->FadeOut(seconds);
}

void MusicTrackManager::GetMusicTrackSourceState(std::string id, ALint& sourceState)
{
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Error("GetMusicTrackSourceState() couldn't find: " + id);
		return;
	}

	this->musicTracks[id]->GetSourceState(sourceState);
}

void MusicTrackManager::SetGain(std::string id, float gain)
{
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Error("SetGain() couldn't find: " + id);
		return;
	}

	track->SetGain(gain);
}

void MusicTrackManager::SetPitch(std::string id, float pitch)
{
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Error("SetPitch() couldn't find: " + id);
		return;
	}

	track->SetPitch(pitch);
}

MusicTrack* MusicTrackManager::GetMusicTrack(std::string id)
{
	MusicTrack* track = this->musicTracks[id];

	if (!track)
	{
		Logger::Log("Couldn't find music track: " + id);
	}

	return track;
}

void MusicTrackManager::LoopMusicTrack(std::string id, int nrOfTimes)
{
	this->
}

void MusicTrackManager::StopLoopingMusicTrack(std::string id)
{

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
	MusicTrack* newTrack = new MusicTrack();

	if (!newTrack->Initialize(path, id))
	{
		Logger::Error("Error loading music track: " + path);
		delete newTrack;
		return;
	}

	this->musicTracks.insert(std::make_pair(id, newTrack));
}
