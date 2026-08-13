#include "UI/gutils.h"

#include "RCP_Host/RCP_Host.h"
#include "imgui_internal.h"
#include "stb_image.h"

#include "EmbeddedResource.h"
#include "UI/vscode_icons.h"
#include "VERSION.h"

namespace {
    using namespace LRI::RCI;
    GLFWwindow* shared = nullptr;

    GLuint birdTex = 0;

    ImFontAtlas* sharedFonts = nullptr;
    ImFont* normal = nullptr;
    ImFont* bold = nullptr;
    ImFont* italic = nullptr;

    std::string VERSION_STRING;
    ImVec2 VERSION_SIZE;
} // namespace

namespace LRI::RCI::style {
    float scaling_factor = 1;

    void setup() {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        shared = glfwCreateWindow(10, 10, "RCI Shared", nullptr, nullptr);
        glfwMakeContextCurrent(shared);

        // Set up shared bird icon
        {
            EmbeddedResource im("LRI_Logo_big.png");
            int imw, imh;
            unsigned char* imaged = stbi_load_from_memory((unsigned char*) im.getData(),
                                                          static_cast<int>(im.getLength()), &imw, &imh, nullptr, 4);

            glGenTextures(1, &birdTex);
            glBindTexture(GL_TEXTURE_2D, birdTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imw, imh, 0, GL_RGBA, GL_UNSIGNED_BYTE, imaged);
            stbi_image_free(imaged);
        }

        sharedFonts = IM_NEW(ImFontAtlas)();

        {
            constexpr float fontSize = 16;
            ImFontConfig fontConfig;
            fontConfig.FontDataOwnedByAtlas = false;

            ImFontConfig iconConfig;
            iconConfig.FontDataOwnedByAtlas = false;
            iconConfig.PixelSnapH = true;
            iconConfig.GlyphMinAdvanceX = fontSize;
            iconConfig.MergeMode = true;
            iconConfig.GlyphOffset = {0, 3};

            // Load the fonts and add them to imgui. Ubuntu mono my beloved
            EmbeddedResource fonts("font_regular.ttf");
            normal = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()),
                                                       fontSize, &fontConfig);

            fonts = EmbeddedResource("codicon.ttf");
            constexpr ImWchar VSC_CODEPOINTS[] = {ICON_MIN_VS, ICON_MAX_VS, 0};
            sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()), fontSize,
                                              &iconConfig, VSC_CODEPOINTS);

            fonts = EmbeddedResource("font_bold.ttf");
            bold = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()),
                                                     fontSize, &fontConfig);

            fonts = EmbeddedResource("font_italic.ttf");
            italic = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()),
                                                       fontSize, &fontConfig);
        }

        VERSION_STRING = std::string("RCI ") + std::string(RCI_VERSION, RCI_VERSION_END) + std::string("\nRCP ") +
            std::string(RCP_VERSION, RCP_VERSION_END);
        VERSION_SIZE = normal->CalcTextSizeA(16, FLT_MAX, -1, VERSION_STRING.c_str());
        VERSION_SIZE.x = ImCeilFast(VERSION_SIZE.x);
    }

    void cleanup() {
        IM_DELETE(sharedFonts);
        glDeleteTextures(1, &birdTex);
        glfwDestroyWindow(shared);
    }

    GLFWwindow* getSharedResources() { return shared; }
    ImFontAtlas* getSharedFonts() { return sharedFonts; }
    GLuint birdIcon() { return birdTex; }
    void fontNormal(float size) { ImGui::PushFont(normal, size); }
    void fontBold(float size) { ImGui::PushFont(bold, size); }
    void fontItalic(float size) { ImGui::PushFont(italic, size); }

    void setWindowIcon(GLFWwindow* window) {
        EmbeddedResource im("LRI_Logo.png");
        GLFWimage image;
        image.pixels = stbi_load_from_memory((unsigned char*) im.getData(), static_cast<int>(im.getLength()),
                                             &image.width, &image.height, nullptr, 4);
        glfwSetWindowIcon(window, 1, &image);
        stbi_image_free(image.pixels);
    }

    void resetFontFrameCount() { sharedFonts->Builder->FrameCount = 0; }

    const std::string& versionString() { return VERSION_STRING; }
    const ImVec2& verStringSize() { return VERSION_SIZE; }

    void setColors() {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        auto& colors = style.Colors;

        colors[ImGuiCol_WindowBg] = colors::WINDOW_BG;
        colors[ImGuiCol_Text] = colors::TEXT;
    }
} // namespace LRI::RCI::style

namespace LRI::RCI {
    void pingFonts() { ImFontAtlasUpdateNewFrame(sharedFonts, ImGui::GetFrameCount(), true); }
} // namespace LRI::RCI
