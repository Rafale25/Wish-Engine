#include "Context.hpp"
#include "Application.hpp"

int main() {
    Context& ctx = Context::instance();
    ctx.init();

    App appView;

    // ctx.setVsync(VSYNC);
    ctx.setView(appView);
    ctx.run();
}
