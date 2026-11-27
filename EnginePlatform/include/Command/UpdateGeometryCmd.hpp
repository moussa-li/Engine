#pragma once

#include "Command/CommandParam.hpp"
#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// 接收外部建模进程送来的内核序列化 B-Rep 拓扑数据（OCCT BinTools/BRepTools）及对应
    /// 特征名，解析后创建或更新场景中的 GeometryData。
    ///
    /// 载荷布局见 Command/GeometryWire.hpp；目标节点按 dataNodeId 或 nodeName 解析，
    /// 找不到时以 nodeName 新建。
    class UpdateGeometryCmd : public CommandBase<UpdateGeometryParam>
    {
    public:
        explicit UpdateGeometryCmd(const UpdateGeometryParam& params);
        ~UpdateGeometryCmd();

        virtual Common::String getCmdName() const override
        {
            return Common::String("UpdateGeometryCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
