#pragma once

#include <memory>

#include <cornelis/RenderOptions.hpp>
#include <cornelis/SceneDescription.hpp>
#include <functional>

namespace cornelis {
enum class RenderCommand {
    Continue,
    Abort,
};
enum class RenderStatus { Running, Done, Aborted, Failed };
struct RenderProgress {};
class RenderSession {
  public:
    using ProgressCallback =
        std::function<RenderCommand(RenderProgress const &, RenderStatus const &)>;

    RenderSession(SceneDescription const &, RenderOptions options);
    ~RenderSession();

    RenderSession(RenderSession &&) = default;
    auto operator=(RenderSession &&) -> RenderSession & = default;

    RenderSession(RenderSession const &) = delete;
    auto operator=(RenderSession const &) -> RenderSession & = delete;

    /**
     * This is just a shorthand that tries to render until completion.
     */
    auto render() -> void;
    /**
     * Starts the render and repeatedly calls onProgress as the render progresses. Blocks until the
     * render is stopped, either by completion or failure.
     * 
     * This will always try to call onProgress at least once.
     * 
     * The callback has to be thread-safe; it might be called by different threads "at the same time".
     *
     * The callback can abort the render by returning RenderCommand::Abort.
     */
    auto render(ProgressCallback onProgress) -> void;

  private:
    struct State;
    std::unique_ptr<State> me_;
};
} // namespace cornelis
