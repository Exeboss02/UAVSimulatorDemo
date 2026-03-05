#include "game/musicPlayer.h"

MusicPlayer::MusicPlayer()
{
}

MusicPlayer::~MusicPlayer()
{
    //AudioManager::GetInstance().FadeOutStop("courage", 6.0f);
}

void MusicPlayer::Start()
{
    this->GameObject3D::Start();

    // Master Volume
	AudioManager::GetInstance().SetMasterMusicVolume(0.5f);
	AudioManager::GetInstance().SetMasterSoundEffectsVolume(0.5f);

    // Main menu music
	AudioManager::GetInstance().AddMusicTrackStandardFolder("CourageDemo.wav", "courage");
	AudioManager::GetInstance().LoopMusicTrack("courage", true);
	AudioManager::GetInstance().SetGain("courage", 0.4f);

    AudioManager::GetInstance().FadeOutStop("contact", 2.0f);
    AudioManager::GetInstance().Play("courage");
}
