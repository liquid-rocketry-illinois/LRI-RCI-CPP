#include "hardware/json.h"

#include <fstream>

#include "nlohmann/json.hpp"

namespace LRI::RCI {
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetTest, id, name);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetDevice, devclass, ids, names);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetTable6, refresh, ids);
    NLOHMANN_JSON_SERIALIZE_ENUM(SensorViewerMode,
                                 {{SensorViewerMode::INVALID, nullptr},
                                  {SensorViewerMode::CLASSIC, "classic"},
                                  {SensorViewerMode::ABRIDGED, "abridged"},
                                  {SensorViewerMode::MULTI, "multi"}});

    void to_json(nlohmann::json& j, const TargetTable7& t) {
        j["mode"] = t.mode;
        if(t.mode == SensorViewerMode::CLASSIC) j["showControls"] = t.classicShowControls;
        std::vector<nlohmann::json> ids;
        for(const auto& id : t.ids) {
            nlohmann::json jid;
            jid["class"] = id.devclass;
            jid["ids"] = id.ids;

            if(t.mode == SensorViewerMode::ABRIDGED) {
                jid["channel"] = id.channels;
            }

            else if(t.mode == SensorViewerMode::MULTI) {
                jid["channel"] = id.channels[0];
                jid["title"] = id.multiTitle;
            }

            ids.emplace_back(std::move(jid));
        }
        j["ids"] = ids;
    }

    void from_json(const nlohmann::json& j, TargetTable7& t) {
        t.mode = j["mode"].get<SensorViewerMode>();

        if(t.mode == SensorViewerMode::CLASSIC) t.classicShowControls = j["showControls"].get<bool>();

        for(size_t i = 0; i < j["ids"].size(); i++) {
            t.ids.emplace_back();
            TargetTable7::SensorViewerIDData& id = t.ids[i];
            id.devclass = j["ids"][i]["class"].get<RCP_DeviceClass>();
            id.ids = j["ids"][i]["ids"].get<std::vector<uint8_t>>();

            if(t.mode == SensorViewerMode::ABRIDGED) id.channels = j["ids"][i]["channel"].get<std::vector<uint8_t>>();
            else if(t.mode == SensorViewerMode::MULTI) {
                id.channels.push_back(j["ids"][i]["channel"].get<uint8_t>());
                id.multiTitle = j["ids"][i]["title"].get<std::string>();
            }
        }
    }

    bool table6Data(uint8_t type) {
        return type == RCP_DEVCLASS_SIMPLE_ACTUATOR || type == RCP_DEVCLASS_STEPPER ||
            type == RCP_DEVCLASS_ANGLED_ACTUATOR || type == RCP_DEVCLASS_MOTOR || type == RCP_DEVCLASS_BOOL_SENSOR;
    }

    bool table7Data(uint8_t type) {
        return type >= 0x90 && type <= 0xCF && type != RCP_DEVCLASS_BOOL_SENSOR;
    }

    void to_json(nlohmann::json& j, const TargetWModule& t) {
        if(table6Data(t.type)) j = std::move(t.t6data);
        else if(table7Data(t.type)) j = std::move(t.t7data);
        j["type"] = t.type;
    }

    void from_json(const nlohmann::json& j, TargetWModule& t) {
        t.type = j["type"].get<int>();
        if(table6Data(t.type)) t.t6data = j.get<TargetTable6>();
        else if(table7Data(t.type)) t.t7data = j.get<TargetTable7>();
    }

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetWindowlet, title, modules);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetConfig, name, tests, devices, windows);

    TargetConfig readConfig(const std::filesystem::path& path) {
        std::ifstream cfile(path);
        nlohmann::json config = nlohmann::json::parse(cfile);
        return config.get<TargetConfig>();
    }
} // namespace LRI::RCI
