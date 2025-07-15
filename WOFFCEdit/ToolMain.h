#pragma once

#include <afxext.h>
#include "pch.h"
#include "Game.h"
#include "sqlite3.h"
#include "SceneObject.h"

#include "InputCommands.h"

#include "ContinuationBrush.h"
#include "ExtrusionBrush.h"

#include "delegate.h"

#include <vector>
#include <unordered_set>
#include <ppltasks.h>


/**
	* Signify the current mode of interaction.
*/
enum class ToolMode : uint8_t
{
	Picking,
	Painting,
	Inactive
};


/** Forward declaration. */
class Brush;


/**  */
class ToolMain
{
	/** Constructor/Destructor. */
public:
	ToolMain();
	~ToolMain();

	/**  */
	std::vector<Brush*> GetBrushes();

	/** @return	The index of the currently selected object. */
	int	 getCurrentSelectionID();


	/**
		* The "onAction" prefix suggests an interface that the MFC application may purpose.
	*/

	/** Instantiate the DirectX TK renderer and SQLite. */
	void onActionInitialise(HWND handle, int width, int height);

	/**  */
	void onActionFocusCamera() {}

	/** Load the current chunk from the SQLite database. */
	void onActionLoad();

	/** Write the contents of the current chunk to the SQLite database. */
	afx_msg	void onActionSave();

	/** Write the terrain geometry to the SQLite database. */
	afx_msg void onActionSaveTerrain();

	/**
		* 
	*/
	Brush* GetBrushIfActive();

	Brush* GetBrush()
	{
		return ActiveBrush;
	}

	/**  */
	void Tick(MSG *msg);

	/**  */
	void SelectObjectByPick(bool SelectMultiple);

	/**
		Swap between the available Brushes; "Flip,"
		as this functionality is temporary:
		major tool selection is eventually to be done by toolbar button.
	*/
	void FlipBrush();

	/**
		* Recieve the content of a Windows message,
		* and accordingly update the state of current input commands, "m_toolInputCommands".
	*/
	void UpdateInput(MSG *msg);

	/**  */
	void ReportSQLiteError(sqlite3* DatabaseConnection);

	/** @return	The state of user input (Which this class has the privilege to set). */
	InputCommands* GetInputCommands()
	{
		return &m_toolInputCommands;
	}

	/** @return	The duration of a "Short" press. */
	double GetShortPressWindow()
	{
		return ShortPressWindow;
	}


private:
	/**  */
	void onContentAdded() {}



	/** Attributes. */

public:
	/** The "m_sceneGraph" is to store all objects in the current chunk. */
	std::vector<SceneObject> m_sceneGraph;

	/** The (Solitary - in this implementation) landscape chunk. */
	ChunkObject m_chunk;

	/** The object index representative of the current selection. */
	int m_selectedObject = -1;
	std::unordered_set<DisplayObject*> SelectedObjects;

	
private:
	/**
		* "Game" interfaces with the DirectX TK device
		* (In an abstract way (Microsoft, 2022)).
	*/
	Game m_d3dRenderer;

	/** The current state of user Keyboard and Mouse input. */
	InputCommands m_toolInputCommands;

	/**  */
	std::shared_ptr<CameraController> Camera = nullptr;

	/**
		* The active mode of interaction, and the default tool that is to be returned to upon activation.
	*/
	ToolMode ActiveToolMode = ToolMode::Picking;
	ToolMode InactiveTool = ToolMode::Picking;

	/**
		* 
	*/
	ContinuationBrush ContinuationTypeBrush;
	ExtrusionBrush ExtrusionTypeBrush;

	/**  */
	Brush* ActiveBrush = nullptr;

	/**
		* A "Parallel Patterns Library" encapsulation of a cancellation token,
		* permitting the stopping of a concurrent task.
	*/
	std::shared_ptr<concurrency::cancellation_token_source> DeactivationTokenSource = nullptr;

	/**
		* So that the running, concurrent deactivation (Of the active tool) task may be referenced.
	*/
	concurrency::task<void> DeactivationTask;

	/**
		* The duration (In seconds) within which a "Short" Mouse input
		* must be pressed and released.
	*/
	double ShortPressWindow = 0.125;

	/** The window area rectangle. */
	CRect WindowRECT;

	/**  */
	char m_keyArray[256];

	/** SQLite database handle. */
	sqlite3* m_databaseConnection = NULL;

	/**  */
	HWND WindowHandle = nullptr;

	/**
		* The width and height of the game window will be
		* meaningfully initialised by the MFC application.
	*/
	int m_width = 0;
	int m_height = 0;

	/**
		* "The current chunk of the database that we are operating on.
		* Dictates loading and saving."
	*/
	int m_currentChunk = 0;

	/**  */
	bool PickMultiple = false;

	/**  */
	bool ScrollSecondary = false;
	
	/**
		* A fast delegate (Choi, 2007)
		* that is to broadcast a change in the input state.
	*/
	fd::delegate1 <void, int> OnChangeInInput;
};


/**
	* Choi, J.
	* (2007)
	* Fast C++ Delegate.
	* Available at: https://www.codeproject.com/Articles/13287/Fast-C-Delegate?msg=1762300#xx1762300xx
	* (Accessed: 25 February 2024)
*/

/**
	* Microsoft
	* (2022)
	* Using DeviceResources.
	* Available at: https://github.com/microsoft/DirectXTK/wiki/Using-DeviceResources
	* (Accessed: 04 March 2024)
*/

