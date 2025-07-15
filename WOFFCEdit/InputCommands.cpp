#include "InputCommands.h"

#include "pch.h"
#include "Game.h"

#include <stdlib.h>


InputCommands::InputCommands(Game* Game)
	:
	GameInterface(Game)
	, CursorPosition(XMFLOAT2(0.f, 0.f))
{
}


void InputCommands::SetCursorPosition(InputUpdateKey PassKey, XMFLOAT2 UpdatedPosition)
{
	CursorPosition = UpdatedPosition;
}

void InputCommands::SetLeftMouseDown(InputUpdateKey PassKey, bool IsLeftMouseDown
	, double PressWindow, bool* IsShortPress)
{
	LeftMouseDown = IsLeftMouseDown;

	if (GameInterface)
	{
		// Cache the current game-time.

		double GameTime = GameInterface->GetGameTime();

		if (LeftMouseDown)
		{
			// Record the game-time at which the left Mouse button is pressed.

			LeftMouseDownStamp = GameTime;
		}
		else if (IsShortPress != nullptr)
		{
			// The left mouse button has been released.
			
			// Check that the press has been released within "PressWindow" seconds.

			if (GameTime - LeftMouseDownStamp < PressWindow)
			{
				*IsShortPress = true;
			}
			else
			{
				*IsShortPress = false;
			}
		}
	}
}

void InputCommands::SetRightMouseDown(InputUpdateKey PassKey, bool IsRightMouseDown
	, double PressWindow, bool* IsShortPress)
{
	RightMouseDown = IsRightMouseDown;

	if (GameInterface)
	{
		// Cache the current game-time.

		double GameTime = GameInterface->GetGameTime();

		if (RightMouseDown)
		{
			// Record the game-time at which the right Mouse button is pressed.

			RightMouseDownStamp = GameTime;
		}
		else if (IsShortPress != nullptr)
		{
			// The right mouse button has been released.

			// Check that the press has been released within "PressWindow" seconds.

			if (GameTime - RightMouseDownStamp < PressWindow)
			{
				*IsShortPress = true;
			}
			else
			{
				*IsShortPress = false;
			}
		}
	}
}


void InputCommands::DefaultScrollAxis(InputUpdateKey PassKey)
{
	// Reset the scroll variable to indicate no activity.

	Scroll = 0;
}

void InputCommands::SetScrollAxis(InputUpdateKey PassKey, short ScrollDirection)
{
	// 

	Scroll = ScrollDirection / WHEEL_DELTA;
}


void InputCommands::SetForwardAxis(InputUpdateKey PassKey, float UpdatedAxis)
{
	// Update the forward axis class member,
	// ensuring firstly that the updated value is within the range [-1, 1].

	UpdatedAxis = static_cast<float>(__max(-1.f, __min(UpdatedAxis, 1.f)));

	ForwardAxis = UpdatedAxis;
}

void InputCommands::SetLateralAxis(InputUpdateKey PassKey, float UpdatedAxis)
{
	// Update the lateral axis class member,
	// ensuring firstly that the updated value is within the range [-1, 1].

	UpdatedAxis = static_cast<float>(__max(-1.f, __min(UpdatedAxis, 1.f)));

	LateralAxis = UpdatedAxis;
}

void InputCommands::SetVerticalAxis(InputUpdateKey PassKey, float UpdatedAxis)
{
	// Update the vertical axis class member,
	// ensuring firstly that the updated value is within the range [-1, 1].

	UpdatedAxis = static_cast<float>(__max(-1.f, __min(UpdatedAxis, 1.f)));

	VerticalAxis = UpdatedAxis;
}