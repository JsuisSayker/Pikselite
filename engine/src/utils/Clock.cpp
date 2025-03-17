#include <utils/Clock.hpp>

Clock::Clock() : startTime(std::chrono::steady_clock::now())
{
}

/**
 * @brief Get the elapsed time since the clock was started.
 *
 * This function calculates the time elapsed from the moment the clock was started
 * to the current time. The elapsed time is returned as a double representing the
 * duration in seconds.
 *
 * @return The elapsed time in seconds as a double.
 */
double Clock::getElapsedTime() const
{
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - startTime;
    return elapsed.count();
}

/**
 * @brief Restarts the clock and returns the elapsed time since the last restart.
 *
 * This function captures the current time, calculates the duration since the last
 * recorded start time, updates the start time to the current time, and returns
 * the elapsed time in seconds.
 *
 * @return The elapsed time in seconds since the last restart.
 */
double Clock::restart()
{
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = currentTime - startTime;
    startTime = currentTime;
    return elapsed.count();
}