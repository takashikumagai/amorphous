#ifndef __LIGHTSTRUCTS_H__
#define __LIGHTSTRUCTS_H__


#include <float.h>
#include <string>

#include "fwd.hpp"
#include "FloatRGBColor.hpp"
#include "amorphous/3DMath/Vector3.hpp"
#include "amorphous/3DMath/AABB3.hpp" // Used by CZoneAmbientLight
#include "amorphous/Support/Serialization/Serialization.hpp"
#include "amorphous/Support/Serialization/Serialization_3DMath.hpp"
#include "amorphous/Support/Serialization/Serialization_Color.hpp"


namespace amorphous
{
using namespace serialization;


class LightVisitor
{
public:

	virtual void VisitLight( Light& light ) {}
	virtual void VisitAmbientLight( AmbientLight& ambient_light ) {}
	virtual void VisitPointLight( PointLight& point_light ) {}
	virtual void VisitDirectionalLight( DirectionalLight& directional_light ) {}
	virtual void VisitSpotlight( Spotlight& spotlight ) {}
	virtual void VisitHemisphericPointLight( HemisphericPointLight& hs_point_light ) {}
	virtual void VisitHemisphericDirectionalLight( HemisphericDirectionalLight& hs_directional_light ) {}
	virtual void VisitHemisphericSpotlight( HemisphericSpotlight& hs_spotlight ) {}
//	virtual void VisitTriPointLight( CTriPointLight& tri_point_light ) {}
//	virtual void VisitTriDirectionalLight( CTriDirectionalLight& tri_directional_light ) {}
};


class ConstLightVisitor
{
public:

	virtual void VisitLight( const Light& light ) {}
	virtual void VisitAmbientLight( const AmbientLight& ambient_light ) {}
	virtual void VisitPointLight( const PointLight& point_light ) {}
	virtual void VisitDirectionalLight( const DirectionalLight& directional_light ) {}
	virtual void VisitSpotlight( const Spotlight& spotlight ) {}
	virtual void VisitHemisphericPointLight( const HemisphericPointLight& hs_point_light ) {}
	virtual void VisitHemisphericDirectionalLight( const HemisphericDirectionalLight& hs_directional_light ) {}
	virtual void VisitHemisphericSpotlight( const HemisphericSpotlight& hs_spotlight ) {}
//	virtual void VisitTriPointLight( const CTriPointLight& tri_point_light ) {}
//	virtual void VisitTriDirectionalLight( const CTriDirectionalLight& tri_directional_light ) {}
};


class Light : public IArchiveObjectBase
{
public:

	enum Type
	{
		INVALID = -1,
		AMBIENT = 0,
		ZONE_AMBIENT,
		POINT,
		DIRECTIONAL,
		SPOTLIGHT,
		HEMISPHERIC_POINT,
		HEMISPHERIC_DIRECTIONAL,
		HEMISPHERIC_SPOTLIGHT,
		TRI_POINT,
		TRI_DIRECTIONAL,
		NUM_LIGHT_TYPES
	};

protected:

	SFloatRGBColor CalcLightColorFromNdotL( float NdotL )
	{
		if( 0.0f < NdotL )
		{
			return DiffuseColor * NdotL;
		}
		else
			return SFloatRGBColor(0,0,0);
	}

public:

	std::string strName;

	/// Not used by hemispheric lights
	SFloatRGBColor DiffuseColor;

	float fIntensity;

	float fRange;

public:

	inline Light();

	virtual ~Light() {}

	virtual Type GetLightType() const = 0;

	virtual SFloatRGBColor CalcLightAmount( const Vector3& pos, const Vector3& normal ) { return SFloatRGBColor(0,0,0); }

	virtual Vector3 GetPosition() const { return Vector3(0,0,0); }

	virtual Vector3 GetDirection() const { return Vector3(0,-1,0); }

	virtual void SetPosition( const Vector3& pos ) {}

	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void Accept( LightVisitor& visitor ) { visitor.VisitLight( *this ); }

	virtual void Accept( ConstLightVisitor& visitor ) const { visitor.VisitLight( *this ); }
};


class AmbientLight : public Light
{
public:

//	int iAmbientIdentifier;

public:

	AmbientLight() { /*iAmbientIdentifier = 0;*/ }

	Type GetLightType() const { return Light::AMBIENT; }

	virtual SFloatRGBColor CalcLightAmount( const Vector3& pos, const Vector3& normal ) { return DiffuseColor; }

//	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void Accept( LightVisitor& visitor ) { visitor.VisitAmbientLight( *this ); }

	virtual void Accept( ConstLightVisitor& visitor ) const { visitor.VisitAmbientLight( *this ); }
};


class CDistAttenuationLight : public Light
{
public:

	float fAttenuation[3];

public:

	inline CDistAttenuationLight();

	virtual ~CDistAttenuationLight() {}

	inline float CalcAttenuation( float dist_to_light )
	{
		return 1.0f / ( fAttenuation[0] + fAttenuation[1] * dist_to_light + fAttenuation[2] * dist_to_light * dist_to_light );
	}

	inline void SetAttenuation( float a0, float a1, float a2 )
	{
		fAttenuation[0] = a0;
		fAttenuation[1] = a1;
		fAttenuation[2] = a2;
	}

	inline virtual void Serialize( IArchive& ar, const unsigned int version );
};


class DirectionalLight : public Light
{
public:

	Vector3 vPseudoPosition;
	Vector3 vDirection;

public:

	inline DirectionalLight();

	virtual ~DirectionalLight() {}

	Light::Type GetLightType() const { return Light::DIRECTIONAL; }

	virtual SFloatRGBColor CalcLightAmount( const Vector3& pos, const Vector3& normal )
	{
		float d = Vec3Dot( -vDirection, normal );

		if( 0 < d )
			return DiffuseColor * d;
		else
			return SFloatRGBColor(0,0,0);
	}

	virtual Vector3 GetPosition() const { return vPseudoPosition; }

	virtual Vector3 GetDirection() const { return vDirection; }

	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void Accept( LightVisitor& visitor ) { visitor.VisitDirectionalLight( *this ); }

	virtual void Accept( ConstLightVisitor& visitor ) const { visitor.VisitDirectionalLight( *this ); }
};


class PointLight : public CDistAttenuationLight
{
public:

	Vector3 vPosition;

	int FalloffType;

public:

	inline PointLight();

	virtual ~PointLight() {}

	virtual Light::Type GetLightType() const { return Light::POINT; }

	virtual SFloatRGBColor CalcLightAmount( const Vector3& pos, const Vector3& normal )
	{
		Vector3 vToLight = vPosition - pos;
		float dist_to_light = Vec3Length( vToLight );
		Vector3 vDirToLight = vToLight / dist_to_light;

		if( fRange < dist_to_light )
			return SFloatRGBColor(0,0,0);

		float NdotL = Vec3Dot( normal, vDirToLight );

		return CalcPointLightFactor( NdotL ) * fIntensity * CalcAttenuation( dist_to_light );
	}

	virtual Vector3 GetPosition() const { return vPosition; }

	virtual void SetPosition( const Vector3& pos ) { vPosition = pos; }

	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual SFloatRGBColor CalcPointLightFactor( float NdotL )
	{
		return CalcLightColorFromNdotL( NdotL );
	}

	virtual void Accept( LightVisitor& visitor ) { visitor.VisitPointLight( *this ); }

	virtual void Accept( ConstLightVisitor& visitor ) const { visitor.VisitPointLight( *this ); }
};


class Spotlight : public CDistAttenuationLight
{
public:

	Vector3 vPosition;
	Vector3 vDirection;

	float fInnerConeAngle;
	float fOuterConeAngle;

	float fFalloff;

public:

	Spotlight()
		:
	vPosition( Vector3(0,0,0) ),
	vDirection( Vector3(0,-1,0) ),
	fInnerConeAngle(3.14159f * 0.25f),
	fOuterConeAngle(3.14159f * 0.25f),
	fFalloff(1.0f)
	{}

	virtual ~Spotlight() {}

	virtual Light::Type GetLightType() const { return Light::SPOTLIGHT; }

	virtual SFloatRGBColor CalcSpotlightColor( float NdotL )
	{
		return CalcLightColorFromNdotL( NdotL );
	}

	SFloatRGBColor CalcLightAmount( const Vector3& pos, const Vector3& normal )
	{
		Vector3 vToLight = vPosition - pos;
		float dist_to_light = Vec3Length( vToLight );
		Vector3 vDirToLight = vToLight / dist_to_light;

		if( fRange < dist_to_light )
			return SFloatRGBColor(0,0,0);

		float dot = Vec3Dot( -vDirToLight, vDirection );

		if( dot < 0 )
			return SFloatRGBColor(0,0,0);

		const float angle = acos( dot );

		if( fOuterConeAngle < angle )
			return SFloatRGBColor(0,0,0);

/*		else if( fInnerConeAngle < angle )
		{
			if( fFalloff < 1.0f )
			{
				// calc attenuation between inner and outer cone
				return ;
			}
		}*/

		float NdotL = Vec3Dot( normal, vDirToLight );

		return CalcSpotlightColor( NdotL ) * fIntensity * CalcAttenuation( dist_to_light );
	}

	inline virtual void Serialize( IArchive& ar, const unsigned int version );

	virtual void Accept( LightVisitor& visitor ) { visitor.VisitSpotlight( *this ); }

	virtual void Accept( ConstLightVisitor& visitor ) const { visitor.VisitSpotlight( *this ); }
};


//------------------------------- ZoneAmbientLight (experimental) -------------------------------
// A light which is applied to objects included in an aabb
// - introduced experimentally to adjust the indoor brightness

class CZoneAmbientLight : public Light
{
public:

	AABB3 m_AAABB;
};



//========================== inline implementations ==========================

inline Light::Light()
:
DiffuseColor(SFloatRGBColor(0,0,0)),
fIntensity(1.0f),
fRange(FLT_MAX)
{
}


inline void Light::Serialize( IArchive& ar, const unsigned int version )
{
	ar & strName;
	ar & DiffuseColor;
	ar & fIntensity;
	ar & fRange;
}


inline CDistAttenuationLight::CDistAttenuationLight()
{
	fAttenuation[0] = 0.00f;
	fAttenuation[1] = 0.01f;
	fAttenuation[2] = 0.00f;
}


inline void CDistAttenuationLight::Serialize( IArchive& ar, const unsigned int version )
{
	Light::Serialize( ar, version );

	for( int i=0; i<3; i++ )
		ar & fAttenuation[i];
}


inline DirectionalLight::DirectionalLight()
{
	vDirection      = Vector3(0,-1,0);
	vPseudoPosition = Vector3(0,0,0);
}


inline void DirectionalLight::Serialize( IArchive& ar, const unsigned int version )
{
	Light::Serialize( ar, version );

	ar & vPseudoPosition;
	ar & vDirection;
}


inline PointLight::PointLight()
:
vPosition(Vector3(0,0,0))
{
	FalloffType     = 0;
}


inline void PointLight::Serialize( IArchive& ar, const unsigned int version )
{
	CDistAttenuationLight::Serialize( ar, version );

	ar & vPosition;
	ar & FalloffType;
}


inline void Spotlight::Serialize( IArchive& ar, const unsigned int version )
{
	CDistAttenuationLight::Serialize( ar, version );

	ar & vPosition;
	ar & vDirection;
	ar & fInnerConeAngle;
	ar & fOuterConeAngle;
	ar & fFalloff;
}

} // namespace amorphous



#endif  /*  __LIGHTSTRUCTS_H__  */
