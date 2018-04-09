#pragma once

#include "../../sunny.h"
#include "../renderables/Renderable3D.h"
#include "../groups/Group3D.h"
#include "../cameras/Camera.h"
#include "../LightSetup.h"
#include "../../directx/Shader.h"
#include "../shaders/ShaderFactory.h"
#include "../../directx/DeferredBuffer.h"
#include "../GBuffer.h"

namespace sunny
{
	namespace graphics
	{
		struct RendererUniform
		{
			std::string uniform;
			unsigned char* value;
		};

		struct RenderCommand
		{
			Renderable3D* renderable3d;
			maths::mat4 transform;
			maths::vec4 color;
			float hasTexture;
			directx::Shader* shader;
			std::vector<RendererUniform> uniforms;
		};

		class Renderer3D
		{
		private:
			directx::Shader* m_default_forward_shader;
			directx::Shader* m_default_deferred_shader;
			directx::Shader* m_default_light_shader;

			std::vector<RenderCommand> m_forwardCommandQueue;
			std::vector<RenderCommand> m_deferredCommandQueue;


			std::vector<RendererUniform> m_sunnyUniforms;

			unsigned int m_screenBufferWidth, m_screenBufferHeight;

			unsigned char* m_VSSunnyUniformBuffer;
			unsigned int   m_VSSunnyUniformBufferSize;

			unsigned char* m_PSSunnyForwardUniformBuffer;
			unsigned int   m_PSSunnyForwardUniformBufferSize;
			
			unsigned char* m_PSSunnyDeferredUniformBuffer;
			unsigned int   m_PSSunnyDeferredUniformBufferSize;
			
			unsigned char* m_PSSunnyLightUniformBuffer;
			unsigned int   m_PSSunnyLightUniformBufferSize;

			std::vector<unsigned int> m_VSSunnyUniformBufferOffsets;
			std::vector<unsigned int> m_PSSunnyForwardUniformBufferOffsets;
			std::vector<unsigned int> m_PSSunnyDeferredUniformBufferOffsets;
			std::vector<unsigned int> m_PSSunnyLightUniformBufferOffsets;

			std::vector<Renderable3D*> m_renderables;
			
			GBuffer* m_gBuffer;

		public:
			Renderer3D();
			Renderer3D(unsigned with, unsigned height);
			~Renderer3D() {};

			void Init();
			void Begin();
			void BeginScene(Camera* camera);
			void Submit(Renderable3D* renderable);
			void Submit(const RenderCommand& command);
			void SubmitRenderable3D(Renderable3D* renderable);
			void SubmitGroup3D(Group3D* group3d);
			void SubmitLight(const LightSetup& lightSetup);
			void EndScene();
			void End();
			void Present();

			inline const void SetScreenBufferSize(unsigned int width, unsigned int height) { m_screenBufferWidth = width; m_screenBufferHeight = height; }
		
		private:
			void ForwardPresentInternal();
			void DeferredBlendFalsePresentInternal();
			void DeferredBlendTruePresentInternal();

			void SetSunnyVSUniforms(directx::Shader* shader);
			void SetSunnyForwardUniforms(directx::Shader* shader);
			void SetSunnyDeferredUniforms(directx::Shader* shader);
			void SetSunnyLightUniforms(directx::Shader* shader);
		};
	}
}