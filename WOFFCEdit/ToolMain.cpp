#include "ToolMain.h"
#include "resource.h"

#include "CameraController.h"
#include "Brush.h"

#include <atlstr.h>
#include <vector>
#include <sstream>

#include "ConcurrencyHelper.h"
//#include <ppl.h>
//#include <agents.h>

using namespace concurrency;


// Hexidecimal value for the "SHIFT" virtual-key.

#define SHIFT_DOWN 0x10


ToolMain::ToolMain()
	:
	m_d3dRenderer(this)
	, m_toolInputCommands(&m_d3dRenderer)
	, m_keyArray{}
{
	// Access the virtual camera by way of the renderer:
	// the camera is bound to a fast delegate.
	
	// (This delegate broadcasts a change in the m_toolInputCommands structure,
	// and avoids "Content Coupling" with the camera class).

	if (Camera = m_d3dRenderer.GetCamera())
	{
		OnChangeInInput.bind(&CameraController::OnChangeInInput, Camera.get());
	}

	// Default the active Brush to reference the extrusion Brush tool.

	ActiveBrush = &ExtrusionTypeBrush;


	// Clear the container of all objects in the current chunk.

	m_sceneGraph.clear();
}

ToolMain::~ToolMain()
{
	// Close the database connection.

	sqlite3_close(m_databaseConnection);
}


std::vector<Brush*> ToolMain::GetBrushes()
{
	// Return a vector of pointers to each Brush.

	return std::vector<Brush*>{ &ContinuationTypeBrush, &ExtrusionTypeBrush };
}


int ToolMain::getCurrentSelectionID()
{
	return m_selectedObject;
}


void ToolMain::ReportSQLiteError(sqlite3* DatabaseConnection)
{
	// TODO: A more complex (Still "Modal") dialogue may better inform the end user
	
	//		 (A message may, for example, require the user to exit or continue the application;
	//		 another might force the application closed upon user confirmation).


	// Retrieve textual description of an SQLite error
	// and purpose a CString utility (The structure of which is accepted by "AfxMessageBox").

	const char* ErrorMessage = sqlite3_errmsg(DatabaseConnection);
	CString UnicodeErrorMessage(ErrorMessage);

	// AfxMessageBox automates a dialogue child of the main window.

	AfxMessageBox(UnicodeErrorMessage, MB_ICONERROR);
}

void ToolMain::onActionInitialise(HWND handle, int width, int height)
{
	// Store the width and height of game window in class member variables.

	WindowHandle = handle;

	m_width		= width;
	m_height	= height;

	// Initialise the renderer device resources and peripheral interfaces
	// (The Keyboard and Mouse, for example).
	
	m_d3dRenderer.Initialize(WindowHandle, m_width, m_height);


	// Establish a connection to the SQLite database,
	// and validate a successful connection.

	int ResultCode = sqlite3_open_v2("database/test.db"
		, &m_databaseConnection, SQLITE_OPEN_READWRITE, NULL);

	if (ResultCode != SQLITE_OK)
	{
		// The operation was unsuccessful; the return code is indicative of some error.

		if (ResultCode == SQLITE_READONLY)
		{
			// TODO: Here, there is room to validate whether read-only access is suitable,
			//		 should that it satisfy the requirements of the module.
		}
		else if (ResultCode == SQLITE_NOTFOUND || ResultCode == SQLITE_CANTOPEN)
		{
			ReportSQLiteError(m_databaseConnection);
		}
	}
	else 
	{
		TRACE("Database opened successfully.");
	}

	onActionLoad();
}


void ToolMain::onActionLoad()
{
	//load current chunk and objects into lists
	if (!m_sceneGraph.empty())		//is the vector empty
	{
		m_sceneGraph.clear();		//if not, empty it
	}

	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	sqlite3_stmt *pResultsChunk;

	//OBJECTS IN THE WORLD
	//prepare SQL Text
	sqlCommand = "SELECT * from Objects";				//sql command which will return all records from the objects table.
	//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0 );
	
	//loop for each row in results until there are no more rows.  ie for every row in the results. We create and object
	while (sqlite3_step(pResults) == SQLITE_ROW)
	{	
		SceneObject newSceneObject;
		newSceneObject.ID = sqlite3_column_int(pResults, 0);
		newSceneObject.chunk_ID = sqlite3_column_int(pResults, 1);
		newSceneObject.model_path		= reinterpret_cast<const char*>(sqlite3_column_text(pResults, 2));
		newSceneObject.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 3));
		newSceneObject.posX = sqlite3_column_double(pResults, 4);
		newSceneObject.posY = sqlite3_column_double(pResults, 5);
		newSceneObject.posZ = sqlite3_column_double(pResults, 6);
		newSceneObject.rotX = sqlite3_column_double(pResults, 7);
		newSceneObject.rotY = sqlite3_column_double(pResults, 8);
		newSceneObject.rotZ = sqlite3_column_double(pResults, 9);
		newSceneObject.scaX = sqlite3_column_double(pResults, 10);
		newSceneObject.scaY = sqlite3_column_double(pResults, 11);
		newSceneObject.scaZ = sqlite3_column_double(pResults, 12);
		newSceneObject.render = sqlite3_column_int(pResults, 13);
		newSceneObject.collision = sqlite3_column_int(pResults, 14);
		newSceneObject.collision_mesh = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 15));
		newSceneObject.collectable = sqlite3_column_int(pResults, 16);
		newSceneObject.destructable = sqlite3_column_int(pResults, 17);
		newSceneObject.health_amount = sqlite3_column_int(pResults, 18);
		newSceneObject.editor_render = sqlite3_column_int(pResults, 19);
		newSceneObject.editor_texture_vis = sqlite3_column_int(pResults, 20);
		newSceneObject.editor_normals_vis = sqlite3_column_int(pResults, 21);
		newSceneObject.editor_collision_vis = sqlite3_column_int(pResults, 22);
		newSceneObject.editor_pivot_vis = sqlite3_column_int(pResults, 23);
		newSceneObject.pivotX = sqlite3_column_double(pResults, 24);
		newSceneObject.pivotY = sqlite3_column_double(pResults, 25);
		newSceneObject.pivotZ = sqlite3_column_double(pResults, 26);
		newSceneObject.snapToGround = sqlite3_column_int(pResults, 27);
		newSceneObject.AINode = sqlite3_column_int(pResults, 28);
		newSceneObject.audio_path = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 29));
		newSceneObject.volume = sqlite3_column_double(pResults, 30);
		newSceneObject.pitch = sqlite3_column_double(pResults, 31);
		newSceneObject.pan = sqlite3_column_int(pResults, 32);
		newSceneObject.one_shot = sqlite3_column_int(pResults, 33);
		newSceneObject.play_on_init = sqlite3_column_int(pResults, 34);
		newSceneObject.play_in_editor = sqlite3_column_int(pResults, 35);
		newSceneObject.min_dist = sqlite3_column_double(pResults, 36);
		newSceneObject.max_dist = sqlite3_column_double(pResults, 37);
		newSceneObject.camera = sqlite3_column_int(pResults, 38);
		newSceneObject.path_node = sqlite3_column_int(pResults, 39);
		newSceneObject.path_node_start = sqlite3_column_int(pResults, 40);
		newSceneObject.path_node_end = sqlite3_column_int(pResults, 41);
		newSceneObject.parent_id = sqlite3_column_int(pResults, 42);
		newSceneObject.editor_wireframe = sqlite3_column_int(pResults, 43);
		newSceneObject.name = reinterpret_cast<const char*>(sqlite3_column_text(pResults, 44));

		newSceneObject.light_type = sqlite3_column_int(pResults, 45);
		newSceneObject.light_diffuse_r = sqlite3_column_double(pResults, 46);
		newSceneObject.light_diffuse_g = sqlite3_column_double(pResults, 47);
		newSceneObject.light_diffuse_b = sqlite3_column_double(pResults, 48);
		newSceneObject.light_specular_r = sqlite3_column_double(pResults, 49);
		newSceneObject.light_specular_g = sqlite3_column_double(pResults, 50);
		newSceneObject.light_specular_b = sqlite3_column_double(pResults, 51);
		newSceneObject.light_spot_cutoff = sqlite3_column_double(pResults, 52);
		newSceneObject.light_constant = sqlite3_column_double(pResults, 53);
		newSceneObject.light_linear = sqlite3_column_double(pResults, 54);
		newSceneObject.light_quadratic = sqlite3_column_double(pResults, 55);
	

		//send completed object to scenegraph
		m_sceneGraph.push_back(newSceneObject);
	}

	//THE WORLD CHUNK
	//prepare SQL Text
	sqlCommand = "SELECT * from Chunks";				//sql command which will return all records from  chunks table. There is only one tho.
														//Send Command and fill result object
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResultsChunk, 0);


	sqlite3_step(pResultsChunk);
	m_chunk.ID = sqlite3_column_int(pResultsChunk, 0);
	m_chunk.name = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 1));
	m_chunk.chunk_x_size_metres = sqlite3_column_int(pResultsChunk, 2);
	m_chunk.chunk_y_size_metres = sqlite3_column_int(pResultsChunk, 3);
	m_chunk.chunk_base_resolution = sqlite3_column_int(pResultsChunk, 4);
	m_chunk.heightmap_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 5));
	m_chunk.tex_diffuse_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 6));
	m_chunk.tex_splat_alpha_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 7));
	m_chunk.tex_splat_1_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 8));
	m_chunk.tex_splat_2_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 9));
	m_chunk.tex_splat_3_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 10));
	m_chunk.tex_splat_4_path = reinterpret_cast<const char*>(sqlite3_column_text(pResultsChunk, 11));
	m_chunk.render_wireframe = sqlite3_column_int(pResultsChunk, 12);
	m_chunk.render_normals = sqlite3_column_int(pResultsChunk, 13);
	m_chunk.tex_diffuse_tiling = sqlite3_column_int(pResultsChunk, 14);
	m_chunk.tex_splat_1_tiling = sqlite3_column_int(pResultsChunk, 15);
	m_chunk.tex_splat_2_tiling = sqlite3_column_int(pResultsChunk, 16);
	m_chunk.tex_splat_3_tiling = sqlite3_column_int(pResultsChunk, 17);
	m_chunk.tex_splat_4_tiling = sqlite3_column_int(pResultsChunk, 18);


	//Process REsults into renderable
	m_d3dRenderer.BuildDisplayList(&m_sceneGraph);
	//build the renderable chunk 
	m_d3dRenderer.BuildDisplayChunk(&m_chunk);

}


void ToolMain::onActionSave()
{
	//SQL
	int rc;
	char *sqlCommand;
	char *ErrMSG = 0;
	sqlite3_stmt *pResults;								//results of the query
	

	//OBJECTS IN THE WORLD Delete them all
	//prepare SQL Text
	sqlCommand = "DELETE FROM Objects";	 //will delete the whole object table.   Slightly risky but hey.
	rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand, -1, &pResults, 0);
	sqlite3_step(pResults);

	//Populate with our new objects
	std::wstring sqlCommand2;
	int numObjects = m_sceneGraph.size();	//Loop thru the scengraph.

	for (int i = 0; i < numObjects; i++)
	{
		std::stringstream command;
		command << "INSERT INTO Objects " 
			<<"VALUES(" << m_sceneGraph.at(i).ID << ","
			<< m_sceneGraph.at(i).chunk_ID  << ","
			<< "'" << m_sceneGraph.at(i).model_path <<"'" << ","
			<< "'" << m_sceneGraph.at(i).tex_diffuse_path << "'" << ","
			<< m_sceneGraph.at(i).posX << ","
			<< m_sceneGraph.at(i).posY << ","
			<< m_sceneGraph.at(i).posZ << ","
			<< m_sceneGraph.at(i).rotX << ","
			<< m_sceneGraph.at(i).rotY << ","
			<< m_sceneGraph.at(i).rotZ << ","
			<< m_sceneGraph.at(i).scaX << ","
			<< m_sceneGraph.at(i).scaY << ","
			<< m_sceneGraph.at(i).scaZ << ","
			<< m_sceneGraph.at(i).render << ","
			<< m_sceneGraph.at(i).collision << ","
			<< "'" << m_sceneGraph.at(i).collision_mesh << "'" << ","
			<< m_sceneGraph.at(i).collectable << ","
			<< m_sceneGraph.at(i).destructable << ","
			<< m_sceneGraph.at(i).health_amount << ","
			<< m_sceneGraph.at(i).editor_render << ","
			<< m_sceneGraph.at(i).editor_texture_vis << ","
			<< m_sceneGraph.at(i).editor_normals_vis << ","
			<< m_sceneGraph.at(i).editor_collision_vis << ","
			<< m_sceneGraph.at(i).editor_pivot_vis << ","
			<< m_sceneGraph.at(i).pivotX << ","
			<< m_sceneGraph.at(i).pivotY << ","
			<< m_sceneGraph.at(i).pivotZ << ","
			<< m_sceneGraph.at(i).snapToGround << ","
			<< m_sceneGraph.at(i).AINode << ","
			<< "'" << m_sceneGraph.at(i).audio_path << "'" << ","
			<< m_sceneGraph.at(i).volume << ","
			<< m_sceneGraph.at(i).pitch << ","
			<< m_sceneGraph.at(i).pan << ","
			<< m_sceneGraph.at(i).one_shot << ","
			<< m_sceneGraph.at(i).play_on_init << ","
			<< m_sceneGraph.at(i).play_in_editor << ","
			<< m_sceneGraph.at(i).min_dist << ","
			<< m_sceneGraph.at(i).max_dist << ","
			<< m_sceneGraph.at(i).camera << ","
			<< m_sceneGraph.at(i).path_node << ","
			<< m_sceneGraph.at(i).path_node_start << ","
			<< m_sceneGraph.at(i).path_node_end << ","
			<< m_sceneGraph.at(i).parent_id << ","
			<< m_sceneGraph.at(i).editor_wireframe << ","
			<< "'" << m_sceneGraph.at(i).name << "'" << ","

			<< m_sceneGraph.at(i).light_type << ","
			<< m_sceneGraph.at(i).light_diffuse_r << ","
			<< m_sceneGraph.at(i).light_diffuse_g << ","
			<< m_sceneGraph.at(i).light_diffuse_b << ","
			<< m_sceneGraph.at(i).light_specular_r << ","
			<< m_sceneGraph.at(i).light_specular_g << ","
			<< m_sceneGraph.at(i).light_specular_b << ","
			<< m_sceneGraph.at(i).light_spot_cutoff << ","
			<< m_sceneGraph.at(i).light_constant << ","
			<< m_sceneGraph.at(i).light_linear << ","
			<< m_sceneGraph.at(i).light_quadratic

			<< ")";
		std::string sqlCommand2 = command.str();
		rc = sqlite3_prepare_v2(m_databaseConnection, sqlCommand2.c_str(), -1, &pResults, 0);
		sqlite3_step(pResults);	
	}
	MessageBox(NULL, L"Objects Saved", L"Notification", MB_OK);
}


void ToolMain::onActionSaveTerrain()
{
	m_d3dRenderer.SaveDisplayChunk(&m_chunk);
}


Brush* ToolMain::GetBrushIfActive()
{
	if (ActiveToolMode == ToolMode::Painting && ActiveBrush)
	{
		return ActiveBrush;
	}
	return nullptr;
}


void ToolMain::Tick(MSG *msg)
{
	/** The following is required for debugging/demonstration;
	its usefulness should be better realised by Toolbar functionality. */

	switch (ActiveToolMode)
	{
	case ToolMode::Picking:

		m_d3dRenderer.var = L"Picking";
		break;
	case ToolMode::Painting:

		// Again, this statement and required access are
		// a shortcut for adequate demonstration.

		if (dynamic_cast<ExtrusionBrush*>(ActiveBrush))
		{
			m_d3dRenderer.var = L"Extrusion rate: " + std::to_wstring(ExtrusionTypeBrush.Climb);
		}
		else
		{
			std::wstring Flatten = ContinuationTypeBrush.Flatten ? L"Flatten" : L"Don't flatten";
			m_d3dRenderer.var = L"Continuation: " + Flatten;
		}
		break;
	}

	/**  */
	
	if (ActiveToolMode == ToolMode::Painting && ActiveBrush
		&& m_toolInputCommands.LeftMouseIsDown())
	{
		// A primary mouse button press, during "Painting,"
		// performs the primary Brush action.

		ActiveBrush->Stroke(&m_d3dRenderer);
	}

	// Tick the renderer (Interface).

	m_d3dRenderer.Tick();
}


void ToolMain::SelectObjectByPick(bool SelectMultiple)
{
	// Control multiple object selection;
	// "Pick" casts to the screen for a selectable object.

	
	if (DisplayObject* ObjectPick = m_d3dRenderer.Pick())
	{
		if (SelectedObjects.find(ObjectPick) == SelectedObjects.end())
		{
			// "ObjectPick" is not currently selected.

			if (!SelectMultiple && !SelectedObjects.empty())
			{
				// Where multiple objects are currently selected and
				// additional selection is not enabled, the previous selection is discarded.

				SelectedObjects.clear();
			}

			SelectedObjects.insert(ObjectPick);
		}
		else
		{
			if (!SelectMultiple && SelectedObjects.size() > 1)
			{
				// Where multiple objects are currently selected and additional selection is
				// not enabled, the "Pick" is honoured and the previous selection, discarded.

				SelectedObjects.clear();
				SelectedObjects.insert(ObjectPick);
			}
			else
			{
				SelectedObjects.erase(ObjectPick);
			}
		}
	}
	else
	{
		// In the case of the "Picking" of any negative space,
		// de-select all.

		SelectedObjects.clear();
	}
}

void ToolMain::FlipBrush()
{
	// TODO: "Flip" functionality is temporary: major tool selection is -
	// eventually - to be done by toolbar button.

	if (dynamic_cast<ExtrusionBrush*>(ActiveBrush))
	{
		ActiveBrush = &ContinuationTypeBrush;

		ActiveBrush->OnActive(ExtrusionTypeBrush, this);
	}
	else
	{
		ActiveBrush = &ExtrusionTypeBrush;

		ActiveBrush->OnActive(ContinuationTypeBrush, this);
	}
}

void ToolMain::UpdateInput(MSG* msg)
{
	/**
		* TODO: The broadcasting of an input update is flawed and the "ChangeInInput" optimisation, redundant:
		* WM_MOUSEMOVE messages signal where there is zero cursor movement, during a mouse button press
		
		* (i.e., redundant updates of the input class negate any optimisation off-the-back of "ChangeInInput" validation).
	*/

	// A change in m_toolInputCommands is to be broadcast.

	bool ChangeInInputState = false;

	
	switch (msg->message)
	{
	case WM_KEYDOWN:

		m_keyArray[msg->wParam] = true;
		break;

	case WM_KEYUP:

		m_keyArray[msg->wParam] = false;
		break;

	case WM_MOUSEMOVE:

		m_toolInputCommands.SetCursorPosition({}, XMFLOAT2(GET_X_LPARAM(msg->lParam)
			, GET_Y_LPARAM(msg->lParam)));

		ChangeInInputState = true;

		if (ActiveToolMode == ToolMode::Painting && ActiveBrush)
		{
			// A Brush indicator corresponds to the cursor position.

			ActiveBrush->UpdateIndicator(WindowHandle, m_toolInputCommands
				, &m_d3dRenderer);
		}
		break;
	
	case WM_LBUTTONDOWN:
	{
		bool IsShortPress = false;

		m_toolInputCommands.SetLeftMouseDown({}, true
			, ShortPressWindow, &IsShortPress);

		if (ActiveToolMode == ToolMode::Painting && ActiveBrush)
		{
			// Resolve a primary mouse button down.

			ActiveBrush->OnPrimary();
		}

		ChangeInInputState = true;
	}
		break;

	case WM_LBUTTONUP:

		m_toolInputCommands.SetLeftMouseDown({}, false);

		ChangeInInputState = true;

		switch (ActiveToolMode)
		{
		case ToolMode::Painting:

			if (ActiveBrush)
			{
				ActiveBrush->Release(m_toolInputCommands
					, &m_d3dRenderer);
			}
			
			break;

		case ToolMode::Picking:

			SelectObjectByPick(PickMultiple);
			break;

		default:
			break;
		}
		break;

	case WM_RBUTTONDOWN:
	{
		m_toolInputCommands.SetRightMouseDown({}, true);

		ChangeInInputState = true;

		// The use of any tool is precluded during camera-control, as of a secondary
		// mouse press longer than that of a short press ("ShortPressWindow").

		ConcurrencyHelper::PostDelayFunction DeactivateTool = [&InactiveTool = InactiveTool
			, &ActiveToolMode = ActiveToolMode]()
			{
				InactiveTool = ActiveToolMode;
				ActiveToolMode = ToolMode::Inactive;
			};

		// 

		unsigned int Delay = static_cast<int>(std::round(ShortPressWindow * 1000));

		DeactivationTokenSource = std::make_shared<cancellation_token_source>();

		DeactivationTask = ConcurrencyHelper::CompleteAfterDelay(DeactivationTokenSource->get_token()
			, Delay
			, DeactivateTool);
	}
		break;

	case WM_RBUTTONUP:
	{
		bool IsShortPress = false;

		// Update the corresponding input command, and receive whether or not
		// ("IsShortPress") the press was completed within a designer-value duration.

		m_toolInputCommands.SetRightMouseDown({}, false, ShortPressWindow, &IsShortPress);

		ChangeInInputState = true;

		
		if (DeactivationTokenSource)
		{
			cancellation_token DeactivationToken = DeactivationTokenSource->get_token();

			if(!DeactivationToken.is_canceled())
			{
				// Cancel the delayed (By "ShortPressWindow" seconds) deactivation
				// of the current tool mode, which has been scheduled to preclude
				// the use of the current tool *during a secondary mouse button hold*.

				DeactivationTokenSource->cancel();
				DeactivationTask.wait();
			}
		}

		// The release of the secondary mouse press always restores the active tool.

		ActiveToolMode = InactiveTool;

		if (ActiveToolMode == ToolMode::Painting && ActiveBrush
			&& IsShortPress)
		{
			// A short press of the secondary mouse button envokes
			// some supplemental action during "Painting."

			ActiveBrush->OnSecondary();
		}
	}
		break;

	case WM_MOUSEWHEEL:

		m_toolInputCommands.SetScrollAxis({}, GET_WHEEL_DELTA_WPARAM(msg->wParam));

		ChangeInInputState = true;

		// (An input system may have been preferred in which any module response
		// is envoked by the "OnChangeInInput" delegate).

		if (Camera && Camera->CameraIsFree())
		{
			Camera->OnScrollInput(&m_toolInputCommands);
		}
		else if (ActiveToolMode == ToolMode::Painting && ActiveBrush)
		{
			ActiveBrush->OnScrollInput(m_toolInputCommands, ScrollSecondary);
		}

		// Default, having resolved scroll wheel input.

		m_toolInputCommands.DefaultScrollAxis({});

		break;

	case WM_MOUSEHOVER:

		break;
	}

	
	// "SHIFT_DOWN" indicates the selection of multiple scene objects.

	PickMultiple = m_keyArray[SHIFT_DOWN];

	// The "C" key functions as an input modifier for the "WM_MOUSEWHEEL."

	ScrollSecondary = m_keyArray['C'];


	// "WASD" input describes movement in two axes (The XZ-plane;
	// the forward - "WS" - and lateral - "AD" - axes).

	if (m_keyArray['W'])
	{
		m_toolInputCommands.SetForwardAxis({}, 1.f);

		ChangeInInputState = true;
	}
	else if (m_keyArray['S'])
	{
		m_toolInputCommands.SetForwardAxis({}, -1.f);

		ChangeInInputState = true;
	}
	else
	{
		m_toolInputCommands.SetForwardAxis({}, 0.f);
	}

	// (And for lateral, "AD," movement...).

	if (m_keyArray['D'])
	{
		m_toolInputCommands.SetLateralAxis({}, 1.f);

		ChangeInInputState = true;
	}
	else if (m_keyArray['A'])
	{
		m_toolInputCommands.SetLateralAxis({}, -1.f);

		ChangeInInputState = true;
	}
	else
	{
		m_toolInputCommands.SetLateralAxis({}, 0.f);
	}

	// "Q" and "E" key input are set to describe movement along the "Up" axis.

	if (m_keyArray['Q'])
	{
		// "Q" describes movement along the vertical axis in the negative direction.

		m_toolInputCommands.SetVerticalAxis({}, -1.f);

		ChangeInInputState = true;
	}
	else if (m_keyArray['E'])
	{
		m_toolInputCommands.SetVerticalAxis({}, 1.f);

		ChangeInInputState = true;
	}
	else
	{
		m_toolInputCommands.SetVerticalAxis({}, 0.f);
	}


	// The active tool can be assigned by key control.

	if (m_keyArray['V'])
	{
		// Select the "Picking" tool, and reset "V" key control.

		ActiveToolMode = ToolMode::Picking;

		m_keyArray['V'] = false;

		// Validate, firstly, that the camera mode
		// does not warrant a hidden cursor.

		if (Camera && !Camera->CameraIsFree())
		{
			// A "true" setting increments "winuser.h"'s internal display counter
			// (Microsoft Learn, 2024); during "Picking," the mouse cursor is visible.

			while (ShowCursor(true) < 0);
		}
	}
	if (m_keyArray['R'])
	{
		// Toggle the "Painting" tool.

		if (ActiveToolMode == ToolMode::Picking)
		{
			ActiveToolMode = ToolMode::Painting;

			// At the transition from the picking tool,
			// clear all selected objects.

			SelectedObjects.clear();
		}
		else
		{
			ActiveToolMode = ToolMode::Picking;

			if (Camera && !Camera->CameraIsFree())
			{
				while (ShowCursor(true) < 0);
			}
		}

		// Reset "R" key control.

		m_keyArray['R'] = false;
	}
	if (m_keyArray['T'])
	{
		// On "T," the current Brush is to toggled,
		// (Between the Extrusion and Continuation brushes).

		if (ActiveToolMode != ToolMode::Painting)
		{
			// On toggle, the Brush is the active tool.

			ActiveToolMode = ToolMode::Painting;

			SelectedObjects.clear();
		}

		FlipBrush();

		// Reset "T" key control.

		m_keyArray['T'] = false;
	}


	if (m_keyArray['F'])
	{
		m_d3dRenderer.ToggleWireframe();

		// Reset "F" key control.

		m_keyArray['F'] = false;
	}


	if (ChangeInInputState)
	{
		// Broadcast the change in the input state (The camera controller, for example,
		// may skip some expensive calculations where there is no change).

		OnChangeInInput(0);
	}
}


/*
	* Microsoft Learn
	* (2024)
	* ShowCursor function (winuser.h) - Win32 apps.
	* Available at: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showcursor
	* (Accessed: 26 February 2024).
*/

