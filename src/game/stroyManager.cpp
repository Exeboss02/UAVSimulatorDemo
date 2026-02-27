#include "core/assetManager.h"
#include "game/storyManager.h"
#include "nlohmann/json.hpp"
#include "core/filepathHolder.h"
#include "utilities/logger.h"
#include <iostream>

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


		SoundClip* soundClip = AssetManager::GetInstance().GetSoundClip(soundClipPath);
		

		this->storyParts.emplace_back(storyTexts, soundClip);
	}
}

void StoryManager::PlayNextStoryPart() { return; }

void StoryManager::Tick() { return; }

void StoryManager::Start() { 
	this->LoadStory(FilepathHolder::GetAssetsDirectory() / "story/chapter1.json"); 
}

TextPart::TextPart(std::string text, float displayTime) : text(text), displayTime(displayTime) {}

std::string_view TextPart::GetText() const { return this->text; }

float TextPart::GetDisplayTime() const { return this->displayTime; }


StoryPart::StoryPart(std::vector<TextPart> textParts, SoundClip* soundClip)
	: textParts(textParts), soundClip(soundClip) {}

void StoryPart::PlayStoryPart() const { return; }
