#include "Context.hpp"
#include "Application.hpp"

int main() {
    Context ctx;
    App appView(ctx);

    ctx.init();

    // ctx.setVsync(VSYNC);
    ctx.setView(appView);
    ctx.run();
}
