#pragma once
#include <unordered_map>
#include "Texture.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Util.h"
#include "EngineCommon/util/GeometryGenerator.h"
#include "EngineCommon/include/types.h"

namespace engine::graphics
{
	struct Material
	{
		TextureHandle occlusionTexture;
		//TextureHandle emmisiveTexture;
	};
	class RenderContext;
	struct Submesh
	{
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		INT BaseVertexLocation = 0;
		Submesh(UINT indexCount, UINT startIndex, UINT baseVertexLoc) : IndexCount(indexCount), StartIndexLocation(startIndex),
			BaseVertexLocation(baseVertexLoc) {}
		Submesh() = default;

		void Draw(ID3D12GraphicsCommandList* cmList);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<util::Geometry::MeshData> mesh);
		static std::vector<std::pair< Material, std::vector<Submesh> >> GetSubmeshes(std::vector<std::pair< Material, std::vector<util::Geometry::MeshData> >>& mesh);
	};

	struct Mesh
	{
	public:
		using MeshDataVector = std::vector < util::Geometry::MeshData>;
		Mesh() = default;
		Mesh(
			RenderContext& context,
			const void* vertexData, UINT vertexDataSize, UINT structSize,
			const void* indexData, UINT indexDataSize, Material material = Material())
		{
			Init(context, vertexData, vertexDataSize, structSize, indexData, indexDataSize, material);
		}

		void Init(
			RenderContext& context,
			const void* vertexData, UINT vertexDataSize, UINT structSize,
			const void* indexData, UINT indexDataSize, Material material = Material());

	public:

		std::string Name;
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
		// Data about the buffers.
		u32 VertexByteStride = 0;
		u32 VertexBufferByteSize = 0;
		u32 IndexBufferByteSize = 0;
		u32 indexCount = 0;

		//std::unordered_map<Material, std::vector<Submesh>> DrawArgs;
		Material m_material;

	};
};
