#include "test_render.h"

#include "RenderEngine/Core/Window.hpp"

TEST_F(TestRender, window)
{
    EgLab::Window window(300, 400);
    window.exec();
}