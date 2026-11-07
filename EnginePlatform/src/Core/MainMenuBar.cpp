#include "Core/MainMenuBar.hpp"

#include <filesystem>

#include "Common/HashMap.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include "Command/CommandManager.hpp"
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
    LSTATUS writeCurrentProcessIdToRegistry()
    {
        HKEY key = nullptr;
        const LSTATUS openStatus =
            RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\EngineLab", 0, nullptr,
                            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &key, nullptr);
        if (openStatus != ERROR_SUCCESS)
        {
            return openStatus;
        }

        const DWORD processId = GetCurrentProcessId();
        const LSTATUS writeStatus =
            RegSetValueExW(key, L"CURRENT_ENGINELAB_PROCESSID", 0, REG_DWORD,
                           reinterpret_cast<const BYTE*>(&processId), sizeof(processId));
        const LSTATUS closeStatus = RegCloseKey(key);
        return writeStatus == ERROR_SUCCESS ? closeStatus : writeStatus;
    }
} // namespace
#endif

namespace
{
    // Per-menu animation state, keyed by ImGui control ID.
    struct MenuAnimation
    {
        bool wasOpen = false;        // Whether the menu was open on the previous frame
        bool closing = false;        // Whether the close animation is playing
        float progress = 1.0f;       // 0 (animation start) .. 1 (animation done)
        float duration = 0.5f;       // Opening animation duration in seconds
        float closeDuration = 0.15f; // Closing animation duration in seconds
        float slideDistance =
            -32.0f; // Vertical offset from the final position; negative slides in from above
        float hoverOutTimer = 0.0f; // Time since the mouse left both the menu item and its popup
        float closeDelay = 0.15f;   // Grace period before closing on mouse leave
    };

    MenuAnimation& getMenuAnimation(ImGuiID id)
    {
        static EgLab::Common::HashMap<ImGuiID, MenuAnimation> states;
        return states[id];
    }

    // ImGui::BeginMenu wrapper with a slide-down animation.
    bool animatedBeginMenu(const char* label, bool enabled = true)
    {
        const ImGuiID id = ImGui::GetID(label);
        MenuAnimation& anim = getMenuAnimation(id);

        // Suppress the mouse click while submitting the menu item so that clicking an
        // already-open menu does not close it. Hovering is used to open/switch menus.
        ImGuiIO& io = ImGui::GetIO();
        const bool savedMouseClicked = io.MouseClicked[0];
        io.MouseClicked[0] = false;

        bool open = ImGui::BeginMenu(label, enabled);

        io.MouseClicked[0] = savedMouseClicked;

        // Open the menu automatically on hover, without a click.
        if (!open && enabled && ImGui::IsItemHovered())
        {
            ImGui::OpenPopup(id);
        }

        // Restart the animation when the menu opens for the first time.
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
                // Start the close animation once the mouse leaves both the menu item and the popup.
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
                // Cancel closing and reopen if the mouse comes back.
                anim.closing = false;
                anim.hoverOutTimer = 0.0f;
            }

            // Advance the animation progress: forward while opening, backward while closing.
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

            // Ease-out and slide the menu in from above to its final position.
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
            // Reset after closing so the animation replays on the next open.
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
        const bool openRequested =
            ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal);
        if (openRequested)
        {
            requestShowPanel("MeshImport");
        }

#ifdef _WIN32
        static bool registryWriteAttempted = false;
        static LSTATUS registryWriteStatus = ERROR_SUCCESS;
#endif

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
            if (ImGui::Button("Write PID to Registry"))
            {
                registryWriteStatus = writeCurrentProcessIdToRegistry();
                registryWriteAttempted = true;
            }
            if (registryWriteAttempted)
            {
                ImGui::SameLine();
                if (registryWriteStatus == ERROR_SUCCESS)
                {
                    ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f),
                                       "PID saved to HKCU\\Software\\EngineLab");
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                                       "Registry write failed (error %ld)",
                                       static_cast<long>(registryWriteStatus));
                }
            }
#endif

            ImGui::EndMainMenuBar();
        }
    }

} // namespace EgLab::Platform