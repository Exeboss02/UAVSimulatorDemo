#pragma once
#include "core/audio/soundClip.h"
#include "game/player.h"
#include "gameObjects/gameObject.h"
#include <filesystem>

class TextPart {
public:
	TextPart(std::string text, float displayTime);
	const std::string& GetText() const;
	float GetDisplayTime() const;

private:
	std::string text;
	float displayTime;
};

class StoryPart {
public:
	StoryPart(std::vector<TextPart> textParts, SoundClip* soundClip);
	bool PlayStoryPart(std::shared_ptr<Player> player);

private:
	bool playing = false;
	size_t currentTextPart;
	float currentPartTime;
	std::vector<TextPart> textParts;
	SoundClip* soundClip;
};

class StoryManager : public GameObject {
public:
	StoryManager();
	void LoadStory(std::filesystem::path path);
	void PlayNextStoryPart();

	const std::string& GetEndScreenText() const;

	void Tick() override;
	void Start() override;

private:

	bool playing = false;
	size_t currentStoryPart = 0;


	std::vector<StoryPart> storyParts;
	std::weak_ptr<Player> player;
	size_t currentStoryIndex = 0;

	std::string endScreenText;
};
