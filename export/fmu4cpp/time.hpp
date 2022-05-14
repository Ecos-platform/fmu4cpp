
#ifndef FMU4CPP_TIME_HPP
#define FMU4CPP_TIME_HPP

#include <ctime>
#include <string>

namespace fmu4cpp {
    std::string now() {
        time_t now = time(nullptr);
        char buf[42];
        tm now_tm{};

#ifdef _MSC_VER
        auto err = gmtime_s(&now_tm, &now);
        if (!err) {
            std::strftime(buf, sizeof(buf), "%FT%TZ", &now_tm);
            return buf;
        } else {
            return "1970-01-01T00:00:00Z";
        }
#else
        gmtime_r(&now, &now_tm);
        std::strftime(buf, sizeof(buf), "%FT%TZ", &now_tm);
        return buf;
#endif
    }

}// namespace fmu4cpp

#endif//FMU4CPP_TIME_HPP
