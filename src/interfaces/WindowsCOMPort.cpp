#include "interfaces/COMPort.h"

namespace LRI::RCI {
    COMPort::COMPort(const std::string&& portname, unsigned long baudrate, bool arduinoMode) :
        portname(portname), baudrate(baudrate), arduinoMode(arduinoMode), port(nullptr) {
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

        // Breaks STM, needed for arduino
        if(arduinoMode) params.fDtrControl = DTR_CONTROL_ENABLE;

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

        // Only needed for STM
        if(!arduinoMode) EscapeCommFunction(port, SETDTR);
        isPortOpen = true;
    }

    bool COMPort::writeBytes(const uint8_t* bytes, size_t length) {
        DWORD _written = 0;
        if(!WriteFile(port, bytes, static_cast<DWORD>(length), &_written, nullptr) || static_cast<size_t>(_written) != length) {
            lastErrorStage = 5;
            lastErrorCode = GetLastError();
            return false;
        }

        return true;
    }

    bool COMPort::readBytes(uint8_t* bytes, size_t bufLength, size_t& written) {
        DWORD _written = 0;
        if(!ReadFile(port, bytes, static_cast<DWORD>(bufLength), &_written, nullptr)) {
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

} // namespace LRI::RCI
