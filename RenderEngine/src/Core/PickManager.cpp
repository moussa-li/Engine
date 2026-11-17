#include "RenderEngine/Core/PickManager.hpp"

#include "Common/Utils.hpp"

namespace EgLab::RE
{
    PickManager::PickToken PickManager::acquire(PickKind kind, const Common::String &owner)
    {
        if (kind == PickKind::None)
        {
            return kInvalidToken;
        }

        PickRequest request;
        request.token = _nextToken++;
        request.kind = kind;
        request.owner = owner;
        _requests.pushBack(Common::move(request));
        return _requests[_requests.size() - 1].token;
    }

    void PickManager::release(PickToken token)
    {
        if (token == kInvalidToken)
        {
            return;
        }

        Common::DynamicArray<PickRequest> remaining;
        remaining.reserve(_requests.size());
        for (size_t index = 0; index < _requests.size(); ++index)
        {
            if (_requests[index].token != token)
            {
                remaining.pushBack(Common::move(_requests[index]));
            }
        }
        _requests = Common::move(remaining);
    }

    void PickManager::releaseOwner(const Common::String &owner)
    {
        Common::DynamicArray<PickRequest> remaining;
        remaining.reserve(_requests.size());
        for (size_t index = 0; index < _requests.size(); ++index)
        {
            if (!(_requests[index].owner == owner))
            {
                remaining.pushBack(Common::move(_requests[index]));
            }
        }
        _requests = Common::move(remaining);
    }

    uint32_t PickManager::activeKinds() const
    {
        uint32_t mask = 0;
        for (size_t index = 0; index < _requests.size(); ++index)
        {
            mask |= toPickMask(_requests[index].kind);
        }
        return mask;
    }

    bool PickManager::isActive(PickKind kind) const
    {
        return isPickKindEnabled(activeKinds(), kind);
    }
} // namespace EgLab::RE
