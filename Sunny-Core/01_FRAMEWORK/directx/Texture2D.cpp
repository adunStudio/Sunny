#include "Texture2D.h"

namespace sunny
{
	namespace directx
	{
		/* STATIC */

		Texture2D* Texture2D::Create(unsigned int width, unsigned int height, TextureParameters parameters, TextureLoadOptions loadOptions)
		{
			return new Texture2D(width, height, parameters, loadOptions);
		}

		Texture2D* Texture2D::CreateFromFile(const std::string& filepath, TextureParameters parameters, TextureLoadOptions loadOptions)
		{
			return new Texture2D(filepath, filepath, parameters, loadOptions);
		}

		Texture2D* Texture2D::CreateFromFile(const std::string& filepath, TextureLoadOptions loadOptions)
		{
			return new Texture2D(filepath, filepath, TextureParameters(), loadOptions);
		}
		
		Texture2D* Texture2D::CreateFromFile(const std::string& name, const std::string& filepath, TextureParameters parameters, TextureLoadOptions loadOptions)
		{
			return new Texture2D(name, filepath, parameters, loadOptions);
		}
		
		Texture2D* Texture2D::CreateFromFile(const std::string& name, const std::string& filepath, TextureLoadOptions loadOptions)
		{
			return new Texture2D(name, filepath, TextureParameters(), loadOptions);
		}

		Texture2D* Texture2D::FromFile(const std::string&  filepath)
		{
			return new Texture2D(filepath, filepath);
		}

		DXGI_FORMAT Texture2D::TextureFormatToD3D(TextureFormat format)
		{
			switch (format)
			{
			case TextureFormat::RGB:
			case TextureFormat::RGBA:
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}


		/* ������ */

		Texture2D::Texture2D(unsigned int width, unsigned int height, TextureParameters parameters, TextureLoadOptions loadOptions)
		: m_fileName("NULL"), m_width(width), m_height(height), m_parameters(parameters), m_loadOptions(loadOptions)
		{
			Load();
		}
		
		Texture2D::Texture2D(const std::string& name, const std::string& filename, TextureParameters parameters, TextureLoadOptions loadOptions)
		: m_fileName(filename), m_name(name), m_parameters(parameters), m_loadOptions(loadOptions)
		{
			Load();
		}

		Texture2D::Texture2D(const std::string& filename, TextureParameters parameters, TextureLoadOptions loadOptions)
		: m_fileName(filename), m_name(filename), m_parameters(parameters), m_loadOptions(loadOptions)
		{
			Load();
		}

		Texture2D::~Texture2D()
		{

		}


		/* LOAD */

		void Texture2D::Load()
		{
			unsigned char* data = nullptr;
		
			if (m_fileName != "NULL")
			{
				data = utils::LoadSunnyImage(m_fileName, &m_width, &m_height, &m_bitsPerPixel, !m_loadOptions.flipY);
				m_parameters.format = m_bitsPerPixel == 24 ? TextureFormat::RGB : TextureFormat::RGBA;
			}

			bool generateMips = (data != nullptr);

			unsigned int stride = 4;

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem          = data;                         // �ʱ�ȭ �����Ϳ� ���� ������
			initData.SysMemPitch      = stride * m_width;             // �ؽ�ó�� Ư�� ���� ���ۿ��� ���� �� ���۱����� ����Ʈ ��
			initData.SysMemSlicePitch = m_width * m_height * stride;  // Ư�� ���� ������ ���ۿ��� ���� ���� ���������� ����Ʈ ��(3������ �ƴ� ��� �ǹ� ����.)

			D3D11_SUBRESOURCE_DATA* initDataPtr = nullptr;

			unsigned int mipLevels = 1;

			// �Ӹ�
			if (generateMips)
			{
				unsigned int width  = m_width;
				unsigned int height = m_height;

				while (width > 1 && height > 1)
				{
					// 2�辿 �۾�����.
					width  = max(width  / 2, 1u);
					height = max(height / 2, 1u);
					++mipLevels;
				}
			}
			else
			{
				if (data)
					initDataPtr = &initData;
			}

			DXGI_FORMAT format = TextureFormatToD3D(m_parameters.format);


			// �ؽ��� ���� ����ü
			ZeroMemory(&m_desc, sizeof(D3D11_TEXTURE2D_DESC));

			m_desc.Width  = m_width; // ������ �ؽ����� �ʺ� ����
			m_desc.Height = m_height; // ������ �ؽ����� ���� ����
			m_desc.MipLevels = mipLevels; // // �ִ� �Ӹ� ������ ���� (�Ӹ�: �� ���� �׸��� ���� ������ ����� ������ �ؽ���)
			m_desc.ArraySize = 1;              // �ؽ��� �迭�� �ؽ��� ����
			m_desc.Format = format;
			m_desc.CPUAccessFlags = 0;          // 0�� CPU�� �б�/���⸦ �� �� ������ �ǹ�
			m_desc.Usage = generateMips ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
			m_desc.CPUAccessFlags = m_desc.Usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
			m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;// �ؽ�ó�� ���������� �ܰ迡 ��� ���� �ϴ��� ����
			if (generateMips)
				m_desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			m_desc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
			m_desc.SampleDesc.Count = 1; // ��Ƽ���ø� ����
			m_desc.SampleDesc.Quality = 0;// ��Ƽ���ø� ǰ��

			Context::GetDevice()->CreateTexture2D(&m_desc, initDataPtr, &m_texture);
			// ������ �ؽ��� ���� ����ü
			// �ʱ�ȭ ������(�迭 ������)
			// ������ 2D �ؽ��ĸ� ��ȯ�޴� ������
				
			/* �ؽ��Ŀ� ���� ���̴� �ڿ� �並 �����Ѵ�. */

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			srvDesc.Format = m_desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = m_desc.MipLevels;

			Context::GetDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView);
			if (generateMips)
			{
				Context::GetDeviceContext()->UpdateSubresource(m_texture, 0, nullptr, initData.pSysMem, initData.SysMemPitch, initData.SysMemSlicePitch);	
				Context::GetDeviceContext()->GenerateMips(m_resourceView);
			}

			m_desc.Usage = D3D11_USAGE_DEFAULT;
			m_desc.CPUAccessFlags = 0;
			m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			ZeroMemory(&m_samplerDesc, sizeof(D3D11_SAMPLER_DESC));
			m_samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			m_samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			m_samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			m_samplerDesc.MinLOD = 0;
			m_samplerDesc.MaxLOD = 11;
			m_samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			m_samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			m_samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			Context::GetDevice()->CreateSamplerState(&m_samplerDesc, &m_samplerState);

			if (data != nullptr)
				delete[] data;
		}


		/*     */
		
		void Texture2D::Bind(unsigned int slot) const
		{
			Context::GetDeviceContext()->PSSetShaderResources(slot, 1, &m_resourceView);
			Context::GetDeviceContext()->PSSetSamplers(slot, 1, &m_samplerState);
		}

		void Texture2D::UnBind(unsigned int slot) const
		{
			ID3D11ShaderResourceView* rv = nullptr;
			Context::GetDeviceContext()->PSSetShaderResources(slot, 1, &rv);
		}

		void Texture2D::SetData(const void* pixels)
		{
			D3D11_MAPPED_SUBRESOURCE msr;
			memset(&msr, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

			Context::GetDeviceContext()->Map(m_texture, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
			
			for (unsigned int i = 0; i < m_width * m_height * GetStrideFromFormat(m_parameters.format); i += 4)
			{
				((byte*)msr.pData)[i + 0] = 0xff;
				((byte*)msr.pData)[i + 1] = 0xff;
				((byte*)msr.pData)[i + 2] = 0xff;
				((byte*)msr.pData)[i + 3] = ((byte*)pixels)[i / 2 + 1];
			}
			
			Context::GetDeviceContext()->Unmap(m_texture, NULL);
		}
	}
}