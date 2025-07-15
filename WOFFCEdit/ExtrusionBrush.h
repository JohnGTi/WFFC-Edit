#pragma once

#include "Brush.h"


/**
	* 
*/
class ExtrusionBrush : public Brush
{
private:
	/** Minor, modifiable attributes. */
	enum class Secondary : uint8_t
	{
		Climb,
		Weight
	};

	/**
		* TODO: Encapsulate interpolation functionality so that
		* they may be purposed as static helpers.
		* 
		* (Wikipedia, 2024, Smoothstep).
	*/
	float Clamp(float a, float LowerLimit = 0.0f, float UpperLimit = 1.0f);

	float Smoothstep(float Edge0, float Edge1, float t);


public:
	/**  */
	virtual void OnSecondary() override;

	/**
		* @return	The modified height value of the relevant terrain vertex.
	*/
	virtual float GetHardHeight(class Game* Game, const size_t i, const size_t j
		, float& CurrentHeight) override;

	/**  */
	virtual void OnScrollInput(InputCommands& InputState
		, bool ScrollSecondary = false) override;



	/** Attributes. */

private:
	/** Signify the modifiable minor attribute. */
	Secondary ActiveSecondary = Secondary::Weight;

	/** The rate at which "Climb" is adapted. */
	const float ClimbRate = 5.f;

	/**
		* The rate and direction at which terrain geometry is manipulated.
	*/
public:float Climb = 5.f; // This tool-specific member is temporarily public, for debugging/demonstration.
};


/**
	* Wikipedia
	* (2024)
	* Smoothstep
	* Available at: https://en.wikipedia.org/wiki/Smoothstep
	* (Accessed: 06 May 2024).
*/

