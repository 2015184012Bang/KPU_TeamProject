#include "ClientPCH.h"
#include "Mesh.h"

#include "ResourceManager.h"

Mesh::Mesh()
	: mVertexBufferView()
	, mVertexCount(0)
	, mIndexBufferView()
	, mIndexCount(0)
{

}

void Mesh::Load(const wstring& path)
{
	eMeshType meshType;
	rapidjson::Document doc = openMeshFile(path, &meshType);

	switch (meshType)
	{
	case eMeshType::Static:
		loadStaticMesh(doc);
		break;
	case eMeshType::Skeletal:
		loadSkeletalMesh(doc);
		break;
	default:
		HB_ASSERT(false, "ASSERTION FAILED");
		break;
	}
}

rapidjson::Document Mesh::openMeshFile(const wstring& path, eMeshType* outMeshType)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		HB_LOG("Could not open file: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		HB_LOG("{0} is not valid json file!", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	const rapidjson::Value& textures = doc["textures"];

	for (rapidjson::SizeType i = 0; i < textures.Size(); ++i)
	{
		string texName = textures[i].GetString();
		ResourceManager::GetTexture(s2ws(texName));
	}
	
	string vertexFormat = doc["vertexformat"].GetString();

	if (vertexFormat == "PosNormTex")
	{
		*outMeshType = eMeshType::Static;
	}
	else if (vertexFormat == "PosNormSkinTex")
	{
		*outMeshType = eMeshType::Skeletal;
	}
	else
	{
		HB_LOG("Unknown vertex format: {0}", ws2s(path));
		HB_ASSERT(false, "ASSERTION FAILED");
	}

	return doc;
}

void Mesh::loadStaticMesh(const rapidjson::Document& doc)
{
	{
		const rapidjson::Value& vertsJson = doc["vertices"];

		vector<Vertex> vertices;
		vertices.reserve(vertsJson.Size());

		for (rapidjson::SizeType i = 0; i < vertsJson.Size(); ++i)
		{
			const rapidjson::Value& vert = vertsJson[i];

			if (!vert.IsArray() || vert.Size() != 8)
			{
				HB_ASSERT(false, "Invalid vertex format");
			}

			Vertex v;
			v.Position.x = vert[0].GetFloat();
			v.Position.y = vert[1].GetFloat();
			v.Position.z = vert[2].GetFloat();
			v.Normal.x = vert[3].GetFloat();
			v.Normal.y = vert[4].GetFloat();
			v.Normal.z = vert[5].GetFloat();
			v.UV.x = vert[6].GetFloat();
			v.UV.y = vert[7].GetFloat();

			vertices.push_back(v);
		}

		createVertexBuffer<Vertex>(vertices);
	}

	{
		const rapidjson::Value& indJson = doc["indices"];

		vector<uint32> indices;
		indices.reserve(indJson.Size() * 3);

		for (rapidjson::SizeType i = 0; i < indJson.Size(); ++i)
		{
			const rapidjson::Value& ind = indJson[i];

			if (!ind.IsArray() || ind.Size() != 3)
			{
				HB_ASSERT(false, "Invalid index format");
			}

			indices.push_back(ind[0].GetUint());
			indices.push_back(ind[1].GetUint());
			indices.push_back(ind[2].GetUint());
		}

		createIndexBuffer(indices);
	}
}

void Mesh::loadSkeletalMesh(const rapidjson::Document& doc)
{
	{
		const rapidjson::Value& vertsJson = doc["vertices"];

		vector<SkeletalVertex> vertices;
		vertices.reserve(vertsJson.Size());

		for (rapidjson::SizeType i = 0; i < vertsJson.Size(); ++i)
		{
			const rapidjson::Value& vert = vertsJson[i];

			if (!vert.IsArray() || vert.Size() != 16)
			{
				HB_ASSERT(false, "Invalid skinned vertex format");
			}

			SkeletalVertex v;
			v.Position.x = vert[0].GetFloat();
			v.Position.y = vert[1].GetFloat();
			v.Position.z = vert[2].GetFloat();
			v.Normal.x = vert[3].GetFloat();
			v.Normal.y = vert[4].GetFloat();
			v.Normal.z = vert[5].GetFloat();
			v.Bone[0] = vert[6].GetUint();
			v.Bone[1] = vert[7].GetUint();
			v.Bone[2] = vert[8].GetUint();
			v.Bone[3] = vert[9].GetUint();
			v.BoneWeight.x = vert[10].GetFloat() / 255.0f;
			v.BoneWeight.y = vert[11].GetFloat() / 255.0f;
			v.BoneWeight.z = vert[12].GetFloat() / 255.0f;
			v.BoneWeight.w = vert[13].GetFloat() / 255.0f;
			v.UV.x = vert[14].GetFloat();
			v.UV.y = vert[15].GetFloat();

			vertices.push_back(v);
		}

		createVertexBuffer<SkeletalVertex>(vertices);
	}

	{
		const rapidjson::Value& indJson = doc["indices"];

		vector<uint32> indices;
		indices.reserve(indJson.Size() * 3);

		for (rapidjson::SizeType i = 0; i < indJson.Size(); ++i)
		{
			const rapidjson::Value& ind = indJson[i];

			if (!ind.IsArray() || ind.Size() != 3)
			{
				HB_ASSERT(false, "Invalid index format");
			}

			indices.push_back(ind[0].GetUint());
			indices.push_back(ind[1].GetUint());
			indices.push_back(ind[2].GetUint());
		}

		createIndexBuffer(indices);
	}
}

void Mesh::createIndexBuffer(const vector<uint32>& indices)
{
	if (indices.empty())
	{
		HB_ASSERT(false, "Empty index vector")
	}

	mIndexCount = static_cast<uint32>(indices.size());
	uint32 indexBufferSize = mIndexCount * sizeof(uint32);

	ComPtr<ID3D12Resource> indexUploadBuffer;
	const CD3DX12_HEAP_PROPERTIES uploadBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC uploadbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	gDevice->CreateCommittedResource(
		&uploadBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadbufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&indexUploadBuffer));

	void* mappedData;
	indexUploadBuffer->Map(0, nullptr, &mappedData);
	memcpy(mappedData, indices.data(), indexBufferSize);
	indexUploadBuffer->Unmap(0, nullptr);

	const CD3DX12_HEAP_PROPERTIES defaultBufferHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	const CD3DX12_RESOURCE_DESC defaultbufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	gDevice->CreateCommittedResource(
		&defaultBufferHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&defaultbufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&mIndexBuffer));

	gCmdList->CopyResource(mIndexBuffer.Get(), indexUploadBuffer.Get());

	const auto toDefaultBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	gCmdList->ResourceBarrier(1, &toDefaultBarrier);
	
	RELEASE_UPLOAD_BUFFER(indexUploadBuffer);

	mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = indexBufferSize;
}
