#include "Renderer/Skybox.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace library
{
	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Skybox::Skybox

	  Summary:  Constructor

	  Args:     const std::filesystem::path& cubeMapFilePath
				  Path to the cube map texture to use
				FLOAT scale
				  Scaling factor

	  Modifies: [m_cubeMapFileName, m_scale].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	Skybox::Skybox(_In_ const std::filesystem::path& cubeMapFilePath, _In_ FLOAT scale) :
		Model("Content/Common/Sphere.obj"),
		m_cubeMapFileName(cubeMapFilePath),
		m_scale(scale)
	{ }

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Skybox::Initialize

	  Summary:  Initializes the skybox and cube map texture

	  Args:     ID3D11Device* pDevice
				  The Direct3D device to create the buffers
				ID3D11DeviceContext* pImmediateContext
				  The Direct3D context to set buffers

	  Modifies: [m_aMeshes, m_aMaterials].
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	HRESULT Skybox::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
	{
		HRESULT hr = Model::Initialize(pDevice, pImmediateContext);
		if (FAILED(hr)) return hr;

		m_world = XMMatrixScaling(m_scale, m_scale, m_scale);

		m_aMeshes[0].uMaterialIndex = 0;

		m_aMaterials[0]->pDiffuse = std::make_shared<Texture>(m_cubeMapFileName);

		hr = m_aMaterials[0]->Initialize(pDevice, pImmediateContext);
		if (FAILED(hr)) return hr;

		return S_OK;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Skybox::GetSkyboxTexture

	  Summary:  Returns the cube map texture

	  Returns:  const std::shared_ptr<Texture>&
				  Cube map texture object
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
	const std::shared_ptr<Texture>& Skybox::GetSkyboxTexture() const
	{
		return m_aMaterials[0]->pDiffuse;
	}

	/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
	  Method:   Skybox::initSingleMesh

	  Summary:  Initialize single mesh from a given assimp mesh

	  Args:     UINT uMeshIndex
				  Mesh index
				const aiMesh* pMesh
				  Point to an assimp mesh object
	M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---0M-M*/
	void Skybox::initSingleMesh(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
	{
		// same as the parent's initSingleMesh
		// except that order of indices are reversed
		// 0 1 2, 3 4 5 => 2 1 0, 5 4 3

		XMFLOAT2 zeroVec2(0.0f, 0.0f);
		XMFLOAT3 zeroVec3(0.0f, 0.0f, 0.0f);

		BasicMeshEntry newEntry;
		newEntry.uNumIndices = 0;
		newEntry.uBaseVertex = static_cast<UINT>(m_aVertices.size());
		newEntry.uBaseIndex = static_cast<UINT>(m_aIndices.size());
		newEntry.uMaterialIndex = pMesh->mMaterialIndex;


		// Push back SimpleVertex to the vertices vector
		for (UINT i = 0; i < pMesh->mNumVertices; i++)
		{
			const auto& pos = pMesh->mVertices[i];
			const auto& norm = pMesh->mNormals[i];

			SimpleVertex vertex = {
				.Position = XMFLOAT3(pos.x, pos.y, pos.z),
				.TexCoord = zeroVec2,
				.Normal = XMFLOAT3(norm.x, norm.y, norm.z),
			};

			if (pMesh->HasTextureCoords(0u))
			{
				const auto& tex = pMesh->mTextureCoords[0][i];
				vertex.TexCoord = XMFLOAT2(tex.x, tex.y);
			}

			m_aVertices.push_back(vertex);

			NormalData normalData =
			{
				.Tangent = zeroVec3,
				.Bitangent = zeroVec3
			};

			if (pMesh->HasTangentsAndBitangents())
			{
				const auto& t = pMesh->mTangents[i];
				const auto& b = pMesh->mBitangents[i];
				normalData.Tangent = XMFLOAT3(t.x, t.y, t.z);
				normalData.Bitangent = XMFLOAT3(b.x, b.y, b.z);
			}

			m_aNormalData.push_back(normalData);
		}

		// Push back indices of faces to the indices vector
		for (UINT i = 0; i < pMesh->mNumFaces; i++)
		{
			const auto& face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3u);

			for (int j = 2; j >= 0; j--)
			{
				m_aIndices.push_back(static_cast<WORD>(face.mIndices[j]));
				newEntry.uNumIndices++;
			}
		}

		m_aMeshes.push_back(newEntry);

		initMeshBones(uMeshIndex, pMesh);
	}
}
