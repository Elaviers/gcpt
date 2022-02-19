#pragma once
#include "sound_clip.h"

/*
	Sampler

	Audio Sampler
*/

class Sampler
{
	const SoundClip* _sound;

	uint32 _currentFrame;
	bool _playing;
	bool _loop;
	float _volume;

public:
	Sampler() : _sound(nullptr), _currentFrame(0), _playing(true), _loop(true), _volume(1.f) {}
	~Sampler() {}

	void SetSound(const SoundClip& sound) { _sound = &sound; }
	void SetLooping(bool looping) { _loop = looping; }
	void SetPlaying(bool playing) { _playing = playing; }

	bool IsPlaying() const { return _playing; }
	bool IsFinished() const { return _loop && _currentFrame >= _sound->dataSize / _sound->frameSize; }

	//Reads frames to a sound buffer. Currently only supports 16-bit buffers
	uint32 ReadToSoundBuffer(byte* dest, uint32 sampleCount, uint32 destRate, uint16 destChannelCount, float mix);
};
