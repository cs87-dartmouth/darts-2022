/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/

#include <darts/common.h>
#include <darts/progress.h>

#include <chrono>
#include <spdlog/stopwatch.h>

#if defined(_WIN32)
#include <windows.h>

#ifdef NOMINMAX
#undef NOMINMAX
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

#else

#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif // !_WIN32

#include <signal.h>

using std::endl;
using std::flush;
using std::string;
using std::vector;

namespace
{

// Assumed screen width if we can't find the real value.
#define DEFAULT_SCREEN_WIDTH 80

// A flag that, when set, means SIGWINCH was received.
std::atomic<bool> received_signal(false);
std::atomic<bool> monitoring_signal(false);

// Determine the width of the terminal we're running on.
int terminal_width()
{
#if defined(_WIN32)
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == NULL)
    {
        fprintf(stderr, "GetStdHandle() call failed");
        return DEFAULT_SCREEN_WIDTH;
    }
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = {0};
    GetConsoleScreenBufferInfo(h, &bufferInfo);
    return bufferInfo.dwSize.X;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
    {
        // ENOTTY is fine and expected, e.g. if output is being piped to a file.
        if (errno != ENOTTY)
        {
            static bool warned = false;
            if (!warned)
            {
                warned = true;
                spdlog::error("Error in ioctl() in terminal_width(): {}", errno);
            }
        }
        return DEFAULT_SCREEN_WIDTH;
    }
    return w.ws_col;
#endif
}

inline void show_console_cursor(bool const show)
{
#if defined(_WIN32)
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = show; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
#else
    std::fputs(show ? "\033[?25h" : "\033[?25l", stdout);
#endif
}

#ifdef SIGWINCH
void signal_handler(int sig)
{
    if (sig == SIGWINCH)
    {
        received_signal = true;
        signal(SIGWINCH, signal_handler);
    }
}
#endif

} // namespace

Progress::Progress(const string &title, int64_t totalWork) : m_title(title), m_num_steps(totalWork), m_steps_done(0)
{
    fflush(stdout);
    m_exit = false;

    // zero total work doesn't make sense. use a busy progress instead.
    if (m_num_steps == 0)
        m_num_steps = -3000;

#ifdef SIGWINCH
    if (!monitoring_signal.exchange(true))
        signal(SIGWINCH, signal_handler);
#endif

    show_console_cursor(false);

    m_update_thread = std::thread(
        [this]()
        {
            int                                   screen_width;
            int                                   bar_width;
            int                                   title_width;
            const int                             default_fill_width = 10;
            static const std::vector<std::string> fractional_characters{"", "▏", "▎", "▍", "▌", "▋", "▊", "▉"};
            spdlog::stopwatch                     timer;

            auto reset_bar = [&screen_width, &bar_width, &title_width, this]()
            {
                screen_width = terminal_width();
                if (!screen_width)
                    screen_width = DEFAULT_SCREEN_WIDTH;

                // the text should be at most 50% of the screen width
                title_width = std::min((int)m_title.size(), screen_width / 2);

                bar_width = std::max(1, screen_width - 32 - title_width);
            };

            reset_bar();

            bool done = false;

            do
            {
                // If terminal width might have changed, update the length of the progress bar
                if (received_signal.exchange(false))
                    reset_bar();

                double elp      = timer.elapsed().count() * 1000.;
                double fraction = std::clamp(double(m_steps_done) / double(m_num_steps), 0.0, 1.0);

                bool silent = spdlog::get_level() > spdlog::level::info;
                done        = m_exit || fraction >= 1.f;
                bool busy   = m_num_steps < 0;

                if (!silent)
                {
                    if (busy || done)
                    {

                        double bounce          = (1. - cos(-elp / m_num_steps * M_PI * 2)) * 0.5;
                        int    fill_width      = std::min(bar_width, done ? bar_width : default_fill_width);
                        int    left_padding    = int(round(bounce * (bar_width - fill_width)));
                        int    remaining_width = screen_width - title_width - 2 - bar_width - 1;

                        string time_text = fmt::format(" ({})", time_string(elp));
                        fmt::print("\r{:.{}} │{:<{}}{:█^{}}{:>{}}│{:<{}}", m_title, title_width, "", left_padding, "",
                                   fill_width, "", bar_width - left_padding - fill_width, time_text, remaining_width);
                    }
                    else
                    {
                        int    whole_width     = int(std::floor(fraction * bar_width));
                        float  remainder_width = std::fmod((fraction * bar_width), 1.0f);
                        int    part_width      = int(std::floor(remainder_width * fractional_characters.size()));
                        auto   frac_text       = fractional_characters[part_width];
                        double eta             = elp / fraction - elp;
                        int    remaining_width = screen_width - title_width - 2 - bar_width - 1 - 5;

                        auto time_text =
                            fmt::format(" ({}/{})", time_string(elp), time_string(std::max(0., elp + eta)));
                        fmt::print("\r{:.{}} │{:█>{}}{: <{}}│{:>4d}%{:<{}}", m_title, title_width, "", whole_width,
                                   frac_text, bar_width - whole_width, int(round(fraction * 100)), time_text,
                                   remaining_width);
                    }
                    if (done)
                        fmt::print("\n");
                    fflush(stdout);
                }

                if (!done)
                {
                    using namespace std::chrono_literals;

                    // gradually lengthening the update interval
                    std::this_thread::sleep_for(std::clamp(std::chrono::milliseconds(int(elp * 0.01)), 40ms, 10000ms));
                }
            } while (!done);
        });
}

Progress::~Progress()
{
    set_done();
}

void Progress::set_done()
{
    m_steps_done = m_num_steps;
    m_exit       = true;

    if (m_update_thread.joinable())
        m_update_thread.join();

    show_console_cursor(true);
}
