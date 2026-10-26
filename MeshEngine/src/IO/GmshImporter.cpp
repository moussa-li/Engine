#include "MeshEngine/IO/GmshImporter.hpp"

#include <cstdio>
#include <fstream>
#include <string>

#include "Common/Log.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    constexpr size_t mapNumber = 18;

    class GmshImporter::Impl
    {
    public:
        GmshImporter* _importer;
        ElemType _typeMap[mapNumber] = {
            ElemType::None,  ElemType::None,  ElemType::Tri3,  ElemType::Quad4,
            ElemType::Tet4,  ElemType::Hex8,  ElemType::Wed6,  ElemType::Prd5,
            ElemType::None, // Line3
            ElemType::Tri6,
            ElemType::None, // Quad9
            ElemType::Tet10,
            ElemType::None, // Hex27
            ElemType::None, // Wed15
            ElemType::None, // Prd13
            ElemType::None,  ElemType::Quad8, ElemType::Hex20,
        };

        Common::SharedPtr<Mesh> mesh[4] = {Common::makeShared<Mesh>(), Common::makeShared<Mesh>(),
                                           Common::makeShared<Mesh>(), Common::makeShared<Mesh>()};

        void setup()
        {
            auto filePath = _importer->getFileDir();
            // Common::SharedPtr<Mesh> mesh = Common::makeShared<Mesh>();

            std::ifstream file(filePath.c_str());
            if (!file.is_open())
            {
                LOG(ERROR) << "Failed to open file: " << filePath;
                return;
            }

            std::string line;
            enum Section
            {
                NONE,
                HEAD,
                NODES,
                ELEMENTS
            };
            Section section = NONE;
            int numNodes = 0, numElems = 0;

            while (std::getline(file, line))
            {
                if (line == "$MeshFormat")
                {
                    section = HEAD;
                    do
                    {
                        std::getline(file, line);
                    } while (line.empty());

                    double version;
                    int isBinary;
                    int fSize;
                    if (std::sscanf(line.c_str(), "%lf %d %d", &version, &isBinary, &fSize) != 3)
                    {
                        LOG(ERROR) << "Node line parse error: " << line;
                        return;
                    }

                    if (version * 10 > 22)
                    {
                        LOG(ERROR) << "Error: file version > \"2.2\" .not support yet!";
                        return;
                    }
                    if (isBinary)
                    {
                        LOG(ERROR) << "Error: file format is binar. not support yet!";
                        return;
                    }
                    while (std::getline(file, line))
                    {
                        if (line == "$EndMeshFormat") break;
                    }
                    section = NONE;
                }
                else if (line == "$Nodes")
                {
                    section = NODES;
                    do
                    {
                        std::getline(file, line);
                    } while (line.empty());
                    numNodes = std::stoi(line);
                    // this->nodes.reserve(numNodes);
                    for (int i = 0; i < numNodes; ++i)
                    {
                        do
                        {
                            std::getline(file, line);
                        } while (line.empty());
                        // 解析节点行
                        IdType id;
                        Scalar x, y, z;

                        if (std::sscanf(line.c_str(), "%d %lf %lf %lf", &id, &x, &y, &z) != 4)
                        {
                            LOG(ERROR) << "Node line parse error: " << line;
                            return;
                        }

                        for (int i = 0; i < 4; i++)
                        {
                            Node node;

                            node.setId(id);
                            node.setX(x);
                            node.setY(y);
                            node.setZ(z);
                            mesh[i]->addNode(Common::move(node));
                        }
                    }

                    while (std::getline(file, line))
                    {
                        if (line == "$EndNodes") break;
                    }
                    section = NONE;
                }
                else if (line == "$Elements")
                {
                    section = ELEMENTS;
                    do
                    {
                        std::getline(file, line);
                    } while (line.empty());
                    numElems = std::stoi(line);
                    for (int i = 0; i < numElems; ++i)
                    {
                        do
                        {
                            std::getline(file, line);
                        } while (line.empty());
                        Elem elem;
                        // 先解析id、type、numTags
                        int gmshType = 0, numTags = 0;
                        const char* str = line.c_str();
                        // int n = 0;
                        int offset = 0;
                        IdType id;
                        if (std::sscanf(str, "%d %d %d%n", &id, &gmshType, &numTags, &offset) < 3)
                        {
                            LOG(ERROR) << "Element head parse error: " << line;
                            return;
                        }
                        elem.setId(id);
                        str += offset;
                        // 跳过所有tag
                        for (int t = 0; t < numTags; ++t)
                        {
                            int tag = 0;
                            int tagOffset = 0;
                            if (std::sscanf(str, "%d%n", &tag, &tagOffset) != 1)
                            {
                                LOG(ERROR) << "Element tag parse error: " << line;
                                return;
                            }
                            str += tagOffset;
                        }

                        // 解析节点数
                        if (gmshType >= mapNumber)
                        {
                            LOG(ERROR) << "Element type out of size!";
                            return;
                        }
                        if (gmshType == 1 || gmshType == 15) continue;
                        ElemType type = _typeMap[gmshType];

                        if (type == ElemType::None)
                        {
                            LOG(ERROR) << "Element type unknown : id " << id
                                       << ", type : " << (int)gmshType;
                            continue;
                        }
                        int npe = getNodeNumber(type);
                        int dim = getTypeDim(type);
                        elem.setType(type);
                        for (int n = 0; n < npe; ++n)
                        {
                            int nodeId = 0, nodeOffset = 0;
                            if (std::sscanf(str, "%d%n", &nodeId, &nodeOffset) != 1)
                            {
                                LOG(ERROR) << "Element node parse error: " << line;
                                return;
                            }
                            elem.setNode(n, nodeId);
                            str += nodeOffset;
                        }

                        mesh[dim]->addElem(Common::move(elem));
                    }
                    // 跳到$EndElements

                    while (std::getline(file, line))
                    {
                        if (line == "$EndElements") break;
                    }
                    section = NONE;
                }
            }
            file.close();
        }

        Impl(GmshImporter* importer) : _importer(importer)
        {
        }
    };

    GmshImporter::GmshImporter(const Common::String& fileDir)
        : Importer(fileDir), _impl(Common::makeUnique<Impl>(this))
    {
        _impl->setup();
    }

    GmshImporter::~GmshImporter()
    {
    }

    Common::SharedPtr<Mesh> GmshImporter::getMesh(int dim)
    {
        switch (dim)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            return _impl->mesh[dim];
        default:
            LOG(ERROR) << "Invalid mesh dimension: " << dim;
            return Common::SharedPtr<Mesh>();
        }
    }

} // namespace EgLab::ME