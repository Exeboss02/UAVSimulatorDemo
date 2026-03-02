#include "game/storyManager.h"
#include "core/assetManager.h"
#include "core/filepathHolder.h"
#include "nlohmann/json.hpp"
#include "utilities/logger.h"
#include <iostream>

StoryManager::StoryManager() { this->LoadStory(FilepathHolder::GetAssetsDirectory() / "story/chapter1.json"); }

void StoryManager::LoadStory(std::filesystem::path path) {
	std::ifstream storyText(path);

	this->storyParts.clear();

	nlohmann::json storyJson = nlohmann::json::parse(storyText);
	for (auto& storyPartIt : storyJson["storyParts"]) {
		std::vector<TextPart> storyTexts;

		for (auto& textIt : storyPartIt["textParts"]) {
			std::string text = textIt["text"].get<std::string>();
			float displayTime = textIt["displayTime"].get<float>();
			Logger::Log(text, ", displayTime: ", displayTime);
			storyTexts.emplace_back(std::move(text), displayTime);
		}

		std::string soundClipPath = storyPartIt["soundClipPath"].get<std::string>();

		SoundClip* soundClip = AssetManager::GetInstance().GetDialogueSoundClip(soundClipPath);

		this->storyParts.emplace_back(storyTexts, soundClip);
	}

	this->endScreenText = storyJson["endScreenText"].get<std::string>();
}

void StoryManager::PlayNextStoryPart() {
	if (currentStoryPart >= this->storyParts.size()) {
		return;
	}
	this->currentStoryPart++;
	this->playing = true;
	Logger::Log("Playing next storypart");
}

const std::string& StoryManager::GetEndScreenText() const { return this->endScreenText; }

void StoryManager::Tick() {
	this->GameObject::Tick();

	if (this->playing) {
		if (this->player.expired()) {
			Logger::Error("StoryManager player ref was expired!");
			return;
		}
		StoryPart& storyPart = this->storyParts[this->currentStoryPart - 1];
		bool finishedPlaying = storyPart.PlayStoryPart(this->player.lock());
		if (finishedPlaying) {
			this->playing = false;
		}
	}
}

void StoryManager::Start() {
	this->GameObject::Start();
	this->player = this->factory->FindObjectOfType<Player>();
	if (player.expired()) {
		std::string error = "No player was found when trying to create storyManager";
		Logger::Error(error);
		throw std::runtime_error(error);
	}
}

TextPart::TextPart(std::string text, float displayTime) : text(text), displayTime(displayTime) {}

const std::string& TextPart::GetText() const { return this->text; }

float TextPart::GetDisplayTime() const { return this->displayTime; }

StoryPart::StoryPart(std::vector<TextPart> textParts, SoundClip* soundClip)
	: textParts(textParts), soundClip(soundClip) {}

bool StoryPart::PlayStoryPart(std::shared_ptr<Player> player) {
	// If we have nothing to play, signal that we have finished playing
	if (this->textParts.size() == 0) return true;

	if (!this->playing) {
		this->playing = true;
		this->currentPartTime = this->textParts[0].GetDisplayTime();
		this->currentTextPart = 0;
		if (auto speaker = player->storySpeaker.lock()) {
			if (this->soundClip) {
				speaker->Play(this->soundClip);
			}
		}
		player->hud->SetStoryText(this->textParts[this->currentTextPart].GetText());
		player->hud->SetStoryTextVisibility(true);
	}

	this->currentPartTime -= Time::GetInstance().GetDeltaTime();

	if (currentPartTime < 0) {
		this->currentTextPart++;

		if (this->currentTextPart >= this->textParts.size()) {
			this->playing = false;
			player->hud->SetStoryTextVisibility(false);
			return true;
		}

		player->hud->SetStoryText(this->textParts[this->currentTextPart].GetText());
		this->currentPartTime = this->textParts[this->currentTextPart].GetDisplayTime();
	}
	return false;
}
