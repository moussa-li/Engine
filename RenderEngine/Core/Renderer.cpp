#include "Renderer.hpp"

#include "Camera.hpp"
#include "Entity.hpp"
#include "RenderConfigure.hpp"
#include "Window.hpp"

namespace EgLab
{
    void Renderer::clear()
    {
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < _entities.size(); ++i)
        {
            _entities[i]->update(time);
        }
    }

    void Renderer::draw() const
    {
        for (size_t i = 0; i < _entities.size(); ++i)
        {
            auto &e = _entities[i];
            e->draw();
        }
    }

    UniquePtr<RenderConfigure> &Renderer::getConfigure()
    {
        return _configure;
    }

    Renderer::Renderer(UniquePtr<Camera> &&camera)
        : _camera(move(camera)), _configure(makeUnique<RenderConfigure>())
    {
    }

} // namespace EgLab