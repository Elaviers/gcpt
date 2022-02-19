#include "sound_session.h"
#include "shared/maths.h"

template <typename T>
__forceinline void TryRelease(T* ptr)
{
	if (ptr) ptr->Release();
}

AudioSession::~AudioSession()
{
	if (_audioClient) _audioClient->Stop();

	TryRelease(_renderClient);
	TryRelease(_audioClient);
	TryRelease(_audioDevice);
	TryRelease(_enumerator);
}

#define CHECK(result) if (!SUCCEEDED(result)) goto Finished

void AudioSession::Initialise(uint32 minimumBufferDurationMillis)
{
	WAVEFORMATEX* descriptor = NULL;

	HRESULT result = CoInitialize(NULL);

	result = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&_enumerator);
	CHECK(result);

	result = _enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &_audioDevice);
	CHECK(result);

	result = _audioDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&_audioClient);
	CHECK(result);

	REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;

	result = _audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
	CHECK(result);

	{
		const REFERENCE_TIME buffer_duration = Maths::Max((REFERENCE_TIME)minimumBufferDurationMillis * 10000, minimumDevicePeriod);

		result = _audioClient->GetMixFormat(&descriptor);
		CHECK(result);

		_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		_waveFormat.wBitsPerSample = 16;
		_waveFormat.nChannels = 2;
		_waveFormat.nSamplesPerSec = descriptor->nSamplesPerSec;
		_waveFormat.nBlockAlign = (_waveFormat.wBitsPerSample * _waveFormat.nChannels) / 8;
		_waveFormat.nAvgBytesPerSec = _waveFormat.nSamplesPerSec * _waveFormat.nBlockAlign;

		result = _audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, buffer_duration, 0 /*shared*/, &_waveFormat, NULL);
		CHECK(result);

		result = _audioClient->GetBufferSize(&_bufferFrameCount);
		CHECK(result);

		result = _audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&_renderClient);
		CHECK(result);

		result = _audioClient->Start();
		CHECK(result);
	}

	CoTaskMemFree(descriptor);
	return;

Finished:
	CoTaskMemFree(descriptor);
	Debug::Error("Could not initialise audio session (this is probably a WASAPI problem...)");
}

void AudioSession::FillBuffer()
{
	UINT32 padding = 0;
	HRESULT result = _audioClient->GetCurrentPadding(&padding);

	uint32 availableFrames = _bufferFrameCount - padding;
	uint32 destFramesWritten = 0;

	CHECK(result);

	if (availableFrames)
	{
		byte* buffer;
		result = _renderClient->GetBuffer(availableFrames, &buffer);
		CHECK(result);

		ZeroMemory(buffer, (size_t)availableFrames * _waveFormat.nBlockAlign);

		List<Sampler>::Iterator it = _playingSounds.begin();

		while (it)
		{
			uint32 framesWritten = it->ReadToSoundBuffer(buffer, availableFrames, _waveFormat.nSamplesPerSec, _waveFormat.nChannels, .5f);

			destFramesWritten = Maths::Max(destFramesWritten, framesWritten);

			if (it->IsFinished())
				it = _playingSounds.Remove(it);
			else
				++it;
		}

		result = _renderClient->ReleaseBuffer(destFramesWritten, 0);
		CHECK(result);
	}

	return;

Finished:
	Debug::Error("WASAPI buffer error");
}

Sampler& AudioSession::PlaySound(const SoundClip& sound)
{
	Sampler& sampler = _playingSounds.AddBack(Sampler());
	sampler.SetSound(sound);
	sampler.SetLooping(false);
	return sampler;
}



#include "imgui.h"
#include <propsys.h>
#include <propvarutil.h>

static void WaveFormatImGui(const WAVEFORMATEX& wf)
{
	ImGui::Text("FormatTag:%d\nChannels:%d\nSamplesPerSec:%d\nAvgBytesPerSec:%d\nBlockAlign:%d\nBitsPerSample:%d",
		wf.wFormatTag, wf.nChannels, wf.nSamplesPerSec, wf.nAvgBytesPerSec, wf.nBlockAlign, wf.wBitsPerSample);

}

void AudioSession::ImGui()
{
	LPWSTR deviceID_w;
	_audioDevice->GetId(&deviceID_w);

	ImGui::Text("Device");
	ImGui::Indent();

	char deviceID[64];
	wcstombs_s(nullptr, deviceID, deviceID_w, sizeof(deviceID));
	ImGui::Text("id:\t%s", deviceID);

	DWORD state;
	_audioDevice->GetState(&state);
	const char* stateString = "???";
	switch (state)
	{
	case DEVICE_STATE_ACTIVE:
		stateString = "DEVICE_STATE_ACTIVE";
		break;
	case DEVICE_STATE_DISABLED:
		stateString = "DEVICE_STATE_DISABLED";
		break;
	case DEVICE_STATE_NOTPRESENT:
		stateString = "DEVICE_STATE_NOTPRESENT";
		break;
	case DEVICE_STATE_UNPLUGGED:
		stateString = "DEVICE_STATE_UNPLUGGED";
		break;
	}

	ImGui::Text(CSTR("state:\t", stateString));

	IPropertyStore* propertyStore;
	_audioDevice->OpenPropertyStore(STGM_READ, &propertyStore);

	if (ImGui::TreeNode("PropertyStore"))
	{
		DWORD propertyCount;
		propertyStore->GetCount(&propertyCount);
		for (DWORD i = 0; i < propertyCount; ++i)
		{
			PROPERTYKEY key;
			propertyStore->GetAt(i, &key);

			PROPVARIANT prop;
			propertyStore->GetValue(key, &prop);

			WCHAR propNameBuf_w[128];
			char propNameBuf[128];
			PSStringFromPropertyKey(key, propNameBuf_w, 128);
			wcstombs_s(nullptr, propNameBuf, propNameBuf_w, 128);

			WCHAR propBuf_w[128];
			char propBuf[128];
			PropVariantToString(prop, propBuf_w, 128);
			wcstombs_s(nullptr, propBuf, propBuf_w, 128);

			ImGui::Text("%s:\t%s", propNameBuf, propBuf);
		}

		ImGui::TreePop();
	}

	ImGui::Unindent();

	ImGui::Text("AudioClient");
	ImGui::Indent();

	UINT32 num;
	_audioClient->GetBufferSize(&num);
	ImGui::Text("BufferSize:%d", num);

	_audioClient->GetCurrentPadding(&num);
	ImGui::Text("CurrentPadding:%d", num);

	REFERENCE_TIME min, max;
	_audioClient->GetDevicePeriod(&min, &max);
	ImGui::Text("DevicePeriod: [min:%lld, max:%lld]", min, max);

	WAVEFORMATEX* mixFmt;
	_audioClient->GetMixFormat(&mixFmt);
	if (ImGui::TreeNode("MixFormat"))
	{
		WaveFormatImGui(*mixFmt);
		ImGui::TreePop();
	}

	_audioClient->GetStreamLatency(&min);
	ImGui::Text("StreamLatency:%lld", min);

	ImGui::Unindent();
}

