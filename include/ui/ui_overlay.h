#include <imgui.h>
#include <memory>

class UIOverlay;
typedef std::weak_ptr<UIOverlay> UIOverlayPtr;

class UIOverlay
{
    void drawImGui();
};

