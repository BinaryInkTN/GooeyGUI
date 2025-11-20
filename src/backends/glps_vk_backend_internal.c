/*
Copyright (c) 2024 Yassine Ahmed Ali

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#define STB_IMAGE_IMPLEMENTATION
#define NANOSVG_IMPLEMENTATION
#include "backends/utils/nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "backends/utils/nanosvg/nanosvgrast.h"
#define GLPS_USE_VULKAN
#include "backends/utils/backend_utils_internal.h"
#if (TFT_ESPI_ENABLED == 0)
#include "backends/utils/stb_image/stb_image.h"
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"
#include <time.h>
#include <nfd.h>
#include <math.h>

#define VK_CHECK(x)                                                          \
    do                                                                       \
    {                                                                        \
        VkResult err = x;                                                    \
        if (err != VK_SUCCESS)                                               \
        {                                                                    \
            LOG_INFO("Vulkan error %d at %s:%d\n", err, __FILE__, __LINE__); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

// Uniform buffer structures matching the shaders
typedef struct
{
    bool useTexture;
    bool isRounded;
    bool isHollow;
    int shapeType;
    float radius;
    float borderWidth;
    float size[2];
    float padding[2];
} ShapeParamsUBO;

typedef struct
{
    float projection[16];
} TextMatricesUBO;

typedef struct
{
    float textColor[3];
    float padding;
} TextParamsUBO;

typedef struct
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    uint32_t vertex_count;
} VulkanBuffer;

typedef struct
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkSampler sampler;
    int width, height;
} VulkanTexture;

typedef struct
{
    VkDescriptorSetLayout set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;
} VulkanPipeline;

typedef struct
{
    VulkanTexture texture;
    int width, height;
    int bearingX, bearingY;
    unsigned int advance;
} VulkanCharacter;

// Batch rendering structures
typedef struct
{
    Vertex *vertices;
    uint32_t vertex_count;
    uint32_t vertex_capacity;
} VertexBatch;

typedef struct
{
    ShapeParamsUBO params;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    VkDescriptorSet descriptor_set;
} DrawCommand;

typedef struct
{
    VertexBatch shape_batch;
    VertexBatch text_batch;
    DrawCommand *draw_commands;
    uint32_t command_count;
    uint32_t command_capacity;
    bool needs_redraw;
} WindowRenderState;

typedef struct
{
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family;

    VkSurfaceKHR *surfaces;
    VkSwapchainKHR *swapchains;
    VkRenderPass *render_passes;
    VkFramebuffer **framebuffers;
    VkCommandPool *command_pools;
    VkCommandBuffer **command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;
    VkFence **images_in_flight;

    VulkanPipeline text_pipeline;
    VulkanPipeline shape_pipeline;

    VulkanBuffer *text_vertex_buffers;
    VulkanBuffer *shape_vertex_buffers;

    // Uniform buffers
    VulkanBuffer *shape_uniform_buffers;
    VulkanBuffer *text_matrices_uniform_buffers;
    VulkanBuffer *text_params_uniform_buffers;

    // Descriptor sets
    VkDescriptorPool descriptor_pool;
    VkDescriptorSetLayout shape_set_layout;
    VkDescriptorSetLayout text_set_layout;
    VkDescriptorSet *shape_descriptor_sets;
    VkDescriptorSet *text_descriptor_sets;

    VulkanCharacter characters[128];
    VulkanTexture *loaded_textures;
    size_t texture_count;

    uint32_t *image_indices;
    uint32_t *swapchain_image_counts;
    VkExtent2D *window_extents;
    VkFormat *swapchain_formats;

    glps_WindowManager *wm;
    size_t active_window_count;

    unsigned int selected_color;
    bool is_running;
    bool inhibit_reset;

    glps_timer **timers;
    size_t timer_count;

    // Frame synchronization
    uint32_t current_frame;
    uint32_t max_frames_in_flight;

    // Batch rendering state
    WindowRenderState render_states[MAX_WINDOWS];

} VulkanBackendContext;

static VulkanBackendContext ctx = {0};

static bool validate_window_id(int window_id)
{
    return (window_id >= 0 && window_id < MAX_WINDOWS);
}

static uint32_t find_memory_type(VkPhysicalDevice gpu, uint32_t type_filter, VkMemoryPropertyFlags props)
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(gpu, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    LOG_INFO("Failed to find suitable memory type!\n");
    exit(1);
}

static VulkanBuffer create_buffer(VkDevice device, VkPhysicalDevice gpu, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VulkanBuffer buffer = {0};

    if (device == VK_NULL_HANDLE)
    {
        LOG_INFO("ERROR: Invalid device passed to create_buffer\n");
        return buffer;
    }
    if (gpu == VK_NULL_HANDLE)
    {
        LOG_INFO("ERROR: Invalid physical device passed to create_buffer\n");
        return buffer;
    }
    if (size == 0)
    {
        LOG_INFO("ERROR: Zero size buffer creation requested\n");
        return buffer;
    }

    LOG_INFO("Creating buffer: size=%zu, usage=0x%X, properties=0x%X\n",
             (size_t)size, usage, properties);

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VkResult result = vkCreateBuffer(device, &buffer_info, NULL, &buffer.buffer);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to create buffer: %d\n", result);
        return buffer;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits, properties)};

    result = vkAllocateMemory(device, &alloc_info, NULL, &buffer.memory);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to allocate buffer memory: %d\n", result);
        vkDestroyBuffer(device, buffer.buffer, NULL);
        return buffer;
    }

    result = vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to bind buffer memory: %d\n", result);
        vkDestroyBuffer(device, buffer.buffer, NULL);
        vkFreeMemory(device, buffer.memory, NULL);
        return buffer;
    }

    LOG_INFO("✓ Buffer created successfully\n");
    return buffer;
}

static void destroy_buffer(VkDevice device, VulkanBuffer *buffer)
{
    if (buffer->buffer)
        vkDestroyBuffer(device, buffer->buffer, NULL);
    if (buffer->memory)
        vkFreeMemory(device, buffer->memory, NULL);
    memset(buffer, 0, sizeof(VulkanBuffer));
}

static void copy_to_buffer(VkDevice device, VulkanBuffer *buffer, void *data, VkDeviceSize size)
{
    void *mapped_data;
    vkMapMemory(device, buffer->memory, 0, size, 0, &mapped_data);
    memcpy(mapped_data, data, size);
    vkUnmapMemory(device, buffer->memory);
}

static VkShaderModule create_shader_module(VkDevice device, const uint32_t *code, size_t size)
{
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = code};

    VkShaderModule shader_module;
    VK_CHECK(vkCreateShaderModule(device, &create_info, NULL, &shader_module));
    return shader_module;
}

static uint32_t *read_spirv_file(const char *filename, size_t *size)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (*size % 4 != 0)
    {
        fclose(file);
        return NULL;
    }

    uint32_t *code = (uint32_t *)malloc(*size);
    if (!code || fread(code, 1, *size, file) != *size)
    {
        free(code);
        fclose(file);
        return NULL;
    }
    fclose(file);
    return code;
}

static VulkanPipeline create_graphics_pipeline(VkDevice device, VkRenderPass render_pass,
                                               const char *vert_path, const char *frag_path,
                                               VkVertexInputBindingDescription *binding_desc,
                                               VkVertexInputAttributeDescription *attr_descs,
                                               uint32_t attr_count,
                                               VkDescriptorSetLayout set_layout)
{
    VulkanPipeline pipeline = {0};

    // Load SPIR-V
    size_t vert_size, frag_size;
    uint32_t *vert_code = read_spirv_file(vert_path, &vert_size);
    uint32_t *frag_code = read_spirv_file(frag_path, &frag_size);
    if (!vert_code || !frag_code)
    {
        free(vert_code);
        free(frag_code);
        return pipeline;
    }

    pipeline.vert_shader = create_shader_module(device, vert_code, vert_size);
    pipeline.frag_shader = create_shader_module(device, frag_code, frag_size);
    free(vert_code);
    free(frag_code);

    VkPipelineShaderStageCreateInfo stages[2] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = pipeline.vert_shader,
         .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = pipeline.frag_shader,
         .pName = "main"}};

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = binding_desc,
        .vertexAttributeDescriptionCount = attr_count,
        .pVertexAttributeDescriptions = attr_descs};

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f};

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE};

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment};

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states};

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = 0};
    VK_CHECK(vkCreatePipelineLayout(device, &layout_info, NULL, &pipeline.pipeline_layout));

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipeline.pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE};

    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline.pipeline));
    return pipeline;
}

static void destroy_pipeline(VkDevice device, VulkanPipeline *pipeline)
{
    if (pipeline->pipeline)
        vkDestroyPipeline(device, pipeline->pipeline, NULL);
    if (pipeline->pipeline_layout)
        vkDestroyPipelineLayout(device, pipeline->pipeline_layout, NULL);
    if (pipeline->vert_shader)
        vkDestroyShaderModule(device, pipeline->vert_shader, NULL);
    if (pipeline->frag_shader)
        vkDestroyShaderModule(device, pipeline->frag_shader, NULL);
    memset(pipeline, 0, sizeof(VulkanPipeline));
}

static VulkanTexture create_texture(VkDevice device, VkPhysicalDevice gpu,
                                    int width, int height, void *data, VkFormat format)
{
    VulkanTexture texture = {0};
    texture.width = width;
    texture.height = height;
    texture.image = VK_NULL_HANDLE;
    texture.memory = VK_NULL_HANDLE;
    texture.view = VK_NULL_HANDLE;
    texture.sampler = VK_NULL_HANDLE;

    if (width <= 0 || height <= 0)
        return texture;

    VkResult res;

    uint32_t bpp = 4;
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
        bpp = 1;
        break;
    case VK_FORMAT_R8G8_UNORM:
        bpp = 2;
        break;
    case VK_FORMAT_R8G8B8A8_UNORM:
        bpp = 4;
        break;
    case VK_FORMAT_B8G8R8A8_UNORM:
        bpp = 4;
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        bpp = 16;
        break;
    default:
        bpp = 4;
        break;
    }

    VkDeviceSize image_size = (VkDeviceSize)width * (VkDeviceSize)height * (VkDeviceSize)bpp;

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;

    VkBufferCreateInfo buffer_info = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .size = image_size,
                                      .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                      .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    res = vkCreateBuffer(device, &buffer_info, NULL, &staging_buffer);
    if (res != VK_SUCCESS)
        goto fail;

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device, staging_buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_requirements.size,
        .memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
    res = vkAllocateMemory(device, &alloc_info, NULL, &staging_memory);
    if (res != VK_SUCCESS)
        goto fail;

    vkBindBufferMemory(device, staging_buffer, staging_memory, 0);

    void *mapped_data = NULL;
    res = vkMapMemory(device, staging_memory, 0, mem_requirements.size, 0, &mapped_data);
    if (res != VK_SUCCESS)
        goto fail;
    memcpy(mapped_data, data, (size_t)image_size);
    vkUnmapMemory(device, staging_memory);

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {(uint32_t)width, (uint32_t)height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    res = vkCreateImage(device, &image_info, NULL, &texture.image);
    if (res != VK_SUCCESS)
        goto fail;

    vkGetImageMemoryRequirements(device, texture.image, &mem_requirements);

    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(gpu, mem_requirements.memoryTypeBits,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    res = vkAllocateMemory(device, &alloc_info, NULL, &texture.memory);
    if (res != VK_SUCCESS)
        goto fail;

    vkBindImageMemory(device, texture.image, texture.memory, 0);

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = ctx.queue_family,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};

    VkCommandPool command_pool = VK_NULL_HANDLE;
    res = vkCreateCommandPool(device, &pool_info, NULL, &command_pool);
    if (res != VK_SUCCESS)
        goto fail;

    VkCommandBufferAllocateInfo alloc_info_cb = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    res = vkAllocateCommandBuffers(device, &alloc_info_cb, &command_buffer);
    if (res != VK_SUCCESS)
        goto fail_with_pool;

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

    vkBeginCommandBuffer(command_buffer, &begin_info);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture.image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1},
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT};

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1},
        .imageOffset = {0, 0, 0},
        .imageExtent = {(uint32_t)width, (uint32_t)height, 1}};

    vkCmdCopyBufferToImage(command_buffer, staging_buffer, texture.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer};

    res = vkQueueSubmit(ctx.queue, 1, &submit_info, VK_NULL_HANDLE);
    if (res != VK_SUCCESS)
        goto fail_free_cmd;
    vkQueueWaitIdle(ctx.queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    vkDestroyCommandPool(device, command_pool, NULL);

    vkDestroyBuffer(device, staging_buffer, NULL);
    vkFreeMemory(device, staging_memory, NULL);

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = texture.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    res = vkCreateImageView(device, &view_info, NULL, &texture.view);
    if (res != VK_SUCCESS)
        goto fail_image;

    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f};

    res = vkCreateSampler(device, &sampler_info, NULL, &texture.sampler);
    if (res != VK_SUCCESS)
        goto fail_view;

    return texture;

fail_view:
    vkDestroyImageView(device, texture.view, NULL);
fail_image:
    if (texture.memory != VK_NULL_HANDLE)
        vkFreeMemory(device, texture.memory, NULL);
    if (texture.image != VK_NULL_HANDLE)
        vkDestroyImage(device, texture.image, NULL);
fail_free_cmd:
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    vkDestroyCommandPool(device, command_pool, NULL);
fail_with_pool:
    if (staging_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(device, staging_buffer, NULL);
    if (staging_memory != VK_NULL_HANDLE)
        vkFreeMemory(device, staging_memory, NULL);
fail:
    VulkanTexture empty = {0};
    return empty;
}

static void destroy_texture(VkDevice device, VulkanTexture *texture)
{
    if (texture->sampler)
        vkDestroySampler(device, texture->sampler, NULL);
    if (texture->view)
        vkDestroyImageView(device, texture->view, NULL);
    if (texture->image)
        vkDestroyImage(device, texture->image, NULL);
    if (texture->memory)
        vkFreeMemory(device, texture->memory, NULL);
    memset(texture, 0, sizeof(VulkanTexture));
}

// Batch rendering functions
static void init_window_render_state(int window_id)
{
    WindowRenderState *state = &ctx.render_states[window_id];
    memset(state, 0, sizeof(WindowRenderState));

    // Initial capacity
    state->shape_batch.vertex_capacity = 1024;
    state->shape_batch.vertices = malloc(state->shape_batch.vertex_capacity * sizeof(Vertex));
    state->command_capacity = 64;
    state->draw_commands = malloc(state->command_capacity * sizeof(DrawCommand));
    state->needs_redraw = true;
}

static void cleanup_window_render_state(int window_id)
{
    WindowRenderState *state = &ctx.render_states[window_id];
    if (state->shape_batch.vertices)
    {
        free(state->shape_batch.vertices);
    }
    if (state->draw_commands)
    {
        free(state->draw_commands);
    }
    memset(state, 0, sizeof(WindowRenderState));
}

static void add_shape_to_batch(int window_id, Vertex vertices[6], ShapeParamsUBO params)
{
    WindowRenderState *state = &ctx.render_states[window_id];

    // Grow vertex buffer if needed
    if (state->shape_batch.vertex_count + 6 > state->shape_batch.vertex_capacity)
    {
        state->shape_batch.vertex_capacity *= 2;
        state->shape_batch.vertices = realloc(state->shape_batch.vertices,
                                              state->shape_batch.vertex_capacity * sizeof(Vertex));
    }

    // Grow command buffer if needed
    if (state->command_count >= state->command_capacity)
    {
        state->command_capacity *= 2;
        state->draw_commands = realloc(state->draw_commands,
                                       state->command_capacity * sizeof(DrawCommand));
    }

    // Add vertices
    uint32_t vertex_offset = state->shape_batch.vertex_count;
    memcpy(&state->shape_batch.vertices[vertex_offset], vertices, 6 * sizeof(Vertex));
    state->shape_batch.vertex_count += 6;

    // Add draw command
    DrawCommand *cmd = &state->draw_commands[state->command_count++];
    cmd->params = params;
    cmd->vertex_offset = vertex_offset;
    cmd->vertex_count = 6;
    cmd->descriptor_set = ctx.shape_descriptor_sets[window_id];

    state->needs_redraw = true;
}

static void render_batch(int window_id)
{
    if (!ctx.render_states[window_id].needs_redraw)
    {
        return;
    }

    WindowRenderState *state = &ctx.render_states[window_id];
    if (state->shape_batch.vertex_count == 0)
    {
        return;
    }

    // Wait for previous frame to complete
    vkWaitForFences(ctx.device, 1, &ctx.in_flight_fences[window_id], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(ctx.device, ctx.swapchains[window_id], UINT64_MAX,
                                            ctx.image_available_semaphores[window_id], VK_NULL_HANDLE,
                                            &ctx.image_indices[window_id]);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to acquire swapchain image for window %d: %d\n", window_id, result);
        return;
    }

    // Check if a previous frame is using this image
    if (ctx.images_in_flight[window_id][ctx.image_indices[window_id]] != VK_NULL_HANDLE)
    {
        vkWaitForFences(ctx.device, 1, &ctx.images_in_flight[window_id][ctx.image_indices[window_id]], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    ctx.images_in_flight[window_id][ctx.image_indices[window_id]] = ctx.in_flight_fences[window_id];

    vkResetFences(ctx.device, 1, &ctx.in_flight_fences[window_id]);

    // Update vertex buffer
    copy_to_buffer(ctx.device, &ctx.shape_vertex_buffers[window_id],
                   state->shape_batch.vertices,
                   state->shape_batch.vertex_count * sizeof(Vertex));

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL};

    VK_CHECK(vkBeginCommandBuffer(ctx.command_buffers[window_id][ctx.image_indices[window_id]], &begin_info));

    VkClearValue clear_color = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx.render_passes[window_id],
        .framebuffer = ctx.framebuffers[window_id][ctx.image_indices[window_id]],
        .renderArea = {{0, 0}, ctx.window_extents[window_id]},
        .clearValueCount = 1,
        .pClearValues = &clear_color};

    vkCmdBeginRenderPass(ctx.command_buffers[window_id][ctx.image_indices[window_id]],
                         &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(ctx.command_buffers[window_id][ctx.image_indices[window_id]],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ctx.shape_pipeline.pipeline);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)ctx.window_extents[window_id].width,
        .height = (float)ctx.window_extents[window_id].height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f};
    vkCmdSetViewport(ctx.command_buffers[window_id][ctx.image_indices[window_id]], 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = ctx.window_extents[window_id]};
    vkCmdSetScissor(ctx.command_buffers[window_id][ctx.image_indices[window_id]], 0, 1, &scissor);

    VkBuffer vertex_buffers[] = {ctx.shape_vertex_buffers[window_id].buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(ctx.command_buffers[window_id][ctx.image_indices[window_id]],
                           0, 1, vertex_buffers, offsets);

    // Draw all batched commands
    for (uint32_t i = 0; i < state->command_count; i++)
    {
        DrawCommand *cmd = &state->draw_commands[i];

        // Update uniform buffer for this command
        copy_to_buffer(ctx.device, &ctx.shape_uniform_buffers[window_id],
                       &cmd->params, sizeof(ShapeParamsUBO));

        vkCmdBindDescriptorSets(ctx.command_buffers[window_id][ctx.image_indices[window_id]],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                ctx.shape_pipeline.pipeline_layout,
                                0, 1, &cmd->descriptor_set,
                                0, NULL);

        vkCmdDraw(ctx.command_buffers[window_id][ctx.image_indices[window_id]],
                  cmd->vertex_count, 1, cmd->vertex_offset, 0);
    }

    vkCmdEndRenderPass(ctx.command_buffers[window_id][ctx.image_indices[window_id]]);
    VK_CHECK(vkEndCommandBuffer(ctx.command_buffers[window_id][ctx.image_indices[window_id]]));

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx.image_available_semaphores[window_id],
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &ctx.command_buffers[window_id][ctx.image_indices[window_id]],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &ctx.render_finished_semaphores[window_id]};

    VK_CHECK(vkQueueSubmit(ctx.queue, 1, &submit_info, ctx.in_flight_fences[window_id]));

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &ctx.render_finished_semaphores[window_id],
        .swapchainCount = 1,
        .pSwapchains = &ctx.swapchains[window_id],
        .pImageIndices = &ctx.image_indices[window_id],
        .pResults = NULL};

    result = vkQueuePresentKHR(ctx.queue, &present_info);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to present image for window %d: %d\n", window_id, result);
        return;
    }

    // Reset batch for next frame
    state->shape_batch.vertex_count = 0;
    state->command_count = 0;
    state->needs_redraw = false;

    // Advance to next frame
    ctx.current_frame = (ctx.current_frame + 1) % ctx.max_frames_in_flight;
}

void glps_setup_shared()
{
    LOG_INFO("=== Starting Vulkan initialization ===\n");

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Gooey Vulkan",
        .apiVersion = VK_API_VERSION_1_0};

    glps_VulkanExtensionArray exts_arr = glps_wm_vk_get_extensions_arr();
    LOG_INFO("Number of instance extensions: %zu\n", exts_arr.count);
    for (size_t i = 0; i < exts_arr.count; i++)
        LOG_INFO("  Extension %zu: %s\n", i, exts_arr.names[i]);

    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = exts_arr.count,
        .ppEnabledExtensionNames = exts_arr.names};

    VK_CHECK(vkCreateInstance(&inst_info, NULL, &ctx.instance));
    LOG_INFO("✓ Vulkan instance created successfully\n");

    uint32_t gpu_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(ctx.instance, &gpu_count, NULL));
    if (gpu_count == 0)
    {
        LOG_INFO("No physical devices found\n");
        return;
    }

    VkPhysicalDevice devices[8];
    VK_CHECK(vkEnumeratePhysicalDevices(ctx.instance, &gpu_count, devices));
    ctx.gpu = devices[0];
    LOG_INFO("✓ Selected physical device: %p\n", (void *)ctx.gpu);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.gpu, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.gpu, &queue_family_count, queue_families);

    ctx.queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            ctx.queue_family = i;
            LOG_INFO("✓ Selected queue family %u (graphics)\n", i);
            break;
        }
    }
    if (ctx.queue_family == UINT32_MAX)
    {
        LOG_INFO("No graphics queue found\n");
        return;
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = ctx.queue_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};

    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions};

    VK_CHECK(vkCreateDevice(ctx.gpu, &device_info, NULL, &ctx.device));
    LOG_INFO("✓ Logical device created: %p\n", (void *)ctx.device);

    vkGetDeviceQueue(ctx.device, ctx.queue_family, 0, &ctx.queue);
    LOG_INFO("✓ Device queue obtained\n");

    // Frame synchronization setup
    ctx.max_frames_in_flight = 2;
    ctx.current_frame = 0;

    // Allocate arrays
    ctx.surfaces = calloc(MAX_WINDOWS, sizeof(VkSurfaceKHR));
    ctx.swapchains = calloc(MAX_WINDOWS, sizeof(VkSwapchainKHR));
    ctx.render_passes = calloc(MAX_WINDOWS, sizeof(VkRenderPass));
    ctx.framebuffers = calloc(MAX_WINDOWS, sizeof(VkFramebuffer *));
    ctx.command_pools = calloc(MAX_WINDOWS, sizeof(VkCommandPool));
    ctx.command_buffers = calloc(MAX_WINDOWS, sizeof(VkCommandBuffer *));
    ctx.image_available_semaphores = calloc(MAX_WINDOWS, sizeof(VkSemaphore));
    ctx.render_finished_semaphores = calloc(MAX_WINDOWS, sizeof(VkSemaphore));
    ctx.in_flight_fences = calloc(MAX_WINDOWS, sizeof(VkFence));
    ctx.images_in_flight = calloc(MAX_WINDOWS, sizeof(VkFence *));
    ctx.image_indices = calloc(MAX_WINDOWS, sizeof(uint32_t));
    ctx.swapchain_image_counts = calloc(MAX_WINDOWS, sizeof(uint32_t));
    ctx.text_vertex_buffers = calloc(MAX_WINDOWS, sizeof(VulkanBuffer));
    ctx.shape_vertex_buffers = calloc(MAX_WINDOWS, sizeof(VulkanBuffer));
    ctx.window_extents = calloc(MAX_WINDOWS, sizeof(VkExtent2D));
    ctx.swapchain_formats = calloc(MAX_WINDOWS, sizeof(VkFormat));

    // New uniform buffers and descriptor sets
    ctx.shape_uniform_buffers = calloc(MAX_WINDOWS, sizeof(VulkanBuffer));
    ctx.text_matrices_uniform_buffers = calloc(MAX_WINDOWS, sizeof(VulkanBuffer));
    ctx.text_params_uniform_buffers = calloc(MAX_WINDOWS, sizeof(VulkanBuffer));
    ctx.shape_descriptor_sets = calloc(MAX_WINDOWS, sizeof(VkDescriptorSet));
    ctx.text_descriptor_sets = calloc(MAX_WINDOWS, sizeof(VkDescriptorSet));

    if (!ctx.surfaces || !ctx.swapchains || !ctx.render_passes || !ctx.framebuffers ||
        !ctx.command_pools || !ctx.command_buffers || !ctx.image_available_semaphores ||
        !ctx.render_finished_semaphores || !ctx.in_flight_fences || !ctx.images_in_flight ||
        !ctx.image_indices || !ctx.swapchain_image_counts || !ctx.text_vertex_buffers ||
        !ctx.shape_vertex_buffers || !ctx.window_extents || !ctx.swapchain_formats ||
        !ctx.shape_uniform_buffers || !ctx.text_matrices_uniform_buffers ||
        !ctx.text_params_uniform_buffers || !ctx.shape_descriptor_sets ||
        !ctx.text_descriptor_sets)
    {
        LOG_INFO("FAILED to allocate Vulkan resource arrays\n");
        return;
    }

    // Create descriptor set layouts
    VkDescriptorSetLayoutBinding shape_bindings[] = {
        {.binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT},
        {.binding = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT}};

    VkDescriptorSetLayoutCreateInfo shape_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = shape_bindings};
    VK_CHECK(vkCreateDescriptorSetLayout(ctx.device, &shape_layout_info, NULL, &ctx.shape_set_layout));

    VkDescriptorSetLayoutBinding text_bindings[] = {
        {.binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT},
        {.binding = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}};

    VkDescriptorSetLayoutCreateInfo text_layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = text_bindings};
    VK_CHECK(vkCreateDescriptorSetLayout(ctx.device, &text_layout_info, NULL, &ctx.text_set_layout));

    // Create descriptor pool
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_WINDOWS * 2},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_WINDOWS * 3}};

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = MAX_WINDOWS * 2,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes};
    VK_CHECK(vkCreateDescriptorPool(ctx.device, &pool_info, NULL, &ctx.descriptor_pool));

    // Allocate descriptor sets
    for (int i = 0; i < MAX_WINDOWS; i++)
    {
        VkDescriptorSetLayout layouts[] = {ctx.shape_set_layout};
        VkDescriptorSetAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = ctx.descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts};
        VK_CHECK(vkAllocateDescriptorSets(ctx.device, &alloc_info, &ctx.shape_descriptor_sets[i]));

        layouts[0] = ctx.text_set_layout;
        VK_CHECK(vkAllocateDescriptorSets(ctx.device, &alloc_info, &ctx.text_descriptor_sets[i]));

        // Initialize render state
        init_window_render_state(i);
    }

    // Synchronization objects
    VkSemaphoreCreateInfo semaphore_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (int i = 0; i < MAX_WINDOWS; i++)
    {
        VK_CHECK(vkCreateSemaphore(ctx.device, &semaphore_info, NULL, &ctx.image_available_semaphores[i]));
        VK_CHECK(vkCreateSemaphore(ctx.device, &semaphore_info, NULL, &ctx.render_finished_semaphores[i]));
        VK_CHECK(vkCreateFence(ctx.device, &fence_info, NULL, &ctx.in_flight_fences[i]));

        // Initialize images_in_flight array
        ctx.images_in_flight[i] = calloc(ctx.max_frames_in_flight, sizeof(VkFence));
    }
    LOG_INFO("✓ Synchronization objects created\n");

    LOG_INFO("=== Vulkan initialization completed successfully ===\n");
}

void glps_setup_seperate_vao(int window_id)
{
    if (!validate_window_id(window_id))
        return;

    LOG_INFO("Setting up VAO for window %d\n", window_id);

    // Create surface
    glps_wm_vk_create_surface(ctx.wm, window_id, &ctx.instance, &ctx.surfaces[window_id]);

    VkSurfaceCapabilitiesKHR caps;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.gpu, ctx.surfaces[window_id], &caps);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to get surface capabilities for window %d: %d\n", window_id, result);
        return;
    }

    uint32_t format_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.gpu, ctx.surfaces[window_id], &format_count, NULL);
    if (result != VK_SUCCESS || format_count == 0)
    {
        LOG_INFO("Failed to get surface formats for window %d: %d (count: %u)\n", window_id, result, format_count);
        return;
    }

    VkSurfaceFormatKHR formats[32];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.gpu, ctx.surfaces[window_id], &format_count, formats);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to get surface formats for window %d: %d\n", window_id, result);
        return;
    }

    VkSurfaceFormatKHR format = formats[0];
    ctx.swapchain_formats[window_id] = format.format;

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
    {
        extent = (VkExtent2D){800, 600};
    }
    ctx.window_extents[window_id] = extent;

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
    {
        image_count = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ctx.surfaces[window_id],
        .minImageCount = image_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE};

    result = vkCreateSwapchainKHR(ctx.device, &swapchain_info, NULL, &ctx.swapchains[window_id]);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to create swapchain for window %d: %d\n", window_id, result);
        return;
    }

    result = vkGetSwapchainImagesKHR(ctx.device, ctx.swapchains[window_id], &ctx.swapchain_image_counts[window_id], NULL);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to get swapchain image count for window %d: %d\n", window_id, result);
        return;
    }

    VkImage images[8];
    result = vkGetSwapchainImagesKHR(ctx.device, ctx.swapchains[window_id], &ctx.swapchain_image_counts[window_id], images);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to get swapchain images for window %d: %d\n", window_id, result);
        return;
    }

    VkImageView *image_views = malloc(ctx.swapchain_image_counts[window_id] * sizeof(VkImageView));
    if (!image_views)
    {
        LOG_INFO("Failed to allocate image views for window %d\n", window_id);
        return;
    }

    for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
    {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1}};

        result = vkCreateImageView(ctx.device, &view_info, NULL, &image_views[i]);
        if (result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create image view %u for window %d: %d\n", i, window_id, result);
            for (uint32_t j = 0; j < i; j++)
            {
                vkDestroyImageView(ctx.device, image_views[j], NULL);
            }
            free(image_views);
            return;
        }
    }

    VkAttachmentDescription color_attachment = {
        .format = format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref};

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass};

    result = vkCreateRenderPass(ctx.device, &render_pass_info, NULL, &ctx.render_passes[window_id]);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to create render pass for window %d: %d\n", window_id, result);
        for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
        {
            vkDestroyImageView(ctx.device, image_views[i], NULL);
        }
        free(image_views);
        return;
    }

    // Vertex input descriptions
    VkVertexInputBindingDescription shape_vertex_binding = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription shape_attrs[] = {
        {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)}};

    // Text pipeline uses vec4 for vertex data (xy=position, zw=texcoords)
    VkVertexInputBindingDescription text_vertex_binding = {
        .binding = 0,
        .stride = sizeof(float) * 4,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription text_attrs[] = {
        {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0} // Single vec4 attribute
    };

    // Create pipelines with descriptor set layouts
    if (ctx.shape_pipeline.pipeline == VK_NULL_HANDLE)
    {
        ctx.shape_pipeline = create_graphics_pipeline(ctx.device, ctx.render_passes[window_id],
                                                      "./shape_vert.spv", "./shape_frag.spv",
                                                      &shape_vertex_binding, shape_attrs, 3, ctx.shape_set_layout);
        if (ctx.shape_pipeline.pipeline == VK_NULL_HANDLE)
        {
            LOG_INFO("Failed to create shape pipeline for window %d\n", window_id);
            for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
            {
                vkDestroyImageView(ctx.device, image_views[i], NULL);
            }
            free(image_views);
            return;
        }
    }

    if (ctx.text_pipeline.pipeline == VK_NULL_HANDLE)
    {
        ctx.text_pipeline = create_graphics_pipeline(ctx.device, ctx.render_passes[window_id],
                                                     "./text_vert.spv", "./text_frag.spv",
                                                     &text_vertex_binding, text_attrs, 1, ctx.text_set_layout);
        if (ctx.text_pipeline.pipeline == VK_NULL_HANDLE)
        {
            LOG_INFO("Failed to create text pipeline for window %d\n", window_id);
            for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
            {
                vkDestroyImageView(ctx.device, image_views[i], NULL);
            }
            free(image_views);
            return;
        }
    }

    // Continue with framebuffers creation...
    ctx.framebuffers[window_id] = malloc(ctx.swapchain_image_counts[window_id] * sizeof(VkFramebuffer));
    if (!ctx.framebuffers[window_id])
    {
        LOG_INFO("Failed to allocate framebuffers for window %d\n", window_id);
        for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
        {
            vkDestroyImageView(ctx.device, image_views[i], NULL);
        }
        free(image_views);
        return;
    }

    for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
    {
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx.render_passes[window_id],
            .attachmentCount = 1,
            .pAttachments = &image_views[i],
            .width = extent.width,
            .height = extent.height,
            .layers = 1};

        result = vkCreateFramebuffer(ctx.device, &framebuffer_info, NULL, &ctx.framebuffers[window_id][i]);
        if (result != VK_SUCCESS)
        {
            LOG_INFO("Failed to create framebuffer %u for window %d: %d\n", i, window_id, result);
            for (uint32_t j = 0; j < i; j++)
            {
                vkDestroyFramebuffer(ctx.device, ctx.framebuffers[window_id][j], NULL);
            }
            for (uint32_t j = 0; j < ctx.swapchain_image_counts[window_id]; j++)
            {
                vkDestroyImageView(ctx.device, image_views[j], NULL);
            }
            free(image_views);
            free(ctx.framebuffers[window_id]);
            ctx.framebuffers[window_id] = NULL;
            return;
        }
    }

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = ctx.queue_family,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};

    result = vkCreateCommandPool(ctx.device, &pool_info, NULL, &ctx.command_pools[window_id]);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to create command pool for window %d: %d\n", window_id, result);
        for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
        {
            vkDestroyFramebuffer(ctx.device, ctx.framebuffers[window_id][i], NULL);
            vkDestroyImageView(ctx.device, image_views[i], NULL);
        }
        free(image_views);
        free(ctx.framebuffers[window_id]);
        ctx.framebuffers[window_id] = NULL;
        return;
    }

    ctx.command_buffers[window_id] = malloc(ctx.swapchain_image_counts[window_id] * sizeof(VkCommandBuffer));
    if (!ctx.command_buffers[window_id])
    {
        LOG_INFO("Failed to allocate command buffers for window %d\n", window_id);
        for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
        {
            vkDestroyFramebuffer(ctx.device, ctx.framebuffers[window_id][i], NULL);
            vkDestroyImageView(ctx.device, image_views[i], NULL);
        }
        free(image_views);
        free(ctx.framebuffers[window_id]);
        ctx.framebuffers[window_id] = NULL;
        return;
    }

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx.command_pools[window_id],
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ctx.swapchain_image_counts[window_id]};

    result = vkAllocateCommandBuffers(ctx.device, &alloc_info, ctx.command_buffers[window_id]);
    if (result != VK_SUCCESS)
    {
        LOG_INFO("Failed to allocate command buffers for window %d: %d\n", window_id, result);
        for (uint32_t i = 0; i < ctx.swapchain_image_counts[window_id]; i++)
        {
            vkDestroyFramebuffer(ctx.device, ctx.framebuffers[window_id][i], NULL);
            vkDestroyImageView(ctx.device, image_views[i], NULL);
        }
        free(image_views);
        free(ctx.framebuffers[window_id]);
        ctx.framebuffers[window_id] = NULL;
        free(ctx.command_buffers[window_id]);
        ctx.command_buffers[window_id] = NULL;
        return;
    }

    // Create vertex buffers with larger capacity for batching
    ctx.text_vertex_buffers[window_id] = create_buffer(ctx.device, ctx.gpu,
                                                       sizeof(float) * 6 * 4 * 256, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ctx.shape_vertex_buffers[window_id] = create_buffer(ctx.device, ctx.gpu,
                                                        sizeof(Vertex) * 6 * 256, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Create uniform buffers
    ctx.shape_uniform_buffers[window_id] = create_buffer(ctx.device, ctx.gpu,
                                                         sizeof(ShapeParamsUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ctx.text_matrices_uniform_buffers[window_id] = create_buffer(ctx.device, ctx.gpu,
                                                                 sizeof(TextMatricesUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    ctx.text_params_uniform_buffers[window_id] = create_buffer(ctx.device, ctx.gpu,
                                                               sizeof(TextParamsUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Update descriptor sets
    VkDescriptorBufferInfo shape_buffer_info = {
        .buffer = ctx.shape_uniform_buffers[window_id].buffer,
        .offset = 0,
        .range = sizeof(ShapeParamsUBO)};

    VkWriteDescriptorSet shape_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ctx.shape_descriptor_sets[window_id],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &shape_buffer_info};

    VkDescriptorBufferInfo text_matrices_buffer_info = {
        .buffer = ctx.text_matrices_uniform_buffers[window_id].buffer,
        .offset = 0,
        .range = sizeof(TextMatricesUBO)};

    VkWriteDescriptorSet text_matrices_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ctx.text_descriptor_sets[window_id],
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &text_matrices_buffer_info};

    vkUpdateDescriptorSets(ctx.device, 1, &shape_write, 0, NULL);
    vkUpdateDescriptorSets(ctx.device, 1, &text_matrices_write, 0, NULL);

    // Cleanup image views (they're now stored in framebuffers)
    free(image_views);

    LOG_INFO("✓ Window %d setup completed successfully\n", window_id);
}
void glps_draw_rectangle(int x, int y, int width, int height,
                         uint32_t color, float thickness,
                         int window_id, bool isRounded, float cornerRadius,
                         GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    int win_width, win_height;
    get_window_size(ctx.wm, window_id, &win_width, &win_height);

    float ndc_x, ndc_y, ndc_width, ndc_height;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);

    vec3 color_rgb;
    convert_hex_to_rgb(&color_rgb, color);  // Should output normalized [0..1] RGB

    // Rectangle made of two triangles
    Vertex vertices[6] = {
        {{ndc_x,               ndc_y              }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x + ndc_width,   ndc_y              }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x,               ndc_y + ndc_height }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}},

        {{ndc_x + ndc_width,   ndc_y              }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x + ndc_width,   ndc_y + ndc_height }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}},
        {{ndc_x,               ndc_y + ndc_height }, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}}
    };

    ShapeParamsUBO shape_params = {
        .useTexture  = false,
        .isRounded   = isRounded,
        .isHollow    = true, // Only hollow if a nonzero thickness was specified
        .shapeType   = 0,                   // Rectangle type
        .radius      = cornerRadius,
        .borderWidth = thickness,
        .size        = {ndc_width, ndc_height},
        .padding     = {0.0f, 0.0f}         // Always initialize padding to zero
    };

    add_shape_to_batch(window_id, vertices, shape_params);
}


void glps_set_foreground(uint32_t color)
{
    ctx.selected_color = color;
}

void glps_window_dim(int *width, int *height, int window_id)
{
    get_window_size(ctx.wm, window_id, width, height);
}
void glps_draw_line(int x1, int y1, int x2, int y2, uint32_t color, int window_id, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    float ndc_x1, ndc_y1, ndc_x2, ndc_y2;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x1, &ndc_y1, x1, y1);
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x2, &ndc_y2, x2, y2);
    convert_hex_to_rgb(&color_rgb, color);

    // Create a proper quad for the line
    Vertex vertices[6] = {
        {{ndc_x1, ndc_y1}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x2, ndc_y2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}},
        {{ndc_x1, ndc_y1}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x2, ndc_y2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}},
        {{ndc_x1, ndc_y1}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x2, ndc_y2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}}};

    ShapeParamsUBO shape_params = {
        .useTexture = false,
        .isRounded = false,
        .isHollow = false,
        .shapeType = 1, // line
        .radius = 0.0f,
        .borderWidth = 2.0f, // line width
        .size = {fabsf(ndc_x2 - ndc_x1), fabsf(ndc_y2 - ndc_y1)}};

    add_shape_to_batch(window_id, vertices, shape_params);
}
void glps_fill_arc(int x_center, int y_center, int width, int height,
                   int angle1, int angle2, int window_id, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    float ndc_x_center, ndc_y_center, ndc_width, ndc_height;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x_center, &ndc_y_center, x_center, y_center);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);

    vec3 color_rgb;
    convert_hex_to_rgb(&color_rgb, ctx.selected_color);

    // Simple quad covering the arc area
    Vertex vertices[6] = {
        {{ndc_x_center - ndc_width / 2, ndc_y_center - ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x_center + ndc_width / 2, ndc_y_center - ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x_center - ndc_width / 2, ndc_y_center + ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}},
        {{ndc_x_center + ndc_width / 2, ndc_y_center - ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x_center + ndc_width / 2, ndc_y_center + ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}},
        {{ndc_x_center - ndc_width / 2, ndc_y_center + ndc_height / 2}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}}};

    ShapeParamsUBO shape_params = {
        .useTexture = false,
        .isRounded = false,
        .isHollow = false,
        .shapeType = 2, // arc
        .radius = fmin(ndc_width, ndc_height) * 0.5f,
        .borderWidth = 0.0f,
        .size = {ndc_width, ndc_height}};

    add_shape_to_batch(window_id, vertices, shape_params);
}
void glps_draw_image(unsigned int texture_id, int x, int y, int width, int height, int window_id)
{
    if (!validate_window_id(window_id) || texture_id >= ctx.texture_count)
        return;

    float ndc_x, ndc_y, ndc_width, ndc_height;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);

    Vertex vertices[6] = {
        {{ndc_x, ndc_y}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{ndc_x + ndc_width, ndc_y}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{ndc_x, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ndc_x + ndc_width, ndc_y}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{ndc_x + ndc_width, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{ndc_x, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}};

    // Update uniform buffer for textured rectangle
    ShapeParamsUBO shape_params = {
        .useTexture = true,
        .isRounded = false,
        .isHollow = false,
        .shapeType = 0, // rectangle
        .radius = 0.0f,
        .borderWidth = 0.0f,
        .size = {ndc_width, ndc_height}};

    // Update descriptor set with texture
    VkDescriptorImageInfo image_info = {
        .sampler = ctx.loaded_textures[texture_id].sampler,
        .imageView = ctx.loaded_textures[texture_id].view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet texture_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ctx.shape_descriptor_sets[window_id],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info};

    vkUpdateDescriptorSets(ctx.device, 1, &texture_write, 0, NULL);

    add_shape_to_batch(window_id, vertices, shape_params);
}

void glps_fill_rectangle(int x, int y, int width, int height,
                         uint32_t color, int window_id,
                         bool isRounded, float cornerRadius, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    // Get window dimensions for coordinate conversion
    int win_width, win_height;
    get_window_size(ctx.wm, window_id, &win_width, &win_height);

    // Convert coordinates to NDC (Normalized Device Coordinates)
    float ndc_x, ndc_y, ndc_width, ndc_height;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);

    // Convert hex color to RGB
    vec3 color_rgb;
    convert_hex_to_rgb(&color_rgb, color);

    // Create vertices for two triangles forming a rectangle
    Vertex vertices[6] = {
        // First triangle
        {{ndc_x, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 0.0f}},
        {{ndc_x + ndc_width, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}},

        // Second triangle
        {{ndc_x + ndc_width, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 0.0f}},
        {{ndc_x + ndc_width, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0f, 1.0f}},
        {{ndc_x, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0f, 1.0f}}};

    // Update uniform buffer
    ShapeParamsUBO shape_params = {
        .useTexture = false,
        .isRounded = isRounded,
        .isHollow = false,
        .shapeType = 0, // rectangle
        .radius = cornerRadius,
        .borderWidth = 0.0f,
        .size = {ndc_width, ndc_height}};

    add_shape_to_batch(window_id, vertices, shape_params);
}

static void keyboard_callback(size_t window_id, bool state, const char *value, unsigned long keycode,
                              void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;

    event->type = state ? GOOEY_EVENT_KEY_PRESS : GOOEY_EVENT_KEY_RELEASE;
    event->key_press.state = state;
    strncpy(event->key_press.value, value, sizeof(event->key_press.value));
    event->key_press.keycode = keycode;
}

static void mouse_scroll_callback(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;

    event->type = GOOEY_EVENT_MOUSE_SCROLL;

    if (axe == GLPS_SCROLL_H_AXIS)
        event->mouse_scroll.x = value;
    else
        event->mouse_scroll.y = value;
}

static void mouse_click_callback(size_t window_id, bool state, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    event->type = state ? GOOEY_EVENT_CLICK_PRESS : GOOEY_EVENT_CLICK_RELEASE;
    event->click.x = event->mouse_move.x;
    event->click.y = event->mouse_move.y;
}

void glps_request_redraw(GooeyWindow *win)
{
    GooeyEvent *event = (GooeyEvent *)win->current_event;
    event->type = GOOEY_EVENT_REDRAWREQ;
}

void glps_force_redraw()
{
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    event->mouse_move.x = posX;
    event->mouse_move.y = posY;
}

static void window_resize_callback(size_t window_id, int width, int height, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    GooeyWindow *win = (GooeyWindow *)windows[window_id];
    event->type = GOOEY_EVENT_RESIZE;
    win->width = width;
    win->height = height;
}

static void window_close_callback(size_t window_id, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    event->type = GOOEY_EVENT_WINDOW_CLOSE;
}

int glps_init_ft()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "roboto.ttf", 0, &face))
    {
        FT_Done_FreeType(ft);
        return -1;
    }

    FT_Set_Char_Size(face, 0, 28 * 28, 300, 300);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            continue;
        }

        unsigned char *bitmap_data = malloc(face->glyph->bitmap.width * face->glyph->bitmap.rows);
        memcpy(bitmap_data, face->glyph->bitmap.buffer, face->glyph->bitmap.width * face->glyph->bitmap.rows);

        VulkanTexture texture = create_texture(ctx.device, ctx.gpu,
                                               face->glyph->bitmap.width, face->glyph->bitmap.rows,
                                               bitmap_data, VK_FORMAT_R8_UNORM);

        free(bitmap_data);

        VulkanCharacter character = {
            .texture = texture,
            .width = face->glyph->bitmap.width,
            .height = face->glyph->bitmap.rows,
            .bearingX = face->glyph->bitmap_left,
            .bearingY = face->glyph->bitmap_top,
            .advance = (unsigned int)face->glyph->advance.x};
        ctx.characters[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}

int glps_init(int project_branch)
{
    NFD_Init();
    set_logging_enabled(project_branch);
    ctx.inhibit_reset = 0;
    ctx.selected_color = 0x000000;
    ctx.active_window_count = 0;
    ctx.wm = glps_wm_init();
    ctx.timers = (glps_timer **)calloc(MAX_TIMERS, sizeof(glps_timer *));
    ctx.timer_count = 0;
    ctx.is_running = true;
    ctx.loaded_textures = NULL;
    ctx.texture_count = 0;
    return 0;
}

int glps_get_current_clicked_window(void)
{
    return -1;
}

void glps_draw_text(int x, int y, const char *text, uint32_t color, float font_size, int window_id, GooeyTFT_Sprite *sprite)
{
    // Text rendering temporarily disabled for focus on shape fixes
    return;
}

void glps_unload_image(unsigned int texture_id)
{
    if (texture_id < ctx.texture_count)
    {
        destroy_texture(ctx.device, &ctx.loaded_textures[texture_id]);
    }
}

static int is_stb_supported_image_format(const char *path)
{
    if (!path)
        return 0;
    const char *ext = strrchr(path, '.');
    if (!ext)
        return 0;
    ext++;
    if (strcasecmp(ext, "png") == 0 || strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
    {
        return 1;
    }
    return 0;
}

unsigned int glps_load_image(const char *image_path)
{
    int width, height, channels;
    unsigned char *data = NULL;

    if (!is_stb_supported_image_format(image_path))
    {
        NSVGimage *image = nsvgParseFromFile(image_path, "px", 96);
        if (image)
        {
            NSVGrasterizer *rast = nsvgCreateRasterizer();
            width = (int)image->width;
            height = (int)image->height;
            channels = 4;
            data = malloc(width * height * 4);
            nsvgRasterize(rast, image, 0, 0, 1, data, width, height, width * 4);
            nsvgDeleteRasterizer(rast);
            nsvgDelete(image);
        }
    }
    else
    {
        stbi_set_flip_vertically_on_load(1);
        data = stbi_load(image_path, &width, &height, &channels, 0);
    }

    if (!data)
    {
        return 0;
    }

    VkFormat format = (channels == 4) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM;
    VulkanTexture texture = create_texture(ctx.device, ctx.gpu, width, height, data, format);

    if (is_stb_supported_image_format(image_path))
    {
        stbi_image_free(data);
    }
    else
    {
        free(data);
    }

    ctx.loaded_textures = realloc(ctx.loaded_textures, (ctx.texture_count + 1) * sizeof(VulkanTexture));
    ctx.loaded_textures[ctx.texture_count] = texture;
    return ctx.texture_count++;
}

unsigned int glps_load_image_from_bin(unsigned char *data, long unsigned binary_len)
{
    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char *img_data = stbi_load_from_memory(data, binary_len, &width, &height, &channels, 0);
    if (!img_data)
    {
        return 0;
    }

    VkFormat format = (channels == 4) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM;
    VulkanTexture texture = create_texture(ctx.device, ctx.gpu, width, height, img_data, format);

    stbi_image_free(img_data);

    ctx.loaded_textures = realloc(ctx.loaded_textures, (ctx.texture_count + 1) * sizeof(VulkanTexture));
    ctx.loaded_textures[ctx.texture_count] = texture;
    return ctx.texture_count++;
}

GooeyWindow *glps_create_window(const char *title, int width, int height)
{
    GooeyWindow *window = (GooeyWindow *)malloc(sizeof(GooeyWindow));

    size_t window_id = glps_wm_window_create(ctx.wm, title, width, height);
    window->creation_id = window_id;

    if (window->creation_id == 0)
        glps_setup_shared();

    glps_init_ft();

    glps_setup_seperate_vao(window->creation_id);
    ctx.active_window_count++;

    return window;
}

void glps_make_window_visible(int window_id, bool visibility)
{
}

void glps_set_window_resizable(bool value, int window_id)
{
    glps_wm_window_is_resizable(ctx.wm, value, window_id);
}

void glps_hide_current_child(void)
{
}

void glps_destroy_windows()
{
}

void glps_clear(GooeyWindow *win)
{
    size_t window_id = win->creation_id;
    if (!validate_window_id(window_id))
        return;

    // Clear the batch
    WindowRenderState *state = &ctx.render_states[window_id];
    state->shape_batch.vertex_count = 0;
    state->command_count = 0;
    state->needs_redraw = true;

    // Force a redraw with clear color
    render_batch(window_id);
}

void glps_update_background(GooeyWindow *win)
{
}

void glps_render(GooeyWindow *win)
{
    render_batch(win->creation_id);
}

float glps_get_text_width(const char *text, int length)
{
    float total_width = 0.0f;
    for (int i = 0; i < length; ++i)
    {
        total_width += (ctx.characters[(int)text[i]].advance / 64.0f) * 18.0f;
    }
    return total_width;
}

float glps_get_text_height(const char *text, int length)
{
    float max_height = 0;
    for (int i = 0; i < length; ++i)
    {
        float char_height = ctx.characters[(int)text[i]].height * 18.0f;
        if (char_height > max_height)
        {
            max_height = char_height;
        }
    }
    return max_height;
}

const char *glps_get_key_from_code(void *gooey_event)
{
    if (!gooey_event)
        return NULL;
    GooeyEvent *event = (GooeyEvent *)gooey_event;
    return event->key_press.value;
}

void glps_set_cursor(GOOEY_CURSOR cursor)
{
    switch (cursor)
    {
    case GOOEY_CURSOR_HAND:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_HAND);
        break;
    case GOOEY_CURSOR_TEXT:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_IBEAM);
        break;
    case GOOEY_CURSOR_ARROW:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_ARROW);
        break;
    case GOOEY_CURSOR_CROSSHAIR:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_CROSSHAIR);
        break;
    case GOOEY_CURSOR_RESIZE_H:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_HRESIZE);
        break;
    case GOOEY_CURSOR_RESIZE_V:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_VRESIZE);
        break;
    default:
        break;
    }
}

void glps_stop_cursor_reset(bool state)
{
    ctx.inhibit_reset = state;
}

void glps_destroy_window_from_id(int window_id)
{
    glps_wm_window_destroy(ctx.wm, window_id);
    cleanup_window_render_state(window_id);
    ctx.active_window_count--;
}

static void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff, int x, int y, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyWindow *window = windows[origin_window_id];
    GooeyEvent *event = (GooeyEvent *)window->current_event;
    event->type = GOOEY_EVENT_DROP;
    event->drop_data.drop_x = x;
    event->drop_data.drop_y = y;
    strncpy(event->drop_data.file_path, buff, sizeof(event->drop_data.file_path) - 1);
    event->drop_data.file_path[sizeof(event->drop_data.file_path) - 1] = '\0';
    strncpy(event->drop_data.mime, buff, sizeof(event->drop_data.mime) - 1);
    event->drop_data.mime[sizeof(event->drop_data.mime) - 1] = '\0';
    glps_wm_window_update(ctx.wm, window->creation_id);
}

void glps_setup_callbacks(void (*callback)(size_t window_id, void *data), void *data)
{
    glps_wm_set_keyboard_callback(ctx.wm, keyboard_callback, data);
    glps_wm_set_mouse_move_callback(ctx.wm, mouse_move_callback, data);
    glps_wm_set_mouse_click_callback(ctx.wm, mouse_click_callback, data);
    glps_wm_set_scroll_callback(ctx.wm, mouse_scroll_callback, data);
    glps_wm_window_set_resize_callback(ctx.wm, window_resize_callback, data);
    glps_wm_window_set_close_callback(ctx.wm, window_close_callback, data);
    glps_wm_window_set_frame_update_callback(ctx.wm, callback, data);
}

void glps_run()
{
    while (!glps_wm_should_close(ctx.wm) && ctx.is_running)
    {
        glps_wm_window_update(ctx.wm, 0);
        for (size_t i = 0; i < ctx.active_window_count; ++i)
        {
            //          glps_wm_window_update(ctx.wm, 0);
        }

        usleep(500);
    }
}

GooeyTimer *glps_create_timer()
{
    glps_timer *timer = glps_timer_init();
    ctx.timers[ctx.timer_count++] = timer;
    GooeyTimer *gooey_timer = (GooeyTimer *)calloc(1, sizeof(GooeyTimer));
    gooey_timer->timer_ptr = timer;
    return gooey_timer;
}

void glps_stop_timer(GooeyTimer *timer)
{
    // glps_timer_stop((glps_timer *)timer->timer_ptr);
    glps_render(0);
}

void glps_destroy_timer(GooeyTimer *gooey_timer)
{
    if (!gooey_timer || !gooey_timer->timer_ptr)
        return;
    glps_timer *internal_timer = (glps_timer *)gooey_timer->timer_ptr;
    for (size_t i = 0; i < ctx.timer_count; ++i)
    {
        if (ctx.timers[i] == internal_timer)
        {
            glps_timer_destroy(internal_timer);
            memmove(&ctx.timers[i], &ctx.timers[i + 1], (ctx.timer_count - i - 1) * sizeof(glps_timer *));
            ctx.timer_count--;
            ctx.timers[ctx.timer_count] = NULL;
            break;
        }
    }
    free(gooey_timer);
}

void glps_set_callback_for_timer(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data)
{
    glps_timer_start((glps_timer *)timer->timer_ptr, time, callback, user_data);
}

void glps_window_toggle_decorations(GooeyWindow *win, bool enable)
{
    if (!win)
        return;
    glps_wm_toggle_window_decorations(ctx.wm, enable, win->creation_id);
}

size_t glps_get_active_window_count()
{
    return ctx.active_window_count;
}

size_t glps_get_total_window_count()
{
    return glps_wm_get_window_count(ctx.wm);
}

void glps_reset_events(GooeyWindow *win)
{
    GooeyEvent *event = (GooeyEvent *)win->current_event;
    event->type = GOOEY_EVENT_RESET;
}

void glps_set_viewport(size_t window_id, int width, int height)
{
}

double glps_get_window_framerate(int window_id)
{
    static struct timespec last_time[MAX_WINDOWS] = {0};
    static double last_fps[MAX_WINDOWS] = {0};
    static struct timespec now;

    if (last_time[window_id].tv_sec == 0 && last_time[window_id].tv_nsec == 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &last_time[window_id]);
        last_fps[window_id] = glps_wm_get_fps(ctx.wm, window_id);
        return last_fps[window_id];
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (now.tv_sec - last_time[window_id].tv_sec) +
                     (now.tv_nsec - last_time[window_id].tv_nsec) / 1000000000.0;

    if (elapsed >= 1.0)
    {
        last_fps[window_id] = glps_wm_get_fps(ctx.wm, window_id);
        last_time[window_id] = now;
    }

    return last_fps[window_id];
}

GooeyTFT_Sprite *glps_create_widget_sprite(int x, int y, int width, int height)
{
    return NULL;
}

void glps_redraw_sprite(GooeyTFT_Sprite *sprite)
{
}

void glps_clear_area(int x, int y, int width, int height)
{
}

void glps_clear_old_widget(GooeyTFT_Sprite *sprite)
{
}

void glps_create_view()
{
}

void glps_destroy_ultralight()
{
}

void glps_draw_webview(GooeyWindow *win, void *webview, int x, int y, int width, int height, int window_id)
{
}

void glps_request_close()
{
    ctx.is_running = false;
}

void glps_init_fdialog()
{
}

static nfdwindowhandle_t nfd_get_window_type()
{
    nfdwindowhandle_t parentWindow = {
        .type = glps_wm_get_platform(),
        .handle = glps_wm_window_get_native_ptr(ctx.wm, 0)};
    return parentWindow;
}

void glps_open_fdialog(const char *start_path, nfdu8filteritem_t *filters, size_t filter_count, void (*on_file_selected)(const char *file_path))
{
    nfdwindowhandle_t parentWindow = nfd_get_window_type();
    nfdu8char_t *outPath = NULL;
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = filter_count;
    args.defaultPath = start_path;
    args.parentWindow = parentWindow;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        if (on_file_selected && outPath)
            on_file_selected((const char *)outPath);
        NFD_FreePathU8(outPath);
    }
}

void glps_get_platform_name(char *platform, size_t max_length)
{
    const uint8_t platform_type = glps_wm_get_platform();
    char platform_name[1024];
    switch (platform_type)
    {
    case 0:
        strncpy(platform_name, "Windows", sizeof(platform_name));
        break;
    case 3:
        strncpy(platform_name, "Linux X11", sizeof(platform_name));
        break;
    case 1:
        strncpy(platform_name, "Linux Wayland", sizeof(platform_name));
        break;
    default:
        strncpy(platform_name, "Unknown", sizeof(platform_name));
        break;
    }
    strncpy(platform, platform_name, max_length - 1);
    platform[max_length - 1] = '\0';
}

void glps_make_window_transparent(GooeyWindow *win, int blur_radius, float opacity)
{
    if (!win)
        return;
    glps_wm_set_window_background_transparent(ctx.wm, win->creation_id);
    glps_wm_set_window_opacity(ctx.wm, win->creation_id, opacity);
    glps_wm_set_window_blur(ctx.wm, win->creation_id, true, blur_radius);
}

void glps_cleanup()
{
    ctx.is_running = false;

    for (int i = 0; i < 128; i++)
    {
        if (ctx.characters[i].texture.image != VK_NULL_HANDLE)
        {
            destroy_texture(ctx.device, &ctx.characters[i].texture);
        }
    }

    for (size_t i = 0; i < ctx.texture_count; i++)
    {
        destroy_texture(ctx.device, &ctx.loaded_textures[i]);
    }
    free(ctx.loaded_textures);

    for (int i = 0; i < MAX_WINDOWS; i++)
    {
        cleanup_window_render_state(i);

        if (ctx.text_vertex_buffers[i].buffer)
            destroy_buffer(ctx.device, &ctx.text_vertex_buffers[i]);
        if (ctx.shape_vertex_buffers[i].buffer)
            destroy_buffer(ctx.device, &ctx.shape_vertex_buffers[i]);
        if (ctx.shape_uniform_buffers[i].buffer)
            destroy_buffer(ctx.device, &ctx.shape_uniform_buffers[i]);
        if (ctx.text_matrices_uniform_buffers[i].buffer)
            destroy_buffer(ctx.device, &ctx.text_matrices_uniform_buffers[i]);
        if (ctx.text_params_uniform_buffers[i].buffer)
            destroy_buffer(ctx.device, &ctx.text_params_uniform_buffers[i]);

        if (ctx.command_buffers[i])
        {
            vkFreeCommandBuffers(ctx.device, ctx.command_pools[i], ctx.swapchain_image_counts[i], ctx.command_buffers[i]);
            free(ctx.command_buffers[i]);
        }

        if (ctx.command_pools[i])
            vkDestroyCommandPool(ctx.device, ctx.command_pools[i], NULL);

        if (ctx.framebuffers[i])
        {
            for (uint32_t j = 0; j < ctx.swapchain_image_counts[i]; j++)
            {
                vkDestroyFramebuffer(ctx.device, ctx.framebuffers[i][j], NULL);
            }
            free(ctx.framebuffers[i]);
        }

        if (ctx.render_passes[i])
            vkDestroyRenderPass(ctx.device, ctx.render_passes[i], NULL);
        if (ctx.swapchains[i])
            vkDestroySwapchainKHR(ctx.device, ctx.swapchains[i], NULL);
        if (ctx.surfaces[i])
            vkDestroySurfaceKHR(ctx.instance, ctx.surfaces[i], NULL);

        if (ctx.image_available_semaphores[i])
            vkDestroySemaphore(ctx.device, ctx.image_available_semaphores[i], NULL);
        if (ctx.render_finished_semaphores[i])
            vkDestroySemaphore(ctx.device, ctx.render_finished_semaphores[i], NULL);
        if (ctx.in_flight_fences[i])
            vkDestroyFence(ctx.device, ctx.in_flight_fences[i], NULL);

        if (ctx.images_in_flight[i])
            free(ctx.images_in_flight[i]);
    }

    destroy_pipeline(ctx.device, &ctx.text_pipeline);
    destroy_pipeline(ctx.device, &ctx.shape_pipeline);

    if (ctx.shape_set_layout)
        vkDestroyDescriptorSetLayout(ctx.device, ctx.shape_set_layout, NULL);
    if (ctx.text_set_layout)
        vkDestroyDescriptorSetLayout(ctx.device, ctx.text_set_layout, NULL);
    if (ctx.descriptor_pool)
        vkDestroyDescriptorPool(ctx.device, ctx.descriptor_pool, NULL);

    free(ctx.surfaces);
    free(ctx.swapchains);
    free(ctx.render_passes);
    free(ctx.framebuffers);
    free(ctx.command_pools);
    free(ctx.command_buffers);
    free(ctx.image_available_semaphores);
    free(ctx.render_finished_semaphores);
    free(ctx.in_flight_fences);
    free(ctx.images_in_flight);
    free(ctx.image_indices);
    free(ctx.swapchain_image_counts);
    free(ctx.text_vertex_buffers);
    free(ctx.shape_vertex_buffers);
    free(ctx.window_extents);
    free(ctx.swapchain_formats);
    free(ctx.shape_uniform_buffers);
    free(ctx.text_matrices_uniform_buffers);
    free(ctx.text_params_uniform_buffers);
    free(ctx.shape_descriptor_sets);
    free(ctx.text_descriptor_sets);

    if (ctx.timers)
    {
        for (size_t i = 0; i < ctx.timer_count; i++)
        {
            if (ctx.timers[i])
            {
                glps_timer_destroy(ctx.timers[i]);
            }
        }
        free(ctx.timers);
    }

    if (ctx.device)
        vkDestroyDevice(ctx.device, NULL);
    if (ctx.instance)
        vkDestroyInstance(ctx.instance, NULL);
    if (ctx.wm)
        glps_wm_destroy(ctx.wm);

    NFD_Quit();
}

GooeyBackend glps_vk_backend = {
    .Init = glps_init,
    .Run = glps_run,
    .Cleanup = glps_cleanup,
    .SetupCallbacks = glps_setup_callbacks,
    .RequestRedraw = glps_request_redraw,
    .SetViewport = glps_set_viewport,
    .GetActiveWindowCount = glps_get_active_window_count,
    .GetTotalWindowCount = glps_get_total_window_count,
    .GCreateWindow = glps_create_window,
    .MakeWindowVisible = glps_make_window_visible,
    .MakeWindowResizable = glps_set_window_resizable,
    .WindowToggleDecorations = glps_window_toggle_decorations,
    .GetCurrentClickedWindow = glps_get_current_clicked_window,
    .DestroyWindows = glps_destroy_windows,
    .DestroyWindowFromId = glps_destroy_window_from_id,
    .HideCurrentChild = glps_hide_current_child,
    .UpdateBackground = glps_update_background,
    .Clear = glps_clear,
    .Render = glps_render,
    .SetForeground = glps_set_foreground,
    .DrawText = glps_draw_text,
    .LoadImage = glps_load_image,
    .LoadImageFromBin = glps_load_image_from_bin,
    .DrawImage = glps_draw_image,
    .FillRectangle = glps_fill_rectangle,
    .DrawRectangle = glps_draw_rectangle,
    .FillArc = glps_fill_arc,
    .GetKeyFromCode = glps_get_key_from_code,
    .ResetEvents = glps_reset_events,
    .GetWinDim = glps_window_dim,
    .GetWinFramerate = glps_get_window_framerate,
    .DrawLine = glps_draw_line,
    .GetTextWidth = glps_get_text_width,
    .GetTextHeight = glps_get_text_height,
    .SetCursor = glps_set_cursor,
    .UnloadImage = glps_unload_image,
    .CreateTimer = glps_create_timer,
    .SetTimerCallback = glps_set_callback_for_timer,
    .StopTimer = glps_stop_timer,
    .DestroyTimer = glps_destroy_timer,
    .CursorChange = glps_set_cursor,
    .StopCursorReset = glps_stop_cursor_reset,
    .ForceCallRedraw = glps_force_redraw,
    .RequestClose = glps_request_close,
    .CreateSpriteForWidget = glps_create_widget_sprite,
    .RedrawSprite = glps_redraw_sprite,
    .ClearArea = glps_clear_area,
    .ClearOldWidget = glps_clear_old_widget,
    .CreateView = glps_create_view,
    .DestroyUltralight = glps_destroy_ultralight,
    .DrawWebview = glps_draw_webview,
    .OpenFileDialog = glps_open_fdialog,
    .GetPlatformName = glps_get_platform_name,
    .MakeWindowTransparent = glps_make_window_transparent,
    .RenderBatch = render_batch,
};

#endif