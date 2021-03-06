#ifndef  __D3DFixedFunctionPipelineLightManager_HPP__
#define  __D3DFixedFunctionPipelineLightManager_HPP__


#include <d3d9.h>
#include "Graphics/Shader/ShaderLightManager.hpp"
#include "Graphics/Direct3D/Conversions.hpp"
#include "Graphics/Direct3D/Direct3D9.hpp"


namespace amorphous
{


class CD3DFixedFunctionPipelineLightManager : public ShaderLightManager
{
//	static const int m_iNumMaxLights = 6;	error

	enum eNumLights
	{
		NUM_MAX_LIGHTS = 8
	};

	int m_NumLights;

public:

	CD3DFixedFunctionPipelineLightManager();

	~CD3DFixedFunctionPipelineLightManager();

	bool Init();

	void CommitChanges();

	inline void SetAmbientLight( const AmbientLight& light );
	inline void SetDirectionalLight( const DirectionalLight& light );
	inline void SetPointLight( const PointLight& light );
	inline void SetSpotlight( const Spotlight& light );
	inline void SetHemisphericDirectionalLight( const HemisphericDirectionalLight& light );
	inline void SetHemisphericPointLight( const HemisphericPointLight& light );
	inline void SetHemisphericSpotlight( const HemisphericSpotlight& light );
//	inline void SetTriDirectionalLight( const TriDirectionalLight& light );
//	inline void SetTriPointLight( const TriPointLight& light );

	/// set light to shader variables
	/// user is responsible for calling CommitChanges() after setting the light
//	inline void SetLight( const int index, const CHemisphericPointLight& rLight );

//	inline void SetLight( const int index, const CHemisphericDirectionalLight& rLight );

	/// set the number of directional lights
	/// user is responsible for calling CommitChanges() after this call
	inline void SetNumDirectionalLights( const int iNumDirectionalLights );

	inline void SetDirectionalLightOffset( const int iDirectionalLightOffset );

	/// set the number of point lights
	/// user is responsible for calling CommitChanges() after this call
	inline void SetNumPointLights( const int iNumPointLights );

	inline void SetPointLightOffset( const int iPointLightOffset );

	void LoadGraphicsResources( const GraphicsParameters& rParam );

	void ReleaseGraphicsResources();

//	void ClearLights() { m_LightCache.Clear(); }
	void ClearLights() { m_NumLights = 0; }

	friend class ShaderLightManager;

private:

//	CLightCache m_LightCache;
};


//================================= inline implementations =================================

inline D3DCOLORVALUE ToD3DCOLORVALUE( const SFloatRGBColor& src )
{
	D3DCOLORVALUE dest;
	dest.r = src.red;
	dest.g = src.green;
	dest.b = src.blue;
	dest.a = 1.0f;
	return dest;
}

inline D3DCOLORVALUE ToD3DCOLORVALUE( const SFloatRGBAColor& src )
{
	D3DCOLORVALUE dest;
	dest.r = src.red;
	dest.g = src.green;
	dest.b = src.blue;
	dest.a = src.alpha;
	return dest;
}

/*
inline void CD3DFixedFunctionPipelineLightManager::SetNumDirectionalLights( const int iNumDirectionalLights )
{
}


inline void CD3DFixedFunctionPipelineLightManager::SetDirectionalLightOffset( const int iDirectionalLightOffset )
{
}


inline void CD3DFixedFunctionPipelineLightManager::SetNumPointLights( const int iNumPointLights )
{
}


inline void CD3DFixedFunctionPipelineLightManager::SetPointLightOffset( const int iPointLightOffset )
{
}
*/
/*
inline void CD3DFixedFunctionPipelineLightManager::SetLight( const int index, const HemisphericPointLight& rLight )
{
}


inline void CD3DFixedFunctionPipelineLightManager::SetLight( const int index, const HemisphericDirectionalLight& rLight )
{
}
*/

inline void CD3DFixedFunctionPipelineLightManager::SetAmbientLight( const AmbientLight& light )
{
}


inline void CD3DFixedFunctionPipelineLightManager::SetDirectionalLight( const DirectionalLight& light )
{
//	m_LightCache.vecDirecitonalLight.push_back( light );
	D3DLIGHT9 d3d_light;
	d3d_light.Type          = D3DLIGHT_DIRECTIONAL;
	d3d_light.Diffuse       = ToD3DCOLORVALUE( light.DiffuseColor );
	d3d_light.Specular      = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Ambient       = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Position      = ToD3DVECTOR( Vector3(0,0,0) );
	d3d_light.Direction     = ToD3DVECTOR( light.vDirection );
	d3d_light.Range         = 100000.0f;
	d3d_light.Falloff       = 0.0f;
	d3d_light.Attenuation0  = 0.0f; 
	d3d_light.Attenuation1  = 0.0f; 
	d3d_light.Attenuation2  = 0.0f; 
	d3d_light.Theta         = 0.0f;
	d3d_light.Phi           = 0.0f;

	HRESULT hr = S_OK;
	hr = DIRECT3D9.GetDevice()->LightEnable( m_NumLights, TRUE );
	hr = DIRECT3D9.GetDevice()->SetLight( m_NumLights, &d3d_light );
	m_NumLights += 1;
}


inline void CD3DFixedFunctionPipelineLightManager::SetPointLight( const PointLight& light )
{
//	m_LightCache.vecPointLight.push_back( light );
	D3DLIGHT9 d3d_light;
	d3d_light.Type          = D3DLIGHT_POINT;
	d3d_light.Diffuse       = ToD3DCOLORVALUE( light.DiffuseColor );
	d3d_light.Specular      = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Ambient       = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Position      = ToD3DVECTOR( light.vPosition );
	d3d_light.Direction     = ToD3DVECTOR( Vector3(0,0,0) );
	d3d_light.Range         = 100000.0f;
	d3d_light.Falloff       = 0.0f;
	d3d_light.Attenuation0  = light.fAttenuation[0]; 
	d3d_light.Attenuation1  = light.fAttenuation[1]; 
	d3d_light.Attenuation2  = light.fAttenuation[2]; 
	d3d_light.Theta         = 0.0f;
	d3d_light.Phi           = 0.0f;

	HRESULT hr = S_OK;
	hr = DIRECT3D9.GetDevice()->LightEnable( m_NumLights, TRUE );
	hr = DIRECT3D9.GetDevice()->SetLight( m_NumLights, &d3d_light );
	m_NumLights += 1;
}


inline void CD3DFixedFunctionPipelineLightManager::SetSpotlight( const Spotlight& light )
{
//	m_LightCache.vecPointLight.push_back( light );
	D3DLIGHT9 d3d_light;
	d3d_light.Type          = D3DLIGHT_SPOT;
	d3d_light.Diffuse       = ToD3DCOLORVALUE( light.DiffuseColor );
	d3d_light.Specular      = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Ambient       = ToD3DCOLORVALUE( SFloatRGBColor::Black() );
	d3d_light.Position      = ToD3DVECTOR( light.vPosition );
	d3d_light.Direction     = ToD3DVECTOR( Vector3(0,0,0) );
	d3d_light.Range         = 100000.0f;
	d3d_light.Falloff       = 0.0f;
	d3d_light.Attenuation0  = light.fAttenuation[0]; 
	d3d_light.Attenuation1  = light.fAttenuation[1]; 
	d3d_light.Attenuation2  = light.fAttenuation[2]; 
	d3d_light.Theta         = 0.0f;
	d3d_light.Phi           = 0.0f;

	HRESULT hr = S_OK;
	hr = DIRECT3D9.GetDevice()->LightEnable( m_NumLights, TRUE );
	hr = DIRECT3D9.GetDevice()->SetLight( m_NumLights, &d3d_light );
	m_NumLights += 1;
}


inline void CD3DFixedFunctionPipelineLightManager::SetHemisphericDirectionalLight( const HemisphericDirectionalLight& light )
{
//	m_LightCache.vecHSDirecitonalLight.push_back( light );
}


inline void CD3DFixedFunctionPipelineLightManager::SetHemisphericPointLight( const HemisphericPointLight& light )
{
//	m_LightCache.vecHSPointLight.push_back( light );
}


inline void CD3DFixedFunctionPipelineLightManager::SetHemisphericSpotlight( const HemisphericSpotlight& light )
{
//	m_LightCache.vecHSSpotlight.push_back( light );
}


} // namespace amorphous



#endif /* __D3DFixedFunctionPipelineLightManager_HPP__ */
