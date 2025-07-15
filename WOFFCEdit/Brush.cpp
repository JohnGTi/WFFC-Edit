#define NOMINMAX

#include "ToolMain.h"
#include "Brush.h"

#include "DirectXCollision.h"
#include "DeviceResources.h"

#include "Game.h"
#include "DisplayChunk.h"
#include "InputCommands.h"

#include "CameraController.h"

using namespace DirectX;


Brush::Brush()
{
	// Prepare and map the palette of colours from which the indicator geometry is drawn.

	// The "Weight" colour values are part of a colour vision deficiency-robust pair,
	// "Deep Cerulean" (#0072b2) and "Disco" (A purple - #882255) designed using (Nichols, no date).

	IndicatorColours.emplace(Indicator::Size, Colors::White);
	IndicatorColours.emplace(Indicator::Weight, DirectX::XMVECTORF32({ { { 0.53125f, 0.1328125f, 0.33203125f, 1.f } } }));
}


void Brush::OnChangeInCameraMode(CameraController* Camera)
{
	if (Camera)
	{
		if (Camera->CameraIsFree())
		{
			IsAwake = false;
		}
		else
		{
			IsAwake = true;
		}
	}
}


void Brush::CreateDeviceDependentGeometry(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
	if (Device && DeviceContext)
	{
		// Create a custom effect that totally colours the primitive drawing,
		// and default the colour argument to that of the "Size" indicator.
		
		IndicatorEffect = std::make_unique<BasicEffect>(Device);

		if (IndicatorEffect)
		{
			IndicatorEffect->SetFogEnabled(true);

			auto ColourIterator = IndicatorColours.find(Indicator::Size);

			if (ColourIterator != IndicatorColours.end())
			{
				IndicatorEffect->SetFogColor(ColourIterator->second);
			}
		}
		
		// 

		auto SizeIndicator = DirectX::GeometricPrimitive::CreateSphere(DeviceContext, Size, 8);

		float WeightDiameter = Size * std::min(1, static_cast<int>(Weight));

		auto WeightIndicator = DirectX::GeometricPrimitive::CreateSphere(DeviceContext, WeightDiameter, 8);

		// Create the input layout (By way of any indicator geometry).

		SizeIndicator->CreateInputLayout(IndicatorEffect.get(), InputLayout.ReleaseAndGetAddressOf());

		// Transfer the ownership of the geometries to a map class member,
		// keyed by a representative state enum.

		IndicatorGeometry.emplace(Indicator::Size, std::move(SizeIndicator));
		IndicatorGeometry.emplace(Indicator::Weight, std::move(WeightIndicator));
	}
}


void Brush::ResetGeometry()
{
	// 

	for (auto& Indicator : IndicatorGeometry)
	{
		Indicator.second.reset();
	}
	
	IndicatorEffect.reset();

	InputLayout.Reset();
}


std::pair<Brush::Anchor, SimpleMath::Vector3> Brush::AssignValidPlacement(float MaximumDistance, float EdgeSize
	, DirectX::XMVECTOR RelativePosition, std::array<DirectX::XMVECTOR, 3>& Vertices)
{
	/** Determine the vertex or minor edge mid-point closest to the relative position. */

	// Cache the foremost (Either the bottom left or bottom right)
	// and second (The bottom right, or top right) vertices.

	SimpleMath::Vector3 ForemostVertex = Vertices.at(0);
	SimpleMath::Vector3 SecondVertex = Vertices.at(1);

	float HalfEdge = EdgeSize / 2.f;

	// Declare a map of global coordinate and placement specifier pairs.

	std::map<Anchor, SimpleMath::Vector3> ValidPlacements;

	// If the half-stepped foremost vertex is greater than the second vertex in the x-
	// dimension (In the global coordinate system), then the triangle is the leftmost.

	if ((ForemostVertex.x - HalfEdge) > SecondVertex.x)
	{
		// Firstly, emplace the known vertices (Of the left triangle) in the anchor/vertex map.

		ValidPlacements.emplace(Anchor::BottomLeft, Vertices.at(0));
		ValidPlacements.emplace(Anchor::BottomRight, Vertices.at(1));
		ValidPlacements.emplace(Anchor::TopLeft, Vertices.at(2));

		// Determine and emplace: the coordinate values of the minor edge mid-points.

		ValidPlacements.emplace(Anchor::Bottom
			, DisplayChunk::GetEdgeMidpoint(Vertices.at(0), Vertices.at(1)));

		ValidPlacements.emplace(Anchor::Left
			, DisplayChunk::GetEdgeMidpoint(Vertices.at(2), Vertices.at(0)));
	}
	else
	{
		// Do as above, for the right triangle.

		ValidPlacements.emplace(Anchor::BottomRight, Vertices.at(0));
		ValidPlacements.emplace(Anchor::TopRight, Vertices.at(1));
		ValidPlacements.emplace(Anchor::TopLeft, Vertices.at(2));

		ValidPlacements.emplace(Anchor::Right
			, DisplayChunk::GetEdgeMidpoint(Vertices.at(0), Vertices.at(1)));

		ValidPlacements.emplace(Anchor::Top
			, DisplayChunk::GetEdgeMidpoint(Vertices.at(1), Vertices.at(2)));
	}

	// Declare the working return value,
	// and define the shortest viable distance (Within the triangle bounds).

	std::pair<Anchor, SimpleMath::Vector3> WorkingPlacement;

	float ShortestSquaredDistance = MaximumDistance * MaximumDistance;

	for (const auto& Placement : ValidPlacements)
	{
		// Calculate the squared distance between the viable placement
		// and the relative position.

		XMVECTOR Difference = XMVectorSubtract(RelativePosition, Placement.second);

		float SquaredDistance = 0.f;

		XMStoreFloat(&SquaredDistance, XMVector3LengthSq(Difference));

		if (SquaredDistance < ShortestSquaredDistance)
		{
			// Cache the minimum squared distance
			// and the corresponding "Placement" attributes.

			ShortestSquaredDistance = SquaredDistance;

			WorkingPlacement.first = Placement.first;
			WorkingPlacement.second = Placement.second;
		}
	}

	// Return a pair of the placement identifier
	// and global space coordinate.

	return WorkingPlacement;
}

void Brush::UpdateIndicator(HWND WindowHandle, InputCommands& InputState, Game* RendererInterface)
{
	if (!IsAwake)
	{
		return;
	}

	/** Determine the cast direction and global coordinate origin and ray destination.  */

	if (RendererInterface == nullptr)
	{
		return;
	}

	// Retrieve a pair of the respective minimum
	// and maximum screen viewport depth values.

	std::pair<float, float> ViewportDepth = RendererInterface->GetViewportDepth();

	// Project the screen-space cursor coordinates to the near
	// and far planes of the viewing frustrum.

	XMFLOAT2 CursorPosition = InputState.GetCursorPosition();

	const XMVECTOR NearSource = XMVectorSet(CursorPosition.x, CursorPosition.y, 0.0f, 1.0f);
	const XMVECTOR FarSource = XMVectorSet(CursorPosition.x, CursorPosition.y, 1.0f, 1.0f);

	// Retrieve the dimensions of the client window.

	RECT WindowDimensions;

	GetClientRect(WindowHandle, &WindowDimensions);

	// Unproject the cursor coordinates from the viewing frustrum to global space.

	XMVECTOR RayOrigin = XMVector3Unproject(NearSource, 0.0f, 0.0f, WindowDimensions.right
		, WindowDimensions.bottom
		, ViewportDepth.first
		, ViewportDepth.second
		, RendererInterface->GetProjection()
		, RendererInterface->GetView()
		, RendererInterface->GetWorld());

	XMVECTOR RayEnd = XMVector3Unproject(FarSource, 0.0f, 0.0f, WindowDimensions.right
		, WindowDimensions.bottom
		, ViewportDepth.first
		, ViewportDepth.second
		, RendererInterface->GetProjection()
		, RendererInterface->GetView()
		, RendererInterface->GetWorld());

	// 

	XMVECTOR Difference = RayEnd - RayOrigin;

	float CastMagnitude = XMVectorGetX(XMVector3Length(Difference));

	// Compose a vector from the global-space near and far sources.

	XMVECTOR CastDirection = XMVector3Normalize(Difference);


	/**  */

	// Retrieve the current (Only) chunk of terrain.

	DisplayChunk* Chunk = RendererInterface->GetDisplayChunk();

	if (Chunk == nullptr)
	{
		return;
	}

	// The chunk geometry is to be represented in the x-z (Devoid of height - "y")
	// dimensions.
	
	// This truncation facilitates an optimised search of intersecting
	// quadrilaterals to test for ray intersection in (x, y, z).

	VoxelTraversal::RegularGrid ChunkXZ;

	XMFLOAT3 MinimumBound = Chunk->GetTerrainVertex(0, 0);
	XMFLOAT3 MaximumBound = Chunk->GetTerrainVertex(TERRAINRESOLUTION - 1, TERRAINRESOLUTION - 1);

	ChunkXZ.MinimumBound = SimpleMath::Vector2(MinimumBound.x, MinimumBound.z);
	ChunkXZ.MaximumBound = SimpleMath::Vector2(MaximumBound.x, MaximumBound.z);

	ChunkXZ.VoxelEdgeSize = Chunk->GetCellWidth();

	// The search along intersecting voxels requires a Ray
	// in the same 2-dimensional space as the grid.

	VoxelTraversal::Ray Ray;

	Ray.Origin.x = XMVectorGetX(RayOrigin);
	Ray.Origin.y = XMVectorGetZ(RayOrigin);

	Ray.End.x = XMVectorGetX(RayEnd);
	Ray.End.y = XMVectorGetZ(RayEnd);

	Ray.Direction.x = XMVectorGetX(CastDirection);
	Ray.Direction.y = XMVectorGetZ(CastDirection);


	// Test ray/quadrilateral (/Triangle pair) in (x, y, z) for every
	// intersecting voxel found by "TraverseVoxelGrid," in (x, z).

	VoxelTraversal::OnPerVoxel TestQuadrilateral = [Chunk, &RayOrigin, &CastDirection, EdgeSize = ChunkXZ.VoxelEdgeSize, &Brush = *this](const size_t i, const size_t j)
		{
			// "OnPerVoxel" expects a pair of indices counting from "1;"
			// subtract "1" to index from zero.

			const size_t x = i - 1;
			const size_t z = j - 1;

			if (Chunk)
			{
				// The terrain geometry data is arranged in a two-dimensional array,
				// where the major dimension is representative of the z-axis:

				XMVECTOR BottomLeft = XMLoadFloat3(&Chunk->GetTerrainVertex(z, x + 1));
				XMVECTOR BottomRight = XMLoadFloat3(&Chunk->GetTerrainVertex(z, x));

				XMVECTOR TopLeft = XMLoadFloat3(&Chunk->GetTerrainVertex(z + 1, x + 1));
				XMVECTOR TopRight = XMLoadFloat3(&Chunk->GetTerrainVertex(z + 1, x));

				// The magnitude from the ray origin to an intersection point.

				float CollisionDistance = 0.f;

				bool Collision = false;


				// First, try intersection with the left triangle.

				// (Default the collection of near vertices to that of the left triangle).

				std::array<XMVECTOR, 3> NearVertices = { BottomLeft
							, BottomRight
							, TopLeft
				};

				// Test.

				if (TriangleTests::Intersects(RayOrigin, CastDirection
					, BottomLeft, BottomRight, TopLeft, CollisionDistance))
				{
					// Flag the success of the collision.

					Collision = true;
				}
				else if (TriangleTests::Intersects(RayOrigin, CastDirection
					, BottomRight, TopRight, TopLeft, CollisionDistance))
				{
					// Overwrite the collection of triangle vertices
					// to be that of this triangle.

					NearVertices = { BottomRight, TopRight, TopLeft };

					Collision = true;
				}

				if (Collision)
				{
					// Determine the point of intersection according to the magnitude
					// of the former part of the intersecting ray.

					XMVECTOR IntersectionPoint = RayOrigin + (CastDirection * CollisionDistance);

					// According to the vertices of the intersecting triangle,
					// select the nearest viable indicator placement (A vertex, or edge mid-point).

					auto WorkingPair = Brush.AssignValidPlacement(Chunk->GetMaximumHeight(), EdgeSize
						, IntersectionPoint, NearVertices);

					Brush.IndicatorPlace.Situation = WorkingPair.first;
					Brush.IndicatorPlace.GlobalPosition = WorkingPair.second;

					// Index the indicator placement by the bottom left vertex.

					Brush.IndicatorPlace.Origin = std::make_pair(z, x + 1);

					// Flag the positive detection.

					return true;
				}
			}

			// Return that the ray does not intersect the quadrilateral in (x, y, z).

			return false;
		};

	// Traverse the x-z grid of the current chunk, according to the determined ray (In x-z),
	// and incrementally test the intersecting voxels for a valid intersection in (x, y, z).
	
	IsIndicatorValid = VoxelTraversal::TraverseVoxelGrid(Ray, ChunkXZ, 0.f, CastMagnitude
		, TestQuadrilateral);
}


void Brush::OnActive(Brush& PreviousBrush, ToolMain* Framework)
{
	if (Framework)
	{
		// Upon swapping to this derived Brush,
		// adopt the previous indicator attributes.

		if (this == Framework->GetBrushIfActive())
		{
			IndicatorPlace = PreviousBrush.GetIndicatorPlace();

			Size = PreviousBrush.GetSize();
			Weight = PreviousBrush.GetWeight();
		}
	}
}


void Brush::Stroke(Game* RendererInterface)
{
	if (RendererInterface == nullptr)
	{
		return;
	}

	DisplayChunk* Chunk = RendererInterface->GetDisplayChunk();

	if (Chunk == nullptr)
	{
		return;
	}

	/**  */

	// Cast to usable float values, the relevant indicator parameters,
	// and retrieve the size of a triangle edge.

	float Diameter = static_cast<float>(Size);
	float InnerRadius = static_cast<float>(Weight) * 0.5f;

	float EdgeSize = Chunk->GetCellWidth();

	// Cache indicator placement attributes for ease-of-access,
	// and retrieve the global coordinates of the quadrilateral's origin.

	SimpleMath::Vector3 GlobalPosition = IndicatorPlace.GlobalPosition;

	std::pair<size_t, size_t> Origin(
		IndicatorPlace.Origin.first
		, IndicatorPlace.Origin.second);

	SimpleMath::Vector3 GlobalOrigin = Chunk->GetTerrainVertex(
		Origin.first
		, Origin.second);

	// The maximum coordinate in (z-x) is to determine the vertex from which
	// vertices are collected for testing, and equates to the Brush radius away
	// from the concered (Indicator) coordinate.

	SimpleMath::Vector2 MaximumVertexInZX(
		IndicatorPlace.GlobalPosition.z + (Diameter * EdgeSize / 2.f)
		, IndicatorPlace.GlobalPosition.x + (Diameter * EdgeSize / 2.f));

	// Accounting for the discrepency between the indicator coordinate (Situated on a vertex or edge mid-point)
	// from the quadrilateral origin (The bottom left vertex), calculate the range of cells that separate the
	// indicator and maximum bound so as to result in the an index pair for the maximum bound.

	std::pair<size_t, size_t> MaximumBound(
		Origin.first + static_cast<size_t>(std::ceil((MaximumVertexInZX.x - GlobalOrigin.z) / EdgeSize))
		, Origin.second + static_cast<size_t>(std::ceil((MaximumVertexInZX.y - GlobalOrigin.x) / EdgeSize)));


	/** Identify a number of vertices in the voxelised (z-x) range of the Brush,
	and test each vertex for intersection with the bounds of the Brush. */

	// "HardUmbrella" indexes vertices that fall within the Brush's inner bounds;
	// dictated by "Weight," the heights of these vertices are without interpolation.

	std::vector<std::pair<size_t, size_t>> HardUmbrella;

	// "SoftUmbrella" collects vertices by the "Extrusion" grouped list of
	// a terrain geometry index and corresponding global coordinate.

	std::vector<Extrusion> SoftUmbrella;

	// Traverse the vertices from the maximum bound indices,
	// for the scope of the Brush.

	for (int i = MaximumBound.first; i >= (static_cast<int>(MaximumBound.first) - static_cast<int>(Size)); --i)
	{
		for (int j = MaximumBound.second; j >= (static_cast<int>(MaximumBound.second) - static_cast<int>(Size)); --j)
		{
			// Determine the distance between the current vertex and the indicated vertex.

			SimpleMath::Vector3 CurrentVertex = Chunk->GetTerrainVertex(i, j);

			float Distance = std::abs(DirectX::SimpleMath::Vector3::Distance(CurrentVertex, GlobalPosition));

			if (Distance <= InnerRadius)
			{
				// The current vertex falls within the inner radius,
				// defined by the Brush "Weight."

				// Here, the Brush leans the heaviest (Totally).

				HardUmbrella.push_back(std::make_pair(i, j));
			}
			else if (Distance <= (Diameter / 2.f))
			{
				// An affected vertex outwith the dense inner radius
				// is to be manipulated further than a simple value assignment.

				Extrusion SoftVertex(std::make_pair(i, j)
					, CurrentVertex);

				SoftUmbrella.push_back(SoftVertex);
			}
		}
	}


	/** Determine the hard and soft (Interpolated) height values for the collected vertices. */

	for (const auto& Index : HardUmbrella)
	{
		float CurrentHeight = 0.f;

		// A derived class defines its own method of manipulating the height of an affected vertex.

		float Height = GetHardHeight(RendererInterface
			, Index.first, Index.second, CurrentHeight);

		// Assign the newly determined height.

		Chunk->SetTerrainHeightByVertex(Index.first, Index.second, Height);
	}

	for (const auto& SoftExtrusion : SoftUmbrella)
	{
		/** Interpolate between the current and hard heights, 
		according to the current coordinate's distance from the inner bounds. */

		float CurrentHeight = 0.f;

		float HardHeight = GetHardHeight(RendererInterface
			, SoftExtrusion.Index.first, SoftExtrusion.Index.second, CurrentHeight);

		float FromCentre = DirectX::SimpleMath::Vector3::Distance(SoftExtrusion.GlobalPosition, GlobalPosition);

		// The temporal parameter is a fraction (The distance from the inner bounds to the concerned vertex)
		// of the extent of the Brush (From the Brush centre).

		float t = (FromCentre - InnerRadius) / ((Diameter * 0.5f) - InnerRadius);

		/**
			* Further work might see a variety of interpolation curves,
			* selectable by a context menu.
			* 
			* float Smooth = Smoothstep(HardHeight, CurrentHeight, t);
		*/

		float Height = HardHeight + t * (CurrentHeight - HardHeight);

		Chunk->SetTerrainHeightByVertex(SoftExtrusion.Index.first, SoftExtrusion.Index.second, Height);
	}
}


void Brush::OnScrollInput(InputCommands& InputState, bool ScrollSecondary)
{
	if (!IsAwake || !IsIndicatorValid)
	{
		// Modification of Brush settings yields no results,
		// where such changes cannot be represented.

		return;
	}

	// Retrieve the direction along the scroll axis.

	short ScrollDirection = InputState.GetScrollAxis();

	if (ScrollSecondary)
	{
		// The manipulation of the weight of the Brush features as a
		// modifiable component of input that alters "Size."

		// The incremented "Weight" must be within the bounds of
		// the granularity of this Brush, and its size.

		short NewWeight = static_cast<short>(Weight) + ScrollDirection;

		if (NewWeight >= BrushGranularity && NewWeight <= Size)
		{
			Weight += ScrollDirection;
		}
	}
	else
	{
		// The incremented "Size" cannot be less than the
		// granularity of this Brush.

		if ((static_cast<short>(Size) + ScrollDirection) >= BrushGranularity)
		{
			Size += ScrollDirection;

			// Maintain that the weight of a stroke cannot exceed
			// the size of its impression.

			if (Size < Weight)
			{
				Weight = Size;
			}
		}
	}
}


void Brush::Release(InputCommands& InputState, Game* Game)
{
	if (Game && !InputState.LeftMouseIsDown())
	{
		// Upon the release of a brush stroke,
		// refresh the working height map by terrain data.

		if (DisplayChunk* Chunk = Game->GetDisplayChunk())
		{
			Chunk->UpdateHeightMap();
		}
	}
}


void Brush::DrawIndicatorGeometry(DirectX::XMMATRIX World)
{
	// Retrieve state information of the global cursor (The retrieval
	// requires the correct size of structure (Microsoft Learn, 2024a)).

	GlobalCursorInfo.cbSize = sizeof(CURSORINFO);

	bool GlobalQuery = GetCursorInfo(&GlobalCursorInfo);

	if (!IsAwake || !IsIndicatorValid)
	{
		// Where the tool-specific indicator is not in use,
		// employ the mouse cursor.

		if (GlobalQuery && GlobalCursorInfo.flags == 0)
		{
			// A "true" setting increments "winuser.h"'s internal display counter
			// (Microsoft Learn, 2024b); loop until the display counter is true.

			while (ShowCursor(true) < 0);
		}

		return;
	}
	else
	{
		if (GlobalQuery && GlobalCursorInfo.flags == CURSOR_SHOWING)
		{
			// The global cursor is not to display where the
			// tool-specific cursor is drawn.

			while (ShowCursor(false) >= 0);
		}
	}


	/** Draw the indicator geometries. */

	if (IndicatorEffect)
	{
		// Explicitly cast the indicator size attributes for a following transformation.

		float OuterScale = static_cast<float>(Size);
		float InnerScale = static_cast<float>(Weight);

		for (auto& Indicator : IndicatorGeometry)
		{
			// The first item (The key) of the key-value pair identifies
			// the kind of indicator (Indicative of "Size" or "Weight").

			// Situate the indicator.

			SimpleMath::Matrix Transformation;

			switch (Indicator.first)
			{
			case Indicator::Weight:

				Transformation = World * XMMatrixScaling(InnerScale, InnerScale, InnerScale)
					* XMMatrixTranslationFromVector(IndicatorPlace.GlobalPosition);

				break;

			case Indicator::Size:

				Transformation = World * XMMatrixScaling(OuterScale, OuterScale, OuterScale)
					* XMMatrixTranslationFromVector(IndicatorPlace.GlobalPosition);

				break;
			}

			IndicatorEffect->SetWorld(Transformation);

			// Colour the indicator.

			auto ColourIterator = IndicatorColours.find(Indicator.first);

			if (ColourIterator != IndicatorColours.end())
			{
				IndicatorEffect->SetFogColor(ColourIterator->second);
			}
			
			// The second item (The value) of the key-value pair is the
			// indicator geometry: draw the geometric primitive.

			Indicator.second->Draw(IndicatorEffect.get(), InputLayout.Get()
				, false, Wireframe);
		}
	}
}


/*
	* Microsoft Learn
	* (2024a)
	* GetCursorInfo function (winuser.h) - Win32 apps.
	* Available at: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getcursorinfo
	* (Accessed: 03 May 2024).
*/

/*
	* Microsoft Learn
	* (2024b)
	* ShowCursor function (winuser.h) - Win32 apps.
	* Available at: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showcursor
	* (Accessed: 26 February 2024).
*/

/**
	* Nichols, D.
	* (no date)
	* Coloring for Colorblindness.
	* Available at: https://davidmathlogic.com/colorblind/#%23D81B60-%231E88E5-%23FFC107-%23004D40
	* (Accessed: 26 April 2024).
*/
