#include <fcntl.h>
#include <cerrno>
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <filesystem>

#include "interfaces/LinuxCOMPort.h"
#include "hardware/TestState.h"

namespace LRI::RCI {
    COMPort::COMPort(std::string portName, unsigned long baudRate)
        : ICOMPort(std::move(portName), baudRate) {
    }

    void COMPort::threadRead() {
        const int port = open(portName.c_str(), O_RDWR);

        if(port < 0) {
            lastErrorVal = errno;
            portopen = false;
            return;
        }

        termios tty{};
        if(tcgetattr(port, &tty) != 0) {
            lastErrorVal = errno;
            portopen = false;
            close(port);
            return;
        }

        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOP;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;
        tty.c_cflag |= CRTSCTS;
        tty.c_cflag |= CREAD | CLOCAL;

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_lflag &= ~ECHOE;
        tty.c_lflag &= ~ECHONL;
        tty.c_lflag &= ~ISIG;

        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

        tty.c_oflag &= ~(OPOST | ONLCR);

        tty.c_cc[VTIME] = 5;
        tty.c_cc[VMIN] = 1;

        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);

        if(tcsetattr(port, TCSANOW, &tty) != 0) {
            lastErrorVal = errno;
            portopen = false;
            close(port);
            return;
        }

        lastErrorVal = 0;

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
                r_nw = false;

                inlock.lock();
                bool res = inbuffer->isFull();
                inlock.unlock();
                if(res) continue;

                uint8_t byte;
                size_t readbytes = read(port, &byte, 1);

                if(readbytes == 0) continue;

                if(readbytes < 0 || readbytes > 1) {
                    lastErrorVal = errno;
                    continue;
                }

                inlock.lock();
                inbuffer->push(byte);
                inlock.unlock();
            }

            else {
                r_nw = true;

                if(!TestState::getInited()) continue;

                outlock.lock();
                bool res = outbuffer->isEmpty();
                if(res) {
                    outlock.unlock();
                    continue;
                }

                // The byte is peeked so that if the write fails the byte remains in the buffer
                uint8_t byte = outbuffer->peek();
                outlock.unlock();

                size_t written = write(port, &byte, 1);

                if(written != 1) {
                    lastErrorVal = errno;
                    continue;
                }

                outlock.lock();
                outbuffer->pop();
                outlock.unlock();
            }
        }
    }

    bool enumSerialDevs(std::vector<std::pair<std::string, std::string>>& portList) {
        for(const auto& folder : std::filesystem::directory_iterator("/sys/class/tty")) {

        }


        return true;
    }
}
