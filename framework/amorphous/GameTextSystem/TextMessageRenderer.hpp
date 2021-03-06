#ifndef  __TextMessageRenderer_H__
#define  __TextMessageRenderer_H__


#include <vector>
#include <string>

#include "amorphous/Graphics/fwd.hpp"
#include "amorphous/Graphics/2DPrimitive/2DRect.hpp"
#include "amorphous/Graphics/TextureHandle.hpp"
#include "amorphous/Graphics/GraphicsEffectManager.hpp"
#include "amorphous/Graphics/Rect.hpp"
#include "amorphous/Support/memory_helpers.hpp"


namespace amorphous
{

#define GTC_NUM_MAXLETTERS_PER_LINE 64


#define TEXTSET_INVALID_INDEX -1


/**
 Defines the interfaces to render messages.
 */
class CTextMessageRenderer : public GraphicsComponent
{
//	CTextMessageWindow *m_pWindow;

protected:

	std::shared_ptr<GraphicsElementAnimationManager> m_pEffectManager;

public:

	CTextMessageRenderer( std::shared_ptr<GraphicsElementAnimationManager> pEffectMgr = std::shared_ptr<GraphicsElementAnimationManager>() );

	virtual ~CTextMessageRenderer();

//	void Release();

	virtual void UpdateSpeaker( const char *pSpeaker ) {}

	virtual void UpdateText( const char *pText ) {}

	virtual void OnTextMessageCleared() {}

	virtual void Update( float dt );

	virtual void Render() = 0;
};


/**
 Renders text messages and related graphical components.
 This is a sample implementation of CTextMessageRenderer
 */
class CDefaultTextMessageRenderer : public CTextMessageRenderer
{
	enum PrivateParams
	{
		MAX_TEXT_LENGTH = 256,
		NUM_MAX_ICONTEXTURES = 64,
	};

	enum FontTypes
	{
		FONT_SPEAKER,
		FONT_TEXT,
		NUM_FONTS
	};

	int m_aFontID[NUM_FONTS];

	/// holds window size in 800x600 resolution
	SRect m_BaseWindowRect;

	/// background rectangle of window
	C2DRect m_WindowRect;

	std::shared_ptr<FillRectElement> m_pWindowBGRect;

	std::shared_ptr<TextElement> m_pText;

	std::shared_ptr<TextElement> m_pSpeaker;

	/// texture for window rectangle
	TextureHandle m_WindowTexture;

	C2DRect m_TexturedIcon;

	GraphicsElementAnimationHandle m_BGRectFade;

	float m_fBGRectAlpha;


//	TCFixedVector< TextureHandle, NUM_MAX_ICONTEXTURES > m_IconTexture;

//	std::string m_strFontName[NUM_FONTS];

	SPoint m_BaseFontSize;

	/// buffer to hold speaker name
	char m_acSpeaker[MAX_TEXT_LENGTH-1];

	/// buffer to hold message text
    char m_acText[MAX_TEXT_LENGTH-1];

	SPoint m_vTextPos;
	SPoint m_vSpeakerPos;

public:

	CDefaultTextMessageRenderer( std::shared_ptr<GraphicsElementAnimationManager> pEffectMgr,
		int top_layer,
		int bottom_layer );

	~CDefaultTextMessageRenderer();

	inline bool NoMessage();

	void Update( float dt );

	void Render();

	void UpdateSpeaker( const char *pSpeaker );

	void UpdateText( const char *pText );

	void OnTextMessageCleared();

//	void UpdateScreenSize();
	void ReleaseGraphicsResources();
	void LoadGraphicsResources( const GraphicsParameters& rParam );
};


inline bool CDefaultTextMessageRenderer::NoMessage()
{
	if( strlen(m_acSpeaker) == 0 && strlen(m_acText) == 0 )
		return true;
	else
		return false;
}


class CNullTextMessageRenderer : public CTextMessageRenderer
{

public:

	CNullTextMessageRenderer( std::shared_ptr<GraphicsElementAnimationManager> pEffectMgr,
		int top_layer,
		int bottom_layer ) {}

	~CNullTextMessageRenderer() {}

	void Update( float dt ) {}

	void Render() {}

	void UpdateSpeaker( const char *pSpeaker ) {}

	void UpdateText( const char *pText ) {}

	void ReleaseGraphicsResources() {}
	void LoadGraphicsResources( const GraphicsParameters& rParam ) {}
};

} // namespace amorphous



#endif		/*  __TextMessageRenderer_H__  */
