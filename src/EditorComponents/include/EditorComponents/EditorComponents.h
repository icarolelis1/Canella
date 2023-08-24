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
        void camera_input_keys();

        float sensitivity = 0.005f;
        float speed = 0.0050f;
        float drag_speed = 0.01f;
        glm::quat orientation = glm::quat(glm::vec3(0.f,0.f,0.f));
        double last_x;
        double last_y;
        int drag_x;
        int drag_y;
        float rotating_x;
        float rotating_y;
        bool dragging_position = false;
        bool camera_rotating = false;
        glm::vec3 camera_rotation;
        float time = 0.0f;
        void set_mouse_callbacks();
        void update_euler_directions();
        double last_yaw,last_pich;

    };

}

#endif