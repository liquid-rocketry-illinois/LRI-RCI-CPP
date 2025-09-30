#include "EmbeddedResource.h"

namespace LRI::RCI {
    EmbeddedResource::EmbeddedResource(const std::string&& path) {
#ifdef _WIN32
        info = FindResourceA(nullptr, path.c_str(), "EMBEDDEDDATA");
        resource = LoadResource(nullptr, info);
        length = SizeofResource(nullptr, info);
        data = static_cast<const char*>(LockResource(resource));
#endif
    }

    EmbeddedResource::~EmbeddedResource() {
#ifdef _WIN32
        UnlockResource(resource);
        FreeResource(resource);
#endif
    }

    size_t EmbeddedResource::getLength() const { return length; }

    const char* EmbeddedResource::getData() const { return data; }
} // namespace LRI::RCI
