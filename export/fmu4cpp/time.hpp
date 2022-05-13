
#ifndef FMU4CPP_TIME_HPP
#define FMU4CPP_TIME_HPP

#include <string>
#include <ctime>

namespace fmu4cpp {
    std::string now()
    {
        time_t now;
        time(&now);
        char buf[42];
        tm now_tm{};
        auto err = gmtime_s(&now_tm, &now);
        if (!err) {
            std::strftime(buf, sizeof(buf), "%FT%TZ", &now_tm);
            return buf;
        } else {
            return "1970-01-01T00:00:00Z";
        }
    }
}

#endif//FMU4CPP_TIME_HPP
