#ifndef  __SimpleMotionBlur_HPP__
#define  __SimpleMotionBlur_HPP__


#include <memory>
#include "fwd.hpp"


namespace amorphous
{


/**

 SimpleMotionBlur blur;

 blur.Begin();
 
 /// render scene ///

 blur.End();

 // render the scene with motion blur
 blur.Render();
 
 NOTE: derived from GraphicsComponent to retrieve the screen resolution in InitForScreenSize()
 */
class SimpleMotionBlur// : public GraphicsComponent
{
	/// used to render the scene
	std::shared_ptr<TextureRenderTarget> m_pSceneRenderTarget;

	std::shared_ptr<TextureRenderTarget> m_apTexRenderTarget[2];

	int m_TextureWidth;
	int m_TextureHeight;

	int m_TargetTexIndex;

	float m_fBlurWeight;

	bool m_bFirst;
	
private:

	void InitTextureRenderTargetBGColors();
	
public:

	SimpleMotionBlur();

	SimpleMotionBlur( int texture_width, int texture_height );

	~SimpleMotionBlur();

	void Init( int texture_width, int texture_height );

	/// Initialize for fullscreen motion blur effect.
	/// Texture render targets are automatically resized when the screen resolution is changed.
	void InitForScreenSize();

//	void SetRenderTarget();
//	void ResetRenderTarget();

	void Begin();

	void End();

	/// draw fullscreen rect with blurrred scene texture
	void Render();

	void SetBlurWeight( float weight ) { m_fBlurWeight = weight; }

	void UpdateScreenSize();

	bool LoadTextures();

	void ReleaseTextures();

//	void ReleaseGraphicsResources() {}

//	void LoadGraphicsResources( const GraphicsParameters& rParam ) {}
};

} // namespace amorphous



#endif		/*  __SimpleMotionBlur_HPP__  */
