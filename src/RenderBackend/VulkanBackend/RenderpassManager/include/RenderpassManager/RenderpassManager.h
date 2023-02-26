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
 *
 */
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            using Renderpasses = std::unordered_map<std::string, RenderPass *>;

            class RenderpassManager
            {

                struct RenderpassDescription
                {
                    uint32_t number_of_attachemnts = 0;
                    uint32_t number_of_subpasses = 0;
                    std::vector<RenderAttachment> attachements;
                    std::vector<Subpass> subpasses;
                };

                typedef struct _RenderpassManagerDescription
                {
                    uint32_t number_of_passes;
                    std::vector<RenderpassDescription> renderpasses_descriptions;
                } RenderpassManagerDescription;

                void loadRenderPassManager(std::string, Swapchain* swapchain, VkExtent2D extent, RenderpassManagerDescription& managerdescriptio);
                Device *device;

            public:
                /**
                 * @brief Construct a new Renderpass Manager object
                 * @param device
                 * @param swapchain
                 * @param render_path
                 */
                RenderpassManager(Device *device, Swapchain *swapchain, const char *render_path);
                Renderpasses renderpasses;
            };

        }
    }
}

#endif