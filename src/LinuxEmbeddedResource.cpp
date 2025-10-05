#include "EmbeddedResource.h"

#include <map>
#include <string>

extern "C" {
extern char font_bold_start[];
extern char font_bold_end[];
extern char font_italic_start[];
extern char font_italic_end[];
extern char font_regular_start[];
extern char font_regular_end[];
extern char LRI_Logo_start[];
extern char LRI_Logo_end[];
extern char LRI_Logo_big_start[];
extern char LRI_Logo_big_end[];
}

namespace LRI::RCI {
    using Pointers = std::pair<char*, size_t>;
    static const std::map<std::string, Pointers> RESOURCES = {
        {"font_regular.ttf", {font_regular_start, static_cast<size_t>(font_regular_end - font_regular_start)}},
        {"font_bold.ttf", {font_bold_start, static_cast<size_t>(font_bold_end - font_bold_start)}},
        {"font_italic.ttf", {font_italic_start, static_cast<size_t>(font_italic_end - font_italic_start)}},
        {"LRI_Logo.png", {LRI_Logo_start, static_cast<size_t>(LRI_Logo_end - LRI_Logo_start)}},
        {"LRI_Logo_big.png", {LRI_Logo_big_start, static_cast<size_t>(LRI_Logo_big_end - LRI_Logo_big_start)}}};

    EmbeddedResource::EmbeddedResource(const std::string&& path) :
        length(RESOURCES.at(path).second), data(RESOURCES.at(path).first) {
    }

    EmbeddedResource::~EmbeddedResource() {}

    size_t EmbeddedResource::getLength() const { return length; }

    const char* EmbeddedResource::getData() const { return data; }
} // namespace LRI::RCI
