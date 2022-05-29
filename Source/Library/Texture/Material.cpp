#include "Material.h"

namespace library
{
	Material::Material(_In_ std::wstring szName)
		: m_padding{ '\0', }
		, m_szName(szName)
		, pDiffuse()
		, pSpecularExponent()
		, pNormal()
	{
	}

	HRESULT Material::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
	{
		HRESULT hr = S_OK;

		if (pDiffuse)
		{
			hr = pDiffuse->Initialize(pDevice, pImmediateContext);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		if (pSpecularExponent)
		{
			hr = pSpecularExponent->Initialize(pDevice, pImmediateContext);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		if (pNormal)
		{
			hr = pNormal->Initialize(pDevice, pImmediateContext);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		return hr;
	}

	std::wstring Material::GetName() const
	{
		return m_szName;
	}

}