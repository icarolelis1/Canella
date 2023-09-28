#pragma once
#ifndef EDITOR_COMPONENTS
#define  EDITOR_COMPONENTS
#include "Components/Components.h"
#include "Input/Input.h"

namespace Canella{

    /**
     * NAVIGATES THE CAMERA IN THE EDITOR MODE
     */
    class CameraEditor :public ScriptableEntity
    {
    public:
        ~CameraEditor() = default;
        CameraComponent* camera_component;
        void on_start() override;
        void on_update(float t)  override;
        void move_and_focus(glm::vec3 positon);
    private:
        float sensitivity = 0.0005f;
        float speed = 0.0050f;
        float drag_speed = 0.01f;
        double last_x;
        double last_y;
        int drag_x;
        int drag_y;
        float rotating_x;
        float rotating_y;
        bool dragging_position = false;
        bool camera_rotating = false;
        float time = 0.0f;
        void set_mouse_callbacks();
        void update_euler_directions();

    };

}

#endif