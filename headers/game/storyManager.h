#pragma once 
#include "game/player.h"
#include <filesystem>
#include "core/audio/soundClip.h"
#include "gameObjects/gameObject.h"
#include <string_view>

class TextPart {
public:
	TextPart(std::string text, float displayTime);
	std::string_view GetText() const;
	float GetDisplayTime() const;

private:
	std::string text;
	float displayTime;
};

class StoryPart {
public:
	StoryPart(std::vector<TextPart> textParts, SoundClip* soundClip);
	void PlayStoryPart() const;

private:
	std::vector<TextPart> textParts;
	SoundClip* soundClip;
};

class StoryManager : GameObject {
public:
	void LoadStory(std::filesystem::path path);
	void PlayNextStoryPart();
	
	void Tick() override;
	void Start() override;

private:
	std::vector<StoryPart> storyParts;

	size_t currentStoryIndex = 0;
};

