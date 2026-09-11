#include "Core/MainMenuBar.hpp"

#include "Command/CommandManager.hpp"
#include "Core/Application.hpp"

namespace EgLab
{
} // namespace EgLab

namespace EgLab::Platform
{
    void renderRibbonToolbar(Application* app)
    {
        (void)app;
        LOG(INFO) << "renderRibbonToolbar moved behind RenderEngine Window UI facade";
    }

} // namespace EgLab::Platform