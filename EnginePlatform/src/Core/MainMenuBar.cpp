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
        bool wasOpen = false;   // Whether the menu was open on the previous frame
        float progress = 1.0f;  // 0 (animation start) .. 1 (animation done)
        float duration = 0.18f; // Animation duration in seconds
        float slideDistance =
            -32.0f; // Vertical offset from the final position; negative slides in from above
    };

    MenuAnimation& getMenuAnimation(ImGuiID id)
    {
        static EgLab::Common::HashMap<ImGuiID, MenuAnimation> states;
        return states[id];
    }

    // ImGui::BeginMenu wrapper with a slide-down animation.
    bool animatedBeginMenu(const char* label, bool enabled = true)
    {
        MenuAnimation& anim = getMenuAnimation(ImGui::GetID(label));

        const bool open = ImGui::BeginMenu(label, enabled);

        // Open the menu automatically on hover, without a click.
        if (!open && enabled && ImGui::IsItemHovered())
        {
            ImGui::OpenPopup(label);
        }

        // Restart the animation when the menu opens for the first time.
        if (open && !anim.wasOpen)
        {
            anim.progress = 0.0f;
        }
        anim.wasOpen = open;

        if (open)
        {
            // Advance the animation progress.
            if (anim.progress < 1.0f)
            {
                const float delta = ImGui::GetIO().DeltaTime;
                anim.progress += delta / anim.duration;
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