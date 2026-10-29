#include "hardware/EventLog.h"

#include <filesystem>
#include <format>
#include "utils.h"

// These functions are placed into their own file to concentrate all the HDF shenanigans in one place

#include "hdf5.h"

#define U8ToHex(num) std::format("{:02X}", num).c_str()
#define ToHex(devclass) U8ToHex(static_cast<uint8_t>(devclass))

// Typing namespace
// namespace {
//     using namespace LRI::RCI;
//     hid_t qualType;
//     hid_t varStrType;
//     hid_t htimeType;
//     hid_t ttimeType;
//     hid_t ctimeType;
//     hid_t errType;
//     hid_t readReqType;
//     hid_t hostStringType;
//
//     inline const char* deepCopyString(const char* str) {
//         size_t slen = strlen(str);
//         char* buf = new char[slen + 1];
//         memcpy(buf, str, slen);
//         buf[slen] = 0;
//         return buf;
//     }
//
//     struct SerializableHardwareQualifier {
//         RCP_DeviceClass devclass;
//         uint8_t id;
//         const char* name;
//
//         SerializableHardwareQualifier(const HardwareQualifier& qual) :
//             devclass(qual.devclass), id(qual.id), name(deepCopyString(qual.name.c_str())) {}
//
//         SerializableHardwareQualifier() : devclass(RCP_DEVCLASS_TEST_STATE), id(0), name(nullptr) {}
//
//         SerializableHardwareQualifier(const SerializableHardwareQualifier& o) :
//             devclass(o.devclass), id(o.id), name(deepCopyString(o.name)) {}
//
//         SerializableHardwareQualifier& operator=(const SerializableHardwareQualifier& o) {
//             if(&o == this) return *this;
//             delete[] name;
//
//             devclass = o.devclass;
//             id = o.id;
//             name = deepCopyString(o.name);
//
//             return *this;
//         }
//
//         ~SerializableHardwareQualifier() { delete[] name; }
//
//         // Used for ordering
//         auto operator<=>(const SerializableHardwareQualifier& rhf) const {
//             return std::tie(devclass, id) <=> std::tie(rhf.devclass, rhf.id);
//         }
//     };
//
//     struct SerializableError {
//         HostTime htime;
//         const char* errStr;
//         int level;
//
//         SerializableError(HostTime htime, const char* errStr, Error::Level level) :
//             htime(htime), errStr(deepCopyString(errStr)), level(level) {}
//         SerializableError(const SerializableError& o) :
//             htime(o.htime), errStr(deepCopyString(o.errStr)), level(o.level) {}
//         SerializableError(const Error& e) : SerializableError(e.htime, e.error.c_str(), e.level) {}
//         SerializableError& operator=(const SerializableError& o) {
//             if(&o == this) return *this;
//             htime = o.htime;
//             errStr = deepCopyString(o.errStr);
//             level = o.level;
//             return *this;
//         }
//         ~SerializableError() { delete[] errStr; }
//     };
//
//     struct SerializableReadRequest {
//         HostTime htime;
//         SerializableHardwareQualifier squal;
//
//         SerializableReadRequest(const std::pair<HostTime, HardwareQualifier>& rr) : htime(rr.first), squal(rr.second)
//         {}
//     };
//
//     struct SerializableHostString {
//         HostTime ctime;
//         const char* const pstr;
//     };
//
//     void setupTypes() {
//         // Just fact checking
//         static_assert(sizeof(RCP_DeviceClass) == 4);
//
//         varStrType = H5Tcopy(H5T_C_S1);
//         H5Tset_size(varStrType, H5T_VARIABLE);
//
//         qualType = H5Tcreate(H5T_COMPOUND, sizeof(SerializableHardwareQualifier));
//         H5Tinsert(qualType, "devclass", HOFFSET(SerializableHardwareQualifier, devclass), H5T_NATIVE_INT);
//         H5Tinsert(qualType, "id", HOFFSET(SerializableHardwareQualifier, id), H5T_NATIVE_UINT8);
//         H5Tinsert(qualType, "name", HOFFSET(SerializableHardwareQualifier, name), varStrType);
//
//         htimeType = H5Tcopy(H5T_NATIVE_UINT64);
//         ttimeType = H5Tcopy(H5T_NATIVE_UINT32);
//         ctimeType = H5Tcreate(H5T_COMPOUND, sizeof(CombinedTime));
//         H5Tinsert(ctimeType, "htime", HOFFSET(CombinedTime, htime), htimeType);
//         H5Tinsert(ctimeType, "ttime", HOFFSET(CombinedTime, ttime), ttimeType);
//
//         errType = H5Tcreate(H5T_COMPOUND, sizeof(SerializableError));
//         H5Tinsert(errType, "htime", HOFFSET(SerializableError, htime), htimeType);
//         H5Tinsert(errType, "errStr", HOFFSET(SerializableError, errStr), varStrType);
//         H5Tinsert(errType, "level", HOFFSET(SerializableError, level), H5T_NATIVE_INT);
//
//         readReqType = H5Tcreate(H5T_COMPOUND, sizeof(SerializableReadRequest));
//         H5Tinsert(readReqType, "htime", HOFFSET(SerializableReadRequest, htime), htimeType);
//         H5Tinsert(readReqType, "qual", HOFFSET(SerializableReadRequest, squal), qualType);
//     }
//
//     void releaseTypes() {
//         H5Tclose(readReqType);
//         H5Tclose(qualType);
//         H5Tclose(errType);
//         H5Tclose(varStrType);
//         H5Tclose(htimeType);
//         H5Tclose(ttimeType);
//         H5Tclose(ctimeType);
//     }
//
//
//
//     void hostHDF(const EventLog& log, hid_t file, const std::vector<SerializableHardwareQualifier>& quals) {
//         hid_t group = H5Gcreate(file, "host", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//
//         vectorIntoH5(group, "txdata", *log.getTXBytes(), H5T_NATIVE_UINT8);
//
//         std::vector<SerializableError> errors;
//         for(const auto& e : *log.getErrors()) errors.emplace_back(e);
//         vectorIntoH5(group, "errors", errors, errType);
//
//         std::vector<SerializableReadRequest> reads;
//         for(const auto& r : *log.getReadRequests()) reads.emplace_back(r);
//         vectorIntoH5(group, "readRequests", reads, readReqType);
//
//         hid_t testStateG = H5Gcreate(group, "0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//
//         HardwareChannel ch{RCP_DEVCLASS_TEST_STATE, 0, 0};
//         for(const auto& c : TestStateChannels::ALL_HCH) {
//             ch.channel = c;
//             hid_t channelGroup = H5Gcreate(testStateG, ToHex(c), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//
//             vectorIntoH5(channelGroup, "times", *log.getTestInformation(c).times, htimeType);
//             if(TestStateChannels::hostChannelHasMeta(c)) {
//                 vectorIntoH5(channelGroup, "data", *log.getTestInformation(c).values, H5T_NATIVE_UINT8);
//             }
//
//             H5Gclose(channelGroup);
//         }
//
//         H5Gclose(testStateG);
//
//         hid_t promptG = H5Gcreate(group, "3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//         EventLog::PromptResponse responses = log.getPromptResponses();
//         vectorIntoH5(promptG, "times", *responses.times, htimeType);
//         vectorIntoH5(promptG, "0", *responses.floatResp, H5T_NATIVE_FLOAT);
//         vectorIntoH5(promptG, "1", *responses.boolResp, H5T_NATIVE_UINT8);
//         H5Gclose(promptG);
//
//         H5Gclose(group);
//     }
//
//     void targetHDF(const EventLog& log, hid_t file, const std::vector<SerializableHardwareQualifier>& quals) {
//         hid_t targetG = H5Gcreate(file, "target", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//
//         hid_t group = H5Gcreate(targetG, "0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//         EventLog::TargetUint testData = log.getTestInformation(0);
//         hid_t space = spaceFromVector(*testData.times);
//         vectorIntoH5(group, "times", *testData.times, ctimeType, space);
//         for(const auto& c : TestStateChannels::ALL_TCH) {
//             testData = log.getTestInformation(c);
//             vectorIntoH5(group, ToHex(c), *testData.values, H5T_NATIVE_UINT8, space);
//         }
//         H5Sclose(space);
//         H5Gclose(group);
//
//         group = H5Gcreate(targetG, "3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//         std::string e;
//         e.c_str();
//     }
// } // namespace

namespace {
    using namespace LRI::RCI;

    hid_t vstr;
    hid_t htimeType;
    hid_t ttimeType;
    hid_t ctimeType;

    struct SerializableQual {
        uint8_t devclass;
        uint8_t id;
        const char* name;

        SerializableQual(const HardwareQualifier& q) : devclass(q.devclass), id(q.id), name(q.name.c_str()) {}

        auto operator<=>(const SerializableQual& rhs) const {
            return std::tie(devclass, id) <=> std::tie(rhs.devclass, rhs.id);
        }

        static hid_t htype;
    };

    hid_t SerializableQual::htype;

    template<typename T>
    hid_t spaceFromVector(const std::vector<T>& data) {
        hsize_t dims[1] = {data.size()};
        return H5Screate_simple(1, dims, nullptr);
    }

    template<typename T>
    void vectorIntoH5(hid_t destination, const char* name, const std::vector<T>& data, hid_t type, hid_t space) {
        hid_t dset = H5Dcreate(destination, name, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if(!data.empty()) H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data.at(0));
        H5Dclose(dset);
    }

    template<typename T>
    void vectorIntoH5(hid_t destination, const char* name, const std::vector<T>& data, hid_t type) {
        hid_t space = spaceFromVector(data);
        vectorIntoH5(destination, name, data, type, space);
        H5Sclose(space);
    }

    std::vector<const char*> cstrVector(const std::vector<std::string>& v) {
        std::vector<const char*> csv;
        csv.reserve(v.size());
        for(const auto& str : v) csv.push_back(str.c_str());
        return csv;
    }

    void setupTypes() {
        vstr = H5Tcopy(H5T_C_S1);
        H5Tset_size(vstr, H5T_VARIABLE);

        hid_t temp = H5Tcreate(H5T_COMPOUND, sizeof(SerializableQual));
        H5Tinsert(temp, "devclass", HOFFSET(SerializableQual, devclass), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "id", HOFFSET(SerializableQual, id), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "name", HOFFSET(SerializableQual, name), vstr);
        SerializableQual::htype = temp;

        htimeType = H5Tcopy(H5T_NATIVE_UINT64);
        ttimeType = H5Tcopy(H5T_NATIVE_UINT32);
        ctimeType = H5Tcreate(H5T_COMPOUND, sizeof(CombinedTime));
        H5Tinsert(ctimeType, "htime", HOFFSET(CombinedTime, htime), htimeType);
        H5Tinsert(ctimeType, "ttime", HOFFSET(CombinedTime, ttime), ttimeType);
    }

    void releaseTypes() {
        H5Tclose(SerializableQual::htype);
        H5Tclose(vstr);

        H5Tclose(ctimeType);
        H5Tclose(ttimeType);
        H5Tclose(htimeType);
    }

    void hostHDF(const EventLog& log, hid_t file, const std::vector<SerializableQual>& quals) {
        hid_t hostG = H5Gcreate(file, "host", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        vectorIntoH5(hostG, "txdata", *log.getTXBytes(), H5T_NATIVE_UINT8);

        H5Gclose(hostG);
    }

    template<typename U>
    struct DevclassTypeInfo {};

    template<>
    struct DevclassTypeInfo<uint8_t> {
        static hid_t htype() { return H5T_NATIVE_UINT8; }
        static EventLog::TargetUint elogGetter(const EventLog& log, const HardwareChannel& ch) {
            return log.getChannelUintData(ch);
        }
    };

    template<>
    struct DevclassTypeInfo<float> {
        static hid_t htype() { return H5T_NATIVE_FLOAT; }
        static EventLog::TargetFloat elogGetter(const EventLog& log, const HardwareChannel& ch) {
            return log.getChannelFloatData(ch);
        }
    };

    template<typename T>
    void hdfDevclass(const EventLog& log, hid_t dcGroup, const SerializableQual& qual) {
        HardwareChannel ch = {static_cast<RCP_DeviceClass>(qual.devclass), qual.id, 0};
        auto data = DevclassTypeInfo<T>::elogGetter(log, ch);

        hid_t idg = H5Gcreate(dcGroup, U8ToHex(qual.id), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        hid_t space = spaceFromVector(*data.times);
        vectorIntoH5(idg, "times", *data.times, ctimeType, space);

        while(data.times != nullptr) {
            vectorIntoH5(idg, ToHex(ch.channel), *data.values, DevclassTypeInfo<T>::htype(), space);
            ch.channel++;
            data = DevclassTypeInfo<T>::elogGetter(log, ch);
        }

        H5Sclose(space);
        H5Gclose(idg);
    }

    void targetHDF(const EventLog& log, hid_t file, const std::vector<SerializableQual>& quals) {
        hid_t targetG = H5Gcreate(file, "target", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        vectorIntoH5(targetG, "rxdata", *log.getRXBytes(), H5T_NATIVE_UINT8);

        hid_t group = H5Gcreate(targetG, "00", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        // Since all channels have the same number of elements, we can make a common space from the times array
        EventLog::TargetUint testData = log.getTestInformation(0);
        hid_t space = spaceFromVector(*testData.times);
        vectorIntoH5(group, "times", *testData.times, ctimeType, space);
        for(const auto& c : TestStateChannels::ALL_TCH) {
            testData = log.getTestInformation(c);
            vectorIntoH5(group, ToHex(c), *testData.values, H5T_NATIVE_UINT8, space);
        }
        H5Sclose(space);
        H5Gclose(group);

        group = H5Gcreate(targetG, "03", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        EventLog::HostString hsdata = log.getPromptRequests();
        space = spaceFromVector(*hsdata.times);
        vectorIntoH5(group, "times", *hsdata.times, htimeType, space);
        vectorIntoH5(group, "00", cstrVector(*hsdata.values), vstr, space);
        H5Sclose(space);
        H5Gclose(group);

        group = H5Gcreate(targetG, "80", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        hsdata = log.getLogs();
        space = spaceFromVector(*hsdata.times);
        vectorIntoH5(group, "times", *hsdata.times, htimeType, space);
        vectorIntoH5(group, "00", cstrVector(*hsdata.values), vstr, space);
        H5Sclose(space);

        // Ordinarily we would need to close the group, however the loop below will close the group on the first
        // iteration
        // H5Gclose(group);

        uint8_t prevclass = RCP_DEVCLASS_AMALGAMATE;
        for(size_t i = 0; i < quals.size(); i++) {
            const auto& qual = quals[i];
            // Since quals is sorted based on devclass first, we will have all quals of a particular class in a
            // contiguous stream

            if(qual.devclass != prevclass) {
                H5Gclose(group);
                group = H5Gcreate(targetG, ToHex(qual.devclass), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                prevclass = qual.devclass;
            }

            // The worst switch youve ever seen, all to dispatch based on the underlying devclass type
            switch(qual.devclass) {
            case RCP_DEVCLASS_DISCRETE_ACTUATOR:
            case RCP_DEVCLASS_BOOL_SENSOR:
                hdfDevclass<uint8_t>(log, group, qual);
                break;

            case RCP_DEVCLASS_STEPPER:
            case RCP_DEVCLASS_ANGLED_ACTUATOR:
            case RCP_DEVCLASS_MOTOR:
            case RCP_DEVCLASS_AM_PRESSURE:
            case RCP_DEVCLASS_TEMPERATURE:
            case RCP_DEVCLASS_PRESSURE_TRANSDUCER:
            case RCP_DEVCLASS_RELATIVE_HYGROMETER:
            case RCP_DEVCLASS_LOAD_CELL:
            case RCP_DEVCLASS_FLOW_METER:
            case RCP_DEVCLASS_ALTITUDE:
            case RCP_DEVCLASS_RADIO_STRENGTH:
            case RCP_DEVCLASS_POWERMON:
            case RCP_DEVCLASS_ACCELEROMETER:
            case RCP_DEVCLASS_GYROSCOPE:
            case RCP_DEVCLASS_MAGNETOMETER:
            case RCP_DEVCLASS_RPY:
            case RCP_DEVCLASS_GPS:
            case RCP_DEVCLASS_QUATERNION:
                hdfDevclass<float>(log, group, qual);
                break;

            default:
                continue;
            }
        }

        H5Gclose(group);

        H5Gclose(targetG);
    }
} // namespace

namespace LRI::RCI {
    EventLog::EventLog(const std::string& hdf) { setupTypes(); }

    void EventLog::serialize(const std::string& hdf) const {
        std::filesystem::path hdfPath = getRoamingFolder() / "exports" / hdf;

        setupTypes();

        hid_t file = H5Fcreate(hdfPath.string().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

        std::vector<SerializableQual> squals;
        squals.reserve(registeredQuals.size());
        for(auto& qual : registeredQuals) squals.emplace_back(qual);
        std::sort(squals.begin(), squals.end());

        // Create /registeredQuals, which is an array storing all non-implicit qualifiers for reference
        vectorIntoH5(file, "registeredQuals", squals, SerializableQual::htype);

        hostHDF(*this, file, squals);
        targetHDF(*this, file, squals);

        H5Fclose(file);

        releaseTypes();
    }
} // namespace LRI::RCI

/*
HDF Format:
root/
 |- registeredQuals (qual type)
 |- host/
   |- txdata (uint8)
   |- errors
   |- readRequests
   |- tares/ - missing for now
     |- times (htime)
     |- offsets (float)
   |- 0/
     |- <for each test state channel>/
       |- times (htime)
       |- data (if necessary, uint8)
   |- 3/
     |- times (htime)
     |- float response
     |- uint response
   |- <all other device classes>/
     |- times (htime)
     |- control values
 |- target/
   |- rxdata (uint8)
   |- 0/
     |- <for each test state channel>
       |- times (ctime)
       |- value
   |- 3/
     |- times (ctime)
     |- value (string)
   |- 80/
     |- times (ctime)
     |- value (string)
   |- <for all other classes>/
     |- times (ctime)
     |- value (uint, float)




vectorIntoH5(hostDataG, "rxdata", rxbytes, H5T_NATIVE_UINT8);
        vectorIntoH5(hostDataG, "txdata", txbytes, H5T_NATIVE_UINT8);

        {
            // Insert test state data
            hid_t testStateG = H5Gcreate(hostDataG, "0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

            HardwareChannel ch{RCP_DEVCLASS_TEST_STATE, 0, 0};
            for(const auto& c : TestStateChannels::ALL_HCH) {
                ch.channel = c;

                auto& times = host.ctrltimestamps.at(ch);
                space = spaceFromVector(times);

                vectorIntoH5(testStateG, "times", times, htimeType, space);

                // None of these have associated metadata
                if(c == TestStateChannels::H_HEARTBEAT || c == TestStateChannels::H_DEV_RESET ||
                   c == TestStateChannels::H_TIME_RESET)
                    continue;

                auto& data = host.ctrl_uints.at(ch);
                vectorIntoH5(testStateG, ToHex(c), data, H5T_NATIVE_UINT8, space);

                H5Sclose(space);
            }

            H5Gclose(testStateG);
        }


        {
            // Insert prompt data
            hid_t promptG = H5Gcreate(hostDataG, "3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        }

        H5Gclose(hostDataG);

        hid_t targetDataG = H5Gcreate(file, "TargetData", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        {
            // Insert target log data
            hid_t targetLogG = H5Gcreate(hostDataG, "80", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            HardwareQualifier q{RCP_DEVCLASS_TARGET_LOG, 0};

            vectorIntoH5(targetLogG, "times", host.acttimestamps.at(q), htimeType);
            vectorIntoH5(targetLogG, "0", host.strings.at(q), varStrType);

            H5Gclose(targetLogG);
        }

        H5Gclose(targetDataG);
 */
