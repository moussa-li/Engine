#include "GeometryEngine/Common/GeometryImportReport.hpp"

namespace EgLab::GE
{
    GeometryImportReport::GeometryImportReport() = default;

    Common::Return GeometryImportReport::getStatus() const
    {
        return _status;
    }

    void GeometryImportReport::setStatus(Common::Return status)
    {
        _status = status;
    }

    bool GeometryImportReport::isSucceed() const
    {
        return _status == Common::Return::Succeed;
    }

    const Common::String& GeometryImportReport::getMessage() const
    {
        return _message;
    }

    void GeometryImportReport::setMessage(const Common::String& message)
    {
        _message = message;
    }

    const Common::String& GeometryImportReport::getKernelName() const
    {
        return _kernelName;
    }

    void GeometryImportReport::setKernelName(const Common::String& kernelName)
    {
        _kernelName = kernelName;
    }

    const Common::String& GeometryImportReport::getFilePath() const
    {
        return _filePath;
    }

    void GeometryImportReport::setFilePath(const Common::String& filePath)
    {
        _filePath = filePath;
    }

    std::size_t GeometryImportReport::getFeatureCount(TopologyType type) const
    {
        return countSlot(type);
    }

    void GeometryImportReport::setFeatureCount(TopologyType type, std::size_t count)
    {
        countSlot(type) = count;
    }

    std::size_t GeometryImportReport::getTriangleCount() const
    {
        return _triangleCount;
    }

    void GeometryImportReport::setTriangleCount(std::size_t count)
    {
        _triangleCount = count;
    }

    void GeometryImportReport::reset()
    {
        _status = Common::Return::Succeed;
        _message = Common::String();
        _kernelName = Common::String();
        _filePath = Common::String();
        for (std::size_t i = 0; i < TOPOLOGY_TYPE_COUNT; ++i)
        {
            _featureCounts[i] = 0;
        }
        _triangleCount = 0;
    }

    std::size_t& GeometryImportReport::countSlot(TopologyType type)
    {
        const auto index = static_cast<std::size_t>(type);
        return _featureCounts[index < TOPOLOGY_TYPE_COUNT ? index : 0];
    }

    const std::size_t& GeometryImportReport::countSlot(TopologyType type) const
    {
        const auto index = static_cast<std::size_t>(type);
        return _featureCounts[index < TOPOLOGY_TYPE_COUNT ? index : 0];
    }

} // namespace EgLab::GE
