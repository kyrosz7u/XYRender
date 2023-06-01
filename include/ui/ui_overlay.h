#include <imgui.h>
#include <memory>

#pragma once

class UIOverlay;
typedef std::weak_ptr<UIOverlay> UIOverlayPtr;

class UIOverlay
{
public:
    void drawImGui();
};

