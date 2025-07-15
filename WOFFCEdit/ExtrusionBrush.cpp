#include "ExtrusionBrush.h"

#include <d3d12.h>
#include "SimpleMath.h"

#include <cmath>

#include "DisplayChunk.h"
#include "Game.h"
#include "InputCommands.h"


float ExtrusionBrush::Clamp(float a, float LowerLimit, float UpperLimit)
{
	if (a < LowerLimit) return LowerLimit;
	if (a > UpperLimit) return UpperLimit;

	return a;
}

float ExtrusionBrush::Smoothstep(float Edge0, float Edge1, float t)
{
	// Scale and clamp "t" to [0, 1].

	t = Clamp((t - Edge0) / (Edge1 - Edge0));

	// Evaluate Hermite polynomial.

	return t * t * (3.f - (2.f * t));
}


void ExtrusionBrush::OnSecondary()
{
	// Flip the active "Secondary" control.

	if (ActiveSecondary == Secondary::Weight)
	{
		ActiveSecondary = Secondary::Climb;
	}
	else if (ActiveSecondary == Secondary::Climb)
	{
		ActiveSecondary = Secondary::Weight;
	}
}


float ExtrusionBrush::GetHardHeight(Game* Game
	, const size_t i
	, const size_t j
	, float& CurrentHeight)
{
	if (Game)
	{
		DisplayChunk* Chunk = Game->GetDisplayChunk();

		if (Chunk)
		{
			// Assign the height of the terrain geometry at the index.

			CurrentHeight = Chunk->GetTerrainVertex(i, j).y;

			// Scale the current height by the "Climb" modifier
			// (In some direction).

			return CurrentHeight + Climb * static_cast<float>(Game->GetDeltaTime());
		}
	}

	return 0.0f;
}


void ExtrusionBrush::OnScrollInput(InputCommands& InputState, bool ScrollSecondary)
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
		switch (ActiveSecondary)
		{
		case Secondary::Weight:
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
		break;

		case Secondary::Climb:

			// A modified mouse wheel scroll may instead adapt
			// the rate at which an extrusion or impression is made.

			Climb += ScrollDirection * ClimbRate;

			break;
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
