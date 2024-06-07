#pragma once
#include "Demo/passes/Pass.h"
#include "EngineCommon/math/Vector.h"
#include "EngineCommon/math/Matrix.h"
#include "EngineCommon/include/types.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineGfx/Texture.h"

using namespace engine;

struct Vertex
{
    Vector3 position;
    Vector2 uv;
};

struct BindlessSP
{
	u32 vertexBufferIndex;
	u32 constantBufferIndex;
	u32 textureIndex;
};

struct ConstantBufferData
{
	f32 scale = 2;
	Vector3 translate;
	uint samplerIndex = 0;
};

class SimplePass : public Pass
{
public:
	SimplePass() {}
	SimplePass(graphics::RenderContext& context);
	void Initialize(graphics::RenderContext& context);
	void Draw(ID3D12GraphicsCommandList*, u32 nt)override;
private:
	BindlessSP m_rootIndexData;
	graphics::Texture m_texture;
	graphics::TextureHandle m_textureNew;
	graphics::Buffer m_vertexBuffer;
	graphics::Buffer m_indexBuffer;
	graphics::ConstantBuffer m_constantBuffer;
	graphics::ConstantBuffer m_rootStructure;
	u32 test = 0;
	ConstantBufferData m_data;

	D3D12_CPU_DESCRIPTOR_HANDLE m_samplers[engine::config::NumFrames];

};

