#include "Renderer.hpp"

#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Entity.hpp"
#include "RenderEngine/Core/RenderConfigure.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/Window.hpp"

namespace EgLab
{
    void Renderer::clear()
    {
    }

    void Renderer::addViewport(RenderViewport &viewport)
    {
        _viewports.pushBack(viewport);
    }

    Return Renderer::addEntity(SharedPtr<Entity> entity)
    {
        if (entity == nullptr) return Return::Failed;
        _entities.pushBack(entity);

        return Return::Succeed;
    }

    void Renderer::update(DeltaTime time)
    {
        if (_configure->enableDeepTest.get())
        {
            glEnable(GL_DEPTH_TEST);
            switch (_configure->deepTestType.get())
            {
            case RenderConfigure::DeepTestType::LESS:
                glDepthFunc(GL_LESS);
                break;
                // TODO:add other deep test type
            default:
                glDepthFunc(GL_LESS);
                break;
            }
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < _entities.size(); ++i)
        {
            _entities[i]->update(time);
        }
    }

    void Renderer::draw(const Scene &scene, const Camera &camera) const
    {
        for (IdxType i = 0; i < _viewports.size(); ++i)
        {
            const auto &viewport = _viewports[i];
            for (IdxType j = 0; j < _entities.size(); ++j)
            {
                auto &e = _entities[j];
                e->loadCamera(viewport.camera);
                e->draw();
            }
        }
    }

    UniquePtr<RenderConfigure> &Renderer::getConfigure()
    {
        return _configure;
    }

    Renderer::Renderer() : _configure(makeUnique<RenderConfigure>())
    {
    }

} // namespace EgLab