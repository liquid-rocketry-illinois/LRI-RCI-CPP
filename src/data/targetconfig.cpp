#include "data/TargetConfig.h"

#include <fstream>
#include <ranges>

#include "yaml-cpp/yaml.h"

namespace YAML {
    using LRI::RCI::target::IDNamePair;
    using LRI::RCI::target::TargetConfig;

    template<>
    struct convert<IDNamePair> {
        static Node encode(const IDNamePair& i) {
            Node n;
            n["id"] = i.id;
            n["name"] = i.name;
            return n;
        }

        static bool decode(const Node& node, IDNamePair& i) {
            if(!node["name"] || !node["id"]) return false;
            i.id = node["id"].as<uint8_t>();
            i.name = node["name"].as<std::string>();
            return true;
        }
    };

    template<>
    struct convert<TargetConfig> {
        static Node encode(const TargetConfig& config) {
            Node n;
            n["name"] = config.name;
            n["tests"] = config.tests;

            std::set<RCP_DeviceClass> foundClasses;
            for(const auto& qual : config.quals) {
                if(foundClasses.contains(qual.devclass)) continue;
                foundClasses.insert(qual.devclass);

                std::vector<IDNamePair> instances;
                for(const auto& q : config.quals | std::views::filter([&qual](const LRI::RCI::HardwareQualifier& p) {
                                        return p.devclass == qual.devclass;
                                    })) {
                    instances.emplace_back(q.id, q.name);
                }

                Node hardware;
                hardware["devclass"] = static_cast<uint8_t>(qual.devclass);
                hardware["instances"] = instances;
                n["hardware"].push_back(hardware);
            }

            return n;
        }

        static bool decode(const Node& n, TargetConfig& config) {
            if(!n["name"] || !n["tests"] || !n["tests"].IsSequence() || !n["hardware"] || !n["hardware"].IsSequence())
                return false;

            config.name = n["name"].as<std::string>();
            config.tests = n["name"].as<std::vector<IDNamePair>>();

            for(const auto& group : n["hardware"]) {
                auto devclass = static_cast<RCP_DeviceClass>(group["devclass"].as<uint8_t>());
                auto instances = group["instances"].as<std::vector<IDNamePair>>();
                for(const auto& [id, name] : instances) config.quals.emplace(devclass, id, name);
            }
            return true;
        }
    };
} // namespace YAML

namespace LRI::RCI::target {
    void serializeConfig(const std::filesystem::path& path, const TargetConfig& config) {
        YAML::Node n(config);
        YAML::Emitter em;
        em << n;
        std::ofstream out(path);
        out << em.c_str();
    }

    std::optional<std::string> deserializeConfig(const std::filesystem::path& path, TargetConfig& config) {
        try {
            YAML::Node n = YAML::LoadFile(path.string());
            config = n.as<TargetConfig>();
        }

        catch(const YAML::Exception& e) {
            return e.msg;
        }

        return std::nullopt;
    }
} // namespace LRI::RCI::target
