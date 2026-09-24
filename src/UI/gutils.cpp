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

    std::string renderLatestReadingsString(const HardwareChannel& qual, float data) {
        switch(qual.devclass) {
        case RCP_DEVCLASS_ANGLED_ACTUATOR:
            return std::format("{:4.1f} d", data);

        case RCP_DEVCLASS_MOTOR:
            return std::format("{:6.1f} rpm", data);

        case RCP_DEVCLASS_AM_PRESSURE:
            return std::format("{:.3f} mbar", data);

        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            return std::format("{: 5.1f} psi", data);

        case RCP_DEVCLASS_TEMPERATURE:
            return std::format("{: 5.1f} C", data);

        case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            return std::format("{:.3f} %", data);

        case RCP_DEVCLASS_LOAD_CELL:
            return std::format("{: 5.1f} kg", data);

        case RCP_DEVCLASS_FLOW_METER:
            return std::format("{:.3f} GPM", data);

        case RCP_DEVCLASS_ALTITUDE:
            return std::format("{} m", data);

        case RCP_DEVCLASS_RADIO_STRENGTH:
            return std::format("{} dBm", data);

        case RCP_DEVCLASS_POWERMON:
            if(qual.channel == 0) return std::format("Voltage: {:.3f} V", data);
            return std::format("Power: {:.3f} W", data);

        case RCP_DEVCLASS_ACCELEROMETER: {
            constexpr char AXIS[] = {'X', 'Y', 'Z'};
            return std::format("{}: {:.3f} m/s/s", AXIS[qual.channel], data);
        }

        case RCP_DEVCLASS_GYROSCOPE: {
            constexpr char AXIS[] = {'X', 'Y', 'Z'};
            return std::format("{}: {:.3f} d/s", AXIS[qual.channel], data);
        }

        case RCP_DEVCLASS_MAGNETOMETER: {
            constexpr char AXIS[] = {'X', 'Y', 'Z'};
            return std::format("{}: {:.3f} G", AXIS[qual.channel], data);
        }

        case RCP_DEVCLASS_RPY: {
            constexpr const char* AXIS[] = {"Roll", "Pitch", "Yaw"};
            return std::format("{}: {:.3f} d", AXIS[qual.channel], data);
        }

        case RCP_DEVCLASS_GPS: {
            constexpr const char* AXIS[] = {"Latitude", "Longitude", "Altitude", "Ground Speed"};
            constexpr const char* UNIT[] = {"d", "d", "m", "m/s"};
            return std::format("{}: {:.3f} {}", AXIS[qual.channel], data, UNIT[qual.channel]);
        }

        case RCP_DEVCLASS_QUATERNION: {
            constexpr char AXIS[] = {'W', 'X', 'Y', 'Z'};
            return std::format("{}: {:.3f}", AXIS[qual.channel], data);
        }
        default:
            return "unknown";
        }
    }

    ImPlotPoint implotTargetFloat(int index, void* data) {
        auto* tdata = static_cast<EventLog::TargetFloat*>(data);
        return ImPlotPoint{static_cast<double>(tdata->times->at(index).ttime),
                           static_cast<double>(tdata->values->at(index))};
    }

    namespace GraphInfo {
        static float min3(float a, float b, float c) { return std::min(a, std::min(b, c)); }

        // This structure is an abomination to all good code style. I apologize for my crimes.
        // clang-format off
        const std::map<const RCP_DeviceClass, const GraphInfo> GRAPHINFO {
            {RCP_DEVCLASS_MOTOR,               {{{"Speed (rpm)", "Motor"}},                                                                                                                                            {{0, "RPM"}}}},
            {RCP_DEVCLASS_AM_PRESSURE,         {{{"Pressure (mbars)", "Pressure"}},                                                                                                                                    {{0, "MBar"}}}},
            {RCP_DEVCLASS_TEMPERATURE,         {{{"Temperature (Celsius)", "Temperature"}},                                                                                                                            {{0, "C"}}}},
            {RCP_DEVCLASS_PRESSURE_TRANSDUCER, {{{"Pressure (psi)", "Pressure"}},                                                                                                                                      {{0, "PSI"}}}},
            {RCP_DEVCLASS_RELATIVE_HYGROMETER, {{{"Humidity (Relative %)", "Humidity"}},                                                                                                                               {{0, "%"}}}},
            {RCP_DEVCLASS_FLOW_METER,          {{{"Flow Rate (GPM)", "Flow Rate"}},                                                                                                                                    {{0, "GPM"}}}},
            {RCP_DEVCLASS_LOAD_CELL,           {{{"Mass (kg)", "Mass"}},                                                                                                                                               {{0, "kg"}}}},
            {RCP_DEVCLASS_ALTITUDE,            {{{"Altitude (m)", "Altitude"}},                                                                                                                                        {{0, "m"}}}},
            {RCP_DEVCLASS_RADIO_STRENGTH,      {{{"RSSI (dBm)", "Signal Strength"}},                                                                                                                                   {{0, "dBm"}}}},
            {RCP_DEVCLASS_POWERMON,            {{{"Voltage", "Voltage"}, {"Power (W)", "Power"}},                                                                                   {{0, "Voltage"},  {1, "Power"}}}},
            {RCP_DEVCLASS_ACCELEROMETER,       {{{"Acceleration (m/s/s)", "Acceleration"}},                                                                                                                            {{0, "X"},        {0, "Y"},         {0, "Z"}}}},
            {RCP_DEVCLASS_GYROSCOPE,           {{{"Rotation (deg/s)", "Rotation"}},                                                                                                                                    {{0, "X"},        {0, "Y"},         {0, "Z"}}}},
            {RCP_DEVCLASS_MAGNETOMETER,        {{{"Magnetic Field (Gauss)", "Magnetic Field"}},                                                                                                                        {{0, "X"},        {0, "Y"},         {0, "Z"}}}},
            {RCP_DEVCLASS_RPY,                 {{{"Orientation (degrees)", "Orientation"}},                                                                                                                            {{0, "Roll"},     {0, "Pitch"},     {0, "Yaw"}}}},
            {RCP_DEVCLASS_GPS,                 {{{"Lat/Lon", "Position"}, {"Altitude (m)", "Altitude"}, {"Ground Speed (m/s)", "Ground Speed"}}, {{0, "Latitude"}, {0, "Longitude"}, {2, "Altitude"}, {3, "Ground Speed"}}}},
            {RCP_DEVCLASS_QUATERNION,          {{{"Q", "Quaternion"}},                                                                                                                                                 {{0, "W"},        {0, "X"},         {0, "Y"},        {0, "Z"}}}},
        };
        // clang-format on

        ImVec2 calcPlotSize() {
            const float xsize = ImGui::GetWindowWidth() - 27_sc;
            return {xsize, min3(xsize * (9.0f / 16.0f), 500_sc, ImGui::GetWindowHeight() - 27_sc)};
        }
    } // namespace
} // namespace LRI::RCI
