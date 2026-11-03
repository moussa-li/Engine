#pragma once
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Window;
    class Camera;
    class Entity;
    class Light;
    class Scene;
    class Shader;
    class RenderConfigure;
    class RenderPrimitive;
    class Selector;

    struct RenderViewport
    {
        /// Window associated with this viewport.
        Common::SharedPtr<Window> window;
        int x, y;          ///< Viewport origin in window coordinates.
        int width, height; ///< Viewport dimensions in pixels.
    };

    /// Material parameters used for the renderer's directional light.
    struct DirectionalLightSettings
    {
        /// Ambient light contribution.
        Common::Vector3f ambient{0.15f, 0.15f, 0.15f};
        /// Diffuse light contribution.
        Common::Vector3f diffuse{0.8f, 0.8f, 0.8f};
        /// Specular light contribution.
        Common::Vector3f specular{0.35f, 0.35f, 0.35f};
        /// Specular shininess exponent.
        Scalar shininess{32.0f};
    };

    /// Coordinates scene rendering, viewport management, and primitive selection.
    class RenderEngineAPI Renderer
    {
    public:
        /// @brief Adds a viewport to the renderer.
        /// @details The viewport configuration is retained by the renderer.
        void addViewport(RenderViewport &);

        /// @brief Clears the configured viewports and renderer state.
        void clear();

        /** @brief Draws the scene from the specified camera.
         * @param scene Scene containing renderable primitives.
         * @param camera Camera used to produce the view and projection matrices.
         */
        void draw(const Common::SharedPtr<Scene> &scene,
                  const Common::SharedPtr<Camera> &camera) const;

        /** @brief Retrieves the most recently picked node, if available.
         * @param primitive Receives the picked primitive.
         * @param nodeIndex Receives the picked node index.
         * @return true if a node pick is available.
         */
        bool tryGetPickedNode(Common::SharedPtr<RenderPrimitive> &primitive,
                              IdxType &nodeIndex) const;

        /// @brief Sets the renderer background color.
        /// @param color RGBA background color.
        void setBackGroundColor(const Common::Vector4f &color);

        /// @brief Returns the renderer background color.
        Common::Vector4f getBackGroundColor() const;

        /// @brief Sets the directional light material settings.
        /// @param settings New directional light settings.
        void setLightSettings(const DirectionalLightSettings &settings);
        /// @brief Returns the current directional light material settings.
        DirectionalLightSettings getLightSettings() const;

        /// @brief Updates renderer state for the elapsed time.
        /// @param deltaTime Time elapsed since the previous update.
        void update(DeltaTime deltaTime);

        /// @brief Returns the mutable render configuration.
        Common::UniquePtr<RenderConfigure> &getConfigure();

        /// @brief Sets the primitive selector used for picking.
        /// @param selector Selector implementation to use.
        void setSelector(Common::SharedPtr<Selector> selector);
        /// @brief Returns the current primitive selector.
        Common::SharedPtr<Selector> getSelector() const;

        /// @brief Enables or disables rendering of nodes.
        /// @param enabled Whether node rendering is enabled.
        void setRenderNodesEnabled(bool enabled);
        /// @brief Reports whether node rendering is enabled.
        bool isRenderNodesEnabled() const;
        /// @brief Enables or disables rendering of edges.
        /// @param enabled Whether edge rendering is enabled.
        void setRenderEdgesEnabled(bool enabled);
        /// @brief Reports whether edge rendering is enabled.
        bool isRenderEdgesEnabled() const;
        /// @brief Enables or disables rendering of faces.
        /// @param enabled Whether face rendering is enabled.
        void setRenderFacesEnabled(bool enabled);
        /// @brief Reports whether face rendering is enabled.
        bool isRenderFacesEnabled() const;

        /// @brief Creates a renderer with default settings.
        Renderer();

        /// @brief Releases renderer resources.
        ~Renderer();

    private:
        void loadCamera(const Common::SharedPtr<Shader> &shader,
                        const Common::SharedPtr<Camera> &camera,
                        const Common::SharedPtr<Scene> &scene) const;
        void loadLight(const Common::SharedPtr<Shader> &shader,
                       const Common::SharedPtr<Camera> &camera) const;
        bool isPrimitiveVisible(const Common::SharedPtr<RenderPrimitive> &primitive) const;

    private:
        Common::DynamicArray<RenderViewport> _viewports;

        Common::DynamicArray<Light *> _lights;

        Common::UniquePtr<RenderConfigure> _configure;
        Common::SharedPtr<Selector> _selector;

        Common::Vector4f _backGroundColor;
        mutable std::mutex _lightMutex;
        DirectionalLightSettings _lightSettings;

        mutable Common::SharedPtr<RenderPrimitive> _pickedPrimitive{nullptr};
        mutable IdxType _pickedNodeIndex{static_cast<IdxType>(-1)};
        bool _renderNodesEnabled{true};
        bool _renderEdgesEnabled{true};
        bool _renderFacesEnabled{true};
    };

} // namespace EgLab::RE