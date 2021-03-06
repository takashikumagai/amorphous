#ifndef  __SCREENEFFECTMANAGER_H__
#define  __SCREENEFFECTMANAGER_H__

#include <map>
#include <memory>
#include "amorphous//base.hpp"
#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/TextureRenderTarget.hpp"
#include "amorphous/Graphics/GraphicsComponentCollector.hpp"
#include "amorphous/Graphics/Shader/ShaderTechniqueHandle.hpp"
#include "amorphous/Graphics/PseudoNoiseEffect.hpp"
#include "amorphous/Support/FixedVector.hpp"
#include "ScreenEffect.hpp"


namespace amorphous
{


class CScreenEffectTargetSceneRenderer : public GraphicsComponent
{
public:

	CScreenEffectTargetSceneRenderer() {}
	virtual ~CScreenEffectTargetSceneRenderer() {}
	
	virtual void CreateRenderTasks( bool create_scene_render_task ) {}

	virtual void RenderSceneForScreenEffect( Camera& rCamera ) {}
};


#define NUM_MAX_SIMULTANEOUS_FADES 8

/*
enum ePPEffectFile
{
	PP_COLOR_GBLUR_H,
	PP_COLOR_GBLUR_V,
    PP_COLOR_INVERSE,
	PP_COLOR_MONOCHROME,
	PP_COLOR_DOWNFILTER4,
	PP_COLOR_UPFILTER4,
	PP_COLOR_BLOOM_H,
	PP_COLOR_BLOOM_V,
	PP_COLOR_BRIGHTPASS,
	PP_COLOR_COMBINE4,
	NUM_PP_EFFECT_FILES
};


enum eSizeFilterType
{
	SF_DOWNFILTER_FOR_BLUR,
	SF_UPFILTER_FOR_BLUR,
	SF_DOWNFILTER_FOR_BLOOM,
	SF_UPFILTER_FOR_BLOOM,
	SF_BRIGHTPASS,
	NUM_FILTERS,
};
*/

class ScreenEffect
{
public:
	enum screen_effect_flag
	{
		PostProcessEffects	= (1 << 0),	///< enables/disables all the rests of the post process effects
		PseudoNightVision	= (1 << 1),
		Noise				= (1 << 2),
		PseudoBlur			= (1 << 3),
		PseudoMotionBlur	= (1 << 4),
		MonochromeColor		= (1 << 5),
		Glare				= (1 << 6),
		ShadowMap			= (1 << 7),	///< experimental: should not be handled by ScreenEffectManager
		CubeEnvMap          = (1 << 8),
		HDRLighting         = (1 << 9),
	};
};


/**
 - 'ScreenEffectManager' is a misnomer. This class has a couple of roles
   - holds the global shader
   - manages post process(PP) effects

 - About the global shader
   - the global shader is used to render all the entities except for static geometries
   - set view and projection matrices in BeginRender(camera)
	 - use the camera handed as the argument to get view and projection matrices

 - About Post Process Effects
   - There are 2 types of post process effects
     - pp effect by pixel shader
	   - uses PostProcessEffectManager class
     - simpler pp effect
	   - rendering screen size quad over the entire screen
	     - e.g.) fade in/out, noise
   - call BeginRender() before rendering the scene
     - if pp effects are enabled, texture render target is set and original render target is saved
   - (call EndRender() after rendering the scene)
   - then, call RenderPostProcessEffects() to render the scene with post process effects
     to the original render target

*/
class ScreenEffectManager : public GraphicsComponent
{
public:

	enum eParam
	{
		NUM_MAX_EXTRA_TEXTURES = 8,
		MIN_EFFECT_PRIORITY_ID = 256,
		MAX_EFFECT_PRIORITY_ID = 1024,
		NUM_MAX_PARAM_SETS_PER_EFFECT = 8,
	};

	/// simple post process effect which render the textured rect over the entire screen
	class CExtraTextureEffect
	{
	public:

		TextureResourceDesc m_Desc;
		TextureHandle m_Texture;
		C2DRect Rect;

	public:

		CExtraTextureEffect() {}

		void SetTextureFilename( const std::string& texture_filepath ) { m_Desc.ResourcePath = texture_filepath; }
		bool LoadTexture();
//		void ReleaseTexture() { m_Texture.Release(); }
	};

private:

	/// holds the active effects
	unsigned int m_EffectFlag;

	FadeEffect m_aFadeEffect[NUM_MAX_SIMULTANEOUS_FADES];

	C2DRect m_ScreenColor;

	/// manages post-process effects
	std::shared_ptr<PostProcessEffectManager> m_pPPEffectManager;

//	int m_aPPEffectIndex[NUM_PP_EFFECT_FILES];

//	int m_aFilterIndex[NUM_FILTERS];

	std::shared_ptr<SimpleMotionBlur> m_pSimpleMotionBlur;

	PseudoNoiseEffect m_NoiseEffect;

	TCFixedVector< CExtraTextureEffect, NUM_MAX_EXTRA_TEXTURES > m_vecExtraTexEffect;

	std::shared_ptr<LensFlare> m_pLensFlare;

//	TextureRenderTarget m_TexRenderTarget;

	// 0 if no texture is set as a render target
	// non zero if a texture is set as a render target
	int m_iTexRenderTargetEnabled;

	float m_fBlurWidth;
	float m_fBlurHeight;

	std::map< int,float,std::greater<int> > m_mapGlareLuminanceThreshold;

	std::map< int,Vector2,std::greater<int> > m_mapBlurStrength;

	std::map< int,float,std::greater<int> > m_mapMotionBlurStrength;

	std::map< int,SFloatRGBColor,std::greater<int> > m_mapMonochromeColor;

	float m_fGlareLuminanceThreshold;

//	float m_MonochromeColorOffset[4];

	CScreenEffectTargetSceneRenderer *m_pTargetSceneRenderer;

	ShaderTechniqueHandle m_DefaultShaderTechnique;

	bool m_bPostProcessEffectEnabled; ///< used to temporarily hold whether post process is enabled during one CreateRenderTask() & Render() turn

	bool m_bPseudoMotionBlurEnabled; ///< used to temporarily hold whether motion blur is enabled during one CreateRenderTask() & Render() turn


//	static std::string ms_DefaultShaderFilename;


private:

    void UpdateBlurEffect();

	void UpdateGlareEffect();

	void UpdateMonochromeColorEffect();

	void UpdateMotionBlurParams();

	void RenderOverlayEffects();

public:

	ScreenEffectManager();

	~ScreenEffectManager();

	Result::Name InitPostProcessEffect();

	void ReleasePostProcessEffect();

	Result::Name InitMotionBlurEffect();

	void ReleaseMotionBlurEffect();

	Result::Name InitNoiseEffect();

	void ReleaseNoiseEffect();

	bool Init();

	/// set texture as render target to apply post process effects later
	void BeginRender( const Camera &rCam );

	void EndRender();

	void Render( Camera &rCam );

	/// call this after render the scene
	/// \param frametime used by fade-in/fade-out effects
	/// should be no BeginScene() & EndScene() pair inside?
	void RenderPostProcessEffects();

	/// must be called every frame
	void Update( float frametime );

//	void AddScreenColor( U32 dwColor );
	void FadeOutTo( const SFloatRGBAColor& color, float fTime, AlphaBlend::Mode alpha_blend_mode );
	void FadeInFrom( const SFloatRGBAColor& color, float fTime, AlphaBlend::Mode alpha_blend_mode );
	void SetFadeEffect( int iType, const SFloatRGBAColor& color, float fTime, AlphaBlend::Mode alpha_blend_mode );
//	bool LoadShader();

	unsigned int GetEffectFlag() const { return m_EffectFlag; }

	inline void SetEffectFlag( unsigned int flag ) { m_EffectFlag = flag; }

	inline void RaiseEffectFlag( unsigned int flag ) { m_EffectFlag |= flag; }

	inline void ClearEffectFlag( unsigned int flag ) { m_EffectFlag &= ~flag; }

	inline void SetBlurEffect( float fBlurFactor, int priority_id = MIN_EFFECT_PRIORITY_ID );
	inline void SetBlurEffect( int width, int height, int priority_id = MIN_EFFECT_PRIORITY_ID );
	inline void SetBlurEffect( float width, float height, int priority_id = MIN_EFFECT_PRIORITY_ID );
//	inline void ClearBlurEffect() { m_fBlurWidth = m_fBlurHeight = 0; ClearEffectFlag( ScreenEffect::PseudoBlur ); }
	inline void ClearBlurEffect( int priority_id = MIN_EFFECT_PRIORITY_ID );

	inline void SetMotionBlurWeight( float motion_blur_weight, int priority_id = MIN_EFFECT_PRIORITY_ID );
	inline void ClearMotionBlur( int priority_id = MIN_EFFECT_PRIORITY_ID );

	/// \param blend_ratio (NOT IMPLEMENTED YET) How much monochrome you want to make the rendered image
	inline void SetMonochromeEffect( float r, float g, float b, float blend_ratio = 1.0f, int priority_id = MIN_EFFECT_PRIORITY_ID );
	inline void ClearMonochromeEffect( int priority_id = MIN_EFFECT_PRIORITY_ID );

//	inline void SetGlareLuminanceThreshold( float fLuminance ) { m_fGlareLuminanceThreshold = fLuminance; }

	inline void SetGlareLuminanceThreshold( float fLuminance, int priority_id = MIN_EFFECT_PRIORITY_ID );
	inline void ClearGlareLuminanceThreshold( int priority_id = MIN_EFFECT_PRIORITY_ID );

	Result::Name SetHDRLightingParams( U32 param_flags, const HDRLightingParams& params );
	Result::Name EnableHDRLighting( bool enable );

	std::shared_ptr<LensFlare> GetLensFlare() { return m_pLensFlare; }

	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const GraphicsParameters& rParam );
	void UpdateScreenSize();

//	void SetShaderManager();

	void CreateRenderTasks();

	void SetTargetSceneRenderer( CScreenEffectTargetSceneRenderer* pTargetSceneRenderer ) { m_pTargetSceneRenderer = pTargetSceneRenderer; }

//	static void SetDefaultShaderFilename( const std::string& defualt_shader_filename ) { ms_DefaultShaderFilename = defualt_shader_filename; }

};


//========================== inline implementations ==========================

inline void ScreenEffectManager::SetBlurEffect( float fBlurFactor, int priority_id )
{
	SetBlurEffect( fBlurFactor, fBlurFactor, priority_id );
}


inline void ScreenEffectManager::SetBlurEffect( int width, int height, int priority_id )
{
	SetBlurEffect( (float)width, (float)height, priority_id );
}


inline void ScreenEffectManager::SetBlurEffect( float width, float height, int priority_id )
{
//	m_fBlurWidth  = width;
//	m_fBlurHeight = height;

	if( m_mapBlurStrength.size() < NUM_MAX_PARAM_SETS_PER_EFFECT )
	{
		m_mapBlurStrength[priority_id] = Vector2( width, height );
		RaiseEffectFlag( ScreenEffect::PseudoBlur );
	}
}


inline void ScreenEffectManager::ClearBlurEffect( int priority_id )
{
	m_mapBlurStrength.erase( priority_id );

	if( m_mapBlurStrength.size() == 0 )
		ClearEffectFlag( ScreenEffect::PseudoBlur );
}


//	inline void  { m_fBlurWidth = m_fBlurHeight = 0; ClearEffectFlag( ScreenEffect::PseudoBlur ); }


inline void ScreenEffectManager::SetMotionBlurWeight( float motion_blur_weight, int priority_id )
{
	if( m_mapMotionBlurStrength.size() < NUM_MAX_PARAM_SETS_PER_EFFECT )
	{
		m_mapMotionBlurStrength[priority_id] = motion_blur_weight;
		RaiseEffectFlag( ScreenEffect::PseudoMotionBlur );
	}
}


inline void ScreenEffectManager::ClearMotionBlur( int priority_id )
{
	m_mapMotionBlurStrength.erase( priority_id );

	if( m_mapMotionBlurStrength.size() == 0 )
		ClearEffectFlag( ScreenEffect::PseudoMotionBlur );
}


inline void ScreenEffectManager::SetMonochromeEffect( float r, float g, float b, float blend_ratio, int priority_id )
{
	if( m_mapMonochromeColor.size() < NUM_MAX_PARAM_SETS_PER_EFFECT )
	{
		m_mapMonochromeColor[priority_id] = SFloatRGBColor( r, g, b );
		RaiseEffectFlag( ScreenEffect::MonochromeColor );
	}

/*	m_MonochromeColorOffset[0] = r;
	m_MonochromeColorOffset[1] = g;
	m_MonochromeColorOffset[2] = b;
	m_MonochromeColorOffset[3] = 1.0f;*/
}


inline void ScreenEffectManager::ClearMonochromeEffect( int priority_id )
{
	m_mapMonochromeColor.erase( priority_id );

	if( m_mapMonochromeColor.size() == 0 )
		ClearEffectFlag( ScreenEffect::MonochromeColor );
}


inline void ScreenEffectManager::SetGlareLuminanceThreshold( float fLuminance, int priority_id )
{
	if( m_mapGlareLuminanceThreshold.size() < NUM_MAX_PARAM_SETS_PER_EFFECT )
	{
		m_mapGlareLuminanceThreshold[priority_id] = fLuminance;
		RaiseEffectFlag( ScreenEffect::Glare );
	}
}


inline void ScreenEffectManager::ClearGlareLuminanceThreshold( int priority_id )
{
	m_mapGlareLuminanceThreshold.erase( priority_id );

	if( m_mapGlareLuminanceThreshold.size() == 0 )
		ClearEffectFlag( ScreenEffect::Glare );
}

} // namespace amorphous



#endif		/*  __SCREENEFFECTMANAGER_H__  */
