#include "Context.hpp"
#include "Application.hpp"
#include "Logger.hpp"

int main() {
    Context& ctx = Context::instance();
    ctx.init();

    {
        App appView;
        // ctx.setVsync(VSYNC);
        ctx.setView(appView);
        ctx.run();
    }

    logD("Main end");
}
