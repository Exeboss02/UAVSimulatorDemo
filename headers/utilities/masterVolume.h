#pragma once

class MasterVolume
{
public:
	MasterVolume() = default;
	~MasterVolume() = default;

	void SetMusicGain(float musicMaster);
	float GetMusicGain() const;
	void SetSoundEffectsGain(float musicMaster);
	float GetSoundEffectsGain() const;

private:

	float music = 1.0f;
    float soundEffects = 1.0f;
};

inline void MasterVolume::SetMusicGain(float musicMaster) 
{
	this->music = musicMaster;
}

inline float MasterVolume::GetMusicGain() const
{
	return this->music;
}

inline void MasterVolume::SetSoundEffectsGain(float soundEffectsMaster)
{
	this->soundEffects = soundEffectsMaster;
}

inline float MasterVolume::GetSoundEffectsGain() const
{
	return this->soundEffects;
}
