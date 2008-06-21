
#include "FG_StdControlRenderer.h"
#include "FG_StdControlRendererManager.h"
#include "UI/ui.h"

#include "3DCommon/GraphicsEffectManager.h"

using namespace std;


CFG_StdControlRenderer::CFG_StdControlRenderer()
{
	m_aColor[CGM_Control::STATE_NORMAL]			= SFloatRGBAColor( 0.8f, 0.8f, 0.8f, 1.0f );
    m_aColor[CGM_Control::STATE_DISABLED]		= SFloatRGBAColor( 0.3f, 0.3f, 0.3f, 1.0f );
    m_aColor[CGM_Control::STATE_HIDDEN]			= SFloatRGBAColor( 0.1f, 0.1f, 0.1f, 1.0f );
    m_aColor[CGM_Control::STATE_FOCUS]			= SFloatRGBAColor( 0.2f, 1.0f, 0.2f, 1.0f );
    m_aColor[CGM_Control::STATE_MOUSEOVER]		= SFloatRGBAColor( 0.2f, 0.7f, 0.2f, 1.0f );
    m_aColor[CGM_Control::STATE_PRESSED]		= SFloatRGBAColor( 0.6f, 1.0f, 0.6f, 1.0f );
    m_aColor[CGM_Control::STATE_SUBDIALOGOPEN]	= SFloatRGBAColor( 0.5f, 0.9f, 0.5f, 1.0f );
}

/*
void CFG_StdControlRenderer::OnFocused()
{
}
void CFG_StdControlRenderer::OnFocusCleared()
{
}
void CFG_StdControlRenderer::OnMouseCursorEntered()
{
}
void CFG_StdControlRenderer::OnMouseCursorLeft()
{
}
*/

void CFG_StdControlRenderer::OnPressed()
{
}

void CFG_StdControlRenderer::OnReleased()
{
}


void CFG_StdControlRenderer::OnChecked()
{
}

void CFG_StdControlRenderer::OnCheckCleared()
{
}


void CFG_StdControlRenderer::OnFocused()
{
	ChangeColorToCurrentState();
}


void CFG_StdControlRenderer::OnFocusCleared()
{
	ChangeColorToCurrentState();
}


void CFG_StdControlRenderer::OnMouseCursorEntered()
{
	ChangeColorToCurrentState();
}


void CFG_StdControlRenderer::OnMouseCursorLeft()
{
	ChangeColorToCurrentState();
}


void CFG_StdControlRenderer::OnItemSelectionFocusCreated()
{
}

void CFG_StdControlRenderer::OnItemSelectionChanged()
{
}

void CFG_StdControlRenderer::OnItemSelected()
{
}


void CFG_StdControlRenderer::OnSliderValueChanged()
{
}


void CFG_StdControlRenderer::OnDialogOpened()
{
}

void CFG_StdControlRenderer::OnDialogClosed()
{
}

void CFG_StdControlRenderer::OnOpenDialogAttemptedToClose()
{
}


void CFG_StdControlRenderer::OnParentSubDialogButtonFocused()
{
}

void CFG_StdControlRenderer::OnParentSubDialogButtonFocusCleared()
{
}


void CFG_StdStaticRenderer::Init()
{
	CGM_Static *pStatic = GetStatic();
	if( !pStatic )
		return;

	int font_id = 0;
	int x = pStatic->GetLocalRect().left;
	int y = pStatic->GetLocalRect().top;
	int w = 0;//m_pGraphicsElementManager->GetFont(font_id)->GetFontWidth();// 8;
	int h = 0;//m_pGraphicsElementManager->GetFont(font_id)->GetFontHeight();// 16;
	const SFloatRGBAColor& normal_color = m_aColor[CGM_Control::STATE_NORMAL];

	SRect text_rect = pStatic->GetLocalRect();
	text_rect.Inflate( -12, 0 );

	m_pText = m_pGraphicsElementManager->CreateTextBox( 0, pStatic->GetText(), text_rect,
		CGE_Text::TAL_LEFT, CGE_Text::TAL_CENTER, normal_color, w, h );

//	m_pText->SetMargin( LEFT,  6 );
//	m_pText->SetMargin( RIGHT, 6 );

	// render the text on top
	RegisterGraphicsElement( 0, m_pText );

	RegisterColoredElement( m_pText );
}


void CFG_StdStaticRenderer::OnTextChanged()
{
	m_pText->SetText( GetStatic()->GetText() );

	m_pGraphicsEffectManager->SetTimeOffset();
//	m_pGraphicsEffectManager->ChangeColor( 0.0f, 0.2f, 0, SFloatRGBAColor(), SFloatRGBAColor() );
}


void CFG_StdButtonRenderer::Init()
{
	CFG_StdStaticRenderer::Init();

	CGM_Button *pButton = GetButton();
/*
	const SFloatRGBAColor& normal_color = m_aColor[CGM_Control::STATE_NORMAL];
	m_pRect      = m_pGraphicsElementManager->CreateRect( pButton->GetBoundingBox(),      SFloatRGBAColor(0.0f,0.0f,0.0f,0.5f) );
	m_pFrameRect = m_pGraphicsElementManager->CreateFrameRect( pButton->GetBoundingBox(), normal_color, 2 );

	// register elements
	// - set local layer offset to determine rendering order
	RegisterGraphicsElement( 0, m_pFrameRect );
	RegisterGraphicsElement( 1, m_pRect );

	// register elements that chages colors depending on states
	RegisterColoredElement( m_pFrameRect );
*/
}


void CFG_StdCheckBoxRenderer::Init()
{
	CFG_StdButtonRenderer::Init();

	m_BaseTitle = m_pText->GetText();

	m_pText->SetText( ( GetCheckBox()->IsChecked() ? " [x] " : " [ ] " ) + m_BaseTitle );

	m_pText->SetTextAlignment( CGE_Text::TAL_LEFT, CGE_Text::TAL_CENTER );
}


void CFG_StdCheckBoxRenderer::OnChecked()
{
	m_pText->SetText( " [x] " + m_BaseTitle );
}

void CFG_StdCheckBoxRenderer::OnCheckCleared()
{
	m_pText->SetText( " [ ] " + m_BaseTitle );
}


void CFG_StdRadioButtonRenderer::Init()
{
	CFG_StdButtonRenderer::Init();

	m_BaseTitle = m_pText->GetText();

	m_pText->SetText( ( GetCheckBox()->IsChecked() ? " (x) " : " ( ) " ) + m_BaseTitle );

	m_pText->SetTextAlignment( CGE_Text::TAL_LEFT, CGE_Text::TAL_CENTER );
}


void CFG_StdRadioButtonRenderer::OnChecked()
{
	m_pText->SetText( " (x) " + m_BaseTitle );
}

void CFG_StdRadioButtonRenderer::OnCheckCleared()
{
	m_pText->SetText( " ( ) " + m_BaseTitle );
}


void CFG_StdListBoxRenderer::Init()
{
//	CGM_ControlRenderer::Init();

	CGM_ListBox *pListBox = GetListBox();
	if( !pListBox )
		return;

	const SFloatRGBAColor& normal_color = m_aColor[CGM_Control::STATE_NORMAL];
//	m_pRect      = m_pGraphicsElementManager->CreateRect( pListBox->GetBoundingBox(),      SFloatRGBAColor(0.0f,0.0f,0.0f,0.5f) );
//	m_pFrameRect = m_pGraphicsElementManager->CreateFrameRect( pListBox->GetBoundingBox(), normal_color, 2 );

	// create empty text elements
	int x = GetBaseControl()->GetLocalRect().left;
	int y = GetBaseControl()->GetLocalRect().top;
	int text_x = x;
	int text_y = y;
//	int w = 8;
//	int h = 16;
	int i, num_items_in_page = 10;//pListBox->GetScrollBar();
	m_vecpText.resize( num_items_in_page );
	for( i=0; i<num_items_in_page; i++ )
	{
		text_x = x + 16;
		text_y = y + i * pListBox->GetTextHeight();
		m_vecpText[i] = m_pGraphicsElementManager->CreateText( 0, "-", (float)text_x, (float)text_y, m_aColor[CGM_Control::STATE_NORMAL] );
	}

	// set local layer offset to determine rendering order
	int num_text_elements = (int)m_vecpText.size();
	for( i=0; i<num_text_elements; i++ )
		RegisterGraphicsElement( 0, m_vecpText[i] );

//	RegisterGraphicsElement( 0, m_pFrameRect );
//	RegisterGraphicsElement( 1, m_pRect );

	// register elements that chages colors depending on states
//	RegisterColoredElement( m_pFrameRect );
//	for( i=0; i<num_text_elements; i++ )
//		RegisterColoredElement( m_vecpText[i] );

}


void CFG_StdListBoxRenderer::OnFocused()
{
	UpdateItems( false );
}


/*
void CFG_StdListBoxRenderer::SetColorToTextElement( CGE_Text& rTextElement, int state )
{
}
*/


// update all the text of all the text elements
void CFG_StdListBoxRenderer::UpdateItems( bool update_text )
{
	update_text = true; // For now, just always update the texts//The user may have scrolled the page

	CGM_ListBox *pListBox = GetListBox();
	if( !pListBox )
		return;

	const int focused_item_index = pListBox->GetSelectedIndex();

	int i;
	const int num_items_to_display = pListBox->GetNumItemsToDisplay();
	const int first_item_index = pListBox->GetIndexOfFirstItemToDisplay();

	if( update_text )
	{
		for( i=0; i<num_items_to_display; i++ )
		{
			CGM_ListBoxItem *pItem = pListBox->GetItem( first_item_index + i );
			m_vecpText[i]->SetText( pItem->GetText() );
		}
	}

	int color_index = 0;
	for( i=0; i<num_items_to_display; i++ )
	{
		if( first_item_index + i == focused_item_index )
			m_vecpText[i]->SetColor( color_index, m_aColor[CGM_Control::STATE_FOCUS] );
		else
			m_vecpText[i]->SetColor( color_index, m_aColor[CGM_Control::STATE_NORMAL] );
	}
}


// update all the text of all the text elements
void CFG_StdListBoxRenderer::OnItemSelectionChanged()
{
	bool update_text = true;
	UpdateItems( update_text );
}


void CFG_StdListBoxRenderer::OnItemSelected()
{
	CGM_ListBox *pListBox = GetListBox();
	if( !pListBox )
		return;

	int selected_item_index = pListBox->GetSelectedIndex();
	if( selected_item_index < 0 )
		return;

	// TODO: support scrolling
	int text_index = selected_item_index;

	int color_index = 0;
	m_vecpText[text_index]->SetColor( color_index, m_aColor[CGM_Control::STATE_PRESSED] );
//	m_pGraphicsEffectManager->SetTimeOffset();
//	m_pGraphicsEffectManager->ChangeColorTo( m_vecpText[text_index], 0.0, 0.2, m_aColor[CGM_Control::STATE_FOCUS], 0 );
}


void CFG_StdListBoxRenderer::OnItemTextChanged( CGM_ListBoxItem& item )
{
	UpdateItems();
}


void CFG_StdListBoxRenderer::OnItemDescChanged( CGM_ListBoxItem& item )
{
	UpdateItems();
}


void CFG_StdListBoxRenderer::OnItemUpdated( CGM_ListBoxItem& item )
{
	UpdateItems();
}


void CFG_StdListBoxRenderer::OnItemAdded( int index )
{
	UpdateItems();
}


void CFG_StdListBoxRenderer::OnItemInserted( int index )
{
	UpdateItems();
}


void CFG_StdListBoxRenderer::OnItemRemoved( int index )
{
	UpdateItems();
}


void CFG_StdDialogRenderer::Init()
{
	CGM_Dialog *pDialog = GetDialog();
	if( !pDialog )
		return;

	const SFloatRGBAColor& normal_color = m_aColor[CGM_Control::STATE_NORMAL];
	SRect dlg_rect = pDialog->GetLocalRect();
	SRect frame_rect = dlg_rect;
	SRect bg_rect = dlg_rect;
	bg_rect.Inflate( -16, -16 );
	m_pRect      = m_pGraphicsElementManager->CreateRoundRect(    bg_rect,      SFloatRGBAColor(0.0f,0.0f,0.0f,0.5f), 8 );
	m_pFrameRect = m_pGraphicsElementManager->CreateRoundFrameRect( frame_rect, SFloatRGBAColor(1,1,1,1), 14, 8 );

	m_pFrameRect->SetTexture( CFG_StdControlRendererManager::ID_TEX_ROUNDFRAME );

	// render the frame rect on the background rect
	RegisterGraphicsElement( 2, m_pFrameRect );
	RegisterGraphicsElement( 3, m_pRect );

	// overwrite local top left pos of the background rect
//	m_pRect->SetLocalTopLeftPos( SPoint(8,8) );

	// title
	if( 0 < pDialog->GetTitle().length() )
	{
		SRect title_rect = RectLTRB( dlg_rect.left + 24, dlg_rect.top, dlg_rect.right - 24, dlg_rect.top + 40 );

		SFloatRGBAColor upper_color = SFloatRGBAColor( 0.9f, 0.9f, 0.9f, 0.8f );
		SFloatRGBAColor lower_color = SFloatRGBAColor( 0.1f, 0.1f, 0.1f, 0.8f );
		m_pTitleRect = m_pGraphicsElementManager->CreateRect( title_rect, SFloatRGBAColor(1,1,1,1), 4 );
		m_pTitleRect->SetCornerColor( 0, upper_color );
		m_pTitleRect->SetCornerColor( 1, upper_color );
		m_pTitleRect->SetCornerColor( 2, lower_color );
		m_pTitleRect->SetCornerColor( 3, lower_color );

		m_pTitle = m_pGraphicsElementManager->CreateTextBox(
			CFG_StdControlRendererManager::ID_FONT_MAIN,
			pDialog->GetTitle(),
			title_rect,
			CGE_Text::TAL_CENTER, CGE_Text::TAL_CENTER,
			SFloatRGBAColor( 0.9f, 0.9f, 0.9f, 1.0f ),
			16, 32 );

		RegisterGraphicsElement( 0, m_pTitle );
		RegisterGraphicsElement( 1, m_pTitleRect );

//		m_pTitle->SetLocalTopLeftPos(  );
//		m_pTitleRect->SetLocalTopLeftPos(  );
	}
}


CFG_StdDialogRenderer::CFG_StdDialogRenderer()
:
m_pRect(NULL),
m_pFrameRect(NULL),
m_pTitleRect(NULL),
m_pTitle(NULL),
m_SlideEffectEnabled(false)
{
	m_vSlideIn  = Vector2( 50, 0 );
	m_vSlideOut = Vector2(-50, 0 );
}


void CFG_StdDialogRenderer::OnDialogOpened()
{
	if( !m_pGroupElement )
		return;

	m_pGraphicsEffectManager->SetTimeOffset();

	// slide in
	if( m_SlideEffectEnabled )
	{
		// cancel the previous slide in/out effect
		m_pGraphicsEffectManager->CancelEffect( m_PrevSlideEffect );

		const SRect& rect = GetDialog()->GetBoundingBox();
		Vector2 vDestPos = Vector2( (float)rect.left, (float)rect.top );
		m_pGroupElement->SetTopLeftPos( vDestPos - m_vSlideIn );
//		m_PrevSlideEffect = m_pGraphicsEffectManager->TranslateTo( m_pGroupElement, 0.0f, 0.2f, vDestPos, 0, 0 );
		m_PrevSlideEffect = m_pGraphicsEffectManager->TranslateCDV( m_pGroupElement, 0.0f, vDestPos, m_vSlideIn, 0.15f, 0 );
	}

	// fade in (change alpha form 0 to 1)
	int dlg_color_index = m_DialogFadeColorIndex;
	m_pGroupElement->SetAlpha( dlg_color_index, 0.0f );
	m_pGraphicsEffectManager->ChangeAlphaTo( m_pGroupElement, 0.0f, 0.15f, dlg_color_index, 1.0f, 0 );
}


void CFG_StdDialogRenderer::OnDialogClosed()
{
	if( !m_pGroupElement )
		return;

	// cancel the previous slide in/out effect
	m_pGraphicsEffectManager->CancelEffect( m_PrevSlideEffect );

	m_pGraphicsEffectManager->SetTimeOffset();

	// slide out
	if( m_SlideEffectEnabled )
	{
		// cancel the previous slide in/out effect
		m_pGraphicsEffectManager->CancelEffect( m_PrevSlideEffect );

		const SRect& rect = GetDialog()->GetBoundingBox();
		Vector2 vStartPos = Vector2( (float)rect.left, (float)rect.top );
		m_pGroupElement->SetTopLeftPos( vStartPos );
//		m_PrevSlideEffect = m_pGraphicsEffectManager->TranslateTo( m_pGroupElement, 0.0f, 0.2f, vStartPos + Vector2( -50, 0 ), 0, 0 );
		m_PrevSlideEffect = m_pGraphicsEffectManager->TranslateCDV( m_pGroupElement, 0.0f, vStartPos + m_vSlideOut, m_vSlideOut, 0.15f, 0 );
	}

	// fade out (change alpha form 1 to 0)
	int dlg_color_index = m_DialogFadeColorIndex;
	m_pGraphicsEffectManager->ChangeAlphaTo( m_pGroupElement, 0.0f, 0.15f, dlg_color_index, 0.0f, 0 );
}
