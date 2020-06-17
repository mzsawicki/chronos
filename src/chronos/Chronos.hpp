#pragma once
#include "boost/date_time/posix_time/posix_time.hpp"
#include "chronos/Dispatcher.hpp"
#include "chronos/Queue.hpp"
#include "chronos/Schedule.hpp"
#include "chronos/System.hpp"
#include "chronos/Task.hpp"
#include "chronos/Timer.hpp"


namespace chronos
{
    using clock_t = boost::posix_time::second_clock;
    using queue_t = ThreadsafePriorityQueue<Task, task::compare::Later>;
    using schedule_t = Schedule<queue_t, clock_t>;
    using dispatcher_t = Dispatcher<schedule_t, SystemCall>;
}
