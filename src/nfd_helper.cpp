#include "nfd_helper.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw/glfw3native.h"
#include "nfd.h"

namespace LRI::RCI {
    FilePickResult pickRCIFile(Window* owner) {

        auto defPath = getRoamingFolder() / "exports";
        auto defPathStr = defPath.string();

        nfdu8char_t* out;
        nfdu8filteritem_t filter;
        filter.name = "RCI Test File";
        filter.spec = "rci";
        nfdopendialogu8args_t args;
        args.filterList = &filter;
        args.filterCount = 1;
        args.defaultPath = defPathStr.c_str();
        args.parentWindow = {NFD_WINDOW_HANDLE_TYPE_WINDOWS, owner->getHandle()};
        args.acceptLabel = "Open";
        args.title = "Open a test log";
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
    void nfdHelperCleanup() { NFD_Quit(); }
} // namespace LRI::RCI
