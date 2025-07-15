#pragma once

#include <d3d12.h>
#include "SimpleMath.h"


using namespace DirectX;


/**
	* Efficiently search a 2-dimensional plane along a line (Of the same dimensions).
*/
namespace VoxelTraversal
{
	/**
		* An opposing pair of corners (Coordinate data), and the size of a voxel edge
		* that comprise a regular grid in 2-dimensions.
	*/
	struct RegularGrid
	{
		SimpleMath::Vector2 MinimumBound = SimpleMath::Vector2::Zero;
		SimpleMath::Vector2 MaximumBound = SimpleMath::Vector2::Zero;

		float VoxelEdgeSize = 0.f;
	};


	/**
		* The endpoints and directional component of a line in 2-dimensions.
	*/
	struct Ray
	{
		SimpleMath::Vector2 Origin = SimpleMath::Vector2::Zero;
		SimpleMath::Vector2 End = SimpleMath::Vector2::Zero;

		SimpleMath::Vector2 Direction = SimpleMath::Vector2::Zero;
	};


	/**
		* Test for intersection with the bounds of a two-dimensional grid;
		* adapted from Williams et al.'s (2005) improved ray/box intersection (Smits, 1999).
		*
		* @param	tMin	Output either the ray origin,
		* 					or the entry intersection along the ray.
		* 
		* @param	tMax	The exit intersection along the ray.
		* 
		* @param	t0		(t1 > t0; 0.0 <= t0 <= 1.0) Relative to the ray origin,
		*					the beginning of the ray segment to test.
		* 
		* @param	t1		(t1 > t0; 0.0 <= t1 <= 1.0) Relative to the ray origin,
		*					the end of the ray segment to test.
		* 
		* @return	Whether or not the line intersects with the bounds of the grid.
	*/
	bool RayIntersectsBounds(const Ray& Ray, const RegularGrid& Grid
		, float& tMin, float& tMax, float t0, float t1);

	/**
		* The function signature of the vertex search that "TraverseVoxelGrid"
		* performs per voxel.
		* 
		* @param	i	Index the i'th voxel.
		* @param	j	Index the j'th voxel.
	*/
	typedef std::function<bool(const size_t i, const size_t j)> OnPerVoxel;

	/**
		* Implement the 2-dimensional (2D) variant of (Amanatides and Woo, 1987),
		* and efficiently search a regular, 2D voxel space along some "Ray,"
		* performing some operation per ray-intersecting voxel.
		* 
		* @param	Ray			Should the ray intersect with the "Grid,"
		* 						the grid is traversed along the intersecting voxels.
		* 
		* @param	Grid		A regular, 2D grid.
		* 
		* @param	t0			(t1 > t0; 0.0 <= t0 <= 1.0) Relative to the ray origin,
		*						the beginning of the ray segment to test.
		* 
		* @param	t1			(t1 > t0; 0.0 <= t1 <= 1.0) Relative to the ray origin,
		*						the end of the ray segment to test.
		* 
		* @param	OnPerVoxel	The function to perform per intersecting voxel.
		* 
		* @return	Flag a negative intersection or, further,
		*			the result of the per-voxel operation.
	*/
	bool TraverseVoxelGrid(const Ray& Ray
		, const RegularGrid& Grid
		, float t0
		, float t1
		, const OnPerVoxel& OnPerVoxel);
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
	* Smits, B.
	* (2005)
	* 'Efficiency Issues for Ray Tracing',
	* Journal of Graphics Tools,
	* 3(2),
	* pp. 1–14.
	* doi: 10.1080/10867651.1998.10487488
*/

/**
	* Williams, A., Barrus, S., Morley, R.K. and Shirley, P.
	* (2005)
	* 'An Efficient and Robust Ray–Box Intersection Algorithm',
	* ACM SIGGRAPH 2005 Courses,
	* pp. 9-es.
	* doi: 10.1145/1198555.1198748
*/

