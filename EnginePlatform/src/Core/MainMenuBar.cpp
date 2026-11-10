#include "Core/MainMenuBar.hpp"

#include <filesystem>

#include "Command/CommandCatalog.hpp"
#include "Command/CommandManager.hpp"
#include "Command/ShortcutManager.hpp"
#include "Common/HashMap.hpp"
#include "Core/PidRegistry.hpp"
#include "DataBase/Context.hpp"
#include "ImGuiFileDialog.h"

namespace EgLab
{
    inline Common::String getTestDataDir(const Common::String& file)
    {
        std::filesystem::path root(__FILE__);
        root = root.parent_path().parent_path().parent_path();
        root /= "test/testdata";
        root /= file.c_str();
        return Common::String(root.generic_string().c_str());
    }
} // namespace EgLab

#ifdef _WIN32
namespace
{
    // 注册表 PID 状态展示：当前进程就是注册表中记录的那个进程时进行高亮。
    void renderProcessIdStatus()
    {
        EgLab::Platform::PidRegistry& registry = EgLab::Platform::PidRegistry::instance();
        const bool highlighted = registry.isCurrentProcessRegistered();

        if (highlighted)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.52f, 0.26f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.62f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.42f, 0.21f, 1.0f));
        }

        if (ImGui::Button("Write PID to Registry"))
        {
            registry.writeCurrentProcessId();
        }

        if (highlighted)
        {
            ImGui::PopStyleColor(3);
        }

        ImGui::SameLine();

        const ImVec4 textColor =
            highlighted ? ImVec4(0.35f, 0.9f, 0.45f, 1.0f) : ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        const unsigned long registeredPid = registry.registeredProcessId();
        const unsigned long currentPid = registry.currentProcessId();

        if (registeredPid == EgLab::Platform::PidRegistry::emptyProcessId)
        {
            ImGui::TextColored(textColor, "Registry PID: <none>  (this: %lu)", currentPid);
        }
        else if (highlighted)
        {
            ImGui::TextColored(textColor, "Registry PID: %lu  (this process)", registeredPid);
        }
        else
        {
            ImGui::TextColored(textColor, "Registry PID: %lu  (this: %lu)", registeredPid,
                               currentPid);
        }
    }
} // namespace
#endif

namespace
{
    // 每个菜单的动画状态，以 ImGui 控件 ID 作为键。
    struct MenuAnimation
    {
        bool wasOpen = false;         // 上一帧菜单是否处于展开状态
        bool closing = false;         // 是否正在播放关闭动画
        float progress = 1.0f;        // 0（动画开始）.. 1（动画结束）
        float duration = 0.5f;        // 展开动画时长（秒）
        float closeDuration = 0.15f;  // 关闭动画时长（秒）
        float slideDistance = -32.0f; // 相对最终位置的纵向偏移，负值表示从上方滑入
        float hoverOutTimer = 0.0f;   // 鼠标离开菜单项及其弹出框后经过的时间
        float closeDelay = 0.15f;     // 鼠标离开后延迟关闭的宽限时间
    };

    MenuAnimation& getMenuAnimation(ImGuiID id)
    {
        static EgLab::Common::HashMap<ImGuiID, MenuAnimation> states;
        return states[id];
    }

    // 带滑入动画的 ImGui::BeginMenu 包装。
    bool animatedBeginMenu(const char* label, bool enabled = true)
    {
        const ImGuiID id = ImGui::GetID(label);
        MenuAnimation& anim = getMenuAnimation(id);

        // 提交菜单项期间临时屏蔽鼠标左键点击，避免点击已展开的菜单时把它关掉；
        // 菜单的展开与切换统一由悬停驱动。
        ImGuiIO& io = ImGui::GetIO();
        const bool savedMouseClicked = io.MouseClicked[0];
        io.MouseClicked[0] = false;

        bool open = ImGui::BeginMenu(label, enabled);

        io.MouseClicked[0] = savedMouseClicked;

        // 无需点击，鼠标悬停即自动展开菜单。
        if (!open && enabled && ImGui::IsItemHovered())
        {
            ImGui::OpenPopup(id);
        }

        // 菜单首次展开时重新开始动画。
        if (open && !anim.wasOpen)
        {
            anim.progress = 0.0f;
            anim.closing = false;
        }
        anim.wasOpen = open;

        if (open)
        {
            const bool popupHovered = ImGui::IsWindowHovered();
            const bool itemHovered = ImGui::IsItemHovered();
            const bool hovered = popupHovered || itemHovered;

            if (!anim.closing)
            {
                // 鼠标同时离开菜单项与弹出框后，开始播放关闭动画。
                if (!hovered)
                {
                    anim.hoverOutTimer += io.DeltaTime;
                    if (anim.hoverOutTimer >= anim.closeDelay)
                    {
                        anim.closing = true;
                    }
                }
                else
                {
                    anim.hoverOutTimer = 0.0f;
                }
            }
            else if (hovered)
            {
                // 鼠标重新回到菜单上时取消关闭并重新展开。
                anim.closing = false;
                anim.hoverOutTimer = 0.0f;
            }

            // 推进动画进度：展开时向前，关闭时向后。
            if (anim.closing)
            {
                anim.progress -= io.DeltaTime / anim.closeDuration;
                if (anim.progress <= 0.0f)
                {
                    anim.progress = 0.0f;
                    ImGui::CloseCurrentPopup();
                }
            }
            else if (anim.progress < 1.0f)
            {
                anim.progress += io.DeltaTime / anim.duration;
                if (anim.progress > 1.0f)
                {
                    anim.progress = 1.0f;
                }
            }

            // 缓出并从上方滑入到最终位置。
            const float eased = 1.0f - (1.0f - anim.progress) * (1.0f - anim.progress);
            const float offsetY = (1.0f - eased) * anim.slideDistance;
            if (offsetY != 0.0f)
            {
                const ImVec2 pos = ImGui::GetWindowPos();
                ImGui::SetWindowPos(ImVec2(pos.x, pos.y + offsetY));
            }
        }
        else
        {
            // 关闭后复位，使下次展开时动画可以重播。
            anim.progress = 1.0f;
            anim.closing = false;
            anim.hoverOutTimer = 0.0f;
        }

        return open;
    }
} // namespace

namespace EgLab::Platform
{
    void requestShowPanel(const Common::String& panelName)
    {
        EventPacket packet;
        packet.setData("panelName", panelName);
        CommandManager::instance().call(EventId::ShowPanel, Common::move(packet));
    }

    void renderRibbonToolbar()
    {
        // Shortcut system: every binding is dispatched through CommandManager.
        // EventId-only commands run immediately, parameterised ones open the wizard panel.
        ShortcutManager::instance().processInput();

        const bool openRequested =
            ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal);
        if (openRequested)
        {
            requestShowPanel("MeshImport");
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (animatedBeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                { /* ... */
                    requestShowPanel("MeshImport");
                }

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                { /* ... */
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                { /* ... */
                }
                ImGui::EndMenu();
            }
            if (animatedBeginMenu("View"))
            {
                if (ImGui::MenuItem("camera setting", "f5"))
                {
                    requestShowPanel("CameraSetting");
                }

                if (ImGui::MenuItem("background setting", "f5"))
                {
                    requestShowPanel("BackGroundSetting");
                }
                if (ImGui::MenuItem("Light Settings"))
                {
                    requestShowPanel("LightSetting");
                }
                if (ImGui::MenuItem("Data Tree"))
                {
                    requestShowPanel("DataTree");
                }
                if (ImGui::MenuItem("Message Log"))
                {
                    requestShowPanel("MessageLog");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Shortcut Settings"))
                {
                    requestShowPanel(kShortcutSettingPanelName);
                }

                ImGui::EndMenu();
            }
            if (animatedBeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Half Edge Mesh"))
                {
                    requestShowPanel("HEMesh");
                }
                if (ImGui::MenuItem("Mesh Render Settings"))
                {
                    requestShowPanel("MeshRenderSettings");
                }
                ImGui::EndMenu();
            }

#ifdef _WIN32
            ImGui::SameLine();
            renderProcessIdStatus();
#endif

            ImGui::EndMainMenuBar();
        }
    }

} // namespace EgLab::Platform