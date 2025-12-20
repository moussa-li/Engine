#include "Renderer.hpp"

#include "Entity.hpp"
#include "Window.hpp"

namespace EgLab
{
    void Renderer::clear()
    {
    }

    void Renderer::update(DeltaTime time)
    {
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

} // namespace EgLab