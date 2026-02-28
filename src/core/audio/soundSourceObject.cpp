#include "core/audio/soundSourceObject.h"
#include "core/audio/audioManager.h"

SoundSourceObject::SoundSourceObject()
{
	this->currentInstructionSet.fadeIn = false; //will look at this later
	this->currentInstructionSet.fadeInTime = 0;
	this->currentInstructionSet.fadeOut = false;
	this->currentInstructionSet.fadeOutTime = 0;
	this->currentInstructionSet.loopSound = false;
	this->currentInstructionSet.loopNrOfTimes = 0;

	this->sources = new ALuint[this->nrOfSources]; //owns its own sources
	alGenSources(this->nrOfSources, this->sources);

	DirectX::XMVECTOR pos = this->transform.GetGlobalPosition();

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcef(this->sources[i], AL_PITCH, this->pitch);
		alSourcef(this->sources[i], AL_GAIN, this->gain * AudioManager::GetInstance().GetMasterSoundEffectsVolume());
		alSource3f(this->sources[i], AL_POSITION, (ALfloat)pos.m128_f32[0], (ALfloat)pos.m128_f32[1], (ALfloat)pos.m128_f32[2]);
		alSource3f(this->sources[i], AL_VELOCITY, this->velocity[0], this->velocity[1], this->velocity[2]);
		alSourcei(this->sources[i], AL_LOOPING, this->currentInstructionSet.loopSound);

		ALint sampleOffset;
		alGetSourcei(this->sources[i], AL_SAMPLE_OFFSET, &sampleOffset);
		this->lastSampleOffsets.push_back(sampleOffset);
	}

	//ALint state = 0;
	//this->GetSourceState(0, state);
	//Logger::Log("source state: " + std::to_string(state));
}

SoundSourceObject::~SoundSourceObject()
{
	alDeleteSources(this->nrOfSources, this->sources);
	delete[] this->sources;
}

void SoundSourceObject::Tick()
{
	this->GameObject3D::Tick();

	DirectX::XMVECTOR pos = this->transform.GetGlobalPosition();
	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSource3f(this->sources[i], AL_POSITION, (ALfloat)pos.m128_f32[0], (ALfloat)pos.m128_f32[1], (ALfloat)pos.m128_f32[2]);
	}

	if(this->deleteWhenFinnished && this->isPlaying)
	{
		bool somethingIsPlaying = false;

		for (int i = 0; i < this->nrOfSources; i++)
		{
			ALint state = -1;
			this->GetSourceState(i, state);
			if(state == AL_PLAYING) somethingIsPlaying = true;
		}
		
		if(!somethingIsPlaying)
		{
			this->factory->QueueDeleteGameObject(this->GetPtr());
		}
	}

	//if loop count is 0 or negative, it loops until you stop it yourself
	if (this->shouldLoop && this->loopCount >= 1)
    {
        for (int i = 0; i < this->nrOfSources; i++)
        {
            ALint sampleOffset;
            alGetSourcei(this->sources[i], AL_SAMPLE_OFFSET, &sampleOffset);

            // If sampleOffset resets to near 0, a loop has completed
            if (sampleOffset < this->lastSampleOffsets[i])
            {
                this->loopCount--;
                if (this->loopCount <= 1)
                {
					this->StopLoopingSoundEffect();
                }
            }

            this->lastSampleOffsets[i] = sampleOffset;
        }
	}
}

void SoundSourceObject::Play(SoundClip* soundClip) //pointer referece?
{
	if(!soundClip)
	{
		Logger::Error("SoundSourceObject tried to play nullptr SoundClip");
		return;
	}

	this->isPlaying = true;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		int index = (this->sourceIndex + i) % this->nrOfSources;

		ALint state;
		alGetSourcei(this->sources[index], AL_SOURCE_STATE, &state);

		if (state != AL_PLAYING)
		{
			alSourcei(this->sources[index], AL_BUFFER, (ALint)soundClip->buffer); //can a copy occur here?
			alSourcePlay(this->sources[index]);

			// Next search will start after this one
			this->sourceIndex = (index + 1) % this->nrOfSources;
			return;
		}
	}

	// No free source, overwrite the next
	alSourcei(this->sources[this->sourceIndex], AL_BUFFER, (ALint)soundClip->buffer);
	alSourcePlay(this->sources[this->sourceIndex]);

	this->sourceIndex = (this->sourceIndex + 1) % this->nrOfSources;
}

void SoundSourceObject::SetId(int newId)
{
	this->id = newId;
}

int SoundSourceObject::GetId()
{
	return this->id;
}

void SoundSourceObject::GetSourceState(int index, ALint& sourceState)
{
	if (index > this->nrOfSources - 1)
	{
		Logger::Log("soundSource index is out of scope!");
		return;
	}

	alGetSourcei(this->sources[index], AL_SOURCE_STATE, &sourceState);
}

void SoundSourceObject::GetCurrentSourcePosition(ALfloat* position)
{
	alGetSource3f(sources[this->sourceIndex], AL_POSITION, &position[0], &position[1], &position[2]);
}

void SoundSourceObject::SetSourcePosition(float x, float y, float z)
{
	DirectX::XMVECTOR position = {};
	position.m128_f32[0] = x;
	position.m128_f32[1] = y;
	position.m128_f32[2] = z;
	
	this->transform.SetPosition(position);

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSource3f(this->sources[i], AL_POSITION, x, y, z);
	}
}

void SoundSourceObject::SetDeleteWhenFinnished(bool del)
{
	this->deleteWhenFinnished = del;
}

bool SoundSourceObject::GetDeleteWhenFinnished()
{
	return this->deleteWhenFinnished;
}

void SoundSourceObject::LoopSoundEffect(int nrOfTimes)
{
	this->shouldLoop = true;
	this->loopCount = nrOfTimes;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcei(this->sources[i], AL_LOOPING, AL_TRUE);
	}
}

void SoundSourceObject::StopLoopingSoundEffect()
{
	this->shouldLoop = false;
	this->loopCount = 0;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcei(this->sources[i], AL_LOOPING, AL_FALSE);
		this->lastSampleOffsets[i] = 0;
	}
}

bool SoundSourceObject::GetLoopSoundEffect() { return this->shouldLoop; }

void SoundSourceObject::SetRandomPitch(float minPitch, float maxPitch) {
	int tempMin = minPitch * 1000;
	int tempMax = maxPitch * 1000;
	int tempPitch = tempMin + rand() % (tempMax - tempMin);
	float pitch = tempPitch / 1000.0f;

	this->SetPitch(pitch);
}

void SoundSourceObject::SetGain(float gain)
{
	this->gain = gain;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcef(this->sources[i], AL_GAIN, this->gain * AudioManager::GetInstance().GetMasterSoundEffectsVolume());
	}
}

float SoundSourceObject::GetGain()
{
	return this->gain;
}

void SoundSourceObject::SetPitch(float pitch)
{
	this->pitch = pitch;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcef(this->sources[i], AL_PITCH, this->pitch);
	}
}

void SoundSourceObject::ChangePitch(float pitchChange)
{
	this->pitch += pitchChange;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcef(this->sources[i], AL_PITCH, this->pitch);
	}
}

void SoundSourceObject::ChangeGain(float gainChange)
{
	this->gain += gainChange;

	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcef(this->sources[i], AL_GAIN, this->gain * AudioManager::GetInstance().GetMasterSoundEffectsVolume());
	}
}

void SoundSourceObject::SetAudioInstruction(AudioInstruction instructionSet)
{
	this->currentInstructionSet = instructionSet;
	for (int i = 0; i < this->nrOfSources; i++)
	{
		alSourcei(this->sources[i], AL_LOOPING, this->currentInstructionSet.loopSound);
		//add more stuff later
	}
}
