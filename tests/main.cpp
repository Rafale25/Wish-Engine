#include "Context.hpp"
#include "Application.hpp"
#include "Logger.hpp"

int main() {
    Context& ctx = Context::instance();
    ctx.init();

    {
        App appView;
        ctx.viewPush(appView);
        ctx.run();
    }

    logD("Main end");
}
