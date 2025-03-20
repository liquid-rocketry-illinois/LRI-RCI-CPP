#ifndef WMODULE_H
#define WMODULE_H

namespace LRI::RCI {
    class WModule {
    public:
        WModule() = default;
        virtual ~WModule() = default;
        virtual void render() = 0;
    };
}

#endif //WMODULE_H
