#ifndef CAMERA_CONTROLLER
#define CAMERA_CONTROLLER
#include "Component/Component.h"
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <memory>
namespace Canella
{
    class CameraController : public Component
    {
        struct Euler
        {
            glm::vec3 front;
            glm::vec3 up;
            glm::vec3 right;
        };

    public:
        CameraController(std::string id);
        void onUpdate(float dt);
        void onStart();
        void loadState(nlohmann::json &config);
        void saveState(nlohmann::json &config);

        static std::shared_ptr<Canella::Component> create(const nlohmann::json &config);
    private:
        void resetCamera();
        float velocity = .04f;
        void updateDirections();
        float YAW = 90;
        float PITCH = 0;
        float ROLL;
        Euler euler;
        glm::vec3 position;
    };
}

#endif