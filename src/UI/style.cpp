#include "UI/style.h"

#include "stb_image.h"

#include "EmbeddedResource.h"
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
        void setImGuiStyles() {}

        void setScaleFactor(float scale) { scaleFactor = scale; }

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
            ImFont* normal;
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

            ImFontConfig fc;
            fc.FontDataOwnedByAtlas = false;

            EmbeddedResource font("font_regular.ttf");
            normal = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize, &fc);

            font = EmbeddedResource("font_bold.ttf");
            bold = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize, &fc);

            font = EmbeddedResource("font_italic.ttf");
            italic = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize, &fc);

            fc.PixelSnapH = true;
            fc.GlyphMinAdvanceX = fontsize;

            font = EmbeddedResource("fa_900.ttf");
            awesome = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize, &fc,
                                                  AWESOME_CODEPOINTS);

            font = EmbeddedResource("codicons.ttf");
            codicons = fonts->AddFontFromMemoryTTF(const_cast<char*>(font.getData()), static_cast<int>(font.getLength()), fontsize, &fc,
                                                   CODICON_CODEPOINTS);
        }

        void pushNormal() { ImGui::PushFont(normal); }
        void pushBold() { ImGui::PushFont(bold); }
        void pushItalic() { ImGui::PushFont(italic); }
        void pushAwesome() { ImGui::PushFont(awesome); }
        void pushCodicons() { ImGui::PushFont(codicons); }
    } // namespace font
} // namespace LRI::RCI
