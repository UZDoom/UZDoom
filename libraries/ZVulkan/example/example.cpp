
#include <zvulkan/vulkaninstance.h>
#include <zvulkan/vulkandevice.h>
#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkancompatibledevice.h>
#include <zvulkan/vulkanobjects.h>
#include <zvulkan/vulkanswapchain.h>
#include <zvulkan/vulkanbuilders.h>

void VulkanPrintLog(const char* typestr, const std::string& msg)
{
	// Log messages from the validation layer here
}

void VulkanError(const char* message)
{
	throw std::runtime_error(message);
}

#ifdef WIN32

#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

bool exitFlag;

LRESULT CALLBACK ZVulkanWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_PAINT || msg == WM_ERASEBKGND)
	{
		return 0;
	}
	else if (msg == WM_CLOSE)
	{
		exitFlag = true;
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	HWND hwnd = 0;
	try
	{
		// Create a window class for the win32 window
		WNDCLASSEX classdesc = {};
		classdesc.cbSize = sizeof(WNDCLASSEX);
		classdesc.hInstance = GetModuleHandle(nullptr);
		classdesc.lpszClassName = L"ZVulkanWindow";
		classdesc.lpfnWndProc = &ZVulkanWindowProc;
		classdesc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		classdesc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		BOOL result = RegisterClassEx(&classdesc);
		if (!result)
			throw std::runtime_error("RegisterClassEx failed");

		// Create and show the window
		hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"ZVulkanWindow", L"ZVulkan Example", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1280, 1024, 0, 0, GetModuleHandle(nullptr), nullptr);
		if (!hwnd)
			throw std::runtime_error("CreateWindowEx failed");

		// Create vulkan instance
		auto instance = VulkanInstanceBuilder()
			.RequireExtension(VK_KHR_SURFACE_EXTENSION_NAME)
			.RequireExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
			.DebugLayer(true)
			.Create();

		// Create a surface for our window
		
		VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		createInfo.hwnd = hwnd;
		createInfo.hinstance = GetModuleHandle(nullptr);

		VkSurfaceKHR surfaceHandle = {};
		VkResult vkresult = vkCreateWin32SurfaceKHR(instance->Instance, &createInfo, nullptr, &surfaceHandle);
		if (vkresult != VK_SUCCESS)
			throw std::runtime_error("Could not create vulkan surface");

		auto surface = std::make_shared<VulkanSurface>(instance, surfaceHandle);

		// Create the vulkan device
		auto device = VulkanDeviceBuilder()
			.Surface(surface)
			.Create(instance);

		// Create a swap chain for our window
		auto swapchain = VulkanSwapChainBuilder()
			.Create(device.get());

		// Create a command buffer pool
		auto commandPool = CommandPoolBuilder()
			.DebugName("commandPool")
			.Create(device.get());

		// Create semaphore that is set when the swap chain has acquired the image
		auto imageAvailableSemaphore = SemaphoreBuilder()
			.DebugName("imageAvailableSemaphore")
			.Create(device.get());

		// Create semaphore that is set when drawing has completed
		auto renderFinishedSemaphore = SemaphoreBuilder()
			.DebugName("renderFinishedSemaphore")
			.Create(device.get());

		// Create a fence that is set when the command buffer finished executing
		auto presentFinishedFence = FenceBuilder()
			.DebugName("presentFinishedFence")
			.Create(device.get());

		// Keep track on which frame buffers we have created for the current swap chain and what size they have
		std::vector<std::unique_ptr<VulkanFramebuffer>> framebuffers;
		int lastWidth = 0;
		int lastHeight = 0;

		// GLSL for a vertex shader

		std::string includedCode = R"(
			layout(set = 0, binding = 0, std140) uniform Uniforms
			{
				mat4 ProjectionMatrix;
				mat4 ViewMatrix;
			};
		)";

		std::string versionBlock = R"(
			#version 450
			#extension GL_GOOGLE_include_directive : enable
		)";

		std::string vertexCode = R"(

			#include "uniforms.glsl"

			layout(location = 0) in vec4 aPosition;
			layout(location = 1) in vec2 aTexCoord;
			layout(location = 2) in vec4 aColor;

			layout(location = 0) out vec2 texCoord;
			layout(location = 1) out vec4 color;

			void main()
			{
				texCoord = aTexCoord;
				color = aColor;
				gl_Position = ProjectionMatrix * ViewMatrix * aPosition;
			}
		)";

		// GLSL for two fragment shaders. One textured and one without

		std::string fragmentShaderNoTexCode = R"(

			layout(location = 0) in vec2 texCoord;
			layout(location = 1) in vec4 color;
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = color;
			}
		)";

		std::string fragmentShaderTexturedCode = R"(

			layout(set = 1, binding = 0) uniform sampler2D Texture;

			layout(location = 0) in vec2 texCoord;
			layout(location = 1) in vec4 color;
			layout(location = 0) out vec4 outColor;

			void main()
			{
				outColor = texture(Texture, texCoord) * color;
			}
		)";

		struct Vertex
		{
			Vertex() = default;
			Vertex(float x, float y, float z, float u, float v, float r, float g, float b, float a) : x(x), y(y), z(z), u(u), v(v), r(r), g(g), b(b), a(a) { }

			float x, y, z;
			float u, v;
			float r, g, b, a;
		};

		struct Uniforms
		{
			float ProjectionMatrix[16];
			float ViewMatrix[16];
		};

		// Create a vertex shader

		auto vertexShader = GLSLCompiler()
			.Type(ShaderType::Vertex)
			.AddSource("versionblock", versionBlock)
			.AddSource("vertexCode.glsl", vertexCode)
			.OnIncludeLocal([=](auto headerName, auto includerName, size_t depth) { if (headerName == "uniforms.glsl") return ShaderIncludeResult(headerName, includedCode); else return ShaderIncludeResult("File not found: " + headerName); })
			.Compile(device.get());

		// Create fragment shaders

		auto fragmentShaderNoTex = GLSLCompiler()
			.Type(ShaderType::Fragment)
			.AddSource("versionblock", versionBlock)
			.AddSource("fragmentShaderNoTexCode.glsl", fragmentShaderNoTexCode)
			.Compile(device.get());

		auto fragmentShaderTextured = GLSLCompiler()
			.Type(ShaderType::Fragment)
			.AddSource("versionblock", versionBlock)
			.AddSource("fragmentShaderTexturedCode.glsl", fragmentShaderTexturedCode)
			.Compile(device.get());

		// Create descriptor set layouts

		auto uniformSetLayout = DescriptorSetLayoutBuilder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.DebugName("uniformSetLayout")
			.Create(device.get());

		auto textureSetLayout = DescriptorSetLayoutBuilder()
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.DebugName("textureSetLayout")
			.Create(device.get());

		// Create pipeline layouts

		auto pipelineLayoutNoTex = PipelineLayoutBuilder()
			.AddSetLayout(uniformSetLayout.get())
			.DebugName("pipelineLayoutNoTex")
			.Create(device.get());

		auto pipelineLayoutTextured = PipelineLayoutBuilder()
			.AddSetLayout(uniformSetLayout.get())
			.AddSetLayout(textureSetLayout.get())
			.DebugName("pipelineLayoutTextured")
			.Create(device.get());

		// Create a render pass where we clear the frame buffer when it begins and ends the pass by transitioning the image to what the swap chain needs to present it

		auto renderPass = RenderPassBuilder()
			.AddAttachment(VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.AddSubpass()
			.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.Create(device.get());

		// Create pipelines

		std::unique_ptr<VulkanPipeline> libraryNoTex, libraryBlend;
		std::unique_ptr<VulkanPipeline> pipelineNoTex, pipelineTextured;

		// Can we use pipeline libraries?

		if (device->EnabledFeatures.GraphicsPipelineLibrary.graphicsPipelineLibrary)
		{
			libraryNoTex = GraphicsPipelineBuilder()
				.RenderPass(renderPass.get())
				.Layout(pipelineLayoutNoTex.get())
				.Flags(
					VK_PIPELINE_CREATE_LIBRARY_BIT_KHR |
					VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT)
				.LibraryFlags(
					VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
					VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
					VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)
				.AddVertexBufferBinding(0, sizeof(Vertex))
				.AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, x))
				.AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, u))
				.AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, r))
				.AddVertexShader(vertexShader)
				.AddFragmentShader(fragmentShaderNoTex)
				.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
				.DebugName("libraryNoTex")
				.Create(device.get());

			libraryBlend = GraphicsPipelineBuilder()
				.RenderPass(renderPass.get())
				.Layout(pipelineLayoutNoTex.get())
				.Flags(
					VK_PIPELINE_CREATE_LIBRARY_BIT_KHR |
					VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT)
				.LibraryFlags(
					VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)
				.AddColorBlendAttachment(ColorBlendAttachmentBuilder()
					.AlphaBlendMode()
					.Create())
				.DebugName("libraryNoTex")
				.Create(device.get());

			pipelineNoTex = GraphicsPipelineBuilder()
				.Flags(VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT)
				.AddLibrary(libraryNoTex.get())
				.AddLibrary(libraryBlend.get())
				.DebugName("pipelineNoTex")
				.Create(device.get());
		}
		else
		{
			pipelineNoTex = GraphicsPipelineBuilder()
				.RenderPass(renderPass.get())
				.Layout(pipelineLayoutNoTex.get())
				.AddVertexBufferBinding(0, sizeof(Vertex))
				.AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, x))
				.AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, u))
				.AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, r))
				.AddVertexShader(vertexShader)
				.AddFragmentShader(fragmentShaderNoTex)
				.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
				.DebugName("pipelineNoTex")
				.Create(device.get());
		}

		pipelineTextured = GraphicsPipelineBuilder()
			.RenderPass(renderPass.get())
			.Layout(pipelineLayoutTextured.get())
			.AddVertexBufferBinding(0, sizeof(Vertex))
			.AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, x))
			.AddVertexAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, u))
			.AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, r))
			.AddVertexShader(vertexShader)
			.AddFragmentShader(fragmentShaderTextured)
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.DebugName("pipelineTextured")
			.Create(device.get());

		// Create a persistently mapped vertex buffer

		size_t maxVertices = 1'000'000;

		auto vertexBuffer = BufferBuilder()
			.Usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_UNKNOWN, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
			.MemoryType(
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Size(maxVertices)
			.DebugName("vertexBuffer")
			.Create(device.get());

		Vertex* vertices = (Vertex*)vertexBuffer->Map(0, sizeof(Vertex) * 6);

		// Create a persistently mapped uniform buffer

		auto uniformsBuffer = BufferBuilder()
			.Usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_UNKNOWN, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
			.MemoryType(
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Size(sizeof(Uniforms))
			.DebugName("uniformsBuffer")
			.Create(device.get());

		Uniforms* uniforms = (Uniforms*)uniformsBuffer->Map(0, sizeof(Uniforms));

		// Create persistently mapped staging buffer used for texture uploads

		size_t uploadBufferSize = 16 * 1024 * 1024;

		auto uploadBuffer = BufferBuilder()
			.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_UNKNOWN, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT)
			.MemoryType(
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Size(uploadBufferSize)
			.DebugName("uploadBuffer")
			.Create(device.get());

		uint8_t* uploads = (uint8_t*)uploadBuffer->Map(0, uploadBufferSize);

		// Put a texture in the upload buffer

		uint32_t* pixels = (uint32_t*)uploads;
		for (int y = 0; y < 64; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				uint32_t tilepattern = ((x / 4 + y / 4) % 2) * 255;
				uint32_t red = tilepattern;
				uint32_t green = tilepattern;
				uint32_t blue = tilepattern;
				uint32_t alpha = 255;
				pixels[x + y * 64] = red | (green << 8) | (blue << 16) | (alpha << 24);
			}
		}

		// Create sampler object

		auto sampler = SamplerBuilder()
			.MinFilter(VK_FILTER_NEAREST)
			.MagFilter(VK_FILTER_NEAREST)
			.DebugName("sampler")
			.Create(device.get());

		// Create image and view objects

		auto textureImage = ImageBuilder()
			.Usage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.Format(VK_FORMAT_R8G8B8A8_UNORM)
			.Size(64, 64)
			.DebugName("textureImage")
			.Create(device.get());

		auto textureView = ImageViewBuilder()
			.Type(VK_IMAGE_VIEW_TYPE_2D)
			.Image(textureImage.get(), VK_FORMAT_R8G8B8A8_UNORM)
			.DebugName("textureView")
			.Create(device.get());

		// Create descriptor set for the uniform buffer

		auto uniformSetPool = DescriptorPoolBuilder()
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
			.MaxSets(1)
			.DebugName("uniformBufferPool")
			.Create(device.get());

		auto uniformSet = uniformSetPool->allocate(uniformSetLayout.get());

		WriteDescriptors()
			.AddBuffer(uniformSet.get(), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformsBuffer.get())
			.Execute(device.get());

		// Create descriptor set for the texture

		auto textureSetPool = DescriptorPoolBuilder()
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			.MaxSets(1)
			.DebugName("textureSetPool")
			.Create(device.get());

		auto textureSet = textureSetPool->allocate(textureSetLayout.get());

		WriteDescriptors()
			.AddCombinedImageSampler(textureSet.get(), 0, textureView.get(), sampler.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.Execute(device.get());

		// Copy the pixels from the upload buffer to the image object and transition the image to the layout suitable for texture sampling

		auto transfercommands = commandPool->createBuffer();

		transfercommands->begin();

		PipelineBarrier()
			.AddImage(textureImage.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT)
			.Execute(transfercommands.get(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkBufferImageCopy region = {};
		region.imageExtent.width = 64;
		region.imageExtent.height = 64;
		region.imageExtent.depth = 1;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		transfercommands->copyBufferToImage(uploadBuffer->buffer, textureImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		PipelineBarrier()
			.AddImage(textureImage.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(transfercommands.get(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		transfercommands->end();

		// Submit the command buffer and wait for the device to finish the upload

		QueueSubmit()
			.AddCommandBuffer(transfercommands.get())
			.Execute(device.get(), device->GraphicsQueue);

		vkDeviceWaitIdle(device->device);

		// Draw a scene and pump window messages until the window is closed
		while (!exitFlag)
		{
			// Pump the win32 message queue
			MSG msg = {};
			BOOL result = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
			if (result < 0)
				break;
			if (result == TRUE)
			{
				if (msg.message == WM_QUIT)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// How big is the window client area in this frame?
			RECT clientRect = {};
			GetClientRect(hwnd, &clientRect);
			int width = clientRect.right;
			int height = clientRect.bottom;

			// Do we need to resize or recreate the swapchain?
			if (width > 0 && height > 0 && width != lastWidth || height != lastHeight || swapchain->Lost())
			{
				lastWidth = width;
				lastHeight = height;
				framebuffers.clear();

				swapchain->Create(width, height, 1, true, false);

				// Create frame buffer objects for the new swap chain images
				for (int imageIndex = 0; imageIndex < swapchain->ImageCount(); imageIndex++)
				{
					framebuffers.push_back(FramebufferBuilder()
						.RenderPass(renderPass.get())
						.AddAttachment(swapchain->GetImageView(imageIndex))
						.Size(width, height)
						.DebugName("framebuffer")
						.Create(device.get()));
				}
			}

			// Try acquire a frame buffer image from the swap chain
			int imageIndex = swapchain->AcquireImage(imageAvailableSemaphore.get());
			if (imageIndex != -1)
			{
				float identityMatrix[16] =
				{
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};

				// Update the uniform buffer
				memcpy(uniforms->ProjectionMatrix, identityMatrix, sizeof(float) * 16);
				memcpy(uniforms->ViewMatrix, identityMatrix, sizeof(float) * 16);

				// Update the vertex buffer
				vertices[0] = Vertex(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 1.0f);
				vertices[1] = Vertex( 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
				vertices[2] = Vertex( 0.0f,  0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 1.0f);
				vertices[3] = Vertex(-0.4f, 0.4f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f);
				vertices[4] = Vertex(0.4f, 0.4f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
				vertices[5] = Vertex(0.0f, -0.4f, 0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 1.0f);

				// Create a command buffer and begin adding commands to it
				auto drawcommands = commandPool->createBuffer();
				drawcommands->begin();

				// Begin a render pass
				RenderPassBegin()
					.RenderPass(renderPass.get())
					.Framebuffer(framebuffers[imageIndex].get())
					.AddClearColor(0.0f, 0.0f, 0.1f, 1.0f)
					.AddClearDepthStencil(1.0f, 0)
					.RenderArea(0, 0, width, height)
					.Execute(drawcommands.get());

				// Set the viewport to cover the window
				VkViewport viewport = {};
				viewport.width = (float)width;
				viewport.height = (float)height;
				viewport.maxDepth = 1.0f;
				drawcommands->setViewport(0, 1, &viewport);

				// Set the scissor box
				VkRect2D scissor = {};
				scissor.extent.width = width;
				scissor.extent.height = height;
				drawcommands->setScissor(0, 1, &scissor);

				// Bind the vertex buffer
				VkDeviceSize offset = 0;
				drawcommands->bindVertexBuffers(0, 1, &vertexBuffer->buffer, &offset);

				// Bind the uniform buffer once (share it between the pipelines)
				drawcommands->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutNoTex.get(), 0, uniformSet.get());

				// Draw a triangle with gradient
				drawcommands->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineNoTex.get());
				drawcommands->draw(3, 1, 0, 0);

				// Draw a triangle with a texture
				drawcommands->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutTextured.get(), 1, textureSet.get());
				drawcommands->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineTextured.get());
				drawcommands->draw(3, 1, 3, 0);

				// End rendering
				drawcommands->endRenderPass();
				drawcommands->end();

				// Submit command buffer to the graphics queue
				QueueSubmit()
					.AddCommandBuffer(drawcommands.get())
					.AddWait(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, imageAvailableSemaphore.get())
					.AddSignal(renderFinishedSemaphore.get())
					.Execute(device.get(), device->GraphicsQueue, presentFinishedFence.get());

				// Present the frame buffer image
				swapchain->QueuePresent(imageIndex, renderFinishedSemaphore.get());

				// Wait for the swapchain present to finish
				vkWaitForFences(device->device, 1, &presentFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
				vkResetFences(device->device, 1, &presentFinishedFence->fence);
			}
		}

		vertexBuffer->Unmap();
		uniformsBuffer->Unmap();
		uploadBuffer->Unmap();

		return 0;
	}
	catch (const std::exception& e)
	{
		if (hwnd)
			ShowWindow(hwnd, SW_HIDE);
		MessageBoxA(0, e.what(), "Unhandled Exception", MB_OK);
		return 0;
	}
}

#else

int main()
{
	return 0;
}

#endif
