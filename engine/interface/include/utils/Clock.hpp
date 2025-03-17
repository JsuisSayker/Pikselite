#pragma once

#include <iostream>
#include <chrono>

class Clock
{
public:
    /**
     * @brief Default constructor for the Clock class.
     *
     * Initializes a new instance of the Clock class.
     */
    Clock();

    /**
     * @brief Default destructor for the Clock class.
     */
    ~Clock() = default;

    /**
     * @brief Get the elapsed time since the clock was started.
     *
     * @return double The elapsed time in seconds.
     */
    double getElapsedTime() const;

    /**
     * @brief Restarts the clock and returns the elapsed time.
     *
     * This function resets the clock to zero and returns the time elapsed since the last restart or since the clock was created.
     *
     * @return double The elapsed time in seconds.
     */
    double restart();

private:
    /**
     * @brief Represents a point in time, typically used to measure intervals.
     *
     * This member variable holds the starting time point using the steady clock,
     * which is a monotonic clock that is not affected by system clock changes.
     * It is useful for measuring elapsed time durations.
     */
    std::chrono::steady_clock::time_point startTime;
};