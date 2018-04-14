#pragma once

#include "../directx/ShadowBuffer.h"

namespace sunny
{
	namespace graphics
	{
		class ShadowMap
		{
		private:
			const int SHADOW_MAP_INDEX = 1;          // Texture2D shadowMap: register(t1);

		private:
			directx::ShadowBuffer* m_buffer;

		private:
			void Init();

		public:
			ShadowMap();

			void SetShadowMapTexture();

			void Bind();
			void UnBInd();
		};
	}
}