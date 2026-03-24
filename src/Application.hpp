#pragma once

#include "View.hpp"

class App: public View {
public:
    App(Context& ctx);

    void onEnterView();

    void onUpdate(double time_since_start, float dt);
    void onDraw(double time_since_start, float dt);

    void onResize(int width, int height);
};
