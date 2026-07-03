#include "util/settings.h"

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
            if(r.type == RecentType::TARGET) n["conn_text"] = r.connectName;
            return n;
        }

        static bool decode(const Node& node, Recent& r) {
            if(!node["type"] || !node["display_text"] || !node["path"]) return false;
            r.type = node["type"].as<RecentType>();
            if(r.type == RecentType::TARGET) {
                if(!node["conn_text"]) return false;
                r.connectName = node["conn_text"].as<std::string>();
            }
            r.displayName = node["display_text"].as<std::string>();
            r.path = std::filesystem::path(node["path"].as<std::string>());
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
            parseErrorString = std::format("Could not parse Recent field at line {} col {}, in file {}",
                                           e.mark.line, e.mark.column, cfile.string());
            return;
        }
        catch(YAML::TypedBadConversion<std::vector<Recent>>& e) {
            parseErrorString = std::format("Could not parse Recent vector at line {} col {}, in file {}",
                                           e.mark.line, e.mark.column, cfile.string());
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

    const std::vector<Recent>& getRecents() { return recents; }

    // Remove the recent at removeidx if exists, add nrecent to front of array, and cap size at 10
    void updateRecents(ptrdiff_t removeidx, Recent nrecent) {
        if(removeidx > 0 && static_cast<size_t>(removeidx) < recents.size()) recents.erase(recents.begin() + removeidx);
        recents.emplace(recents.begin(), std::move(nrecent));
        if(recents.size() > 10) recents.erase(recents.begin() + 10, recents.end());
    }
} // namespace LRI::RCI::settings
