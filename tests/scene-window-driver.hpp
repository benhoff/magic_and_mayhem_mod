#pragma once
#include "scene.hpp"
#include "scene_canvas.hpp"
#include "playback.hpp"
#include <memory>
namespace mnm::scene::test {
// Test-only observer/action driver; compiled with the production entry point only in its test target.
std::shared_ptr<QObject> startWindowScript(const QString& script,const QString& output,QWidget&,
    SceneCanvas&,const game::MovementSession&,std::function<Frame()>,Playback&,std::function<void()>& afterTick);
}
