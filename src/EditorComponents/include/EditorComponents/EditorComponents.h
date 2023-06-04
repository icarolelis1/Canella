#pragma once
#ifndef EDITOR_COMPONENTS
#define  EDITOR_COMPONENTS
#include "Components/Components.h"

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
        float speed = 0.025f;
        void update_euler_directions();
        float  YAW,PITCH,ROLL;
    };

}

#endif