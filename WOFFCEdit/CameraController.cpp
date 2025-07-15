#include "ToolMain.h"

#include "CameraController.h"
#include "InputCommands.h"

#include "winuser.h"
#include <stdlib.h>


void CameraController::CalculateCameraView(CPoint DeltaCursorPosition = CPoint(0.f, 0.f)
	, float DeltaTime = 0.f
	, SimpleMath::Vector3 MovementDimensions = SimpleMath::Vector3::Zero)
{
	// Decrement the dimensions of orientation by a sensitivity scalar, "RotationGain."
	// (Camera view control is frame-independent, hence, "DeltaTime").

	Pitch -= DeltaCursorPosition.y * RotationGain * DeltaTime;
	Yaw += DeltaCursorPosition.x * RotationGain * DeltaTime;

	// Constrain the rotation of the camera in the "Pitch" axis
	// (Note that the "- 0.01" offset is a preventative measure against Gimbal Lock).

	float Constraint = XM_PIDIV2 - 0.01f;

	Pitch = static_cast<float>(__min(Constraint, Pitch));
	Pitch = static_cast<float>(__max(-Constraint, Pitch));

	// Wrap the "Yaw" axis (Adhere to a manageable range).

	if (Yaw > XM_PI)
	{
		Yaw -= XM_2PI;
	}
	if (Yaw < -XM_PI)
	{
		Yaw += XM_2PI;
	}

	// Prepare cosine and sine constants, and calculate the "Forward"
	// and "Up" direction vectors using trigonometry.

	// ("Roll" benefits the "Up" vector calculation).

	float Roll = 0.f;

	const float CosPitch = XMScalarCos(Pitch);
	const float CosYaw = XMScalarCos(Yaw);
	const float CosRoll = XMScalarCos(Roll);

	const float SinPitch = XMScalarSin(Pitch);
	const float SinYaw = XMScalarSin(Yaw);
	const float SinRoll = XMScalarSin(Roll);

	SimpleMath::Vector3 Forward(CosPitch * SinYaw
		, SinPitch
		, CosPitch * CosYaw);

	SimpleMath::Vector3 Up((CosYaw * SinRoll) - (CosRoll * SinPitch * SinYaw)
		, CosPitch * CosRoll
		, (SinYaw * SinRoll) - (CosYaw * CosRoll * SinPitch));

	// Normalise the "Forward" and "Up" vectors.

	Forward.Normalize();
	Up.Normalize();

	// Determine the right-facing direction.

	SimpleMath::Vector3 Right = Forward.Cross(Up);

	// Adapt the position according to [-1, 1] values in three dimensions,
	// scaled by "MovementGain," and made frame-independent by "DeltaTime."

	Position += (Forward * __max(-1.f, __min(MovementDimensions.x, 1.f)) * MovementGain * DeltaTime);
	Position += (Right * __max(-1.f, __min(MovementDimensions.y, 1.f)) * MovementGain * DeltaTime);
	Position += (Up * __max(-1.f, __min(MovementDimensions.z, 1.f)) * MovementGain * DeltaTime);

	// Compose the camera view matrix, according to the newly created directional information,
	// using a DirectX library helper.

	CameraView = SimpleMath::Matrix::CreateLookAt(Position, (Forward + Position), Up);
}

CameraController::CameraController(ToolMain* FrameworkHead
	, SimpleMath::Vector3 InitialPosition
	, SimpleMath::Vector3 InitialForward)
	:
	Position(InitialPosition)
	, Forward(InitialForward)
{
	if (FrameworkHead)
	{
		std::vector<Brush*> Brushes = FrameworkHead->GetBrushes();

		for (auto Brush : Brushes)
		{
			// The binding of variably relevant modules dampens the cohesion of this module
			
			// (Writing the delegate owner to be responsible for its subscribers, however,
			// ameliorates the ambiguity of the effect of the delegate).

			OnChangeInCameraMode.bind(&Brush::OnChangeInCameraMode, Brush);
		}
	}

	// The following class helper initialises the class member "CameraView,"
	// so that "UpdateViewByInput" may return a meaningful default value.

	CalculateCameraView();
}


void CameraController::OnScrollInput(InputCommands* InputState)
{
	if (InputState)
	{
		// Retrieve the scroll direction input state,
		// and update the movement gain within valid bounds.

		float Direction = static_cast<float>(InputState->GetScrollAxis());

		if ((MovementGain + Direction * StepMovement) >= 1.f)
		{
			MovementGain += Direction * StepMovement;
		}
	}
}


void CameraController::OnChangeInInput(int PlaceHolder)
{
	ChangeInInput = true;
}


void CameraController::CentreAndHideCursor()
{
	// Cache the current position of the cursor according to the input state.
	
	GetCursorPos(&AbsoluteCursorPosition);

	
	// Retrieve the corners of a window's client area,
	// and determine the centre of the window.

	RECT Rectangle;
	GetClientRect(Window, &Rectangle);

	WindowCentre = CPoint((Rectangle.right - Rectangle.left) / 2, (Rectangle.bottom - Rectangle.top) / 2);

	// Prepare a CPoint member for adaptation to screen-space coordinates.

	WindowCentreInScreenSpace = WindowCentre;

	// Translate the client-space coordinates to screen-space,
	// and set the cursor's position.

	ClientToScreen(Window, &WindowCentreInScreenSpace);
	SetCursorPos(WindowCentreInScreenSpace.x, WindowCentreInScreenSpace.y);


	// A "false" setting decrements "winuser.h"'s internal display counter (Microsoft Learn, 2024);
	// this loop ensures that the conditions are met to hide the cursor.

	while (ShowCursor(false) >= 0);
}

void CameraController::AssignCameraMode(CameraMode EnterMode)
{
	// Newly assign, and broadcast the current camera mode.

	CurrentCameraMode = EnterMode;

	OnChangeInCameraMode(this);
}

SimpleMath::Matrix CameraController::UpdateViewByInput(HWND WindowHandle, float DeltaTime
	, InputCommands* InputState)
{
	if (InputState)
	{
		if (InputState->RightMouseIsDown())
		{
			if (CurrentCameraMode == CameraMode::Free && ChangeInInput)
			{
				// The change in orientation of the view matrix is determined by the change in cursor
				// coordinates relative to an initial position (The centre of the game window).
				
				CPoint DeltaCursorPosition(WindowCentre.x - InputState->GetCursorPosition().x
					, InputState->GetCursorPosition().y - WindowCentre.y);

				// 

				SimpleMath::Vector3 RelativeMovement(InputState->GetForwardAxis()
					, InputState->GetLateralAxis()
					, InputState->GetVerticalAxis());
				
				// A class helper calculates the camera view matrix.

				CalculateCameraView(DeltaCursorPosition, DeltaTime, RelativeMovement);

				// Lastly, reset the cursor position to the centre of the window.
				
				SetCursorPos(WindowCentreInScreenSpace.x, WindowCentreInScreenSpace.y);
			}
			else if (CurrentCameraMode == CameraMode::Still)
			{
				// Update the handle to the game window (Protect against a dated class member reference)
				// and prepare the cursor for "Free" camera control.
			
				Window = WindowHandle;

				CentreAndHideCursor();

				// Flag that the current camera control mode is "Free."

				AssignCameraMode(CameraMode::Free);
			}
		}
		else if (CurrentCameraMode == CameraMode::Free)
		{
			// The right mouse button is not down, though the camera mode is "Free"
			// (Indicating the release of the right mouse button).
			
			// Revert the cursor to its cached position, show the cursor,
			// and return the current camera mode to default.
			
			SetCursorPos(AbsoluteCursorPosition.x, AbsoluteCursorPosition.y);

			// Increment the application's internal cursor display counter by repeated
			// (If necessary) "true" calls (Microsoft Learn, 2024).

			while (ShowCursor(true) < 0);

			AssignCameraMode(CameraMode::Still);
		}

		// Any Mouse or Keyboard input has been resolved.

		ChangeInInput = false;
	}

	return CameraView;
}


/*
	* Microsoft Learn
	* (2024)
	* ShowCursor function (winuser.h) - Win32 apps.
	* Available at: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showcursor
	* (Accessed: 26 February 2024)
*/
