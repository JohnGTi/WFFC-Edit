#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <GeometricPrimitive.h>
#include <Effects.h>

#include <array>
#include <map>

#include "VoxelTraversal.h"


/** Forward declaration. */
class InputCommands;
class Game;


/**  */
class Brush
{
private:
	/**
		* Refer to a specific pointer/indicator geometry in a readable way.
	*/
	enum class Indicator : uint8_t
	{
		Weight,
		Size
	};

	/**
		* Signify a corner or edge mid-way of a quadrilateral.
	*/
	enum class Anchor : uint8_t
	{
		Bottom,
		BottomRight,
		Right,
		TopRight,
		Top,
		TopLeft,
		Left,
		BottomLeft
	};

	/**
		* The global space position, and situation (A signifier of the placement along the quadrilateral);
		* and the "DisplayChunk" - terrain data index of the quadrilateral's bottom left;
		* of some item - relative to a ("DisplayChunk") quadrilateral in global space.
	*/
	struct QuadrilateralPlacement
	{
		DirectX::SimpleMath::Vector3 GlobalPosition = DirectX::SimpleMath::Vector3::Zero;

		Anchor Situation = Anchor::BottomLeft;

		/**
			* (The "DisplayChunk" - terrain data index of the bottom left of the relevant quadrilateral).
		*/
		std::pair<size_t, size_t> Origin;
	};

	/**
		* A pair of indices that might index a terrain vertex,
		* and the global coordinates of that vertex.
	*/
	struct Extrusion
	{
		std::pair<size_t, size_t> Index;

		DirectX::SimpleMath::Vector3 GlobalPosition = DirectX::SimpleMath::Vector3::Zero;

		/**  */
		Extrusion(std::pair<size_t, size_t> ij, DirectX::SimpleMath::Vector3 Position)
			: Index(ij), GlobalPosition(Position) {}
	};


public:
	/**
		* Constructor.
		* 
		* (Build the colour palette from which the indicator is drawn).
	*/
	Brush();

	/**
		* Validate the Brush indicator according to the mode of camera control.
	*/
	void OnChangeInCameraMode(class CameraController* Camera);

	/**
		* Build renderer dependent geometry and effects.
	*/
	void CreateDeviceDependentGeometry(ID3D11Device* Device
		, ID3D11DeviceContext* DeviceContext);

	/**  */
	void ResetGeometry();


private:

	/**
		* Determine the nearest valid placement (Vertex or edge mid-point)
		* with the minimum squared distance from some position.
		* 
		* @return	Signify the resultant corner or edge mid-way,
		*			and provide the global space coordinates.
	*/
	std::pair<Anchor, SimpleMath::Vector3> AssignValidPlacement(float MaximumHeight, float EdgeSize
		, DirectX::XMVECTOR RelativePosition, std::array<DirectX::XMVECTOR, 3>& Vertices);


public:
	/**
		* Determine the placement of the indicator - define a triangle-
		* pair intersection test to submit for terrain geometry (In x-z) traversal.
	*/
	virtual void UpdateIndicator(HWND WindowHandle
		, InputCommands& InputState, Game* Game);

	/**  */
	virtual void OnPrimary() {}

	/**  */
	virtual void OnSecondary() {}

	/**  */
	virtual void OnActive(Brush& PreviousBrush, class ToolMain* Framework);

	/**
		* A derived class may define the derived function
		* that determines the hard height value of an augmented vertex.
	*/
	virtual float GetHardHeight(Game* Game, const size_t i, const size_t j
		, float& CurrentHeight) = 0;

	/**
		* Collect a number of vertices in the range of "Size"
		* relative to the indicator,
		* and perform some augmentation to their height values.
	*/
	virtual void Stroke(Game* RendererInterface);

	/**
		* This default implementation adapts either the Brush "Size" or "Weight."
	*/
	virtual void OnScrollInput(InputCommands& InputState
		, bool ScrollSecondary = false);

	/**
		* Upon the release of a brush stroke,
		* refresh the working height map by terrain data.
	*/
	virtual void Release(InputCommands& InputState
		, Game* Game);

	/**
		* Indicator attributes are readable so that an instance of "Brush"
		* might adopt another's attributes.
	*/
	QuadrilateralPlacement GetIndicatorPlace() const
	{
		return IndicatorPlace;
	}

	unsigned int GetSize() const
	{
		return Size;
	}
	unsigned int GetWeight() const
	{
		return Weight;
	}

	/**  */
	bool GetIsAwake() const
	{
		return IsAwake;
	}

	/**  */
	std::shared_ptr<DirectX::BasicEffect> GetIndicatorEffect() const
	{
		return IndicatorEffect;
	}

	/**
		* TODO: I deliberated on encapsulating the drawing functionality for readability,
		* and at the time of writing this would extrapolate the not-
		* computational parts of this function to the renderer interface (Game).
		* 
		* (Remembering that - wherever possible - the simulation and renderer should be separate).
	*/
	void DrawIndicatorGeometry(DirectX::XMMATRIX World);



	/** Attributes. */

protected:
	/**  */
	QuadrilateralPlacement IndicatorPlace;

	/**
		* The validity of the indicator is determined according to
		* the landscape intersection tests.
	*/
	bool IsIndicatorValid = true;

	/**
		* This module is responsible for its own being enabled,
		* and responds to a camera controller broadcast to determine its being enabled.
	*/
	bool IsAwake = true;


private:
	/**  */
	CURSORINFO GlobalCursorInfo;

	/** The colour of an indicator, by indicator type. */
	std::map<Indicator, DirectX::XMVECTORF32> IndicatorColours;


protected:
	/** The total diameter of the Brush's scope. */
	unsigned int Size = 1;

	/**
		* A sub-diameter of the Brush - the inner part within which
		* augmentation may be total ("Hard," and not interpolated).
	*/
	unsigned int Weight = 1;


public:
	/**
		* The Brush functions per-vertex, and the Brush attributes
		* are not to represent values lesser.
	*/
	const unsigned int BrushGranularity = 1;

	/**
		* Essential items that compose the "BasicEffect,"
		* which totally colours the sphere indicators.
	*/
	std::shared_ptr<DirectX::BasicEffect> IndicatorEffect;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;

	std::map<Indicator, std::unique_ptr<DirectX::GeometricPrimitive>> IndicatorGeometry;

	/**
		* The wireframe drawing setting of an indicator geometry.
	*/
	const bool Wireframe = true;
};

