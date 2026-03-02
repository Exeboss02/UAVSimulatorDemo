#pragma once
#include "core/audio/soundEngine.h"
#include "gameObjects/gameObject.h"

class MusicPlayer : public GameObject3D
{
public:
    MusicPlayer();
    ~MusicPlayer();

    void Start() override;
};