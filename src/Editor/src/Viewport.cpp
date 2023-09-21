#include "Editor/Viewport.h"

void Canella::Viewport::build_viewport( ImTextureID texture_id ) {
    auto size = ImGui::GetWindowSize();
    ImGui::Image(texture_id,ImVec2(size.x,size.y),ImVec2(0,0),ImVec2(1,1),ImVec4(1,1,1,1),ImVec4(1,1,1,1));
}