#include "util/settings.h"
#include <fstream>

#include "yaml-cpp/yaml.h"

#include "util/guards.h"
#include "util/system.h"

namespace YAML {
    using LRI::RCI::settings::Recent;
    using LRI::RCI::settings::RecentType;

    template<>
    struct convert<RecentType> {
        static Node encode(const RecentType& type) {
            Node n;
            if(type == RecentType::TARGET) n = "target";
            else if(type == RecentType::TLOG) n = "testlog";
            else n = "unknown";
            return n;
        }

        static bool decode(const Node& node, RecentType& type) {
            if(const auto val = node.as<std::string>(); val == "target") type = RecentType::TARGET;
            else if(val == "testlog") type = RecentType::TLOG;
            else return false;
            return true;
        }
    };

    template<>
    struct convert<Recent> {
        static Node encode(const Recent& r) {
            Node n;
            n["type"] = r.type;
            n["display_text"] = r.displayName;
            n["path"] = r.path.string();
            if(r.type == RecentType::TARGET) {
                n["conn_text"] = r.connectName;
                std::stringstream ss;
                ss << std::hex;
                ss << (r.display_char[0] & 0x0F);
                ss << ((r.display_char[1] & 0x3C) >> 2);
                ss << ((r.display_char[1] & 0x03) << 2 | (r.display_char[2] & 0x30) >> 4);
                ss << (r.display_char[2] & 0x0F);
                n["display_char"] = ss.str();
            }

            return n;
        }

        static bool decode(const Node& node, Recent& r) {
            if(!node["type"] || !node["display_text"] || !node["path"]) return false;
            r.type = node["type"].as<RecentType>();
            r.displayName = node["display_text"].as<std::string>();
            r.path = std::filesystem::path(node["path"].as<std::string>());

            if(r.type != RecentType::TARGET) return true;

            if(!node["conn_text"]) return false;
            r.connectName = node["conn_text"].as<std::string>();

            auto hexstr = node["display_char"].as<std::string>();
            uint8_t vals[4];
            for(uint8_t i = 0; i < 4; i++) vals[i] = std::stoi(hexstr.substr(i, 1), nullptr, 16);

            if(vals[0] == 0) {
                r.display_char.fill(0);
                return true;
            }

            // Create the utf8 char from the hex in the config
            // Its a multibyte char, where for U+WXYZ, it goes to:
            // 1110WWWW 10XXXXYY 10YYZZZZ
            r.display_char[0] = static_cast<char>((0b1110 << 4) | (vals[0] & 0x0F));
            r.display_char[1] = static_cast<char>((0b10 << 6) | ((vals[1] & 0x0F) << 2) | ((vals[2] & 0x0C) >> 2));
            r.display_char[2] = static_cast<char>((0b10 << 6) | ((vals[2] & 0x03) << 4) | (vals[3] & 0x0F));
            r.display_char[3] = 0;


            return true;
        }
    };
} // namespace YAML

namespace LRI::RCI::settings {
    namespace {
        bool parseError = false;
        std::string parseErrorString;

        std::vector<Recent> recents;
    } // namespace

    void loadUsersettings() {
        parseError = true;
        parseErrorString.clear();

        auto cfile = roamingFolder() / "usersettings.yaml";

        if(!std::filesystem::exists(cfile)) {
            loadFreshConfig();
            parseError = true;
            return;
        }

        if(std::filesystem::is_directory(cfile)) {
            parseErrorString = "Config file is directory";
            return;
        }

        YAML::Node config = YAML::LoadFile(cfile.string());

        try {
            recents = config["recents"].as<std::vector<Recent>>();
        }

        catch(YAML::TypedBadConversion<RecentType>& e) {
            parseErrorString = std::format("Could not parse RecentType field at line {} col {}, in file {}",
                                           e.mark.line, e.mark.column, cfile.string());
            return;
        }

        catch(YAML::TypedBadConversion<Recent>& e) {
            parseErrorString = std::format("Could not parse Recent field at line {} col {}, in file {}", e.mark.line,
                                           e.mark.column, cfile.string());
            return;
        }
        catch(YAML::TypedBadConversion<std::vector<Recent>>& e) {
            parseErrorString = std::format("Could not parse Recent vector at line {} col {}, in file {}", e.mark.line,
                                           e.mark.column, cfile.string());
            return;
        }

        parseError = false;
    }

    bool hadParseError() { return parseError; }
    const std::string& getErrorString() { return parseErrorString; }

    void loadFreshConfig() {
        recents.clear();
        parseError = false;
    }

    void writeSettings() {
        YAML::Node n;
        n["recents"] = recents;
        YAML::Emitter em;
        em << n;

        auto cfile = roamingFolder() / "usersettings.yaml";
        std::ofstream out(cfile);
        out << em.c_str();
    }

    const std::vector<Recent>& getRecents() { return recents; }

    // Remove the recent at removeidx if exists, add nrecent to front of array, and cap size at 10
    void updateRecents(ptrdiff_t removeidx, Recent nrecent) {
        if(removeidx >= 0 && static_cast<size_t>(removeidx) < recents.size())
            recents.erase(recents.begin() + removeidx);
        recents.emplace(recents.begin(), std::move(nrecent));
        if(recents.size() > 10) recents.erase(recents.begin() + 10, recents.end());
    }
} // namespace LRI::RCI::settings
