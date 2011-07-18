// ****************************************************************************
//  statistics.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
//
//    Record execution and drawing events and return various statistics
//    (average frames per second, average execution time per frame...)
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************


#include "statistics.h"

TAO_BEGIN

void Statistics::reset()
// ----------------------------------------------------------------------------
//    Reset statistics and start a new measurement
// ----------------------------------------------------------------------------
{
    frames.clear();
    for (int i = 0; i < LAST_OP; i++)
    {
        data[i].clear();
        total[i] = 0;
        max[i] = 0;
        lastMaxTime[i] = 0;
    }
    intervalTimer.start();
}


void Statistics::begin(Operation)
// ----------------------------------------------------------------------------
//    Start measurement
// ----------------------------------------------------------------------------
{
    timer.start();
}


void Statistics::end(Operation op)
// ----------------------------------------------------------------------------
//    End measurement
// ----------------------------------------------------------------------------
{
    Q_ASSERT(op >= 0 && op < LAST_OP);

    int now = intervalTimer.elapsed();
    int elapsed = timer.elapsed();

    // Append measurement to list, keeping only 'interval' seconds of data.
    // Keep total up-to-date.
    date_val val(now, elapsed);
    durations &d = data[op];
    d.append(val);
    total[op] += elapsed;
    while (now - d.first().first > interval)
        total[op] -= d.takeFirst().second;

    // Update max if current value is larger, or peak interval has expired
    if (elapsed > max[op] || (now - lastMaxTime[op]) >= interval)
    {
        max[op] = elapsed;
        lastMaxTime[op] = now;
    }

    if (op == DRAW)
    {
        // A new frame is out, update FPS record
        frames.push_back(now);
        while (now - frames.front() > interval)
            frames.pop_front();
    }
}


double Statistics::fps()
// ----------------------------------------------------------------------------
//    Average frames per second during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    if (intervalTimer.elapsed() < interval)
        return -1.0;
    return (double)frames.size() * 1000 / interval;
}


int Statistics::averageTime(Operation op)
// ----------------------------------------------------------------------------
//    Average duration of specified operation during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    Q_ASSERT(op >= 0 && op < LAST_OP);

    if (intervalTimer.elapsed() < interval)
        return -1;

    durations &d = data[op];
    if (!d.size())
        return -1;
    return total[op]/d.size();
}


int Statistics::maxTime(Operation op)
// ----------------------------------------------------------------------------
//    Maximum duration of specified operation during last 'interval' seconds
// ----------------------------------------------------------------------------
{
    Q_ASSERT(op >= 0 && op < LAST_OP);

    if (intervalTimer.elapsed() < interval)
        return -1;

    return max[op];
}

TAO_END
