#include "UI/gutils.h"

#include "imgui_internal.h"
#include "stb_image.h"

#include "EmbeddedResource.h"

namespace {
    GLFWwindow* shared = nullptr;

    GLuint birdTex = 0;

    ImFontAtlas* sharedFonts = nullptr;
    ImFont* normal = nullptr;
    ImFont* bold = nullptr;
    ImFont* italic = nullptr;
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
            ImFontConfig fontConfig;
            fontConfig.FontDataOwnedByAtlas = false;

            // Load the fonts and add them to imgui. Ubuntu mono my beloved
            EmbeddedResource fonts("font_regular.ttf");
            normal = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()), 16,
                                                       &fontConfig);
            fonts = EmbeddedResource("font_bold.ttf");
            bold = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()), 16,
                                                     &fontConfig);

            fonts = EmbeddedResource("font_italic.ttf");
            italic = sharedFonts->AddFontFromMemoryTTF((void*) fonts.getData(), static_cast<int>(fonts.getLength()), 16,
                                                       &fontConfig);
        }
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
} // namespace LRI::RCI::style

namespace LRI::RCI {
    void pingFonts() { ImFontAtlasUpdateNewFrame(sharedFonts, ImGui::GetFrameCount(), true); }
}
