#include "ConcurrencyHelper.h"

#include <memory>
#include <ppl.h>
#include <agents.h>

using namespace concurrency;


namespace ConcurrencyHelper
{
	task<void> CompleteAfterDelay(cancellation_token& CancellationToken
		, unsigned int Delay
		, const PostDelayFunction& PostDelayFunction)
	{
		// A "task_completion_event" will signal the completion of the delay "timer."

		task_completion_event<void> BroadcastDelayCompletion;

		// Define the delay timer and the callback function
		// (Which envokes the delay completion event).

		auto DelayTimer = std::make_shared<timer<int>>(Delay, 0, nullptr, false);

		auto Callback = std::make_shared<call<int>>([BroadcastDelayCompletion](int)
			{
				BroadcastDelayCompletion.set();
			});

		// Link the call object to the timer and start the timer.

		DelayTimer->link_target(Callback.get());
		DelayTimer->start();

		// "OnDelayCompletion" executes upon the setting of the delay completion event;
		// its continuation task captures the execution of post-delay functionality.

		task<void> OnDelayCompletion(BroadcastDelayCompletion);

		return OnDelayCompletion.then([DelayTimer, Callback, CancellationToken, PostDelayFunction]()
			{
				// Unless this task's corresponding cancellation token has been cancelled
				// (In which case: cancel the working task), execute post-delay functionality.

				if (!CancellationToken.is_canceled())
				{
					PostDelayFunction();
				}
				else
				{
					cancel_current_task();
				}
			});
	}
}
