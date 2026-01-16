#ifndef LRI_CONTROL_PANEL_POSITIONING_H
#define LRI_CONTROL_PANEL_POSITIONING_H

#include "imgui.h"

inline bool operator==(ImVec2 const& v1, ImVec2 const& v2) { return v1.x == v2.x && v1.y == v2.y; }
inline ImVec2 operator+(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x + v2.x, v1.y + v2.y}; }
inline ImVec2 operator+(ImVec2 const& v1, float constant) { return {v1.x + constant, v1.y + constant}; }
inline ImVec2 operator-(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x - v2.x, v1.y - v2.y}; }
inline ImVec2 operator-(ImVec2 const& v1, float constant) { return {v1.x - constant, v1.y - constant}; }
inline ImVec2 operator*(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x * v2.x, v1.y * v2.y}; }
inline ImVec2 operator*(ImVec2 const& v1, float constant) { return {v1.x * constant, v1.y * constant}; }
inline ImVec2 operator/(ImVec2 const& v1, ImVec2 const& v2) { return {v1.x / v2.x, v1.y / v2.y}; }
inline ImVec2 operator/(ImVec2 const& v1, float constant) { return {v1.x / constant, v1.y / constant}; }

namespace LRI::RCI {
    struct Box {
        ImVec2 _tl;
        ImVec2 _br;

        Box() : _tl(0, 0), _br(0, 0) {}
        Box(const ImVec2& tl, const ImVec2& br) : _tl(tl), _br(br) {}
        Box(float t, float l, float b, float r) : _tl(l, t), _br(r, b) {}
        Box(const ImVec2& tl, float b, float r) : _tl(tl), _br(r, b) {}
        Box(float t, float l, const ImVec2& br) : _tl(l, t), _br(br) {}

        Box operator-(const Box& other) const { return {_tl - other._tl, _br - other._br}; }
        Box operator-(const ImVec2& other) const { return {_tl - other, _br - other}; }
        Box operator-(float other) const { return {_tl - other, _br - other}; }
        Box operator+(const Box& other) const { return {_tl + other._tl, _br - other._br}; }
        Box operator+(const ImVec2& other) const { return {_tl + other, _br + other}; }
        Box operator+(float other) const { return {_tl + other, _br + other}; }
        Box operator*(float other) const { return {_tl * other, _br * other}; }

        [[nodiscard]] ImVec2 tl() const { return _tl; }
        [[nodiscard]] ImVec2 tr() const { return {_br.x, _tl.y}; }
        [[nodiscard]] ImVec2 bl() const { return {_tl.x, _br.y}; }
        [[nodiscard]] ImVec2 br() const { return _br; }
        [[nodiscard]] float t() const { return _tl.y; }
        [[nodiscard]] float l() const { return _tl.x; }
        [[nodiscard]] float b() const { return _br.y; }
        [[nodiscard]] float r() const { return _br.x; }
        [[nodiscard]] ImVec2 size() const { return _br - _tl; }
        [[nodiscard]] float height() const { return _br.y - _tl.y; }
        [[nodiscard]] float width() const { return _br.x - _tl.x; }
    };
}

#endif // LRI_CONTROL_PANEL_POSITIONING_H
