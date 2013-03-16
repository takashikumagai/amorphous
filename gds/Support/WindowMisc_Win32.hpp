#ifndef  __WINDOWMISC_H__
#define  __WINDOWMISC_H__


#include <windows.h>


namespace amorphous
{


/// Returns the horizontal and vertical dimensions which do not belong to the client area in the window.
inline void GetNonClientAreaSize( HWND hWnd, long& frame_width, long& frame_height )
{
	RECT rect;
	int ww, wh, cw, ch;

	// calc boundary width outside the client rect
	GetClientRect(hWnd, &rect);		// Get the size of the client area
	cw = rect.right - rect.left;	// The client area width
	ch = rect.bottom - rect.top;	// The client area width

	// calc the entire window size
	GetWindowRect(hWnd, &rect);		// Get the size of the entire window
	ww = rect.right - rect.left;	// The window width
	wh = rect.bottom - rect.top;	// The window height
	frame_width  = ww - cw;			// The horizontal dimension which does not belong to the client area in the window
	frame_height = wh - ch;			// The vertical dimension which does not belong to the client area in the window
}


inline long GetNonClientAreaWidth( HWND hWnd )
{
	RECT rect, client_rect;
	GetClientRect(hWnd, &client_rect);
	GetWindowRect(hWnd, &rect);

	return (rect.right - rect.left) - (client_rect.left - client_rect.right);
}


inline long GetNonClientAreaHeight( HWND hWnd )
{
	RECT rect, client_rect;
	GetClientRect(hWnd, &client_rect);
	GetWindowRect(hWnd, &rect);

	return (rect.bottom - rect.top) - (client_rect.bottom - client_rect.top);
}


/**
 * change the size of the client area.
 * call this function after CreateWindow() to adjust the resolution of the client area
 * in windowed mode.
 */
inline void ChangeClientAreaSize( HWND hWnd, int new_width, int new_height )
{
	long frame_width = 0, frame_height = 0;

	GetNonClientAreaSize( hWnd, frame_width, frame_height );

	// change window size
	BOOL b = SetWindowPos( hWnd, HWND_TOP, 0, 0, new_width + frame_width, new_height + frame_height, SWP_NOMOVE );
}

} // amorphous



#endif  /*  __WINDOWMISC_H__  */
