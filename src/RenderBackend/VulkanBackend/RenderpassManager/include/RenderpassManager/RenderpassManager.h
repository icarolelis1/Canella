#pragma once
#ifndef RENDERPASS_MANAGER
#define RENDERPASS_MANAGER

#include "Device/Device.h"
#include "Swapchain/Swapchain.h"
#include "Renderpass/Renderpass.h"
#include <vulkan/vulkan.h>
#include <json.hpp>
#include <unordered_map>
#include <memory>
#include <string>
/**
 * @brief holds all the renderpasses used by the application
 */
 //TODO still demands a lot of refactor and aditions
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            //todo remove raw pointer usage of RenderPass
            using Renderpasses = std::unordered_map<std::string, std::unique_ptr<RenderPass>>;

            class RenderpassManager
            {
            public:
                 RenderpassManager() = default;
                ~RenderpassManager() = default;

                /**
                 * @brief builds the RenderpassManager object loading a json file located at render_path
                 * @param device vulkan device
                 * @param swapchain vkSwapchain
                 * @param render_path location of render config json
                 * @param ResourcesManager resource manager to allocate images for the framebuffer
                 */
                void build(Device *device, Swapchain *swapchain, const char *render_path,ResourcesManager* resource_manager);

                void rebuild(Device *device, Swapchain *swapchain,ResourcesManager* resource_manager);
                /**
                 * @brief Destroys all the renderpasses and framebuffers associated with it
                 */
                void destroy_renderpasses();

                Renderpasses renderpasses;
            private:
                struct RenderpassDescription
                {
                    uint32_t number_of_attachemnts = 0;
                    uint32_t number_of_subpasses = 0;
                    std::vector<RenderAttachment> attachements;
                    std::vector<Subpass> subpasses;
                };

                typedef struct RenderpassManagerDescription
                {
                    uint32_t number_of_passes;
                    std::vector<RenderpassDescription> renderpasses_descriptions;
                } RenderpassManagerDescription;

                /**
                 *
                 * @param swapchain swapchain
                 * @param extent VkExtent with dimension of the swapchain
                 * @param managerdescriptio struct that contains
                 */
                void loadRenderPassManager(std::string,
                                           Swapchain* swapchain,
                                           VkExtent2D extent,
                                           RenderpassManagerDescription& managerdescriptio,
                                           ResourcesManager* resource_manager);

                /**
                 *
                 * @param format_str
                 * @param swapchain
                 * @return return attachment format
                 */
                VkFormat get_attachment_format(const char* format_str,Swapchain* swapchain);
                nlohmann::json config;
                Device *device;

            };

        }
    }
}

#endif