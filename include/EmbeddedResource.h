#ifndef EMBEDDEDRESOURCE_H
#define EMBEDDEDRESOURCE_H

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace LRI::RCI {
    class EmbeddedResource {
        const char* data;
        size_t length;

        HGLOBAL resource;
        HRSRC info;

    public:
        explicit EmbeddedResource(const std::string&& path);
        ~EmbeddedResource();

        EmbeddedResource(const EmbeddedResource&) = delete;
        EmbeddedResource& operator=(EmbeddedResource&& other) noexcept {
            data = other.data;
            length = other.length;
            resource = other.resource;
            info = other.info;

            other.data = nullptr;
            other.length = 0;
            other.resource = nullptr;
            other.info = nullptr;
            return *this;
        }
        EmbeddedResource(EmbeddedResource&& other)  noexcept : data(other.data), length(other.length), resource(other.resource), info(other.info) {
            other.data = nullptr;
            other.length = 0;
            other.resource = nullptr;
            other.info = nullptr;

        }
        EmbeddedResource& operator=(EmbeddedResource&) = delete;

        [[nodiscard]] const char* getData() const;
        [[nodiscard]] size_t getLength() const;
    };
} // namespace LRI::RCI

#endif // EMBEDDEDRESOURCE_H
