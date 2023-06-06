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
    private:
        void camera_input_keys(float deta_time,
                               glm::vec3 &camera_position,
                               const Euler &camera_euler,
                               Canella::KeyBoard &KeyBoard,
                               Canella::CursorPos& cursor_position);

        float sensitivity = 0.005f;
        float speed = 0.0050f;
        float drag_speed = 0.001f;
        glm::quat orientation = glm::quat(glm::vec3(0.f,0.f,0.f));
        double last_x;
        double last_y;
        int drag_x;
        int drag_y;
        float rotating_x;
        float rotating_y;
        bool dragging_position = false;
        bool camera_rotating = false;

        void set_mouse_callbacks();
        void update_euler_directions();
        double last_yaw,last_pich;
        float  YAW,PITCH,ROLL;

    };

}

#endif