#include <sstream>
#include <string>
#include <utility>

#include <catch2/catch_tostring.hpp>

namespace Catch {

template <typename T1, typename T2>
struct StringMaker<std::pair<T1, T2>> {
    static std::string convert(const std::pair<T1, T2>& pair) {
        std::ostringstream oss;
        oss << "(" << pair.first << "," << pair.second << ")";
        return oss.str();
    }
};

}  // namespace Catch
