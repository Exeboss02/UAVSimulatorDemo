#include "core/audio/musicTrack.h"
#include "core/audio/audioManager.h"

MusicTrack::MusicTrack()
{
	this->audioInstruction.fadeIn = false; //will look at this later
	this->audioInstruction.fadeInTime = 0;
	this->audioInstruction.fadeOut = false;
	this->audioInstruction.fadeOutTime = 0;
	this->audioInstruction.loopSound = false;
	this->audioInstruction.loopNrOfTimes = 0;
}

MusicTrack::~MusicTrack()
{
	alDeleteSources(1, &this->source);

	if (this->sndfile)
	{
		sf_close(this->sndfile);
		this->sndfile = nullptr;
	}

	if (this->membuf) free(this->membuf);
	if (this->buffers) alDeleteBuffers(NUM_BUFFERS, this->buffers);
}

bool MusicTrack::Initialize(std::string filepath, std::string id)
{
	this->filepath = filepath;
	this->id = id;

	return this->LoadTrack();
}

void MusicTrack::Play()
{
	alGetError();
	this->currentGain = this->targetGain;
	alSourcef(this->source, AL_GAIN, this->currentGain * AudioManager::GetInstance().GetMasterMusicVolume());

	for (int i = 0; i < NUM_BUFFERS; i++)
	{
		sf_count_t slen = sf_readf_short(this->sndfile, this->membuf, BUFFER_SAMPLES);
		if (slen < 1) break;

		slen *= this->sfInfo.channels * (sf_count_t)sizeof(short);
		alBufferData(this->buffers[i], this->format, this->membuf, (ALsizei)slen, this->sfInfo.samplerate);
	}

	if (alGetError() != AL_NO_ERROR)
	{
		Logger::Error("error buffering for playback");
		return;
	}

	alSourceQueueBuffers(this->source, NUM_BUFFERS, this->buffers);
	alSourcePlay(this->source);

	if (alGetError() != AL_NO_ERROR)
	{
		Logger::Error("error buffering for playback");
	}

	ALint sampleOffset;
	alGetSourcei(this->source, AL_SAMPLE_OFFSET, &sampleOffset);
}

void MusicTrack::Stop()
{
	alSourceStop(this->source);
	alSourceRewind(this->source);
	alSourcei(this->source, AL_BUFFER, 0);

	// Stop the source and unqueue all buffers
    ALint queued;
    alGetSourcei(this->source, AL_BUFFERS_QUEUED, &queued);
    if (queued > 0)
    {
        std::vector<ALuint> unqueuedBuffers(queued);
        alSourceUnqueueBuffers(this->source, queued, unqueuedBuffers.data());
    }

	if (alGetError() != AL_NO_ERROR)
	{
		Logger::Error("error while stopping music track " + this->id);
	}

	sf_seek(this->sndfile, 0, SEEK_SET); //Rewind
}

void MusicTrack::FadeIn(float startGain, float seconds)
{
	this->fadeInTime = seconds;
	this->currentFadeInTime = 0;
	this->currentGain = startGain;
	this->startGain = startGain;

	alSourcef(this->source, AL_GAIN, startGain * AudioManager::GetInstance().GetMasterMusicVolume());
}

void MusicTrack::FadeOut(float seconds)
{
	this->fadeOutTime = seconds;
	this->currentFadeOutTime = 0;
}

bool MusicTrack::LoadTrack()
{
	alGenSources(1, &this->source);
	alGenBuffers(NUM_BUFFERS, this->buffers);

	alSourcef(this->source, AL_PITCH, this->pitch);
	alSourcef(this->source, AL_GAIN, this->currentGain * AudioManager::GetInstance().GetMasterMusicVolume());
	alSource3f(this->source, AL_POSITION, this->position[0], this->position[1], this->position[2]);
	alSource3f(this->source, AL_VELOCITY, this->velocity[0], this->velocity[1], this->velocity[2]);
	alSourcei(this->source, AL_LOOPING, this->audioInstruction.loopSound);

	std::string fullpath = this->filepath;
	const char* file = fullpath.c_str();

	sndfile = sf_open(file, SFM_READ, &this->sfInfo);

	if (!sndfile)
	{
		return false;
	}

	this->format = AL_NONE;

	switch (sfInfo.channels)
	{
	case 1:
		this->format = AL_FORMAT_MONO16;
		break;

	case 2:
		this->format = AL_FORMAT_STEREO16;
		break;

		//there are 2 more cases, see tutorial part 1 for details

	default:
		//std::cout << "unsupported channel count: " << sfInfo.channels << std::endl;
		Logger::Error("unsupported channel count: ");
		sf_close(sndfile);
		return false;
	}

	std::size_t frameSize = ((size_t)BUFFER_SAMPLES * (size_t)this->sfInfo.channels) * sizeof(short);
	this->membuf = static_cast<short*>(malloc(frameSize));

	return true;
}

void MusicTrack::SetPitch(float pitch)
{
	this->pitch = pitch;
	alSourcef(this->source, AL_PITCH, this->pitch);
}

float MusicTrack::GetGain()
{
	return this->currentGain;
}

void MusicTrack::SetGain(float gain)
{
	this->currentGain = gain;
	this->targetGain = gain;

	alSourcef(this->source, AL_GAIN, gain * AudioManager::GetInstance().GetMasterMusicVolume());
}

void MusicTrack::SetAudioInstruction(AudioInstruction audioInstruction)
{
	this->audioInstruction = audioInstruction;
	alSourcei(this->source, AL_LOOPING, this->audioInstruction.loopSound);
	//add more stuff later
}

void MusicTrack::GetSourceState(ALint& sourceState)
{
	alGetSourcei(this->source, AL_SOURCE_STATE, &sourceState);
}

void MusicTrack::ShouldLoop(bool shouldLoop)
{
	this->shouldLoop = shouldLoop;
}

void MusicTrack::UpdateBufferStream()
{
	ALint processed, state;

	alGetError();
	alGetSourcei(this->source, AL_SOURCE_STATE, &state);
	alGetSourcei(this->source, AL_BUFFERS_PROCESSED, &processed);

	if (alGetError() != AL_NO_ERROR)
	{
		Logger::Error("error checking music source state");
	}

	while (processed > 0)
	{
		ALuint buffer;
		sf_count_t slen;

		alSourceUnqueueBuffers(this->source, 1, &buffer);
		processed--;

		slen = sf_readf_short(this->sndfile, this->membuf, BUFFER_SAMPLES);
		if (slen == 0)
		{
			if(this->shouldLoop)
			{
				// Rewind to the start of the file
                sf_seek(this->sndfile, 0, SEEK_SET);
                // Read the first buffer again
                slen = sf_readf_short(this->sndfile, this->membuf, BUFFER_SAMPLES);
			}

			else
			{
				this->Stop();
				return;
			}
		}

		if (slen > 0)
		{
			slen *= this->sfInfo.channels * (sf_count_t)sizeof(short);
			alBufferData(buffer, this->format, this->membuf, (ALsizei)slen, this->sfInfo.samplerate);
			alSourceQueueBuffers(this->source, 1, &buffer);
		}

		if (alGetError() != AL_NO_ERROR)
		{
			Logger::Error("error buffering music data while streaming");
			return;
		}
	}

	//make sure buffers hasn't underrun
	if (state != AL_PLAYING && state != AL_PAUSED)
	{
		ALint queued;

		//if no buffers are queued, playback is finished
		alGetSourcei(this->source, AL_BUFFERS_QUEUED, &queued);

		if (queued == 0) return;

		alSourcePlay(this->source);
		if (alGetError() != AL_NO_ERROR)
		{
			Logger::Error("error restarting music");
		}
	}

	if (state == AL_PLAYING)
	{
		float deltaTime = Time::GetInstance().GetDeltaTime();
		this->playTime += deltaTime;

		if (this->fadeInTime > 0 && this->currentFadeInTime < this->fadeInTime)
		{
			this->currentFadeInTime += deltaTime;
			this->currentGain = (this->targetGain - this->startGain) * (this->currentFadeInTime / this->fadeInTime) + this->startGain;

			alSourcef(this->source, AL_GAIN, this->currentGain * AudioManager::GetInstance().GetMasterMusicVolume());
		}

		if (this->fadeOutTime > 0 && this->currentFadeOutTime < this->fadeOutTime)
		{
			this->currentFadeOutTime += deltaTime;
			this->currentGain = ((this->fadeOutTime - this->currentFadeOutTime) / this->fadeOutTime) * this->targetGain;

			if (this->currentGain <= 0.01f)
			{
				this->currentGain = 0;
				this->Stop();
			}

			alSourcef(this->source, AL_GAIN, this->currentGain * AudioManager::GetInstance().GetMasterMusicVolume());
		}
	}
}
