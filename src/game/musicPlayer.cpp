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

    AudioManager::GetInstance().FadeOutStop("contact", 2.0f);
    AudioManager::GetInstance().Play("courage");
}
