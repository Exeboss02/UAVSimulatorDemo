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

enum class StoryChecks {
	BuildRoom,
	BuiltBuildable,
	PressedButton,
};


class StoryManager : public GameObject {
public:
	StoryManager();
	void LoadStory(std::filesystem::path path);
	void PlayNextStoryPart();
	void FinishStoryCheck(StoryChecks check);

	const std::string& GetEndScreenText() const;
	bool GetCheckState(StoryChecks check) const;
	bool RoundIsChecked(size_t round) const;

	void Tick() override;
	void Start() override;
	bool GetStoryPlaying() const;

	void SetCurrentStoryPart(size_t part);
	size_t GetCurrentStoryPart();
	void SetPlaying(bool playing);

private:
	bool storyPause = false;


	void DisplayObjective(size_t round);

	bool playing = false;

	// Part 0 is pre-story, as such final story part == storyParts.size()
	size_t currentStoryPart = 0;

	struct StoryCheck {
		size_t associatedRound;
		bool checked;
		std::string objective;
	};

	std::array<StoryCheck, 3> storyChecks;

	std::vector<StoryPart> storyParts;
	std::weak_ptr<Player> player;
	size_t currentStoryIndex = 0;

	std::string endScreenText;
};
