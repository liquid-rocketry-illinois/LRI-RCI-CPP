#include "hardware/EventLog.h"

#include <filesystem>
#include <format>
#include "utils.h"

// These functions are placed into their own file to concentrate all the HDF shenanigans in one place

#include "hdf5.h"

#define U8ToHex(num) std::format("{:02X}", num).c_str()
#define ToHex(devclass) U8ToHex(static_cast<uint8_t>(devclass))

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

    struct SerializableError {
        HostTime htime;
        int level;
        const char* const str;
        static hid_t htype;

        SerializableError(const Error& e) : htime(e.htime), level(e.level), str(e.error.c_str()) {}
    };

    hid_t SerializableError::htype;

    struct SerializableReadReq {
        HostTime htime;
        uint8_t devclass;
        uint8_t id;
        SerializableReadReq(const std::pair<HostTime, HardwareQualifier>& rr) :
            htime(rr.first), devclass(rr.second.devclass), id(rr.second.id) {}
        static hid_t htype;
    };

    hid_t SerializableReadReq::htype;

    struct SerializableTare {
        HostTime htime;
        uint8_t devclass;
        uint8_t id;
        uint8_t channel;
        float off;
        SerializableTare(const TareReq& t) :
            htime(t.htime), devclass(t.ch.devclass), id(t.ch.id), channel(t.ch.channel), off(t.offset) {}
        static hid_t htype;
    };

    hid_t SerializableTare::htype;

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

        temp = H5Tcreate(H5T_COMPOUND, sizeof(SerializableError));
        H5Tinsert(temp, "htime", HOFFSET(SerializableError, htime), htimeType);
        H5Tinsert(temp, "level", HOFFSET(SerializableError, level), H5T_NATIVE_INT);
        H5Tinsert(temp, "str", HOFFSET(SerializableError, str), vstr);
        SerializableError::htype = temp;

        temp = H5Tcreate(H5T_COMPOUND, sizeof(SerializableReadReq));
        H5Tinsert(temp, "htime", HOFFSET(SerializableReadReq, htime), htimeType);
        H5Tinsert(temp, "devclass", HOFFSET(SerializableReadReq, devclass), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "id", HOFFSET(SerializableReadReq, id), H5T_NATIVE_UINT8);
        SerializableReadReq::htype = temp;

        temp = H5Tcreate(H5T_COMPOUND, sizeof(SerializableTare));
        H5Tinsert(temp, "htime", HOFFSET(SerializableTare, htime), htimeType);
        H5Tinsert(temp, "devclass", HOFFSET(SerializableTare, devclass), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "id", HOFFSET(SerializableTare, id), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "channel", HOFFSET(SerializableTare, channel), H5T_NATIVE_UINT8);
        H5Tinsert(temp, "offset", HOFFSET(SerializableTare, off), H5T_NATIVE_FLOAT);
        SerializableTare::htype = temp;
    }

    void releaseTypes() {
        H5Tclose(SerializableQual::htype);
        H5Tclose(SerializableError::htype);
        H5Tclose(SerializableReadReq::htype);
        H5Tclose(SerializableTare::htype);
        H5Tclose(vstr);

        H5Tclose(ctimeType);
        H5Tclose(ttimeType);
        H5Tclose(htimeType);
    }

    void hostHDF(const EventLog& log, hid_t file, const std::vector<SerializableQual>& quals) {
        hid_t hostG = H5Gcreate(file, "host", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        vectorIntoH5(hostG, "txdata", *log.getTXBytes(), H5T_NATIVE_UINT8);

        std::vector<SerializableError> serrs;
        const EventLog::ErrorList* el = log.getErrors();
        serrs.reserve(el->size());
        for(const auto& e : *el) serrs.emplace_back(e);
        vectorIntoH5(hostG, "errors", serrs, SerializableError::htype);

        std::vector<SerializableReadReq> reqs;
        const EventLog::ReadRequestsList* rr = log.getReadRequests();
        reqs.reserve(rr->size());
        for(const auto& r : *rr) reqs.emplace_back(r);
        vectorIntoH5(hostG, "readRequests", reqs, SerializableReadReq::htype);

        std::vector<SerializableTare> treqs;
        const EventLog::TareList* tl = log.getRequestedTares();
        treqs.reserve(tl->size());
        for(const auto& t : *tl) treqs.emplace_back(t);
        vectorIntoH5(hostG, "tares", treqs, SerializableTare::htype);

        hid_t group = H5Gcreate(hostG, "00", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        for(const auto& c : TestStateChannels::ALL_HCH) {
            if(c == TestStateChannels::H_TEST_ID) continue;
            hid_t idg = H5Gcreate(group, ToHex(c), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            EventLog::HostUint hudata = log.getHostTestStateChannel(c);
            hid_t space = spaceFromVector(*hudata.times);
            vectorIntoH5(idg, "times", *hudata.times, htimeType, space);
            if(hudata.values != nullptr) vectorIntoH5(idg, "values", *hudata.values, H5T_NATIVE_UINT8, space);
            H5Sclose(space);

            if(c == TestStateChannels::H_TEST_RUN_STATE)
                vectorIntoH5(idg, "testID", *log.getRequestedTestStartIDs(), H5T_NATIVE_UINT8);
            H5Gclose(idg);
        }
        H5Gclose(group);

        group = H5Gcreate(hostG, "03", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        EventLog::PromptResponse prs = log.getPromptResponses();
        hid_t space = spaceFromVector(*prs.times);
        vectorIntoH5(group, "times", *prs.times, htimeType, space);
        vectorIntoH5(group, "00", *prs.floatResp, H5T_NATIVE_FLOAT, space);
        vectorIntoH5(group, "01", *prs.boolResp, H5T_NATIVE_UINT8, space);
        H5Sclose(space);

        // Ordinarily we would need to close the group, however the loop below will close the group on the first
        // iteration
        // H5Gclose(group);

        uint8_t prevClass = RCP_DEVCLASS_AMALGAMATE;
        for(size_t i = 0; i < quals.size(); i++) {
            auto& qual = quals[i];

            // For now, only devclasses with values < 0x80 are writable
            if(qual.devclass >= RCP_DEVCLASS_TARGET_LOG) continue;

            if(qual.devclass != prevClass) {
                H5Gclose(group);
                group = H5Gcreate(hostG, ToHex(qual.devclass), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                prevClass = qual.devclass;
            }

            hid_t idg = H5Gcreate(group, ToHex(qual.id), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

            switch(qual.devclass) {
            case RCP_DEVCLASS_DISCRETE_ACTUATOR: {
                EventLog::HostUint hudata = log.getRequestedDActWrites(qual.id);
                space = spaceFromVector(*hudata.times);
                vectorIntoH5(idg, "times", *hudata.times, htimeType, space);
                vectorIntoH5(idg, "00", *hudata.values, H5T_NATIVE_UINT8, space);
                H5Sclose(space);
                break;
            }

            case RCP_DEVCLASS_STEPPER: {
                EventLog::StepperWritesList sdata = log.getRequestedStepperWrites(qual.id);
                space = spaceFromVector(*sdata.times);
                vectorIntoH5(idg, "times", *sdata.times, htimeType, space);
                vectorIntoH5(idg, "00", *sdata.modes, H5T_NATIVE_UINT8, space);
                vectorIntoH5(idg, "01", *sdata.values, H5T_NATIVE_FLOAT, space);
                H5Sclose(space);
                break;
            }

            case RCP_DEVCLASS_ANGLED_ACTUATOR:
            case RCP_DEVCLASS_MOTOR: {
                EventLog::HostFloat fdata =
                    (qual.devclass == RCP_DEVCLASS_ANGLED_ACTUATOR ? log.getRequestedAActWrites(qual.id)
                                                                   : log.getRequestedMotorWrites(qual.id));
                space = spaceFromVector(*fdata.times);
                vectorIntoH5(idg, "times", *fdata.times, htimeType, space);
                vectorIntoH5(idg, "00", *fdata.values, H5T_NATIVE_FLOAT, space);
                H5Sclose(space);
                break;
            }

            default:
                break;
            }

            H5Gclose(idg);
        }

        H5Gclose(group);

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
 */
