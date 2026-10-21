#include "MeshEngine/IO/GrdImporter.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "Common/Log.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    namespace
    {
        std::string trim(const std::string &value)
        {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return {};
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }

        bool readDataLine(std::ifstream &file, std::string &line)
        {
            while (std::getline(file, line))
            {
                line = trim(line);
                if (line.empty() || line[0] == '!' || line[0] == '#') continue;
                return true;
            }
            return false;
        }

        bool parseCount(const std::string &line, size_t &count)
        {
            std::istringstream stream(line);
            return static_cast<bool>(stream >> count);
        }
    } // namespace

    GrdImporter::GrdImporter(const Common::String &fileDir) : Importer(fileDir)
    {
    }

    GrdImporter::~GrdImporter() = default;

    Common::SharedPtr<Mesh> GrdImporter::getMesh()
    {
        auto mesh = Common::makeShared<Mesh>();
        std::ifstream file(getFileDir().c_str());
        if (!file.is_open())
        {
            LOG(ERROR) << "Failed to open GRD file: " << getFileDir();
            return mesh;
        }

        std::vector<Node> nodes;
        std::vector<Elem> elements;
        std::unordered_set<IdType> nodeIds;
        std::unordered_set<IdType> elementIds;
        std::string line;
        bool foundNodes = false;

        while (readDataLine(file, line))
        {
            if (line == "*NODES")
            {
                foundNodes = true;
                size_t count = 0;
                if (!readDataLine(file, line) || !parseCount(line, count))
                {
                    LOG(ERROR) << "Invalid GRD node count";
                    return mesh;
                }

                nodes.reserve(count);
                for (size_t i = 0; i < count; ++i)
                {
                    if (!readDataLine(file, line) || line[0] == '*')
                    {
                        LOG(ERROR) << "Unexpected end of GRD node section";
                        return mesh;
                    }

                    Node node;
                    IdType id = 0;
                    Scalar x = 0, y = 0, z = 0;
                    std::istringstream stream(line);
                    if (!(stream >> id >> x >> y >> z) || !nodeIds.insert(id).second)
                    {
                        LOG(ERROR) << "Invalid or duplicate GRD node: " << line;
                        return mesh;
                    }
                    node.setId(id);
                    node.setX(x);
                    node.setY(y);
                    node.setZ(z);
                    nodes.push_back(Common::move(node));
                }
            }
            else if (line == "*ELEMENTS")
            {
                size_t count = 0;
                if (!readDataLine(file, line) || !parseCount(line, count))
                {
                    LOG(ERROR) << "Invalid GRD element count";
                    return mesh;
                }

                elements.reserve(count);
                for (size_t i = 0; i < count; ++i)
                {
                    if (!readDataLine(file, line) || line[0] == '*')
                    {
                        LOG(ERROR) << "Unexpected end of GRD element section";
                        return mesh;
                    }

                    IdType id = 0, materialId = 0;
                    IdType nodeIdsForElement[4]{};
                    std::istringstream stream(line);
                    if (!(stream >> id >> materialId >> nodeIdsForElement[0] >>
                          nodeIdsForElement[1] >> nodeIdsForElement[2] >> nodeIdsForElement[3]) ||
                        !elementIds.insert(id).second)
                    {
                        LOG(ERROR) << "Invalid or duplicate GRD element: " << line;
                        return mesh;
                    }

                    Elem element;
                    element.setId(id);
                    element.setType(ElemType::Tet4);
                    for (IdxType nodeIndex = 0; nodeIndex < 4; ++nodeIndex)
                    {
                        element.setNode(nodeIndex, nodeIdsForElement[nodeIndex]);
                    }
                    elements.push_back(Common::move(element));
                }
            }
        }

        if (!foundNodes)
        {
            LOG(ERROR) << "GRD file has no *NODES section";
            return mesh;
        }

        for (auto &element : elements)
        {
            for (IdxType nodeIndex = 0; nodeIndex < 4; ++nodeIndex)
            {
                if (nodeIds.find(element.getNode(nodeIndex)) == nodeIds.end())
                {
                    LOG(ERROR) << "GRD element references a missing node: " << element.getId();
                    return mesh;
                }
            }
        }

        for (auto &node : nodes)
        {
            mesh->addNode(Common::move(node));
        }
        for (auto &element : elements)
        {
            mesh->addElem(Common::move(element));
        }
        return mesh;
    }

} // namespace EgLab::ME