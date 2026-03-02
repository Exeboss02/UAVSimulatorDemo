#include "core/audio/soundBank.h"
#include "core/filepathHolder.h"

SoundBank::SoundBank()
{
}

SoundBank::~SoundBank()
{
	for (const auto& it : this->soundClips)
	{
		if (it.second != nullptr)
		{
			delete it.second;
		}
	}

	this->soundClips.clear();
}

void SoundBank::Initialize(std::string pathToSoundFolder)
{
	this->pathToSoundFolder = pathToSoundFolder;
}

void SoundBank::AddSoundClipStandardFolder(const std::string filename, const std::string id)
{
	this->CreateSoundBuffer((FilepathHolder::GetAssetsDirectory() / "audio" / "soundeffects" / filename).string(), id);
}

void SoundBank::AddDialogueSoundClipStandardFolder(const std::string filename, const std::string id)
{
	this->CreateSoundBuffer((FilepathHolder::GetAssetsDirectory() / "audio" / "dialogue" / filename).string(), id);
}

void SoundBank::AddSoundClip(const std::string path, const std::string id)
{
	this->CreateSoundBuffer(path, id);
}

std::string SoundBank::GetPathToSoundFolder()
{
	return this->pathToSoundFolder;
}

SoundClip* SoundBank::GetSoundClip(const std::string id)
{
	SoundClip* clip;
	auto clipIt = this->soundClips.find(id);

	if (clipIt == soundClips.end())
	{
		this->AddSoundClipStandardFolder(id, id);
		clipIt = this->soundClips.find(id);

		if (clipIt == soundClips.end())
		{
			Logger::Log("couldn't find or load " + id);
			return nullptr;
		}
		clip = clipIt->second;
	} else {
		clip = clipIt->second;
	}

	return (clip);
}

SoundClip* SoundBank::GetDialogueSoundClip(const std::string id)
{
	SoundClip* clip;
	auto clipIt = this->soundClips.find(id);

	if (clipIt == soundClips.end())
	{
		this->AddDialogueSoundClipStandardFolder(id, id);
		clipIt = this->soundClips.find(id);

		if (clipIt == soundClips.end())
		{
			Logger::Log("couldn't find or load " + id);
			return nullptr;
		}
		clip = clipIt->second;
	} else {
		clip = clipIt->second;
	}

	return (clip);
}

bool SoundBank::RemoveSoundClip(const std::string relativePath)
{
	auto it = this->soundClips.find(relativePath);
	if (it != this->soundClips.end())
	{
		this->soundClips.erase(it);
		return true;
	}

	return false;
}

void SoundBank::CreateSoundBuffer(std::string fullpath, std::string id)
{
	ALenum error, format;
	ALuint buffer = 0;
	SNDFILE* sndfile;
	SF_INFO sfInfo;
	short* membuf;
	sf_count_t numFrames;
	ALsizei numBytes;

	const char* filepath = fullpath.c_str();
	sndfile = sf_open(filepath, SFM_READ, &sfInfo);

	if (!sndfile)
	{
		Logger::Error("Libsndfile error: " + std::string(sf_strerror(NULL)));
		Logger::Log("could not open " + fullpath);

		return;
	}
	if (sfInfo.frames < 1 || sfInfo.frames >(sf_count_t)(INT_MAX / sizeof(short)) / sfInfo.channels)
	{
		Logger::Error("bad sample count in " + fullpath);
		return;
	}

	format = AL_NONE;

	switch (sfInfo.channels)
	{
	case 1:
		format = AL_FORMAT_MONO16;
		break;

	case 2:
		format = AL_FORMAT_STEREO16;
		break;

		//there are 2 more cases, see tutorial part 1 for details

	default:
		Logger::Error("unsupported channel count: ");
		sf_close(sndfile);
		return;
	}

	membuf = static_cast<short*>(malloc((size_t)(sfInfo.frames * sfInfo.channels) * sizeof(short)));

	numFrames = sf_readf_short(sndfile, membuf, sfInfo.frames);
	if (numFrames < 1)
	{
		free(membuf);
		sf_close(sndfile);

		Logger::Error("failed to read samples in ");
		return;
	}

	numBytes = (ALsizei)(numFrames * sfInfo.channels) * (ALsizei)(sizeof(short));
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, membuf, numBytes, sfInfo.samplerate);
	free(membuf);
	sf_close(sndfile);

	error = alGetError();
	if (error != AL_NO_ERROR)
	{
		std::string err = alGetString(error);
		Logger::Error("OpenAL error: " + err);

		if (buffer && alIsBuffer(buffer)) alDeleteBuffers(1, &buffer);
		return;
	}

	SoundClip* newClip = new SoundClip();
	newClip->id = id;
	newClip->buffer = buffer;
	newClip->filepath = fullpath;

	this->soundClips.insert(std::make_pair(id, newClip));
}

void SoundBank::DeleteSoundBuffer(std::string id)
{
}
