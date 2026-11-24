#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"

#include "Common/Log.hpp"

#include "Null/NullGeometryKernelFactory.hpp"

#ifdef GEOMETRY_KERNEL_OCCT
#include "Occt/OcctGeometryKernelFactory.hpp"
#endif

namespace EgLab::GE
{
    class GeometryKernelRegistry::Impl
    {
    public:
        struct Entry
        {
            Common::String name;
            Common::UniquePtr<IGeometryKernelFactory> factory;
        };

        const IGeometryKernelFactory* find(const Common::String& name) const
        {
            for (std::size_t i = 0; i < factories.size(); ++i)
            {
                if (factories[i].name == name) return factories[i].factory.get();
            }
            return nullptr;
        }

        Common::DynamicArray<Entry> factories;
    };

    GeometryKernelRegistry::GeometryKernelRegistry() : _impl(Common::makeUnique<Impl>())
    {
        registerFactory(Common::UniquePtr<IGeometryKernelFactory>(new NullGeometryKernelFactory()));
#ifdef GEOMETRY_KERNEL_OCCT
        registerFactory(Common::UniquePtr<IGeometryKernelFactory>(new OcctGeometryKernelFactory()));
#endif
    }

    GeometryKernelRegistry::~GeometryKernelRegistry() = default;

    bool GeometryKernelRegistry::registerFactory(Common::UniquePtr<IGeometryKernelFactory> factory)
    {
        if (!factory) return false;

        Common::String name = factory->getName();
        if (name.size() == 0 || _impl->find(name) != nullptr) return false;

        _impl->factories.pushBack(Impl::Entry{Common::move(name), Common::move(factory)});
        return true;
    }

    const IGeometryKernelFactory* GeometryKernelRegistry::findFactory(const Common::String& name) const
    {
        return _impl->find(name);
    }

    Common::UniquePtr<IGeometryImporter> GeometryKernelRegistry::createImporter(
        const Common::String& name, const GeometryImportOptions& options) const
    {
        const IGeometryKernelFactory* factory = _impl->find(name);
        if (factory == nullptr)
        {
            if (name.size() > 0)
            {
                LOG(WARNING) << "Geometry kernel is not registered, falling back: " << name;
            }

            factory = _impl->find(getDefaultKernelName());
        }
        if (factory == nullptr)
        {
            factory = _impl->find(Common::String(NULL_GEOMETRY_KERNEL));
        }
        if (factory == nullptr) return Common::UniquePtr<IGeometryImporter>();

        return factory->createImporter(options);
    }

    Common::DynamicArray<Common::String> GeometryKernelRegistry::getKernelNames() const
    {
        Common::DynamicArray<Common::String> names;
        for (std::size_t i = 0; i < _impl->factories.size(); ++i)
        {
            names.pushBack(_impl->factories[i].name);
        }
        return names;
    }

    Common::String GeometryKernelRegistry::getDefaultKernelName()
    {
#ifdef GEOMETRY_KERNEL_OCCT
        return Common::String(DEFAULT_GEOMETRY_KERNEL);
#else
        return Common::String(NULL_GEOMETRY_KERNEL);
#endif
    }

} // namespace EgLab::GE
