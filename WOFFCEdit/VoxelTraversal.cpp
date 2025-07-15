#define NOMINMAX

#include "VoxelTraversal.h"

#include <cmath>


namespace VoxelTraversal
{
	bool VoxelTraversal::RayIntersectsBounds(const Ray& Ray
		, const RegularGrid& Grid
		, float& tMin
		, float& tMax
		, float t0
		, float t1)
	{
		// The intervals of the entry and exit intersections along the ray.

		float tYMin = 0.f;
		float tYMax = 0.f;

		// Determine the direction of the ray along the x-axis, and accordingly:
		// determine the intervals of entry and exit intersections along the ray.

		// (Reduce the number of division operations by calculating the inverse x-direction).

		const float InverseXDirection = 1.f / Ray.Direction.x;

		if (InverseXDirection >= 0)
		{
			// The ray travels in the positive x-direction.

			tMin = (Grid.MinimumBound.x - Ray.Origin.x) * InverseXDirection;
			tMax = (Grid.MaximumBound.x - Ray.Origin.x) * InverseXDirection;
		}
		else
		{
			// The ray travels in the negative x-direction.
			
			// Relative to the ray origin, the maximum bound in the x-dimension
			// is considered the minimum entry bound (And vice versa).

			tMin = (Grid.MaximumBound.x - Ray.Origin.x) * InverseXDirection;
			tMax = (Grid.MinimumBound.x - Ray.Origin.x) * InverseXDirection;
		}

		// Performs the same determination for the intervals of
		// entry and exit intersections, along the ray, in the y-dimension.

		const float InverseYDirection = 1.f / Ray.Direction.y;

		if (InverseYDirection >= 0)
		{
			tYMin = (Grid.MinimumBound.y - Ray.Origin.y) * InverseYDirection;
			tYMax = (Grid.MaximumBound.y - Ray.Origin.y) * InverseYDirection;
		}
		else
		{
			tYMin = (Grid.MaximumBound.y - Ray.Origin.y) * InverseYDirection;
			tYMax = (Grid.MinimumBound.y - Ray.Origin.y) * InverseYDirection;
		}

		if (tMin > tYMax || tYMin > tMax)
		{
			// Either the entry intersection in the x-dimension is greater
			// than the exit intersection in the y-dimension, or the entry
			// intersection in "y" is greater than the exit in the x-dimension:
			
			// the ray is cast outwith the bounds of the grid.

			return false;
		}

		// The maximum entry and minimum exit intersections are preferred
		// (Exploit the shortest segment of the ray required to test).

		tMin = std::max(tYMin, tMin);
		tMax = std::min(tYMax, tMax);

		// Return that the minimum interval is less than the greater segment,
		// and that the maximum interval is greater than the minimum segment.

		return (tMin < t1 && tMax > t0);
	}


	bool VoxelTraversal::TraverseVoxelGrid(const Ray& Ray
		, const RegularGrid& Grid
		, float t0
		, float t1
		, const OnPerVoxel& OnPerVoxel)
	{
		// "tMin" and "tMax," respectively represent the extent along the ray
		// at which the ray initially and finally intersects with the grid.

		float tMin = 0.f;
		float tMax = 0.f;

		// Determine whether or not the ray intersects with the grid
		// (This process involves the computation of "tMin" and "tMax").

		if (!RayIntersectsBounds(Ray, Grid, tMin, tMax, t0, t1))
		{
			return false;
		}

		// "t0" and "t1" permit the caller to traverse a shorter segment of the ray
		// than the extent contained within the grid.

		tMin = std::max(tMin, t0);
		tMax = std::max(tMax, t1);

		// Determine the ray endpoints by casting in the direction of the ray from an endpoint,
		// for the magnitude of the corresponding interval of boundary intersection.

		const SimpleMath::Vector2 StartPoint = Ray.Origin + (Ray.Direction * tMin);
		const SimpleMath::Vector2 EndPoint = Ray.Origin + (Ray.Direction * tMax);


		// Evaluate the current index in a dimension, that is to be at least "1," and otherwise:
		// the difference between the end point and the minimum bound, partitioned by the voxel edge size and rounded.
		
		// (Regarding the use of size_t, (Gyurgyik and Kellison, 2020; Stack Overflow, 2018)).

		size_t CurrentXIndex = std::max(1, static_cast<int>(std::ceil((StartPoint.x - Grid.MinimumBound.x) / Grid.VoxelEdgeSize)));

		const size_t EndXIndex = std::max(1, static_cast<int>(std::ceil((EndPoint.x - Grid.MinimumBound.x) / Grid.VoxelEdgeSize)));

		// "Step" determines the direction of incrementation of the working index during traversal;
		
		int StepX = 0;

		// "tDelta" determines the extent of a movement in the ray direction in which the corresponding
		// horizontal or vertical displacement is equal to the voxel edge size.
		
		// "tMaxX" or "Y," represents the extent of movement in the ray direction
		// that results in an intersection with the corresponding boundary.

		float tDeltaX = 0.f;
		float tMaxX = 0.f;
		
		// The "Step," "tDelta" and "tMaxX" or "Y" variables are dependent on the ray's direction.

		if (Ray.Direction.x > 0.f)
		{
			// In the positive x-direction, the ray origin is a lesser coordinate than the ray end;
			// increment in the positive x-direction.

			StepX = 1;

			tDeltaX = Grid.VoxelEdgeSize / Ray.Direction.x;

			// From the minimum intersection interval, increment - for the difference between the entry index
			// and the minimum boundary - the difference between the minimum bound and the ray origin.

			tMaxX = tMin + (Grid.MinimumBound.x + CurrentXIndex * Grid.VoxelEdgeSize - StartPoint.x)
				/ Ray.Direction.x;
		}
		else if (Ray.Direction.x < 0.f)
		{
			// Increment in the negative x-direction.

			StepX = -1;

			tDeltaX = Grid.VoxelEdgeSize / -Ray.Direction.x;

			// A ray that traverses the negative direction along an axis intersects
			// from the previous index.

			const size_t PreviousXIndex = CurrentXIndex - 1;

			tMaxX = tMin + (Grid.MinimumBound.x + PreviousXIndex * Grid.VoxelEdgeSize - StartPoint.x)
				/ Ray.Direction.x;
		}
		else
		{
			// Do not increment the x-component.

			StepX = 0;

			tDeltaX = tMax;
			tMaxX = tMax;
		}

		// Complete the identical work, as above, for the y-dimension.

		size_t CurrentYIndex = std::max(1, static_cast<int>(std::ceil((StartPoint.y - Grid.MinimumBound.y) / Grid.VoxelEdgeSize)));

		const size_t EndYIndex = std::max(1, static_cast<int>(std::ceil((EndPoint.y - Grid.MinimumBound.y) / Grid.VoxelEdgeSize)));

		int StepY = 0;

		float tDeltaY = 0.f;
		float tMaxY = 0.f;

		if (Ray.Direction.y > 0.f)
		{
			StepY = 1;

			tDeltaY = Grid.VoxelEdgeSize / Ray.Direction.y;

			tMaxY = tMin + (Grid.MinimumBound.y + CurrentYIndex * Grid.VoxelEdgeSize - StartPoint.y)
				/ Ray.Direction.y;
		}
		else if (Ray.Direction.y < 0.f)
		{
			StepY = -1;

			tDeltaY = Grid.VoxelEdgeSize / -Ray.Direction.y;

			const size_t PreviousYIndex = CurrentYIndex - 1;

			tMaxY = tMin + (Grid.MinimumBound.y + PreviousYIndex * Grid.VoxelEdgeSize - StartPoint.y)
				/ Ray.Direction.y;
		}
		else
		{
			StepY = 0;

			tDeltaY = tMax;
			tMaxY = tMax;
		}


		/** Traverse the voxel grid (In either positive or negative directions)
		until an end index is arrived at. */

		// (Gyurgyik and Kellison, 2020) is an untested implementation of (Amanatides and Woo, 1987).
		
		// In addition to minor changes above, the following block (And primary loop)
		// required totally adapting to correctly handle edge cases.

		bool Success = false;

		// Perform some function per intersecting voxel.
		
		while (false == (Success = OnPerVoxel(CurrentXIndex, CurrentYIndex)))
		{
			// Simply, step along an axis in the direction of the ray,
			// for whichever dimension is nearest crossed.

			if (tMaxX < tMaxY)
			{
				if (CurrentXIndex == EndXIndex)
				{
					break;
				}

				tMaxX += tDeltaX;
				CurrentXIndex += StepX;
			}
			else
			{
				if (CurrentYIndex == EndYIndex)
				{
					break;
				}

				tMaxY += tDeltaY;
				CurrentYIndex += StepY;
			}
		}

		if (Success == false)
		{
			// Execute, lastly, for the ray exit.

			return OnPerVoxel(CurrentXIndex, CurrentYIndex);
		}
		else
		{
			return true;
		}
	}
}


/**
	* Amanatides, J. and Woo, A.
	* (1987)
	* 'A Fast Voxel Traversal Algorithm for Ray Tracing',
	* Eurographics,
	* 87(3),
	* pp. 3-10.
*/

/**
	* Gyurgyik, C., Kellison, A.
	* (2020)
	* fast-voxel-traversal-algorithm.
	* Available at: https://github.com/cgyurgyik/fast-voxel-traversal-algorithm
	* (Accessed: 01 May 2024).
*/

/**
	* Stack Overflow
	* (2018)
	* unsigned int vs. size_t
	* Available at: https://stackoverflow.com/a/4295225 .
	* (Accessed: 01 May 2024).
*/
