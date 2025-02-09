#include "timer-id.h"

mg::TimerId::TimerId()
    : _timer(nullptr), _sequence(0)
{
    ;
}

mg::TimerId::TimerId(Timer *timer, int64_t seq) : _timer(timer),
                                                  _sequence(seq)
{
    ;
}
