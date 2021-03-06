#include "HUD_PlayerShip.hpp"
#include "BE_PlayerShip.hpp"
#include "PlayerInfo.hpp"
#include "Input/InputHandler_PlayerShip.hpp"

#include "amorphous/Graphics/GraphicsDevice.hpp"
#include "amorphous/Item/WeaponSystem.hpp"
#include "amorphous/GameCommon/3DActionCode.hpp"
#include "amorphous/GameTextSystem/GameTextWindow.hpp"
#include "amorphous/Support/memory_helpers.hpp"

#include "amorphous/App/GameWindowManager.hpp"


namespace amorphous
{


HUD_PlayerShip::HUD_PlayerShip()
{
	m_pTextWindow = NULL;
}


HUD_PlayerShip::~HUD_PlayerShip()
{
	SafeDelete( m_pTextWindow );
}


void HUD_PlayerShip::UpdateScreenSize()
{
	float fScreenWidth  = (float)GetGameWindowManager().GetScreenWidth();
//	float fScreenHeight = ms_iScreenHeight;

	// background rectangle in translucent black
	Vector2 vMin, vMax;
	vMin = Vector2( (0.5f - 0.0781f) * fScreenWidth, (0.64f - 0.0781f) * fScreenWidth );
	vMax = Vector2( (0.5f + 0.0781f) * fScreenWidth, (0.64f + 0.0781f) * fScreenWidth );
	m_StatusBackGround.SetPosition(vMin,vMax);
	m_StatusBackGround.SetColor(0x50000000);

//	vMin = Vector2(320-40, 370);
//	vMax = Vector2(320+40, 450);
	vMin = Vector2( (0.5f - 0.0625f) * fScreenWidth, (0.64f - 0.0625f) * fScreenWidth );
	vMax = Vector2( (0.5f + 0.0625f) * fScreenWidth, (0.64f + 0.0625f) * fScreenWidth );
	m_ShipIcon.SetPosition(vMin,vMax);
	m_ShipIcon.SetTextureUV( TEXCOORD2(0.0f,0.0f), TEXCOORD2(1.0f,1.0f) );

	vMin = Vector2( (0.5f - 0.05f) * fScreenWidth, (0.375f - 0.05f) * fScreenWidth );
	vMax = Vector2( (0.5f + 0.05f) * fScreenWidth, (0.375f + 0.05f) * fScreenWidth );
	m_Crosshair.SetPosition(vMin,vMax);
	m_Crosshair.SetTextureUV( TEXCOORD2(0.0f,0.0f), TEXCOORD2(1.0f,1.0f) );
	m_Crosshair.SetColor( 0xFFFFFFFF );
}


void HUD_PlayerShip::Init()
{
	LoadTextures();

//	m_StatusFont.InitFont( "Texture\\MainFont.dds", 256, 256, 16, 8 );
	m_StatusFont.InitFont( "./Texture/BitstreamVeraSansMono_Bold.dds", 256, 256, 16, 8 );

	m_pTextWindow = new CGameTextWindow;
	m_pTextWindow->InitFont( "�l�r �S�V�b�N", 0.018f, 0.036f );

	UpdateScreenSize();
}


void HUD_PlayerShip::LoadTextures()
{
	m_ShipIconTexture.Load( "./Texture/ShipIcon.dds" );

	m_CrosshairTexture.Load( "./Texture/Crosshair.dds" );
}


void HUD_PlayerShip::RenderImpl()
{
	// enable alpha blending
//	LPDIRECT3DDEVICE9 pd3dDev = DIRECT3D9.GetDevice();
	GraphicsDevice().Enable( RenderStateType::ALPHA_BLEND );
	GraphicsDevice().SetSourceBlendMode( AlphaBlend::SrcAlpha );
	GraphicsDevice().SetDestBlendMode(   AlphaBlend::One );

	// blend texture color and diffuse color
//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
//	pd3dDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );

//	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
//	pd3dDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	U32 dwFontColor = 0x7030B030;

	// background for status display
	m_StatusBackGround.Draw();

	// display current life by color
	float fLife = SinglePlayerInfo().GetCurrentPlayerBaseEntity()->GetCurrentLife();
	float fOrigLife = fLife;
	float fMaxGreenLife = 100.0f; //PlayerShip.GetMaxLife()
	if( fMaxGreenLife < fLife )
		fLife = fMaxGreenLife;
	else if( fLife < 0.0f )
		fLife = 0.0f;

	U32 dwColor = D3DCOLOR_XRGB( 250 - (int)(250*fLife/fMaxGreenLife),	// red
								  (int)(250*fLife/fMaxGreenLife),			// green
								  0 );										// blue
	m_ShipIcon.SetColor(dwColor);
	m_ShipIcon.Draw( m_ShipIconTexture );

	m_Crosshair.Draw( m_CrosshairTexture );

	// display current life with digits
	char acLife[16];
	sprintf( acLife, "%03d", (int)fOrigLife );

//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	GraphicsDevice().SetDestBlendMode( AlphaBlend::One );
	SetRenderStatesForTextureFont( AlphaBlend::One );

	float fScreenWidth = (float)GetGameWindowManager().GetScreenWidth();
	float fRatio = fScreenWidth / 800.0f;

	Vector2 vPos = Vector2( 400 - 15, 547.5f ) * fRatio;
	int font_width = (int)(10.0f * fRatio);	// font size 10*20 (when screen width = 800)
	m_StatusFont.SetFontSize( font_width, font_width*2 );
	m_StatusFont.SetFontColor( dwColor );
	m_StatusFont.DrawText( acLife, vPos, dwColor );


	// draw weapon info - left bottom corner
	C2DRect rect;
	rect.SetPosition( Vector2(  80, 530 ) * fRatio,
		              Vector2( 300, 560 ) * fRatio );
	rect.SetColor( 0x50000000 );
	rect.Draw();
	rect.SetColor( dwFontColor );
//	rect.SetDestAlphaBlendMode( D3DBLEND_ONE );
	rect.SetDestAlphaBlendMode( AlphaBlend::One );
	rect.DrawWireframe();

	const int iSlot = SinglePlayerInfo().GetWeaponSystem()->GetPrimaryWeaponSlotIndex();
	char str[2][64];
	memset( str, 0, sizeof(str) );

	CGI_Weapon *pWeapon = SinglePlayerInfo().GetWeaponSystem()->GetWeaponSlot(iSlot).pWeapon;
	if( pWeapon )
        sprintf( str[0], "[%d] %s", iSlot, pWeapon->GetName().c_str() );
//	sprintf( str[1], %03d / %03d", );

//	pd3dDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//	pd3dDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	GraphicsDevice().SetDestBlendMode( AlphaBlend::One );
	SetRenderStatesForTextureFont( AlphaBlend::One );

	vPos = Vector2( 85 * fRatio, 535 * fRatio);
	m_StatusFont.SetFontColor( dwFontColor );
	m_StatusFont.DrawText( str[0], vPos, dwColor );

/**	if( 0.0f < ActionInput.GetAction(ACTION_ATK_FIRE) && ActionInput.GetAction(ACTION_ATK_RAISEWEAPON) == 0 )
		m_pTextWindow->Render( GTW_INPUT_OK );
	else
		m_pTextWindow->Render( GTW_INPUT_NOINPUT );
**/

	font_width = (int)(8.0f * fRatio);
	m_StatusFont.SetFontSize( font_width, font_width*2 );

//	m_QuickMenuManager.Render( &m_StatusFont );
}


bool HUD_PlayerShip::OpenTextWindow( CGameTextSet *pTextSet )
{
	m_pTextWindow->OpenTextWindow( pTextSet );
	return true;
}


CGameTextSet *HUD_PlayerShip::GetCurrentTextSetInTextWindow()
{
	return m_pTextWindow->GetCurrentTextSet();
}


} // namespace amorphous
