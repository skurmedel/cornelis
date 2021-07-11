#pragma once

#include <exception>
#include <stdexcept>

namespace cornelis {
class ExpectationException : public std::runtime_error {
  public:
    ExpectationException(char const *e) : std::runtime_error(e) {}
};

namespace detail {
template <typename TPred>
inline auto expectsImpl(TPred &&pred, char const *msg) -> void {
    if (!pred()) {
        throw ExpectationException(msg);
    }
}
} // namespace detail

#define CORNELIS_EXPECTS(pred, msg) detail::expectsImpl([&]() -> bool { return (pred); }, msg)
} // namespace cornelis
