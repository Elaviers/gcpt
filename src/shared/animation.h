#pragma once
#include "anim_track.h"
#include "asset.h"
#include "t_hashmap.h"
#include "string.h"
#include "t_mat4.h"
#include "t_vec3.h"

class Skeleton;

class Animation : public Asset
{
	Hashmap<String, AnimationTrack<Vector3>> _translationTracks;
	Hashmap<String, AnimationTrack<Quaternion>> _rotationTracks;
	Hashmap<String, AnimationTrack<Vector3>> _scalingTracks;

public:
	Animation() {}
	virtual ~Animation() {}

	static Animation* FromData(const Buffer<byte>&);

	void Evaluate(Buffer<Matrix4>& outSkinningMatrices, const Skeleton& skeleton, float time) const;

	AnimationTrack<Vector3>& GetTranslationTrack(const String& joint) { return _translationTracks[joint]; }
	AnimationTrack<Quaternion>& GetRotationTrack(const String& joint) { return _rotationTracks[joint]; }
	AnimationTrack<Vector3>& GetScalingTrack(const String& joint) { return _scalingTracks[joint]; }

	virtual void Read(ByteReader&) override;
	virtual void Write(ByteWriter&) const override;
};
