#pragma once 
#include "gameObjects/gameObject.h"

class SkyBoxLoader : public GameObject {
public:
	virtual void Start() override;
};

void SkyBoxLoader::Start() { RenderQueue::ChangeSkybox("ocean.dds"); }