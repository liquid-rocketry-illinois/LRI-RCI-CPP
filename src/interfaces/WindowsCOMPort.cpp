#include "interfaces/COMPort.h"

#include <SetupAPI.h>
#include <devguid.h>

namespace LRI::RCI {
    COMPort::COMPort(const std::string&& portname, unsigned long baudrate) :
        portname(portname), baudrate(baudrate), port(nullptr) {
        ioUnlock();
    }

     COMPort::~COMPort() {
        ioLock();
    }

    std::string COMPort::interfaceType() const {
        return std::string("Serial Port (") + portname + " @ " + std::to_string(baudrate) + " baud)";
    }

    void COMPort::ioInit() {
        port = CreateFile(portname.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, nullptr);

        if(port == INVALID_HANDLE_VALUE) {
            lastErrorStage = 1;
            lastErrorCode = -1;
            portOpenFail = true;
            return;
        }

        // Setup serial parameters, such as parity, flow control, and baud rate
        DCB params;
        params.DCBlength = sizeof(DCB);

        if(!GetCommState(port, &params)) {
            lastErrorStage = 2;
            lastErrorCode = GetLastError();
            portOpenFail = true;
            return;
        }

        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.StopBits = ONESTOPBIT;
        params.Parity = NOPARITY;
        // params.fDtrControl = DTR_CONTROL_ENABLE; // Not actually needed for arduino, it breaks stm

        if(!SetCommState(port, &params)) {
            lastErrorStage = 3;
            lastErrorCode = GetLastError();
            portOpenFail = true;
            return;
        }

        // Timeouts for IO operations
        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout = 1;
        timeouts.ReadTotalTimeoutConstant = 1;
        timeouts.ReadTotalTimeoutMultiplier = 1;
        timeouts.WriteTotalTimeoutConstant = 1;
        timeouts.WriteTotalTimeoutMultiplier = 1;

        if(!SetCommTimeouts(port, &timeouts)) {
            lastErrorStage = 4;
            lastErrorCode = GetLastError();
            portOpenFail = true;
            return;
        }

        PurgeComm(port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3000ms);

        EscapeCommFunction(port, SETDTR);
        isPortOpen = true;
    }

    bool COMPort::writeBytes(const uint8_t* bytes, size_t length) {
        DWORD _written = 0;
        if(!WriteFile(port, bytes, length, &_written, nullptr) || static_cast<size_t>(_written) != length) {
            lastErrorStage = 5;
            lastErrorCode = GetLastError();
            return false;
        }

        return true;
    }

    bool COMPort::readBytes(uint8_t* bytes, size_t bufLength, size_t& written) {
        DWORD _written = 0;
        if(!ReadFile(port, bytes, bufLength, &_written, nullptr)) {
            lastErrorStage = 6;
            lastErrorCode = GetLastError();
            return false;
        }

        written = static_cast<size_t>(_written);
        return true;
    }

    void COMPort::ioDeinit() {
        CloseHandle(port);
    }

    // Honestly I dont know what this does its some Windows spaghetti I stole from SO but it works so yay
    // https://stackoverflow.com/a/77752863
    bool COMPort::enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portlist) {
        portlist.clear();
        HANDLE devs = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if(devs == INVALID_HANDLE_VALUE) return false;

        SP_DEVINFO_DATA data;
        data.cbSize = sizeof(SP_DEVINFO_DATA);
        char s[80];

        for(DWORD i = 0; SetupDiEnumDeviceInfo(devs, i, &data); i++) {
            HKEY hkey = SetupDiOpenDevRegKey(devs, &data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
            if(hkey == INVALID_HANDLE_VALUE) {
                return false;
            }

            char comname[16];
            DWORD len = 16;

            RegQueryValueEx(hkey, "PortName", nullptr, nullptr, (LPBYTE) comname, &len);
            RegCloseKey(hkey);
            if(comname[0] != 'C') continue;

            SetupDiGetDeviceRegistryProperty(devs, &data, SPDRP_FRIENDLYNAME, nullptr, (PBYTE) s, sizeof(s), nullptr);

            // Somehow we end up with the name we need to open the port, and a more user friendly display string.
            // These get appended to this vector for later
            portlist.push_back(std::make_pair(std::string(comname), std::string(comname) + " : " + std::string(s)));
        }

        SetupDiDestroyDeviceInfoList(devs);
        return true;
    }

} // namespace LRI::RCI
