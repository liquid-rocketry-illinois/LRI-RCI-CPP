#include "interfaces/WindowsCOMPort.h"

#include <SetupAPI.h>
#include <devguid.h>

#include "hardware/TestState.h"
#include "utils.h"

namespace LRI::RCI {
    COMPort::COMPort(std::string portName, unsigned long baudRate) : ICOMPort(portName, baudRate) {
    }

    // The actual work. Alternates between a read and a write on each loop
    void COMPort::threadRead() {
        HANDLE const port = CreateFile(_portname, GENERIC_READ | GENERIC_WRITE, 0,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        // If the port is invalid exit constructor
        if(port == INVALID_HANDLE_VALUE) {
            lastErrorVal = GetLastError();
            portopen = false;
            return;
        }

        // Setup serial parameters, such as parity, flow control, and baud rate
        DCB params;
        params.DCBlength = sizeof(DCB);

        if(!GetCommState(port, &params)) {
            lastErrorVal = GetLastError();
            portopen = false;
            CloseHandle(port);
            return;
        }

        params.BaudRate = baudrate;
        params.ByteSize = 8;
        params.StopBits = ONESTOPBIT;
        params.Parity = NOPARITY;
        params.fDtrControl = DTR_CONTROL_ENABLE;

        if(!SetCommState(port, &params)) {
            lastErrorVal = GetLastError();
            portopen = false;
            CloseHandle(port);
            return;
        }

        // Timeouts for IO operations
        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if(!SetCommTimeouts(port, &timeouts)) {
            lastErrorVal = GetLastError();
            portopen = false;
            CloseHandle(port);
            return;
        }

        lastErrorVal = 0;

        PurgeComm(port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(3000ms);
        inlock.lock();
        inbuffer->clear();
        inlock.unlock();

        outlock.lock();
        outbuffer->clear();
        outlock.unlock();

        portready = true;

        bool r_nw = false;
        while(doComm) {
            if(r_nw) {
                // Read cycle
                r_nw = false;

                // If there is no space in the buffer return
                inlock.lock();
                bool res = inbuffer->isFull();
                inlock.unlock();
                if(res) continue;

                uint8_t byte;
                DWORD read;

                // Read the latest byte from the serial port and write it to inbuffer
                if(!ReadFile(port, &byte, 1, &read, nullptr)) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                if(read == 0) continue;

                inlock.lock();
                inbuffer->push(byte);
                inlock.unlock();
                // std::cout << "rcv: " << std::hex << (int) byte << std::endl;
            }

            else {
                // Write cycle
                r_nw = true;
                if(!TestState::getInited()) continue;

                // If the output buffer is empty return
                outlock.lock();
                bool res = outbuffer->isEmpty();
                if(res) {
                    outlock.unlock();
                    continue;
                }

                // The byte is peeked so that if the write fails the byte remains in the buffer
                uint8_t byte = outbuffer->peek();
                outlock.unlock();

                DWORD written;

                // Write to serial device
                if(!WriteFile(port, &byte, 1, &written, nullptr) || written != 1) {
                    lastErrorVal = GetLastError();
                    continue;
                }

                // Pop byte from buffer if successful
                outlock.lock();
                outbuffer->pop();
                outlock.unlock();
                // std::cout << "snd: " << std::hex << (int) byte << std::endl;
            }
        }

        CloseHandle(port);
    }

    // Honestly I dont know what this does its some Windows spaghetti I stole from SO but it works so yay
    // https://stackoverflow.com/a/77752863
    bool enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portList) {
        portList.clear();
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
