#include "UI/style.h"

#include "util/stb_image.h"

#include "util/EmbeddedResource.h"
#include "UI/fontawesome.h"
#include "UI/vscode_icons.h"


namespace LRI::RCI {
    namespace {
        float scaleFactor;
        GLuint birdTex;
        ImVec2 windowFrameSize;
    } // namespace

    float operator""_sc(unsigned long long value) { return static_cast<float>(value) * scaleFactor; }
    float operator""_sc(long double value) { return static_cast<float>(value) * scaleFactor; }

    namespace style {
        void setImGuiStyles() {
            // ----- Sizes -----
            ImGuiStyle& style = ImGui::GetStyle();
            ImGui::StyleColorsDark();

            style.WindowBorderSize = 0;
            style.PopupBorderSize = 1;
            style.PopupRounding = 3;

            // ----- Colors -----

            auto& colors = style.Colors;

            colors[ImGuiCol_Text] = WHITE;
            colors[ImGuiCol_WindowBg] = DGRAY;
            colors[ImGuiCol_PopupBg] = DGRAY;
            colors[ImGuiCol_Border] = LGRAY_SEMITRANSPARENT;
            colors[ImGuiCol_Button] = CTRANSPARENT;
            colors[ImGuiCol_ButtonHovered] = BUTTONHOVER;
            colors[ImGuiCol_ButtonActive] = BUTTONPRESS;
            // colors[ImGuiCol_Button] = PURPLEF;
            // colors[ImGuiCol_ButtonHovered] = LPURPLEF;
            // colors[ImGuiCol_ButtonActive] = LLPURPLEF;

            // colors[ImGuiCol_Tab] = PURPLEF;
            // colors[ImGuiCol_TabActive] = LLPURPLEF;
            // colors[ImGuiCol_TabHovered] = LPURPLEF;
            // colors[ImGuiCol_TabDimmed] = DPURPLEF;

            // colors[ImGuiCol_FrameBg] = PURPLEF;
            // colors[ImGuiCol_FrameBgHovered] = LPURPLEF;
            // colors[ImGuiCol_FrameBgActive] = LLPURPLEF;

            // colors[ImGuiCol_Header] = PURPLEF;
            // colors[ImGuiCol_HeaderHovered] = LPURPLEF;
            // colors[ImGuiCol_HeaderActive] = LLPURPLEF;

            // colors[ImGuiCol_CheckMark] = WHITEF;
        }

        void setScaleFactor(float scale) {
            scaleFactor = scale;
            ImGui::GetStyle().ScaleAllSizes(scale);
        }

        float getScaleFactor() { return scaleFactor; }

        void setWindowIcon(GLFWwindow* window) {
            EmbeddedResource image("LRI_Logo.png");
            GLFWimage gimage;
            gimage.pixels =
                stbi_load_from_memory(reinterpret_cast<const unsigned char*>(image.getData()),
                                      static_cast<int>(image.getLength()), &gimage.width, &gimage.height, nullptr, 4);
            glfwSetWindowIcon(window, 1, &gimage);
            stbi_image_free(gimage.pixels);
        }

        void setupBirdIcon() {
            EmbeddedResource image("LRI_Logo_big.png");
            int iw, ih;
            unsigned char* idata = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(image.getData()),
                                                         static_cast<int>(image.getLength()), &iw, &ih, nullptr, 4);

            glGenTextures(1, &birdTex);
            glBindTexture(GL_TEXTURE_2D, birdTex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, idata);
            stbi_image_free(idata);
        }

        void cleanupBirdTex() { glDeleteTextures(1, &birdTex); }
        GLuint getBirdTex() { return birdTex; }

        void setFrameWindowSize(ImVec2 size) { windowFrameSize = size; }
        ImVec2 getWindowSize() { return windowFrameSize; }
    } // namespace style

    namespace font {
        namespace {
            ImFont* bold;
            ImFont* italic;
            ImFont* awesome;
            ImFont* codicons;
        } // namespace

        void setupFonts() {
            static constexpr ImWchar AWESOME_CODEPOINTS[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
            static constexpr ImWchar CODICON_CODEPOINTS[] = {ICON_MIN_VS, ICON_MAX_16_VS, 0};

            ImFontAtlas* fonts = ImGui::GetIO().Fonts;
            fonts->ClearFonts();

            const float fontsize = 16_sc;

            ImFontConfig text;
            text.FontDataOwnedByAtlas = false;

            ImFontConfig icons;
            icons.FontDataOwnedByAtlas = false;
            icons.PixelSnapH = true;
            icons.GlyphMinAdvanceX = fontsize;
            icons.MergeMode = true;
            icons.GlyphOffset = {0, 3_sc};

            // Load fontawesome and codicons in merge mode with the regular font so the icons can be used in text
            // We load font_regular twice, once with awesome merged, and once with codicons merged
            EmbeddedResource regular("font_regular.ttf");
            awesome = fonts->AddFontFromMemoryTTF(const_cast<char*>(regular.getData()),
                                                  static_cast<int>(regular.getLength()), fontsize, &text);

            EmbeddedResource font("fa_900.ttf");
            fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize,
                                        &icons, AWESOME_CODEPOINTS);

            codicons = fonts->AddFontFromMemoryTTF(const_cast<char*>(regular.getData()),
                                                   static_cast<int>(regular.getLength()), fontsize, &text);

            font = EmbeddedResource("codicon.ttf");
            fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize,
                                        &icons, CODICON_CODEPOINTS);

            font = EmbeddedResource("font_bold.ttf");
            bold = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()),
                                               fontsize, &text);

            font = EmbeddedResource("font_italic.ttf");
            italic = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()),
                                                 fontsize, &text);
        }

        void pushBold() { ImGui::PushFont(bold); }
        void pushItalic() { ImGui::PushFont(italic); }
        void pushAwesome() { ImGui::PushFont(awesome); }
        void pushCodicons() { ImGui::PushFont(codicons); }
    } // namespace font
} // namespace LRI::RCI
