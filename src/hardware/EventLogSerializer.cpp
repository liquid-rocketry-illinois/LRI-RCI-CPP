#include "hardware/EventLog.h"

#include <filesystem>
#include <format>
#include "utils.h"

// These functions are placed into their own file to concentrate all the HDF shenanigans in one place

#include "hdf5.h"

#define U8ToHex(num) std::format("{:02X}", num).c_str()
#define ToHex(devclass) U8ToHex(static_cast<uint8_t>(devclass))

#define QUALS_DSET_NAME "registeredQuals"
#define TCONF_DSET_NAME "tconfig"
#define error(msg) throw std::runtime_error(std::format("\"{}\" at {}", msg, __LINE__))

using namespace LRI::RCI::TestStateChannels;

namespace {
    using namespace LRI::RCI;

    hid_t vstr;
    hid_t htimeType;
    hid_t ttimeType;
    hid_t ctimeType;

    void checkTypes(hid_t dset, hid_t type, int location) {
        hid_t dtype = H5Dget_type(dset);
        if(!H5Tequal(dtype, type)) throw std::runtime_error(std::format("Type mismatch at: {}", location));
    }

#define TYPECHECK(dset, type) checkTypes(dset, type, __LINE__)

    struct SerializableQual {
        uint8_t devclass;
        uint8_t id;
        const char* name;

        SerializableQual(const HardwareQualifier& q) : devclass(q.devclass), id(q.id), name(q.name.c_str()) {}
        SerializableQual() : devclass(0), id(0), name(nullptr) {}
        [[nodiscard]] HardwareQualifier toQual() const { return {static_cast<RCP_DeviceClass>(devclass), id, name}; }

        auto operator<=>(const SerializableQual& rhs) const {
            return std::tie(devclass, id) <=> std::tie(rhs.devclass, rhs.id);
        }

        static hid_t htype;
    };

    hid_t SerializableQual::htype;

    struct SerializableError {
        HostTime htime;
        int level;
        const char* str;
        static hid_t htype;

        SerializableError(const Error& e) : htime(e.htime), level(e.level), str(e.error.c_str()) {}
        SerializableError() : htime(0), level(0), str(nullptr) {}
        [[nodiscard]] Error toError() const { return {static_cast<Error::Level>(level), str, htime}; }
    };

    hid_t SerializableError::htype;

    struct SerializableReadReq {
        HostTime htime;
        uint8_t devclass;
        uint8_t id;
        SerializableReadReq(const std::pair<HostTime, HardwareQualifier>& rr) :
            htime(rr.first), devclass(rr.second.devclass), id(rr.second.id) {}
        SerializableReadReq() : htime(0), devclass(0), id(0) {}
        [[nodiscard]] std::pair<HostTime, HardwareQualifier> toReadReq() const {
            return {htime, {static_cast<RCP_DeviceClass>(devclass), id}};
        }
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
        SerializableTare() : htime(0), devclass(0), id(0), channel(0), off(0) {}
        [[nodiscard]] TareReq toTareReq() const {
            return {htime, {static_cast<RCP_DeviceClass>(devclass), id, channel}, off};
        }
        static hid_t htype;
    };

    hid_t SerializableTare::htype;

    // Wrapped hid_t struct to automatically close stuff
    using HidCloseFunction = herr_t (*)(hid_t);
    template<HidCloseFunction cfunc>
    struct Whid_t {
        hid_t hid;
        Whid_t(hid_t hid) : hid(hid) {}
        ~Whid_t() {
            if(hid >= 0) cfunc(hid);
        }
        Whid_t(const Whid_t& other) = delete;
        Whid_t& operator=(const Whid_t& other) = delete;
        Whid_t& operator=(hid_t newhid) {
            if(hid >= 0) cfunc(hid);
            hid = newhid;
            return *this;
        }

        hid_t operator()() const { return hid; }
    };

    using DSet = Whid_t<H5Dclose>;
    using Space = Whid_t<H5Sclose>;
    using Type = Whid_t<H5Tclose>;
    using Group = Whid_t<H5Gclose>;
    using File = Whid_t<H5Fclose>;

    template<typename T>
    hid_t spaceFromVector(const std::vector<T>& data) {
        hsize_t dims[1] = {data.size()};
        return H5Screate_simple(1, dims, nullptr);
    }

    template<typename T>
    void vectorIntoH5(hid_t destination, const char* name, const std::vector<T>& data, hid_t type, hid_t space) {
        DSet dset = H5Dcreate(destination, name, type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if(!data.empty()) H5Dwrite(dset(), type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data.at(0));
    }

    template<typename T>
    void vectorIntoH5(hid_t destination, const char* name, const std::vector<T>& data, hid_t type) {
        Space space = spaceFromVector(data);
        vectorIntoH5(destination, name, data, type, space());
    }

    template<typename T>
    void vectorFromH5(hid_t dset, std::vector<T>& data, hid_t type) {
        TYPECHECK(dset, type);

        Space space = H5Dget_space(dset);
        hsize_t dim = H5Sget_simple_extent_ndims(space());
        if(dim != 1) error("Rank is not 1");
        H5Sget_simple_extent_dims(space(), &dim, nullptr);

        if(dim > 0) {
            data.assign(dim, T{});
            H5Dread(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0]);
        }
    }

    template<typename T>
    void vectorFromH5(hid_t location, const char* name, std::vector<T>& data, hid_t type) {
        DSet dset = H5Dopen(location, name, H5P_DEFAULT);
        if(dset() < 0) error(std::format("Cant open dset {}", name));
        vectorFromH5(dset(), data, type);
    }

    // String vectors need a specialization
    template<>
    void vectorFromH5<std::string>(hid_t location, const char* name, std::vector<std::string>& data, hid_t type) {
        DSet dset = H5Dopen(location, name, H5P_DEFAULT);
        if(dset() < 0) error("Cant open dset 00");
        TYPECHECK(dset(), vstr);
        Space space = H5Dget_space(dset());
        hsize_t dim = H5Sget_simple_extent_ndims(space());
        if(dim != 1) error("Rank is not 1");
        H5Sget_simple_extent_dims(space(), &dim, nullptr);
        data.reserve(dim);
        if(dim > 0) {
            std::vector<const char*> cstrv;
            cstrv.assign(dim, nullptr);
            H5Dread(dset(), vstr, H5S_ALL, H5S_ALL, H5P_DEFAULT, &cstrv[0]);
            for(const auto& cs : cstrv) data.emplace_back(cs);
            H5Treclaim(vstr, space(), H5P_DEFAULT, &cstrv[0]);
        }
    }

    std::vector<const char*> cstrVector(const std::vector<std::string>& v) {
        std::vector<const char*> csv;
        csv.reserve(v.size());
        for(const auto& str : v) csv.push_back(str.c_str());
        return csv;
    }

    void strIntoH5(hid_t dest, const char* name, const std::string& str) {
        hsize_t dims[1] = {1};
        Space space = H5Screate_simple(1, dims, nullptr);
        DSet dset = H5Dcreate(dest, name, vstr, space(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        const char* strptr = str.c_str();
        H5Dwrite(dset(), vstr, H5S_ALL, H5S_ALL, H5P_DEFAULT, &strptr);
    }

    void strFromH5(hid_t dset, std::string& str) {
        Space space = H5Dget_space(dset);
        hsize_t dim = H5Sget_simple_extent_ndims(space());
        if(dim != 1) error("Rank is not 1");
        H5Sget_simple_extent_dims(space(), &dim, nullptr);
        if(dim != 1) error("String dset size is not 1");
        char* rstr;
        H5Dread(dset, vstr, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rstr);
        str = std::string(rstr, strlen(rstr));
        H5Treclaim(vstr, space(), H5P_DEFAULT, &rstr);
    }

    void strFromH5(hid_t location, const char* name, std::string& str) {
        DSet dset = H5Dopen(location, name, H5P_DEFAULT);
        strFromH5(dset(), str);
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
} // namespace

namespace {
    void hostHDFSerialize(const EventLog& log, hid_t file, const std::vector<SerializableQual>& quals) {
        Group hostG = H5Gcreate(file, "host", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        vectorIntoH5(hostG(), "txdata", *log.getTXBytes(), H5T_NATIVE_UINT8);

        std::vector<SerializableError> serrs;
        const EventLog::ErrorList* el = log.getErrors();
        serrs.reserve(el->size());
        for(const auto& e : *el) serrs.emplace_back(e);
        vectorIntoH5(hostG(), "errors", serrs, SerializableError::htype);

        std::vector<SerializableReadReq> reqs;
        const EventLog::ReadRequestsList* rr = log.getReadRequests();
        reqs.reserve(rr->size());
        for(const auto& r : *rr) reqs.emplace_back(r);
        vectorIntoH5(hostG(), "readRequests", reqs, SerializableReadReq::htype);

        std::vector<SerializableTare> treqs;
        const EventLog::TareList* tl = log.getRequestedTares();
        treqs.reserve(tl->size());
        for(const auto& t : *tl) treqs.emplace_back(t);
        vectorIntoH5(hostG(), "tares", treqs, SerializableTare::htype);

        Group group = H5Gcreate(hostG(), "00", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        for(const auto& c : ALL_HCH) {
            if(c == H_TEST_ID) continue;
            Group idg = H5Gcreate(group(), ToHex(c), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            EventLog::HostUint hudata = log.getHostTestStateChannel(c);
            Space space = spaceFromVector(*hudata.times);
            vectorIntoH5(idg(), "times", *hudata.times, htimeType, space());
            if(hudata.values != nullptr) vectorIntoH5(idg(), "values", *hudata.values, H5T_NATIVE_UINT8, space());

            if(c == H_TEST_RUN_STATE) vectorIntoH5(idg(), "testID", *log.getRequestedTestStartIDs(), H5T_NATIVE_UINT8);
        }

        group = H5Gcreate(hostG(), "03", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        EventLog::PromptResponse prs = log.getPromptResponses();
        Space space = spaceFromVector(*prs.times);
        vectorIntoH5(group(), "times", *prs.times, htimeType, space());
        vectorIntoH5(group(), "00", *prs.floatResp, H5T_NATIVE_FLOAT, space());
        vectorIntoH5(group(), "01", *prs.boolResp, H5T_NATIVE_UINT8, space());

        uint8_t prevClass = RCP_DEVCLASS_AMALGAMATE;
        for(size_t i = 0; i < quals.size(); i++) {
            auto& qual = quals[i];

            // For now, only devclasses with values < 0x80 are writable
            if(qual.devclass >= RCP_DEVCLASS_TARGET_LOG) continue;

            if(qual.devclass != prevClass) {
                group = H5Gcreate(hostG(), ToHex(qual.devclass), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                prevClass = qual.devclass;
            }

            Group idg = H5Gcreate(group(), ToHex(qual.id), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

            switch(qual.devclass) {
            case RCP_DEVCLASS_DISCRETE_ACTUATOR: {
                EventLog::HostUint hudata = log.getRequestedDActWrites(qual.id);
                space = spaceFromVector(*hudata.times);
                vectorIntoH5(idg(), "times", *hudata.times, htimeType, space());
                vectorIntoH5(idg(), "00", *hudata.values, H5T_NATIVE_UINT8, space());
                break;
            }

            case RCP_DEVCLASS_STEPPER: {
                EventLog::StepperWritesList sdata = log.getRequestedStepperWrites(qual.id);
                space = spaceFromVector(*sdata.times);
                vectorIntoH5(idg(), "times", *sdata.times, htimeType, space());
                vectorIntoH5(idg(), "00", *sdata.modes, H5T_NATIVE_UINT8, space());
                vectorIntoH5(idg(), "01", *sdata.values, H5T_NATIVE_FLOAT, space());
                break;
            }

            case RCP_DEVCLASS_ANGLED_ACTUATOR:
            case RCP_DEVCLASS_MOTOR: {
                EventLog::HostFloat fdata =
                    (qual.devclass == RCP_DEVCLASS_ANGLED_ACTUATOR ? log.getRequestedAActWrites(qual.id)
                                                                   : log.getRequestedMotorWrites(qual.id));
                space = spaceFromVector(*fdata.times);
                vectorIntoH5(idg(), "times", *fdata.times, htimeType, space());
                vectorIntoH5(idg(), "00", *fdata.values, H5T_NATIVE_FLOAT, space());
                break;
            }

            default:
                break;
            }
        }
    }

    void targetHDFSerialize(const EventLog& log, hid_t file, const std::vector<SerializableQual>& quals) {
        Group targetG = H5Gcreate(file, "target", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        vectorIntoH5(targetG(), "rxdata", *log.getRXBytes(), H5T_NATIVE_UINT8);

        Group group = H5Gcreate(targetG(), "00", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        // Since all channels have the same number of elements, we can make a common space from the times array
        EventLog::TargetUint testData = log.getTestInformation(0);
        Space space = spaceFromVector(*testData.times);
        vectorIntoH5(group(), "times", *testData.times, ctimeType, space());
        for(const auto& c : ALL_TCH) {
            testData = log.getTestInformation(c);
            vectorIntoH5(group(), ToHex(c), *testData.values, H5T_NATIVE_UINT8, space());
        }

        group = H5Gcreate(targetG(), "03", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        EventLog::HostString hsdata = log.getPromptRequests();
        space = spaceFromVector(*hsdata.times);
        vectorIntoH5(group(), "times", *hsdata.times, htimeType, space());
        vectorIntoH5(group(), "00", cstrVector(*hsdata.values), vstr, space());

        group = H5Gcreate(targetG(), "80", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        hsdata = log.getLogs();
        space = spaceFromVector(*hsdata.times);
        vectorIntoH5(group(), "times", *hsdata.times, htimeType, space());
        vectorIntoH5(group(), "00", cstrVector(*hsdata.values), vstr, space());

        // Ordinarily we would need to close the group, however the loop below will close the group on the first
        // iteration
        // H5Gclose(group);

        uint8_t prevclass = RCP_DEVCLASS_AMALGAMATE;
        for(size_t i = 0; i < quals.size(); i++) {
            const auto& qual = quals[i];
            // Since quals is sorted based on devclass first, we will have all quals of a particular class in a
            // contiguous stream

            if(qual.devclass != prevclass) {
                group = H5Gcreate(targetG(), ToHex(qual.devclass), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                prevclass = qual.devclass;
            }

            Group idg = H5Gcreate(group(), U8ToHex(qual.id), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

            // The worst switch youve ever seen, all to dispatch based on the underlying devclass type
            switch(qual.devclass) {
            case RCP_DEVCLASS_DISCRETE_ACTUATOR:
            case RCP_DEVCLASS_BOOL_SENSOR: {
                HardwareChannel ch = {static_cast<RCP_DeviceClass>(qual.devclass), qual.id, 0};
                EventLog::TargetUint data = log.getChannelUintData(ch);
                space = spaceFromVector(*data.times);
                vectorIntoH5(idg(), "times", *data.times, ctimeType, space());

                while(data.times != nullptr) {
                    vectorIntoH5(idg(), ToHex(ch.channel), *data.values, H5T_NATIVE_UINT8, space());
                    ch.channel++;
                    data = log.getChannelUintData(ch);
                }

                break;
            }

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
            case RCP_DEVCLASS_QUATERNION: {
                HardwareChannel ch = {static_cast<RCP_DeviceClass>(qual.devclass), qual.id, 0};
                EventLog::TargetFloat data = log.getChannelFloatData(ch);
                space = spaceFromVector(*data.times);
                vectorIntoH5(idg(), "times", *data.times, ctimeType, space());

                while(data.times != nullptr) {
                    vectorIntoH5(idg(), ToHex(ch.channel), *data.values, H5T_NATIVE_FLOAT, space());
                    ch.channel++;
                    data = log.getChannelFloatData(ch);
                }

                break;
            }

            default:
                continue;
            }
        }
    }
} // namespace

namespace LRI::RCI {
    // Unfortunately it is very hard to distribute the deserializaer to static helpers, because friends are hard
    EventLog::EventLog(const std::filesystem::path& hdf) : EventLog() {
        setupTypes();

        File file = H5Fopen(hdf.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        if(file() < 0) error("Failed to open file");

        std::vector<SerializableQual> squals;
        vectorFromH5(file(), QUALS_DSET_NAME, squals, SerializableQual::htype);
        for(const auto& sq : squals) createDevice(sq.toQual());

        strFromH5(file(), TCONF_DSET_NAME, jsonText);


        // Host group data
        {
            Group hostG = H5Gopen(file(), "host", H5P_DEFAULT);
            if(hostG() < 0) error("No host group");

            vectorFromH5(hostG(), "txdata", txbytes, H5T_NATIVE_UINT8);

            std::vector<SerializableError> serrs;
            vectorFromH5(hostG(), "errors", serrs, SerializableError::htype);
            errors.reserve(serrs.size());
            for(const auto& se : serrs) errors.emplace_back(se.toError());

            std::vector<SerializableReadReq> sreqs;
            vectorFromH5(hostG(), "readRequests", sreqs, SerializableReadReq::htype);
            host.readReqs.reserve(sreqs.size());
            for(const auto& rr : sreqs) host.readReqs.emplace_back(rr.toReadReq());

            std::vector<SerializableTare> stare;
            vectorFromH5(hostG(), "tares", stare, SerializableTare::htype);
            host.tares.reserve(stare.size());
            for(const auto& st : stare) host.tares.emplace_back(st.toTareReq());

            Group group = H5Gopen(hostG(), "00", H5P_DEFAULT);
            if(group() < 0) error("No 00 group");
            for(const auto& c : ALL_HCH) {
                if(c == H_TEST_ID) continue;
                Group idg = H5Gopen(group(), ToHex(c), H5P_DEFAULT);
                if(idg() < 0) error(std::format("Could not open 00 group {}", ToHex(c)));

                switch(c) {
                case H_TEST_RUN_STATE:
                    vectorFromH5(idg(), "testID", host.ctrl_uints[TESTID], H5T_NATIVE_UINT8);
                    [[fallthrough]];

                case H_DSTREAM:
                case H_HEARTBEAT_TIME: {
                    HardwareChannel ch = {TEST_STATE, c};
                    vectorFromH5(idg(), "values", host.ctrl_uints[ch], H5T_NATIVE_UINT8);
                    [[fallthrough]];
                }

                case H_HEARTBEAT:
                case H_TIME_RESET:
                case H_DEV_RESET: {
                    HardwareChannel ch = {TEST_STATE, c};
                    vectorFromH5(idg(), "times", host.ctrltimestamps[ch], htimeType);
                    break;
                }

                default:
                    break;
                }
            }

            group = H5Gopen(hostG(), "03", H5P_DEFAULT);
            if(group() < 0) error("No 03 group");
            vectorFromH5(group(), "times", host.ctrltimestamps[PROMPT], htimeType);
            vectorFromH5(group(), "00", host.act_floats[PROMPT], H5T_NATIVE_FLOAT);
            vectorFromH5(group(), "01", host.act_uints[PROMPT], H5T_NATIVE_UINT8);

            for(const auto& qual : registeredQuals) {
                if(qual.devclass >= RCP_DEVCLASS_TARGET_LOG) continue;

                group = H5Gopen(hostG(), ToHex(qual.devclass), H5P_DEFAULT);
                if(group() < 0) error(std::format("No {} group", ToHex(qual.devclass)));

                Group idg = H5Gopen(group(), ToHex(qual.id), H5P_DEFAULT);
                if(idg() < 0) error(std::format("No host entry for {}", qual));

                vectorFromH5(idg(), "times", host.acttimestamps[qual], htimeType);

                switch(qual.devclass) {
                case RCP_DEVCLASS_DISCRETE_ACTUATOR: {
                    vectorFromH5(idg(), "00", host.act_uints[qual], H5T_NATIVE_UINT8);
                    break;
                }

                case RCP_DEVCLASS_STEPPER: {
                    vectorFromH5(idg(), "00", host.act_uints[qual], H5T_NATIVE_UINT8);
                    vectorFromH5(idg(), "01", host.act_floats[qual], H5T_NATIVE_FLOAT);
                    break;
                }

                case RCP_DEVCLASS_ANGLED_ACTUATOR:
                case RCP_DEVCLASS_MOTOR: {
                    vectorFromH5(idg(), "00", host.act_floats[qual], H5T_NATIVE_FLOAT);
                    break;
                }

                default:
                    break;
                }
            }
        }

        // Target group data
        {
            Group targetG = H5Gopen(file(), "target", H5P_DEFAULT);
            if(targetG() < 0) error("No target group");

            vectorFromH5(targetG(), "rxdata", rxbytes, H5T_NATIVE_UINT8);

            {
                Group group = H5Gopen(targetG(), "00", H5P_DEFAULT);
                if(group() < 0) error("No target group 00");

                HardwareChannel ch = {TEST_STATE, 0};
                vectorFromH5(group(), "times", target.timestamps[ch], ctimeType);

                for(const auto& c : ALL_TCH) {
                    ch.channel = c;
                    vectorFromH5(group(), ToHex(c), target.uints[ch], H5T_NATIVE_UINT8);
                }
            }

            {
                Group group = H5Gopen(targetG(), "03", H5P_DEFAULT);
                if(group() < 0) error("No target group 03");
                vectorFromH5(group(), "times", host.acttimestamps[PROMPT], htimeType);
                vectorFromH5(group(), "00", host.strings[PROMPT], vstr);
            }

            {
                Group group = H5Gopen(targetG(), "80", H5P_DEFAULT);
                if(group() < 0) error("No target group 80");
                vectorFromH5(group(), "times", host.acttimestamps[TARGET_LOG], htimeType);
                vectorFromH5(group(), "00", host.strings[TARGET_LOG], vstr);
            }

            for(const auto& qual : registeredQuals) {
                Group group = H5Gopen(targetG(), ToHex(qual.devclass), H5P_DEFAULT);
                if(group() < 0) error(std::format("No {} group", ToHex(qual.devclass)));

                Group idg = H5Gopen(group(), ToHex(qual.id), H5P_DEFAULT);
                if(idg() < 0) error(std::format("No target entry for {}", qual));

                vectorFromH5(idg(), "times", target.timestamps[qual], ctimeType);

                switch(qual.devclass) {
                case RCP_DEVCLASS_DISCRETE_ACTUATOR:
                case RCP_DEVCLASS_BOOL_SENSOR: {
                    HardwareChannel ch = {qual, 0};
                    while(target.uints.contains(ch)) {
                        vectorFromH5(idg(), ToHex(ch.channel), target.uints[ch], H5T_NATIVE_UINT8);
                        ch.channel++;
                    }

                    break;
                }

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
                case RCP_DEVCLASS_QUATERNION: {
                    HardwareChannel ch = {qual, 0};
                    while(target.floats.contains(ch)) {
                        vectorFromH5(idg(), ToHex(ch.channel), target.floats[ch], H5T_NATIVE_FLOAT);
                        ch.channel++;
                    }
                    break;
                }

                default:
                    break;
                }
            }
        }
        releaseTypes();
    }

    void EventLog::serialize(const std::string& hdf) const {
        std::filesystem::path hdfPath = getRoamingFolder() / "exports" / hdf;

        setupTypes();

        File file = H5Fcreate(hdfPath.string().c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

        std::vector<SerializableQual> squals;
        squals.reserve(registeredQuals.size());
        for(auto& qual : registeredQuals) squals.emplace_back(qual);
        std::sort(squals.begin(), squals.end());

        // Create /registeredQuals, which is an array storing all non-implicit qualifiers for reference
        vectorIntoH5(file(), QUALS_DSET_NAME, squals, SerializableQual::htype);
        strIntoH5(file(), TCONF_DSET_NAME, jsonText);

        hostHDFSerialize(*this, file(), squals);
        targetHDFSerialize(*this, file(), squals);

        releaseTypes();
    }
} // namespace LRI::RCI

/*
HDF Format:
root/
 |- registeredQuals (qual type)
 |- tconfig (string)
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
