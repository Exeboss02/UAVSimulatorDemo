#include "game/musicPlayer.h"

MusicPlayer::MusicPlayer()
{
}

MusicPlayer::~MusicPlayer()
{
    AudioManager::GetInstance().FadeOutStop("courage", 6.0f);
}

void MusicPlayer::Start()
{
    AudioManager::GetInstance().FadeOutStop("contact", 2.0f);
    AudioManager::GetInstance().Play("courage");
}
