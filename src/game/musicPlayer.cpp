#include "game/musicPlayer.h"

MusicPlayer::MusicPlayer()
{
}

MusicPlayer::~MusicPlayer()
{
    AudioManager::GetInstance().FadeOutStop("courage", 4.0f);
}

void MusicPlayer::Start()
{
    this->GameObject3D::Start();

    // Main menu music
	AudioManager::GetInstance().AddMusicTrackStandardFolder("CourageDemo.wav", "courage");
	// AudioManager::GetInstance().LoopMusicTrack("courage", true);
	AudioManager::GetInstance().SetGain("courage", 0.4f);

    AudioManager::GetInstance().FadeOutStop("contact", 2.0f);
    AudioManager::GetInstance().Play("courage");
}
