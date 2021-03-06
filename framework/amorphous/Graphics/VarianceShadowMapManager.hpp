#ifndef  __VarianceShadowMapManager_H__
#define  __VarianceShadowMapManager_H__


#include "ShadowMapManager.hpp"
#include "Shader/ShaderParameter.hpp"


namespace amorphous
{


typedef ShaderParameter< std::vector<float> > ShaderParamFloatArray;


/**
 */
class VarianceShadowMapManager : public ShadowMapManager
{
//	LPDIRECT3DTEXTURE9 m_pHBlurredShadowMap;

//	LPDIRECT3DTEXTURE9 m_pBlurredShadowMap;

	std::shared_ptr<TextureRenderTarget> m_pHBlurredShadowMap;

	std::shared_ptr<TextureRenderTarget> m_pBlurredShadowMap;

	ShaderHandle m_BlurShader;

	ShaderParamFloatArray m_SampleOffsetsH;
	ShaderParamFloatArray m_SampleOffsetsV;
	ShaderParamFloatArray m_SampleWeights;

	enum Params
	{
		NUM_GAUSSIAN_SAMPLES = 15
	};

	Vector2 m_avSampleOffsetH[NUM_GAUSSIAN_SAMPLES];
	Vector2 m_avSampleOffsetV[NUM_GAUSSIAN_SAMPLES];

	float m_afSampleWeights[NUM_GAUSSIAN_SAMPLES];

private:

	TextureFormat::Format GetShadowMapTextureFormat() { return TextureFormat::G16R16F; }

public:

	VarianceShadowMapManager();

	~VarianceShadowMapManager();

	/// returns true on success
	bool Init();

//	void BeginSceneShadowMap();

	/// Creates the blurred shadowmap from the shadowmap
	void EndSceneShadowMap();


	void BeginSceneDepthMap();

	void PostProcessShadowMap( ShadowMap& shadow_map );

//	void EndSceneDepthMap();

	void UpdateLightPositionAndDirection();

//	void PostProcessDirectionalLightShadowMap( DirectionalLightShadowMap& shadow_map );
	void PostProcessDirectionalLightShadowMap( OrthoShadowMap& shadow_map );

	void ReleaseGraphicsResources();

	void LoadGraphicsResources( const GraphicsParameters& rParam );
};

} // namespace amorphous



#endif		/*  __VarianceShadowMapManager_H__  */
