#pragma once

#include <memory>

#include <cornelis/RenderOptions.hpp>
#include <cornelis/SceneDescription.hpp>

namespace cornelis {
class RenderSession {
  public:
    RenderSession(SceneDescription const &, RenderOptions options);
    ~RenderSession();

    RenderSession(RenderSession &&) = default;
    auto operator=(RenderSession &&) -> RenderSession & = default;

    RenderSession(RenderSession const &) = delete;
    auto operator=(RenderSession const &) -> RenderSession & = delete;

    auto render() -> void;

  private:
    struct State;
    std::unique_ptr<State> me_;
};
} // namespace cornelis
