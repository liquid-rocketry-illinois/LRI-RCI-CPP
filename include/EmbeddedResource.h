#ifndef EMBEDDEDRESOURCE_H
#define EMBEDDEDRESOURCE_H

#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace LRI::RCI {
    class EmbeddedResource {
        const char* data;
        size_t length;

#ifdef _WIN32
        HGLOBAL resource;
        HRSRC info;
#else
#endif

    public:
        explicit EmbeddedResource(const std::string&& path);
        ~EmbeddedResource();

        [[nodiscard]] const char* getData() const;
        [[nodiscard]] size_t getLength() const;
    };
} // namespace LRI::RCI

#endif // EMBEDDEDRESOURCE_H
