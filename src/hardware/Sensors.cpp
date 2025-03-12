#include "hardware/Sensors.h"

#include <filesystem>
#include <fstream>
#include <ranges>

namespace LRI::RCI {
    bool Sensors::FileWriteThreadData::operator<(FileWriteThreadData const& rhf) const {
        return thread < rhf.thread;
    }

    Sensors* Sensors::instance;

    Sensors* Sensors::getInstance() {
        if(instance == nullptr) instance = new Sensors();
        return instance;
    }

    // Function that gets run in thread to put sensor data into a csv. The vector of data is de-allocated at the end!
    void Sensors::toCSVFile(const HardwareQualifier& qual, const std::vector<DataPoint>* data, std::atomic_bool* done) {
        // Create the exports directory if it does not exist. If it exists as a file, exit early
        if(std::filesystem::exists("exports")) {
            if(!std::filesystem::is_directory("./exports")) {
                *done = true;
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
                file << std::format("{},{},{},{},{}\n", point.timestamp, point.data[0],
                                    point.data[1],
                                    point.data[2], point.data[3]);
            }
            break;

        case RCP_DEVCLASS_MAGNETOMETER:
        case RCP_DEVCLASS_ACCELEROMETER:
        case RCP_DEVCLASS_GYROSCOPE:
            file << "relmillis,x,y,z\n";
            for(const auto& point : *data) {
                file << std::format("{},{},{},{}\n", point.timestamp, point.data[0],
                                    point.data[1],
                                    point.data[2]);
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
        *done = true;
    }

    void Sensors::receiveRCPUpdate(const RCP_OneFloat& data) {
        HardwareQualifier qual = {.devclass = data.devclass, .id = data.ID};
        DataPoint d = {
            .timestamp = static_cast<double>(data.timestamp) / 1'000.0,
            .data = {static_cast<double>(data.data)}
        };

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
        // Clear all sensors from the list
        for(const auto* data : sensors | std::views::values) {
            delete data;
        }

        // Make sure no more file writing threads are active. Then exit
        for(auto& thread : filewritethreads) {
            if(thread.thread) {
                thread.thread->join();
                delete thread.thread;
            }
        }

        filewritethreads.clear();
    }

    void Sensors::update() {
        auto filtered = filewritethreads | std::views::filter([](const auto& val) { return val.done; });
        std::ranges::for_each(filtered, [](const FileWriteThreadData& val) {
            val.thread->join();
            delete val.thread;
        });

        std::set rem(filtered.begin(), filtered.end());
        filewritethreads.erase(rem.cbegin(), rem.cend());
    }

    const std::map<HardwareQualifier, std::vector<Sensors::DataPoint>*>* Sensors::getState() const {
        return &sensors;
    }

    const std::vector<Sensors::DataPoint>* Sensors::getState(const HardwareQualifier& qual) const {
        return sensors.at(qual);
    }

    void Sensors::writeCSV(const HardwareQualifier& qual) {
        std::vector<DataPoint>* copy = new std::vector(*sensors[qual]);
        FileWriteThreadData data;
        data.done = false;
        data.thread = new std::thread(toCSVFile, qual, copy, &data.done);
        filewritethreads.insert(data);
    }
}
