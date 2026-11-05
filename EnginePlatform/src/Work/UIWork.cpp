#include "Work/UIWork.hpp"

#include <GL/glew.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "Common/Log.hpp"
#include "Core/IniManager.hpp"
#include "Core/MainMenuBar.hpp"
#include "DataBase/Context.hpp"
#include "GLFW/glfw3.h"
#include "Panel/PanelManager.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace EgLab::Platform
{
    namespace
    {
        std::string encodeSettings(const std::string& settings)
        {
            constexpr char digits[] = "0123456789abcdef";
            std::string encoded;
            encoded.reserve(settings.size() * 2);
            for (const unsigned char byte : settings)
            {
                encoded.push_back(digits[byte >> 4]);
                encoded.push_back(digits[byte & 0x0f]);
            }
            return encoded;
        }

        std::string decodeSettings(const std::string& encoded)
        {
            if (encoded.size() % 2 != 0)
            {
                return {};
            }

            const auto nibble = [](char value) -> int {
                if (value >= '0' && value <= '9')
                {
                    return value - '0';
                }
                if (value >= 'a' && value <= 'f')
                {
                    return value - 'a' + 10;
                }
                if (value >= 'A' && value <= 'F')
                {
                    return value - 'A' + 10;
                }
                return -1;
            };

            std::string decoded;
            decoded.reserve(encoded.size() / 2);
            for (size_t index = 0; index < encoded.size(); index += 2)
            {
                const int high = nibble(encoded[index]);
                const int low = nibble(encoded[index + 1]);
                if (high < 0 || low < 0)
                {
                    return {};
                }
                decoded.push_back(static_cast<char>((high << 4) | low));
            }
            return decoded;
        }

        void saveImGuiSettings()
        {
            size_t settingsSize = 0;
            const char* settings = ImGui::SaveIniSettingsToMemory(&settingsSize);
            IniManager::instance().setValue(
                "ui.imguiSettingsHex",
                Common::String(encodeSettings(std::string(settings, settingsSize)).c_str()));
            ImGui::GetIO().WantSaveIniSettings = false;
        }
    } // namespace

    UIWork::UIWork()
    {
    }

    UIWork::~UIWork()
    {
        stop();
    }

    void UIWork::bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window)
    {
        _window = window;
    }

    void UIWork::start()
    {
        if (_running.load())
        {
            return;
        }

        _running = true;

        initImGui();
    }

    void UIWork::stop()
    {
        if (_running == false) return;
        _running = false;
        activeContext();
        shutdownImGui();
        deactiveContext();
    }

    void UIWork::subscribe(EventId eventId)
    {
        (void)eventId;
    }

    void UIWork::onUIEvent(const EventPacket& packet)
    {
        (void)packet;
        LOG(INFO) << "UIWork::onUIEvent() receives UI packet and routes to panel/imGui";
    }

    void UIWork::renderFrame()
    {
        if (_window)
        {
            activeContext();
            beginImGuiFrame();
            Context::instance().getPanelManager().loop();
            _window->maskEvent(ImGui::GetIO().WantCaptureMouse);
            renderImGuiFrame();
            deactiveContext();
        }
    }

    void UIWork::activeContext() const
    {
        if (_window != nullptr)
        {
            _window->activeContext();
        }
    }

    void UIWork::deactiveContext() const
    {
        if (_window != nullptr)
        {
            _window->deactiveContext();
        }
    }

    void UIWork::initImGui()
    {
        activeContext();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;

        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(8, 5);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 6);

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        auto& ini = IniManager::instance();
        const auto storedSettings = ini.getValue("ui.imguiSettingsHex");
        if (storedSettings != "")
        {
            const std::string decoded = decodeSettings(storedSettings.c_str());
            if (!decoded.empty())
            {
                ImGui::LoadIniSettingsFromMemory(decoded.data(), decoded.size());
            }
        }
        else
        {
            std::ifstream legacyFile("imgui.ini", std::ios::binary);
            if (legacyFile)
            {
                const std::string legacySettings((std::istreambuf_iterator<char>(legacyFile)),
                                                 std::istreambuf_iterator<char>());
                ImGui::LoadIniSettingsFromMemory(legacySettings.data(), legacySettings.size());
                saveImGuiSettings();
                if (ini.flush())
                {
                    std::remove("imgui.ini");
                }
            }
        }

        ImFont* mainFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 16.0f, nullptr,
                                                        io.Fonts->GetGlyphRangesChineseFull());
        IM_ASSERT(mainFont != nullptr);

        ImGui_ImplGlfw_InitForOpenGL(_window->getNative(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
        deactiveContext();
    }

    void UIWork::beginImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void UIWork::renderImGuiFrame()
    {
        renderRibbonToolbar();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().WantSaveIniSettings)
        {
            saveImGuiSettings();
        }
    }

    void UIWork::shutdownImGui()
    {
        if (ImGui::GetIO().WantSaveIniSettings)
        {
            saveImGuiSettings();
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace EgLab::Platform
