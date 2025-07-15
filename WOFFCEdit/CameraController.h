#pragma once

#include <d3d12.h>
#include <string>

#include "atltypes.h"
#include "SimpleMath.h"

#include "delegate.h"

using namespace DirectX;


/**
	* The state of camera control may be expanded upon to include more modes.
*/
enum class CameraMode : uint8_t
{
	Free,
	Still
};


/** Forward declaration. */
class InputCommands;


/**  */
class CameraController
{
private:
	/**
		* Calculate the camera view matrix according to the change in cursor position
		* and any change in the three dimensions of movement.
	*/
	void CalculateCameraView(CPoint DeltaCursorPosition, float DeltaTime
		, SimpleMath::Vector3 MovementDimensions);


public:
	/**
		* The initial camera position and forward-facing direction may be specified.
	*/
	CameraController(class ToolMain* FrameworkHead
		, SimpleMath::Vector3 InitialPosition = SimpleMath::Vector3(0.f, 0.f, 0.f)
		, SimpleMath::Vector3 InitialForward = SimpleMath::Vector3(0.f, 0.f, 0.f));

	~CameraController() {}

	/**
		* Public queries as to the effect of global structures such as "CameraMode,"
		* avoid common coupling (Wikipedia, 2023, Procedural programming).
	*/
	bool CameraIsFree() const
	{
		return CurrentCameraMode == CameraMode::Free;
	}

	/**  */
	void OnScrollInput(InputCommands* InputState);

	/** (This method encapsulates the flagging of ChangeInInput). */
	void OnChangeInInput(int PlaceHolder);


private:
	/** Free camera control - for example - constrains and hides the mouse cursor. */
	void CentreAndHideCursor();

	/**  */
	void AssignCameraMode(CameraMode EnterMode);


public:
	/**  */
	SimpleMath::Matrix UpdateViewByInput(HWND WindowHandle, float DeltaTime
		, InputCommands* InputState);
	

	
	/** Attributes. */

private:
	/**
		* A fast delegate (Choi, 2007)
		* that is to broadcast a change in the camera mode.
	*/
	fd::delegate1 <void, CameraController*> OnChangeInCameraMode;

	/**
		* Whether or not there has been a relevant input command to necessitate an update.
	*/
	bool ChangeInInput = false;

	/**  */
	CameraMode CurrentCameraMode = CameraMode::Still;

	/**  */
	HWND Window;

	/**
		* The cursor position - in screen-space - is cached upon entering the "Free"
		* camera mode (For example), to be returned to on-egress.
	*/
	CPoint AbsoluteCursorPosition;

	/**  */
	CPoint WindowCentre;
	CPoint WindowCentreInScreenSpace;

	/** A designer value that may scale the rotation of the camera. */
	const float RotationGain = 0.4f;

	/**
		* Analogue (Mouse) input control operates in two dimensions of movement
	*/
	float Pitch = 0.f;
	float Yaw = 0.f;

	/** A designer value scales movement, local to the camera. */
	float MovementGain = 20.f;
	const float StepMovement = 10.f;

	/**
		* The position and forward-direction of the camera.
	*/
	SimpleMath::Vector3 Position = SimpleMath::Vector3::Zero;
	SimpleMath::Vector3 Forward = SimpleMath::Vector3::Zero;

	/**  */
	SimpleMath::Matrix CameraView = SimpleMath::Matrix::Identity;
};


/**
	* Wikipedia
	* (2023)
	* Coupling (computer programming).
	* Available at: https://en.wikipedia.org/wiki/Coupling_(computer_programming)
	* (Accessed: 02 May 2024).
*/

