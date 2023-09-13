#ifndef ASYNC_LOADER
#define ASYNC_LOADER
#include "Device/Device.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {

            class SyncStructure
            {
            public:
                /**
                 *@brief Loads ressources asyncronously
                 * @param transfer_queue queue with transfer capabilities
                 */
                SyncStructure( Device *device);
                ~SyncStructure() = default;
                /**
                 * @brief setup the Pools and semaphores for resources
                 */
                void build();
                /**
                 * @brief destroys the associated semaphores and fence
                 */
                void destroy();

                void submit_request();
                VkSemaphore semaphore;
                VkSemaphore wait_semaphore;
                VkQueue queue;
                VkFence fence;
                Device *device;
                std::mutex pool_mutex;
            private:
            };

        }
    }
}
#endif