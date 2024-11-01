#ifndef RESOURCE_H
#define RESOURCE_H

#include <Windows.h>
#include <string>

namespace LRI::RCI {
    class WindowsResource {
        LPCSTR bdata;
        HGLOBAL resource;
        HRSRC info;
        DWORD size;

    public:
        explicit WindowsResource(const std::string& path, const std::string& type);
        ~WindowsResource();
        LPCSTR getData() const;
        DWORD getSize() const;
    };
}

#endif //RESOURCE_H
