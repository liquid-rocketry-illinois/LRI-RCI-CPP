#ifndef RESOURCE_H
#define RESOURCE_H

#include <Windows.h>
#include <string>

namespace LRI::RCI {
    // Utility class for accessing data bundled via windows resource files. Used for fonts in this project
    class WindowsResource {
        // Raw data buffer
        LPCSTR bdata;

        // The Windows resource struct
        HGLOBAL resource;

        // The Windows info struct
        HRSRC info;

        // Size of the resource
        DWORD size;

    public:
        explicit WindowsResource(const std::string& path, const std::string& type);
        ~WindowsResource();
        [[nodiscard]] LPCSTR getData() const;
        [[nodiscard]] DWORD getSize() const;
    };
} // namespace LRI::RCI

#endif // RESOURCE_H
