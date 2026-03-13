#include "game/musicPlayer.h"

MusicPlayer::MusicPlayer()
{
}

MusicPlayer::~MusicPlayer()
{
    
}

void MusicPlayer::Start()
{
    this->GameObject3D::Start();

    RenderQueue::ChangeSkybox("bright_planet.dds");

    // Master Volume
	AudioManager::GetInstance().SetMasterMusicVolume(0.5f);
	AudioManager::GetInstance().SetMasterSoundEffectsVolume(0.5f);

    // Main menu music
	AudioManager::GetInstance().AddMusicTrackStandardFolder("Courage.wav", "courage");
	AudioManager::GetInstance().LoopMusicTrack("courage", true);
	AudioManager::GetInstance().SetGain("courage", 0.6f);

    AudioManager::GetInstance().FadeOutStop("contact", 2.0f);
    AudioManager::GetInstance().Play("courage");
}
