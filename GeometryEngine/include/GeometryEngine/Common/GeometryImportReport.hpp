#pragma once

#include "Common/Return.hpp"
#include "Common/String.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    /// Result of one kernel import, including per-type feature counts and diagnostics.
    class GeometryEngineAPI GeometryImportReport
    {
    public:
        GeometryImportReport();

        Common::Return getStatus() const;

        void setStatus(Common::Return status);

        bool isSucceed() const;

        const Common::String& getMessage() const;

        void setMessage(const Common::String& message);

        const Common::String& getKernelName() const;

        void setKernelName(const Common::String& kernelName);

        const Common::String& getFilePath() const;

        void setFilePath(const Common::String& filePath);

        std::size_t getFeatureCount(TopologyType type) const;

        void setFeatureCount(TopologyType type, std::size_t count);

        std::size_t getTriangleCount() const;

        void setTriangleCount(std::size_t count);

        void reset();

    private:
        std::size_t& countSlot(TopologyType type);

        const std::size_t& countSlot(TopologyType type) const;

        Common::Return _status{Common::Return::Succeed};
        Common::String _message;
        Common::String _kernelName;
        Common::String _filePath;
        std::size_t _featureCounts[TOPOLOGY_TYPE_COUNT]{};
        std::size_t _triangleCount{0};
    };

} // namespace EgLab::GE
