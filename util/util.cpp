#include<stdio.h>
#include<string>
#include<fstream>
#include<vector>
#include<set>
#include<algorithm>

#include"util.h"

#ifdef _WIN32
#include<windows.h>
#else
#include<unistd.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<libgen.h>
#endif

namespace util {

    uint64_t getSystemTime()
    {
#ifdef _WIN32
        return GetTickCount64();
#else
        struct timeval t;
        gettimeofday(&t, NULL);
        return (uint64_t)t.tv_sec * 1000 + t.tv_usec / 1000;
#endif
    }

    Timer::Timer()
    {
        _startTime = 0;
    }

    void Timer::start()
    {
        _startTime = getSystemTime();
    }

    uint64_t Timer::getTime()
    {
        return getSystemTime() - _startTime;
    }

    void sleep(int seconds)
    {
#ifdef _WIN32
        Sleep(seconds * 1000);
#else
        sleep(seconds);
#endif
		char buf[100] = { 0 };

		sprintf(buf, "%d", value);

		return std::string(buf);
	}

	void removeNewline(std::string &s)
	{
		size_t len = s.length();

		int toRemove = 0;

		if(len >= 2) {
			if(s[len - 2] == '\r' || s[len - 2] == '\n') {
				toRemove++;
			}
		}
		if(len >= 1) {
			if(s[len - 1] == '\r' || s[len - 1] == '\n') {
				toRemove++;
			}
		}

		if(toRemove) {
			s.erase(len - toRemove);
		}
	}

    unsigned int endian(unsigned int x)
    {
        return (x << 24) | ((x << 8) & 0x00ff0000) | ((x >> 8) & 0x0000ff00) | (x >> 24);
    }

    std::string toLower(const std::string &s)
    {
        std::string lowerCase = s;
        std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), ::tolower);

        return lowerCase;
    }

    std::string trim(const std::string &s, char c)
    {
        size_t left = s.find_first_not_of(c);
        size_t right = s.find_last_not_of(c);

        return s.substr(left, right - left + 1);
    }
}