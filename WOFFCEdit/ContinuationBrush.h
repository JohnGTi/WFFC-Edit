#pragma once

#include "Brush.h"


/**
	* Overwrite a vertex according to the height of the indicated
	* vertex at the time of the press.
	* 
	* (This function is not adequately responsible for its control flow,
	* which is correctly handled by "ToolMain").
*/
class ContinuationBrush : public Brush
{
public:
	/**
		* Reserve the height of terrain geometry at the current indicator placement.
	*/
	virtual void OnPrimary() override;

	/**
		* Toggle the mode of continuation (Meet or "Flatten" higher terrain).
	*/
	virtual void OnSecondary() override;

	/**
		* The height value at which the terrain is to be extended and -
		* optionally - flattened.
	*/
	virtual float GetHardHeight(class Game* Game, const size_t i, const size_t j
		, float& CurrentHeight) override;



	/** Attributes. */

private:
	/**
		* The terrain geometry height value that is set
		* upon the Brush action, "".
	*/
	float HardHeight = 0.f;

	/**
		* Permit the assignment of the hard height value,
		* regardless of the height of the affected terrain.
	*/
public:bool Flatten = false; // This tool-specific member is temporarily public, for debugging/demonstration.
};

