#include "game/storyManager.h"
#include "core/assetManager.h"
#include "core/filepathHolder.h"
#include "gameObjects/emergencyExitButton.h"
#include "nlohmann/json.hpp"
#include "utilities/logger.h"
#include <iostream>
#include "game/gameManager.h"

StoryManager::StoryManager() {
	this->LoadStory(FilepathHolder::GetAssetsDirectory() / "story/chapter1.json");
	this->storyChecks = {
		StoryCheck{0, false, "Walk up to a wall and build a room"},
		StoryCheck{1, false, "Walk up to the build platform in the center of the room and build a buildable"},
		StoryCheck{10, false, "Push the \"Gift\" button"}};
}

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

void StoryManager::SetCurrentStoryPart(size_t part)
{
	this->currentStoryPart = part;
}

size_t StoryManager::GetCurrentStoryPart() { return this->currentStoryPart; }

void StoryManager::SetPlaying(bool playing)
{
	std::shared_ptr<Player> player = GameManager::GetInstance()->GetPlayer();
	if(!player->storySpeaker.expired())
	{
		player->storySpeaker.lock()->Stop();
	}

	this->playing = playing;
}

void StoryManager::PlayNextStoryPart() {
	if (currentStoryPart >= this->storyParts.size()) {
		return;
	}

	this->currentStoryPart++;
	this->playing = true;

	if (this->currentStoryPart == this->storyParts.size()) {
		if (auto button = this->factory->FindObjectOfType<EmergenceExitButton>().lock()) {
			button->SetState(EmergenceExitButton::State::PostTouchUp);
		}
	}

	if (!this->RoundIsChecked(GameManager::GetInstance()->GetCurrentRound())) {
		this->storyPause = true;
	} else {
		this->storyPause = false;
	}

	Logger::Log("Playing next storypart");
}

void StoryManager::FinishStoryCheck(StoryChecks check) {
	this->storyChecks[static_cast<size_t>(check)].checked = true;
	this->storyPause = false;
	this->DisplayObjective(GameManager::GetInstance()->GetCurrentRound());
}

const std::string& StoryManager::GetEndScreenText() const { return this->endScreenText; }

bool StoryManager::GetCheckState(StoryChecks check) const {

	return this->storyChecks[static_cast<size_t>(check)].checked;
}

bool StoryManager::RoundIsChecked(size_t round) const {
	for (const auto& storycheck : this->storyChecks) {
		if (storycheck.associatedRound == round && !storycheck.checked) return false;
	}
	return true;
}

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
			this->DisplayObjective(GameManager::GetInstance()->GetCurrentRound());
		}
	}
}

void StoryManager::Start() {
	this->GameObject::Start();
	//this->player = this->factory->FindObjectOfType<Player>();
	//if (player.expired()) {
	//	std::string error = "No player was found when trying to create storyManager";
	//	Logger::Error(error);
	//	throw std::runtime_error(error);
	//}
}

bool StoryManager::GetStoryPlaying() const { return this->storyPause || this->playing; }

void StoryManager::DisplayObjective(size_t round) {
	auto player = GameManager::GetInstance()->GetPlayer();
	for (const auto& storyCheck : this->storyChecks) {
		if (!storyCheck.checked && storyCheck.associatedRound == round) {
			player->hud->SetObjectiveVisible(true);
			player->hud->SetObjective(storyCheck.objective);
			return;
		}
	}
	player->hud->SetObjective("Prepare against the pirates");
	player->hud->SetObjectiveVisible(true);
}

TextPart::TextPart(std::string text, float displayTime) : text(text), displayTime(displayTime) {}

const std::string& TextPart::GetText() const { return this->text; }

float TextPart::GetDisplayTime() const { return this->displayTime; }

StoryPart::StoryPart(std::vector<TextPart> textParts, SoundClip* soundClip)
	: textParts(textParts), soundClip(soundClip) {}

bool StoryPart::PlayStoryPart(std::shared_ptr<Player> player) {
	// If we have nothing to play, signal that we have finished playing
	player->hud->SetObjectiveVisible(false);
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
