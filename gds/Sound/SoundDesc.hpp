#ifndef  __SoundDesc_H__
#define  __SoundDesc_H__


#include "fwd.hpp"
#include "SoundSource.hpp"
#include "../3DMath/Vector3.hpp"
#include <string>


namespace amorphous
{


class CSoundDesc
{
public:

	Vector3 Position;

	Vector3 Direction;

	Vector3 Velocity;

	// CSoundSource::StreamType StreamType;
	bool Streamed;

	bool Loop;

	uchar Volume; ///< range: [0,255]

	float MaxDistance;

	float ReferenceDistance;

	float RollOffFactor;

	CSoundSource::Type SoundSourceType;

	/// Sound source management type. CSoundSource::Manual by default.
	/// - auto ... sound source is automatically released after being played.
	/// - manual ... user is responsible for releasing the sound by calling SoundManager().ReleaseSoundSource()
	CSoundSource::Management SourceManagement;

	/// group to which the sound source belongs
	/// e.g., music, effect, etc.
	/// By default,
	/// - non-streamed sound source: group of the sound buffer
	/// - sreamed sound source: 0
	int SoundGroup;

public:

	CSoundDesc()
		:
	Position(Vector3(0,0,0)),
	Direction(Vector3(0,0,0)),
	Velocity(Vector3(0,0,0)),
	SoundSourceType(CSoundSource::Type_Non3DSound),
	Loop(false),
	Streamed(false),
	MaxDistance(1000.0f),
	ReferenceDistance(100.0f),
	RollOffFactor(1.0f),
	Volume(255),
	SourceManagement(CSoundSource::Manual),
	SoundGroup(-1)
	{}

	friend class CSoundManager;
};

} // namespace amorphous



#endif		/*  __SoundDesc_H__  */
