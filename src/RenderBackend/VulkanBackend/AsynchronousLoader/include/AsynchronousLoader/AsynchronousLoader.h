#ifndef ASYNC_LOADER
#define ASYNC_LOADER
#include "Resources/Resources.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
    
            class AsynchronousLoader
            {
            public:
                /**
                 *@brief Loads ressources asyncronously
                 * @param device Pointer to renderer Device
                 * @param resource_manager resource manager. Contains the stored gpu resources.
                 * @param transfer_queue queue with transfer capabilities
                 */
                AsynchronousLoader(Device *device, ResourcesManager *resource_manager, VkQueue transfer_queue);
                ~AsynchronousLoader() = default;
                /**
                 * @brief setup the Pools and semaphores for resources
                 */
                void build();
                /**
                 * @brief destroys the associated semaphores and fence
                 */
                void destroy();
            
                void submit_request();

            private:
                VkSemaphore semaphore;
                VkQueue queue;
                VkFence fence;
                Device *device;
            };

        }
    }
}
#endif