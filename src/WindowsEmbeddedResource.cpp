#include "EmbeddedResource.h"

namespace LRI::RCI {
    EmbeddedResource::EmbeddedResource(const std::string&& path) {
        info = FindResourceA(nullptr, path.c_str(), "EMBEDDEDDATA");
        resource = LoadResource(nullptr, info);
        length = SizeofResource(nullptr, info);
        data = static_cast<const char*>(LockResource(resource));
    }

    EmbeddedResource::~EmbeddedResource() {
        UnlockResource(resource);
        FreeResource(resource);
    }

    size_t EmbeddedResource::getLength() const { return length; }

    const char* EmbeddedResource::getData() const { return data; }
} // namespace LRI::RCI
