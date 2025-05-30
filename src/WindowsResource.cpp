#include "WindowsResource.h"

namespace LRI::RCI {
    // Opens a resource bundled with a Windows resource file
    WindowsResource::WindowsResource(const std::string& path, const std::string& type) {
        info = FindResourceA(nullptr, path.c_str(), type.c_str());
        resource = LoadResource(nullptr, info);
        size = SizeofResource(nullptr, info);
        bdata = static_cast<LPCSTR>(LockResource(resource));
    }

    WindowsResource::~WindowsResource() {
        UnlockResource(resource);
        FreeResource(resource);
    }

    LPCSTR WindowsResource::getData() const { return bdata; }

    DWORD WindowsResource::getSize() const { return size; }
} // namespace LRI::RCI
