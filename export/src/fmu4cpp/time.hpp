
#ifndef FMU4CPP_TIME_HPP
#define FMU4CPP_TIME_HPP

#include <ctime>
#include <string>

namespace fmu4cpp {
    inline std::string now() {
        time_t now = time(nullptr);
        constexpr size_t buf_size = 42;
        char buf[buf_size];
        tm now_tm{};

#ifdef _MSC_VER
        if (const auto err = gmtime_s(&now_tm, &now); !err) {
            std::strftime(buf, sizeof(buf), "%FT%TZ", &now_tm);
            return buf;
        }


#else
        if (gmtime_r(&now, &now_tm)) {
            std::strftime(buf, sizeof(buf), "%FT%TZ", &now_tm);
            return buf;
        }
#endif

        return "1970-01-01T00:00:00Z";// fallback
    }

}// namespace fmu4cpp

#endif//FMU4CPP_TIME_HPP
