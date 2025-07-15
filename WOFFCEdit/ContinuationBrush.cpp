#include "ContinuationBrush.h"

#include "DisplayChunk.h"
#include "Game.h"


void ContinuationBrush::OnPrimary()
{
	// TODO: This function does not adequately validate that its
	// calling is a product of the intial down press of
	// the primary mouse button, and so is functionally incohesive and incomplete.

	// Reserve the height of terrain geometry at the current
	// indicator placement, for the continuation of the terrain.

	HardHeight = IndicatorPlace.GlobalPosition.y;
}

void ContinuationBrush::OnSecondary()
{
	// Toggle an attribute of this Brush's
	// continuation augmentation.

	Flatten = !Flatten;
}


float ContinuationBrush::GetHardHeight(Game* Game
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

			
			// "Flatten" signifies that the Brush is to be applied - including where
			// the current vertex eclipses the Brush height,
			// otherwise: the Brush height must be greater than the current height.

			if (Flatten || (HardHeight > CurrentHeight))
			{
				return HardHeight;
			}
			else
			{
				return CurrentHeight;
			}
		}
	}

	return 0.0f;
}