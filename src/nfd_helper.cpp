#include "nfd_helper.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include "nfd.h"

namespace LRI::RCI {
    FilePickResult filePicker(Window* owner) {


        nfdu8char_t* out;
        nfdopendialogu8args_t args;
        args.filterList = nullptr;
        args.filterCount = 0;
        args.defaultPath = nullptr;
        args.parentWindow = {NFD_WINDOW_HANDLE_TYPE_WINDOWS, owner->getHandle()};
        args.acceptLabel = "Open";
        args.title = "Open a thingy";
        args.cancelLabel = "Cancel";
        nfdresult_t res = NFD_OpenDialogU8_With(&out, &args);

        FilePickResult fpr;

        if(res == NFD_OKAY) {
            fpr.state = FilePickResult::SUCCESS;
            fpr.outpath = std::filesystem::path(out);
            NFD_FreePathU8(out);
        }

        else if(res == NFD_CANCEL) fpr.state = FilePickResult::CANCELLED;
        else fpr.state = FilePickResult::ERR;

        return fpr;
    }

    void nfdHelperInit() { NFD_Init(); }
} // namespace LRI::RCI
