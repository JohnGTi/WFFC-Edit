#pragma once

#include <ppltasks.h>


/**
	* A home for implementations of recommended "Concurrency Runtime" patterns (Learn Microsoft, 2021a).
*/
namespace ConcurrencyHelper
{
	/**
		* The signature of a function that the "CompleteAfterDelay" task may envoke.
	*/
	typedef std::function<void()> PostDelayFunction;

	/**
		* Execute a function of type "PostDelayFunction" following after delay.
		* (Following (Learn Microsoft, 2021b)).
		* 
		* @param	CancellationToken	A task is cancellable by way of the source of this token object (This helper adapts (Learn Microsoft, 2021b,
										Example: complete_after and cancel_after_timeout functions) to be cancellable (Learn Microsoft, 2021c,
										Using a Cancellation Token to Cancel Parallel Work)).

		* @param	Delay				In milliseconds, the delay duration.
		* @param	PostDelayFunction	The function to execute.
		* 
		* @return	A reference to the concurrent task.
	*/
	concurrency::task<void> CompleteAfterDelay(concurrency::cancellation_token& CancellationToken
		, unsigned int Delay
		, const PostDelayFunction& PostDelayFunction);
}


/**
	* Learn Microsoft
	* (2021a)
	* Task Parallelism (Concurrency Runtime).
	* Available at: https://learn.microsoft.com/en-us/cpp/parallel/concrt/task-parallelism-concurrency-runtime?view=msvc-170
	* (Accessed: 29 April 2024).
*/

/**
	* Learn Microsoft
	* (2021b)
	* How to: Create a task that completes after a delay.
	* Available at: https://learn.microsoft.com/en-us/cpp/parallel/concrt/how-to-create-a-task-that-completes-after-a-delay?view=msvc-170
	* (Accessed : 29 April 2024).
*/

/**
	* Learn Microsoft
	* (2021c)
	* Cancellation in the PPL.
	* Available at: https://learn.microsoft.com/en-us/cpp/parallel/concrt/cancellation-in-the-ppl?view=msvc-170
	* (Accessed: 29 April 2024).
*/

