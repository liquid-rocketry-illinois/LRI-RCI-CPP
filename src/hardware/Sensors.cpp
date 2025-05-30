#include "hardware/Sensors.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <ranges>

namespace LRI::RCI {
    Sensors* Sensors::getInstance() {
        static Sensors* instance = nullptr;
        if(instance == nullptr) instance = new Sensors();
        return instance;
    }

    // Function that gets run in thread to put sensor data into a csv. The vector of data is de-allocated at the end!
    void Sensors::toCSVFile(const HardwareQualifier& qual, const std::vector<DataPoint>* data) {
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
        if(std::filesystem::exists("exports")) {
            if(!std::filesystem::is_directory("./exports")) {
                mapStuff();
                return;
            }
        }

        else std::filesystem::create_directory("./exports");

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
        case RCP_DEVCLASS_AM_TEMPERATURE:
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

    Sensors::~Sensors() { reset(); }


    void Sensors::receiveRCPUpdate(const RCP_OneFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0,
                       .data = {static_cast<double>(data.data)}};

        sensors[qual]->push_back(d);
    }

    void Sensors::receiveRCPUpdate(const RCP_TwoFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 2; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual]->push_back(d);
    }

    void Sensors::receiveRCPUpdate(const RCP_ThreeFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 3; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual]->push_back(d);
    }

    void Sensors::receiveRCPUpdate(const RCP_FourFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {.timestamp = static_cast<double>(data.timestamp) / 1'000.0};
        for(int i = 0; i < 4; i++) d.data[i] = static_cast<double>(data.data[i]);
        sensors[qual]->push_back(d);
    }

    void Sensors::setHardwareConfig(const std::set<HardwareQualifier>& sensids) {
        reset();

        for(const auto& qual : sensids) {
            sensors[qual] = new std::vector<DataPoint>();
            sensors[qual]->reserve(DATA_VECTOR_INITIAL_SIZE);
        }
    }

    void Sensors::reset() {
        destroy = true;

        // Clear all sensors from the list
        for(const auto* data : sensors | std::views::values) {
            delete data;
        }

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

    void Sensors::update() {
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

    const std::vector<Sensors::DataPoint>* Sensors::getState(const HardwareQualifier& qual) const {
        return sensors.at(qual);
    }

    void Sensors::writeCSV(const HardwareQualifier& qual) {
        auto* copy = new std::vector(*sensors[qual]);
        threadSetMux.lock();
        auto* thread = new std::thread(&Sensors::toCSVFile, this, qual, copy);
        activeThreads[thread->get_id()] = thread;
        threadSetMux.unlock();
    }

    void Sensors::tare(const HardwareQualifier& qual, uint8_t dataChannel) {
        if(sensors[qual]->empty()) return;
        auto& data = sensors[qual];
        DataPoint d = data->at(data->size() - 1);
        double prevtime = d.timestamp - 1;
        double total = 0;
        int numElems = 0;
        std::ranges::for_each(*data, [&](const DataPoint& dp) {
            if(dp.timestamp >= prevtime) {
                total += dp.data[dataChannel];
                numElems++;
            }
        });

        auto ftotal = static_cast<float>(total / numElems);
        RCP_requestTareConfiguration(qual.devclass, qual.id, dataChannel, &ftotal, 4);
    }

    void Sensors::clearGraph(const HardwareQualifier& qual) { sensors[qual]->clear(); }

    void Sensors::clearAll() {
        for(const auto& graph : sensors | std::views::values) graph->clear();
    }
} // namespace LRI::RCI
