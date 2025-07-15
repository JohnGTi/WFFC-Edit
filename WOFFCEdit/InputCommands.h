#pragma once

#include "directxmath.h"

using namespace DirectX;


class InputUpdateKey
{
private:
	/**
		*	A friend of this class may access its private constructors.
		*
		*	A class may permit a type (That is declared here as a friend)
		*	exclusive access to a method by including this class as a function argument (Monrocq, 2017).
	*/

	friend class ToolMain;

	InputUpdateKey() {}
	InputUpdateKey(const InputUpdateKey&) {}

	InputUpdateKey& operator=(const InputUpdateKey&) = delete;
};


/** Forward declarations. */
class Game;


/**  */
class InputCommands
{
public:
	/**  */
	InputCommands(Game* Game);
	~InputCommands() {}


	/**
		* Getters.
	*/
	XMFLOAT2 GetCursorPosition() const { return CursorPosition; }
	bool LeftMouseIsDown() const { return LeftMouseDown; }
	bool RightMouseIsDown() const { return RightMouseDown; }

	double GetLeftMouseDownStamp() const { return LeftMouseDownStamp; }
	double GetRightMouseDownStamp() const { return RightMouseDownStamp; }

	short GetScrollAxis() const { return Scroll; }

	float GetForwardAxis() const { return ForwardAxis; }
	float GetLateralAxis() const { return LateralAxis; }
	float GetVerticalAxis() const { return VerticalAxis; }


	/**
		* Setters.
		* 
		* These methods can only be accessed by (And so, this class' private members can only be externally set by)
		* friend classes of "InputUpdateKey" (i.e., the framework head, "ToolMain," respondible for monitoring Keyboard and Mouse input).
	*/
	void SetCursorPosition(InputUpdateKey PassKey, XMFLOAT2 UpdatedPosition);

	/**
		* @param	PressWindow		The duration (In seconds) that the mouse input must be pressed
		* 							and released to be considered a "Short" press.
		* 
		* @param	IsShortPress	Was the press completed within PressWindow seconds?
	*/
	void SetLeftMouseDown(InputUpdateKey PassKey, bool IsLeftMouseDown
		, double PressWindow = 0.2, bool *IsShortPress = nullptr);

	void SetRightMouseDown(InputUpdateKey PassKey, bool IsRightMouseDown
		, double PressWindow = 0.2, bool* IsShortPress = nullptr);

	void DefaultScrollAxis(InputUpdateKey PassKey);

	/**
		* @param	ScrollDirection	The signed distance of a scroll,
					in multiples or division of "WHEEL_DELTA" (Microsoft Learn, 2022, Parameters).
	*/
	void SetScrollAxis(InputUpdateKey PassKey, short ScrollDirection);

	void SetForwardAxis(InputUpdateKey PassKey, float UpdatedAxis = 0.f);
	void SetLateralAxis(InputUpdateKey PassKey, float UpdatedAxis = 0.f);
	void SetVerticalAxis(InputUpdateKey PassKey, float UpdatedAxis = 0.f);



	/** Attributes. */

private:
	/**
		* 
	*/
	Game* GameInterface;

	XMFLOAT2 CursorPosition;

	bool LeftMouseDown = false;
	bool RightMouseDown = false;

	double LeftMouseDownStamp = 0.0;
	double RightMouseDownStamp = 0.0;

	/**
		*  1	:- A forward scroll - opposite the user.
		* -1	:- A backward scroll - towards the user.
		*  0	:- No input.
	*/
	short Scroll = 0;

	/** Float values in the range [-1, 1] (This range is enforced internally). */

	float ForwardAxis = 0.f;
	float LateralAxis = 0.f;

	float VerticalAxis = 0.f;
};


/*
	* Microsoft Learn
	* (2022)
	* WM_MOUSEWHEEL message (Winuser.h) - Win32 apps.
	* Available at:
	* (Accessed: 04 May 2024).
*/

/**
	*	Monrocq, M.
	*	(2017)
	*	clean C++ granular friend equivalent? (Answer: Attorney-Client Idiom).
	*	Available at: https://stackoverflow.com/a/3218920
	*	(Accessed: 24 February 2024)
*/

