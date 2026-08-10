#ifndef LRI_CONTROL_PANEL_DEFERREDVALUE_H
#define LRI_CONTROL_PANEL_DEFERREDVALUE_H

#include <filesystem>
#include <future>
#include <functional>
#include <map>

namespace LRI::RCI {
    class FrameDeferred {
        struct DeferInfo {
            void* value;
            void (*mover)(void*, const void*);
            void (*deleter) (const void*);
        };

        static std::map<void*, DeferInfo> deferredValues;
        static std::vector<std::function<void()>> deferredFuncs;

    public:
        static void updateDeferredValues();

        template<typename T>
        static void defer(T& location, T&& value) {
            T* storedVal = new T(std::forward<T>(value));
            if(deferredValues.contains(&location)) {
                DeferInfo& info = deferredValues[&location];
                info.deleter(info.value);
                info.value = storedVal;
            }
            else {
                deferredValues[&location] = { storedVal,
                    [] (void* d, const void* s) {
                        T* castedD = static_cast<T*>(d);
                        const T* castedS = static_cast<const T*>(s);
                        *castedD = *castedS;
                    },
                    [] (const void* p) { delete static_cast<const T*>(p); }
                };
            }
        }

        static void defer(std::function<void()>&& func) {
            deferredFuncs.emplace_back(std::move(func));
        }

        template<typename T>
        static const T& getDeferredValue(T& location) {
            return *static_cast<T*>(deferredValues[&location].value);
        }
    };
}

#endif // LRI_CONTROL_PANEL_DEFERREDVALUE_H
