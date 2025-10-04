#include "EmbeddedResource.h"

namespace LRI::RCI {
    EmbeddedResource::EmbeddedResource(const std::string&& path) : length(0), data(nullptr) {}

    EmbeddedResource::~EmbeddedResource() {}

    size_t EmbeddedResource::getLength() const { return length; }

    const char* EmbeddedResource::getData() const { return data; }
} // namespace LRI::RCI
