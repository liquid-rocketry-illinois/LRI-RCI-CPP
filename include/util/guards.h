#ifndef LRI_CONTROL_PANEL_GUARDS_H
#define LRI_CONTROL_PANEL_GUARDS_H

#include <functional>

namespace LRI::RCI {
    class ScopeGuard {
        std::function<void()> func;

    public:

        ScopeGuard(std::function<void()>&& func) : func(func) {}
        ~ScopeGuard() {
            func();
        }

        ScopeGuard(ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard& other) {
            func = other.func;
            return *this;
        };

        class Ctor {
        public:
            ScopeGuard operator+(std::function<void()>&& f) const { return ScopeGuard(std::move(f)); }
        };
    };

#define CATIMPL(A, B) A##B
#define CAT(A, B) CATIMPL(A, B)
#define SCOPE_EXIT [[maybe_unused]] ScopeGuard CAT(GUARD, __COUNTER__) = ScopeGuard::Ctor() + [&]()

// #define SCOPE_EXIT [[maybe_unused]] ScopeGuard __COUNTER__;
}

#endif // LRI_CONTROL_PANEL_GUARDS_H
