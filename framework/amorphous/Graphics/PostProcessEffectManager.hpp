#ifndef  __PostProcessEffectManager_HPP__
#define  __PostProcessEffectManager_HPP__


#include "amorphous/3DMath/Vector4.hpp"
#include "FloatRGBAColor.hpp"
#include "ShaderHandle.hpp"
#include "PostProcessEffectFilter.hpp"


namespace amorphous
{


class HDRLightingFilter;
class OriginalSceneFilter;
class FullScreenBlurFilter;
class MonochromeColorFilter;


class HDRLightingParams
{
public:

	enum Flags
	{
		TONE_MAPPING              = (1 << 0),
		KEY_VALUE                 = (1 << 1),
		BULE_SHIFT                = (1 << 2),
		LUMINANCE_ADAPTATION_RATE = (1 << 3),
//		STAR_EFFECT_TYPE          = (1 << 4),
	};

	float key_value;
	bool blue_shift;
	bool tone_mapping;

	/// How quickly camera adjust the brightness
	/// - range: (0.0,1.0]
	/// - brightness is adjusted without delay when this is set to 1.0
	float luminance_adaptation_rate;

//	CStarEffectType::Name start_effect;

	HDRLightingParams()
		:
	key_value(0.5f),
	blue_shift(false),
	tone_mapping(false),
	luminance_adaptation_rate(0.02f)
//	start_effect()
	{}
};


/// \brief A struct that encapsulates aspects of a render target postprocess technique.
class PostProcessFilterShader
{
	enum eParam
	{
		// NUM_PARAMS is the maximum number of changeable parameters supported
		// in an effect.
		NUM_PARAMS = 2,
	};

	/// Effect object for this technique
	ShaderHandle m_Shader;

	/// filepath of the HLSL effect file for the effect interface above
	ShaderResourceDesc m_ShaderDesc;

	/// Render target channel this PP outputs
//	int          m_nRenderTarget;

	/// Indicates whether the post-process technique
	///   outputs data for this render target.
	bool         m_bWrite[4];            
	                                     

//	std::string  m_awszParamName[NUM_PARAMS]; ///< Names of changeable parameters
//	std::string  m_awszParamDesc[NUM_PARAMS]; ///< Description of parameters
//	D3DXHANDLE   m_ahParam[NUM_PARAMS];  ///< Handles to the changeable parameters

	/// Size of the parameter. Indicates how many
	/// components of float4 are used.
//	int          m_anParamSize[NUM_PARAMS];

//	Vector4      m_avParamDef[NUM_PARAMS]; ///< Parameter default

public:

	PostProcessFilterShader();

	inline ~PostProcessFilterShader() {}

	Result::Name Init( const ShaderResourceDesc& shader_desc );

	Result::Name Init( const std::string& filename );
	
	const std::string& GetShaderFilename() const { return m_ShaderDesc.ResourcePath; }

	ShaderHandle GetShader() { return m_Shader; }

//	HRESULT OnLostDevice();
//	HRESULT OnResetDevice( DWORD dwWidth, DWORD dwHeight );
//	HRESULT SetScale( float scale_x, float scale_y );
};


class FilterShaderContainer
{
	std::vector< std::shared_ptr<PostProcessFilterShader> > m_vecpShader;

public:

	std::shared_ptr<PostProcessFilterShader> AddShader( const ShaderResourceDesc& shader_desc );

	Result::Name AddShader( const std::string& name );

	std::shared_ptr<PostProcessFilterShader> AddPostProcessEffectShader( const std::string& effect_name );

	std::shared_ptr<PostProcessFilterShader> GetFilterShader( const std::string& name );

	std::shared_ptr<PostProcessFilterShader> GetShader( const std::string& technique_name );

//	std::shared_ptr<PostProcessFilterShader> GetShaderFromFilename( const std::string& filename );
};


struct CRenderTargetChain
{
	// erased
};


/// \brief A class that manages post process effects
class PostProcessEffectManager : public GraphicsComponent
{
	std::shared_ptr<HDRLightingFilter> m_pHDRLightingFilter;

	std::shared_ptr<FullScreenBlurFilter> m_pFullScreenBlurFilter;

	std::shared_ptr<MonochromeColorFilter> m_pMonochromeColorFilter;


	std::shared_ptr<RenderTargetTextureCache> m_pTextureCache;

	FilterShaderContainer m_FilterShaderContainer;

	/// render target to render the scene
	TextureHandle m_SceneRenderTarget;

	std::shared_ptr<OriginalSceneFilter> m_pOriginalSceneFilter;

	std::shared_ptr<PostProcessEffectFilter> m_pFilter;

	std::shared_ptr<RenderTargetTextureHolder> m_pOrigSceneHolder;


	U32 m_EnabledEffectFlags;

	bool m_IsRedering;

	bool m_bUseMultiSampleFloat16;

//	PDIRECT3DSURFACE9 m_pFloatMSRT;
//	PDIRECT3DSURFACE9 m_pFloatMSDS;

//	PDIRECT3DSURFACE9 m_pSurfLDR; /// Low dynamic range surface for final output (original render target)
//	PDIRECT3DSURFACE9 m_pSurfDS;  /// Low dynamic range depth stencil surface

	/// When set to true, displays adapted luminance on a small rectangle at the top-left corner of the display.
	/// - Used for debugging.
	bool m_DisplayAdaptedLuminance;

private:

	/// Used for debugging. See the member variable 'm_DisplayAdaptedLuminance' above.
	void DisplayAdaptedLuminance();

public:

   PostProcessEffectManager();

	~PostProcessEffectManager();

	/// Returns the index of the added shader.
	/// Returns -1 on failure
	int AddPostProcessShader( const std::string& shader_filename );

//	std::vector<CPProcInstance>& GetPostProcessInstance() { return m_vecPPInstance; }
//	CPostProcess& GetPostProcess( int index ) { return m_aPostProcess[index]; }

	Result::Name BeginRender();

	Result::Name EndRender();

	Result::Name RenderPostProcessEffects();

	void Release();

	void SetFirstFilterParams();

//	Result::Name Init( const std::string& filename );

	Result::Name Init( const std::string& base_shader_directory_path = "" );

	Result::Name InitHDRLightingFilter();

	Result::Name InitBlurFilter();

	Result::Name InitMonochromeColorFilter();
	
	Result::Name EnableHDRLighting( bool enable );

	Result::Name EnableBlur( bool enable );

	Result::Name EnableEffect( U32 effect_flags );

	Result::Name DisableEffect( U32 effect_flags );

	bool IsEnabled( U32 flag ) const { return (m_EnabledEffectFlags & flag) ? true : false; }

	void SetHDRLightingParams( U32 hdr_lighting_param_flags, const HDRLightingParams& params );

	void SetBlurStrength( float fBlurStrength );

	void SetMonochromeColorOffset( const SFloatRGBColor& color );

	void ReleaseGraphicsResources();

	void LoadGraphicsResources( const GraphicsParameters& rParam );
};

} // namespace amorphous



#endif  /* __PostProcessEffectManager_HPP__ */
