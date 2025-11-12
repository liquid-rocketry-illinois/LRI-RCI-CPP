#include "hardware/Sensors.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>

#include "hardware/HardwareControl.h"
#include "utils.h"

namespace LRI::RCI::Sensors {
    // Holds the data vectors mapped to their qualifiers
    static std::map<HardwareQualifier, std::vector<DataPoint>> sensors;

    // Threads are placed in a map that maps the thread ID to its pointer, so threads can move themselves
    // between these two structures
    static std::map<std::thread::id, std::thread*> activeThreads;
    static std::map<std::thread::id, std::thread*> destroyThreads;

    // Lock for the above maps
    static std::mutex threadSetMux;

    // A flag for all threads to indicate if they should self destruct
    static std::atomic_bool destroy;

    // Function that gets run in thread to put sensor data into a csv. The vector of data is de-allocated at the end!
    static void toCSVFile(const HardwareQualifier& qual, const std::vector<DataPoint>* data) {
        static auto mapStuff = [&] {
            threadSetMux.lock();
            if(destroy) return;
            const auto id = std::this_thread::get_id();
            const auto thread = activeThreads[id];
            activeThreads.erase(id);
            destroyThreads[id] = thread;
            threadSetMux.unlock();
        };
        // Create the exports directory if it does not exist. If it exists as a file, exit early
        const auto& exportFolder = getRoamingFolder() / "exports";
        if(std::filesystem::exists(exportFolder)) {
            if(!std::filesystem::is_directory(exportFolder)) {
                mapStuff();
                return;
            }
        }

        else std::filesystem::create_directories(exportFolder);

        const auto now = std::chrono::system_clock::now();
        std::ofstream file(std::format("./exports/{:%d-%m-%Y-%H-%M-%OS}-", now) + qual.asString() + ".csv");

        // The actual file writing. Depending on the device class, a header is created, and each datapoint is iterated
        // through and written to the file
        switch(qual.devclass) {
        case RCP_DEVCLASS_GPS:
            file << "relmillis,latitude,longitude,altitude,groundspeed\n";
            for(const auto& point : *data) {
                file << std::format("{},{},{},{},{}\n", point.timestamp, point.data[0], point.data[1], point.data[2],
                                    point.data[3]);
            }
            break;

        case RCP_DEVCLASS_MAGNETOMETER:
        case RCP_DEVCLASS_ACCELEROMETER:
        case RCP_DEVCLASS_GYROSCOPE:
            file << "relmillis,x,y,z\n";
            for(const auto& point : *data) {
                file << std::format("{},{},{},{}\n", point.timestamp, point.data[0], point.data[1], point.data[2]);
            }
            break;

        case RCP_DEVCLASS_AM_PRESSURE:
        case RCP_DEVCLASS_TEMPERATURE:
        case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            file << "relmillis,data\n";
            for(const auto& point : *data) {
                file << std::format("{},{}\n", point.timestamp, point.data[0]);
            }
            break;

        default:
            file << "Device Class not supported/recognized for export";
            break;
        }

        delete data;
        file.close();
        mapStuff();
    }

    int receiveRCPUpdate(const RCP_OneFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID, .name = ""};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                       .data = {static_cast<double>(data.data)}};

        sensors[qual].push_back(d);
        return 0;
    }

    int receiveRCPUpdate(const RCP_TwoFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID, .name = ""};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0, .data = {}};
        for(int i = 0; i < 2; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
        return 0;
    }

    int receiveRCPUpdate(const RCP_ThreeFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID, .name = ""};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0, .data = {}};
        for(int i = 0; i < 3; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
        return 0;
    }

    int receiveRCPUpdate(const RCP_FourFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID, .name = ""};
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_TARGET, qual});
            return 1;
        }

        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0, .data = {}};
        for(int i = 0; i < 4; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual].push_back(d);
        return 0;
    }

    void setHardwareConfig(const std::set<HardwareQualifier>& sensids) {
        reset();

        for(const auto& qual : sensids) {
            addSensor(qual);
        }
    }

    void reset() {
        destroy = true;

        sensors.clear();

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms); // give time for all file write threads to finish

        threadSetMux.lock();
        destroyThreads.insert(activeThreads.cbegin(), activeThreads.cend());
        activeThreads.clear();
        threadSetMux.unlock();

        std::set<std::thread::id> remo;

        for(const auto& [id, thread] : destroyThreads) {
            thread->join();
            delete thread;
            remo.insert(id);
        }

        for(const auto& id : remo) destroyThreads.erase(id);

        threadSetMux.lock();

#ifndef NDEBUG
        assert((!activeThreads.empty(), "activeThreads not empty!"));
        assert((!destroyThreads.empty(), "destroyThreads not empty!"));
#else
        if(!activeThreads.empty() || !destroyThreads.empty()) abort();
#endif

        threadSetMux.unlock();
        destroy = false;
    }

    void update() {
        threadSetMux.lock();
        std::set<std::thread::id> remo;
        for(const auto& [id, thread] : destroyThreads) {
            thread->join();
            delete thread;
            remo.insert(id);
        }

        for(const auto& id : remo) destroyThreads.erase(id);
        threadSetMux.unlock();
    }

    const std::vector<DataPoint>* getState(const HardwareQualifier& qual) {
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return nullptr;
        }

        return &sensors.at(qual);
    }

    void writeCSV(const HardwareQualifier& qual) {
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        auto* copy = new std::vector(sensors[qual]);
        threadSetMux.lock();
        auto* thread = new std::thread(&toCSVFile, qual, copy);
        activeThreads[thread->get_id()] = thread;
        threadSetMux.unlock();
    }

    void tare(const HardwareQualifier& qual, uint8_t dataChannel) {
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        if(sensors[qual].empty()) return;
        auto& data = sensors[qual];
        DataPoint d = data.at(data.size() - 1);
        double prevtime = d.timestamp - 1;
        double total = 0;
        int numElems = 0;
        std::ranges::for_each(data, [&](const DataPoint& dp) {
            if(dp.timestamp >= prevtime) {
                total += dp.data[dataChannel];
                numElems++;
            }
        });

        auto ftotal = static_cast<float>(total / numElems);
        RCP_requestTareConfiguration(qual.devclass, qual.id, dataChannel, ftotal);
    }

    void clearGraph(const HardwareQualifier& qual) {
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        sensors[qual].clear();
    }

    void clearAll() {
        for(auto& graph : sensors | std::views::values) graph.clear();
    }

    void removeSensor(const HardwareQualifier& qual) {
        if(!sensors.contains(qual)) {
            HWCTRL::addError({HWCTRL::ErrorType::HWNE_HOST, qual});
            return;
        }

        sensors.erase(qual);
    }

    void addSensor(const HardwareQualifier& qual) {
        sensors[qual] = std::vector<DataPoint>();
        sensors[qual].reserve(DATA_VECTOR_INITIAL_SIZE);
    }


} // namespace LRI::RCI
