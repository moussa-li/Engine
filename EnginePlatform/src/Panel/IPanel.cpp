#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    IPanel::IPanel()
    {
    }

    IPanel::~IPanel()
    {
    }

    void IPanel::show()
    {
        _show = true;
        abortShow();
    }

    void IPanel::close()
    {
        _show = false;
        abortClose();
    }
} // namespace EgLab::Platform