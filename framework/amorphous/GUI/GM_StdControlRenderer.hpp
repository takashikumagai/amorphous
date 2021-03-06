#ifndef  __GM_STDCONTROLRENDERER_H__
#define  __GM_STDCONTROLRENDERER_H__


#include "fwd.hpp"
#include "GM_ControlRenderer.hpp"
#include "GM_Control.hpp"

#include "amorphous/Graphics/FloatRGBAColor.hpp"
#include "amorphous/Graphics/GraphicsEffectHandle.hpp"

#include <vector>


namespace amorphous
{


class CGM_StdControlRenderer : public CGM_ControlRenderer
{
private:

	enum Params
	{
		STD_FADE_COLOR_INDEX = 2
	};

	class ColoredElementHolder
	{
	public:

		std::shared_ptr<GraphicsElement> m_pElement;
		GraphicsElementAnimationHandle m_PrevEffect;

		ColoredElementHolder() {}
		ColoredElementHolder(std::shared_ptr<GraphicsElement> pElement) : m_pElement(pElement) {}
	};

	std::vector<ColoredElementHolder> m_vecColoredElement;

protected:

	SFloatRGBAColor m_aColor[ CGM_Control::NUM_CONTROL_STATES ];

	/// used by renderer of dialog
	int m_DialogFadeColorIndex;

protected:

	/// updates the color of graphics elements registered as colored elements
	/// called when a state of the owner control changes
	/// - focus in/out, mouse cursor entered/left
	void ChangeColorToCurrentState();

	void RegisterColoredElement(std::shared_ptr<GraphicsElement> pElement) { m_vecColoredElement.push_back( ColoredElementHolder(pElement) ); }

	virtual void OnGroupElementCreated();

public:

	CGM_StdControlRenderer();

	virtual ~CGM_StdControlRenderer() {}

	virtual void OnFocused( CGM_Control& control );
	virtual void OnFocusCleared( CGM_Control& control );

	/// for button renderers
	virtual void OnPressed( CGM_Button& button );
	virtual void OnReleased( CGM_Button& button );

	/// for check box and radio button renderers
	virtual void OnChecked( CGM_CheckBox& checkbox );
	virtual void OnCheckCleared( CGM_CheckBox& checkbox );

	virtual void OnMouseCursorEntered( CGM_Control& control );
	virtual void OnMouseCursorLeft( CGM_Control& control );

	virtual void OnEnabled( CGM_Control& control );
	virtual void OnDisabled( CGM_Control& control );

	virtual void OnVisibleSetToTrue( CGM_Control& control );
	virtual void OnVisibleSetToFalse( CGM_Control& control );

	/// for list box renderer
	virtual void OnItemSelectionFocusCreated( CGM_ListBox& listbox );
	virtual void OnItemSelectionChanged( CGM_ListBox& listbox );
	virtual void OnItemSelected( CGM_ListBox& listbox );

	/// for slider renderer
	virtual void OnSliderValueChanged( CGM_Slider& slider );

	// for dialog
	virtual void OnDialogOpened( CGM_Dialog& dialog );
	virtual void OnDialogClosed( CGM_Dialog& dialog );
	virtual void OnOpenDialogAttemptedToClose( CGM_Dialog& dialog );

	virtual void OnParentSubDialogButtonFocused();
	virtual void OnParentSubDialogButtonFocusCleared();
};


class CGM_StdStaticRenderer : public CGM_StdControlRenderer
{
protected:

	std::shared_ptr<TextElement> m_pText;

public:

	CGM_StdStaticRenderer() {}

	virtual ~CGM_StdStaticRenderer() {}

	virtual void Init( CGM_Static& static_control );

	virtual void OnTextChanged( CGM_Static& static_control );
};


class CGM_StdButtonRenderer : public CGM_StdStaticRenderer
{
protected:

//	CGE_Rect *m_pRect;
//	CGE_Rect *m_pFrameRect;
	std::shared_ptr<CombinedRectElement> m_pRect;

public:

	CGM_StdButtonRenderer() {}

	virtual ~CGM_StdButtonRenderer() {}

	virtual void Init( CGM_Button& button );

//	virtual void OnMouseCursorEntered( CGM_Control& control );

//	virtual void OnMouseCursorLeft( CGM_Control& control );

	virtual void OnPressed( CGM_Button& button ) {}

	virtual void OnReleased( CGM_Button& button ) {}

//	virtual void OnFocused( CGM_Control& control );

//	virtual void OnFocusCleared( CGM_Control& control );
};


class CGM_StdCheckBoxRenderer : public CGM_StdButtonRenderer
{
protected:

	std::string m_BaseTitle;

public:

	CGM_StdCheckBoxRenderer() {}

	virtual ~CGM_StdCheckBoxRenderer() {}

	virtual void Init( CGM_CheckBox& checkbox );

//	virtual void OnPressed( CGM_Button& button ) { OnPressedStd(); }
//	virtual void OnReleased( CGM_Button& button ) { OnReleasedStd(); }
//	virtual void OnMouseCursorEntered( CGM_Control& control ) {}
//	virtual void OnMouseCursorLeft( CGM_Control& control ) {}

	// what about OnPressed( CGM_Button& button )?
	// - call OnPressed( CGM_Button& button ) & OnChecked( CGM_CheckBox *pCheckBox )
	virtual void OnChecked( CGM_CheckBox& checkbox );

	virtual void OnCheckCleared( CGM_CheckBox& checkbox );
};


class CGM_StdRadioButtonRenderer : public CGM_StdCheckBoxRenderer
{
public:

	CGM_StdRadioButtonRenderer() {}

	virtual ~CGM_StdRadioButtonRenderer() {}

	virtual void Init( CGM_RadioButton& radiobutton );

	virtual void OnChecked( CGM_RadioButton& radiobutton );

	virtual void OnCheckCleared( CGM_RadioButton& radiobutton );
};


/*
class CGM_StdSubDialogButtonRenderer
{
public:

	CGM_SubDialogButtonRenderer() {}
	virtual ~CGM_SubDialogButtonRenderer() {}
	virtual void Init() {}
};


class CGM_StdDialogCloseButtonRenderer
{
public:

	CGM_DialogCloseButtonRenderer() {}
	virtual ~CGM_DialogCloseButtonRenderer() {}
	virtual void Init() {}
};
*/


class CGM_StdListBoxRenderer : public CGM_StdControlRenderer
{
protected:

//	CGE_Rect *m_pRect;
//	CGE_Rect *m_pFrameRect;
	std::shared_ptr<CombinedRectElement> m_pRect;

	std::vector<std::shared_ptr<TextElement> > m_vecpText;

protected:

	void UpdateItems( CGM_ListBox& listbox, bool update_text = true );

public:

	CGM_StdListBoxRenderer() {}

	virtual ~CGM_StdListBoxRenderer() {}

	virtual void Init( CGM_ListBox& listbox );

	virtual void OnItemSelectionChanged( CGM_ListBox& listbox );

	virtual void OnItemSelected( CGM_ListBox& listbox );

	virtual void OnItemAdded( CGM_ListBox& listbox, int index );
	virtual void OnItemInserted( CGM_ListBox& listbox, int index );
	virtual void OnItemRemoved( CGM_ListBox& listbox, CGM_ListBoxItem& item );
	virtual void OnAllItemsRemoved( CGM_ListBox& listbox );
};


class CGM_StdScrollBarRenderer : public CGM_StdControlRenderer
{
protected:

	enum RectElement
	{
		RE_UP_BUTTON,
		RE_DOWN_BUTTON,
		RE_TRACK,
		RE_THUMB,
		NUM_RECT_ELEMENTS
	};

	enum TriangleElement
	{
		TE_ARROW_UP,
		TE_ARROW_DOWN,
		NUM_TRIANGLE_ELEMENTS
	};

//	CGE_Rect *m_apRect[NUM_RECT_ELEMENTS];
//	CGE_Rect *m_apFrameRect[NUM_RECT_ELEMENTS];
	std::shared_ptr<CombinedRectElement> m_apRect[NUM_RECT_ELEMENTS];

//	CGE_Triangle *m_apTriangle[NUM_TRIANGLE_ELEMENTS];
	std::shared_ptr<FillTriangleElement> m_apTriangle[NUM_TRIANGLE_ELEMENTS];

	std::shared_ptr<GraphicsElementGroup> m_pThumbGroup;

protected:

	void UpdateItems( bool update_text = true );

public:

	CGM_StdScrollBarRenderer();

	virtual ~CGM_StdScrollBarRenderer() {}

	virtual void Init( CGM_ScrollBar& scrollbar );

//	virtual void OnItemSelectionChanged( CGM_ListBox& listbox );

//	virtual void OnItemSelected( CGM_ListBox& listbox );

//	virtual void OnItemAdded( CGM_ListBox& listbox, int index );
//	virtual void OnItemInserted( CGM_ListBox& listbox, int index );
//	virtual void OnItemRemoved( CGM_ListBox& listbox, int index );

//	virtual void OnScrolled( CGM_ScrollBar *pScrollbar, int delta );
//	virtual void OnTrackRangeChanged( CGM_ScrollBar *pScrollbar, int start, int end );
	virtual void OnThumbUpdated( CGM_ScrollBar& scrollbar );
};


class CGM_StdSliderRenderer : public CGM_StdControlRenderer
{
//	CGE_Rect *m_pRect;
//	CGE_Rect *m_pFrameRect;
	std::shared_ptr<CombinedRectElement> m_pRect;

	// button
//	CGE_Rect *m_pSliderButtonRect;
//	CGE_Rect *m_pSliderButtonFrameRect;
	std::shared_ptr<CombinedRectElement> m_pSliderButtonRect;
//	CGE_Rect *m_pSliderButtonDot;
	std::shared_ptr<FillRectElement> m_pSliderButtonDot;

	std::shared_ptr<GraphicsElementGroup> m_pSliderButton;

public:

	CGM_StdSliderRenderer()
//		:
//	m_pRect(NULL),
//	m_pFrameRect(NULL),
//	m_pSliderButtonRect(NULL),
//	m_pSliderButtonFrameRect(NULL),
//	m_pSliderButtonDot(NULL),
//	m_pSliderButton(NULL)
	{}

	virtual ~CGM_StdSliderRenderer() {}

	virtual void Init( CGM_Slider& slider );

	virtual void OnSliderValueChanged( CGM_Slider& slider );
};


class CGM_StdDialogRenderer : public CGM_StdControlRenderer
{
//	CGE_Rect *m_pRect;
//	CGE_Rect *m_pFrameRect;
	std::shared_ptr<CombinedRectElement> m_pRect;

	GraphicsElementAnimationHandle m_PrevSlideEffect;

	bool m_bUseSlideEffects;

public:

	CGM_StdDialogRenderer();

	virtual ~CGM_StdDialogRenderer() {}

	virtual void Init( CGM_Dialog& dialog );

	virtual void OnDialogOpened( CGM_Dialog& dialog );
	virtual void OnDialogClosed( CGM_Dialog& dialog );
	virtual void OnOpenDialogAttemptedToClose( CGM_Dialog *pDialog ) {}

	int GetFadeColorIndex() { return m_DialogFadeColorIndex; }
};


class CGM_StdCaptionRenderer : public CGM_StdControlRenderer
{
protected:

	GraphicsElementAnimationHandle m_PrevTextDrawEffect;

	std::shared_ptr<TextElement> m_pCaptionText;

protected:

	void UpdateCaptionText( CGM_Control& control );

public:

	CGM_StdCaptionRenderer() {}

	virtual ~CGM_StdCaptionRenderer() {}

	virtual void InitCaptionRenderer();

	virtual void OnControlFocused( CGM_Control& control );
//	virtual void OnControlFocusCleared( CGM_Control *pControl );

//	virtual void OnFocusedControlChanged( CGM_Control *pNewFocusedControl, CGM_Control *pPrevFocusedControl );

	virtual void OnMouseOverControlChanged( CGM_Control& control_under_mouse );
};

} // namespace amorphous



#endif		/*  __GM_STDCONTROLRENDERER_H__  */
