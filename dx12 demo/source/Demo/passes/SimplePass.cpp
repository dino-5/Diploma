#include "SimplePass.h"
#include "EngineGfx/dx12/PSO.h"
#include "EngineGfx/dx12/RootSignature.h"
#include "EngineGfx/dx12/DescriptorHeap.h"
#include "EngineGfx/RenderContext.h"
#include "EngineCommon/System/Filesystem.h"
#include "EngineCommon/system/InputManager.h"

using namespace engine;

SimplePass::SimplePass(graphics::RenderContext& context)
{
    Initialize(context);
}

void SimplePass::Initialize(graphics::RenderContext& context)
{
    Pass::SetPSO(L"default");
    Pass::SetRootSignature(graphics::ROOT_SIG_BINDLESS);
    ID3D12Device* device = context.GetDevice().native();
    graphics::CommandList& commandList = context.GetList();
    float aspectRatio = context.GetAspectRatio();

    m_texture.Init(graphics::ImageData(g_homeDir / "textures" / "wall.jpg"), device, 
        commandList.GetList());
    float textureAspectRatio = m_texture.m_width / m_texture.m_height;
    float constant = textureAspectRatio*aspectRatio;
    
    Vertex triangleVertices[] =
    {
        {Vector3({-0.5f, -0.5f * constant, .5f }), Vector2({0.f, 1.f})},
        {Vector3({-0.5f,  0.5f * constant, .5f }), Vector2({0.f, 0.f})},
        {Vector3({ 0.5f,  0.5f * constant, .9f }), Vector2({1.f, 0.f})},
        {Vector3({ 0.5f, -0.5f * constant, .9f }), Vector2({1.f, 1.f})},
    };
    u16 indexData[] = { 0, 1, 2, 2, 3, 0  };

    m_vertexBuffer.Init(context, triangleVertices, sizeof(triangleVertices), sizeof(Vertex), 4);
    m_indexBuffer.Init(context, indexData, sizeof(indexData), sizeof(u16), 6);
    m_constantBuffer.Init(device, 1, &m_data, sizeof(m_data));

    m_rootIndexData.vertexBufferIndex = m_vertexBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.constantBufferIndex = m_constantBuffer.GetDescriptorHeapIndex();
    m_rootIndexData.textureIndex =  m_texture.GetDescriptorHeapIndex();
    m_rootStructure.Init(device, 1, &m_rootIndexData, sizeof(m_rootIndexData));

    auto& inputManager = system::InputManager::GetInputManager();

    system::CallbackInfo info;
    info.oneTimeTouch = false;
    info.ptr = [&](){
        m_data.scale += 1;
        m_constantBuffer.Update(&m_data);
        util::PrintInfo("Current scale {}", m_data.scale);
    };
    inputManager.AddCallback(system::Key::E, info);

    info.ptr = [&](){
        if(m_data.scale>1)
            m_data.scale -= 1;
        util::PrintInfo("Current scale {}", m_data.scale);
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::Q, info);

    info.oneTimeTouch = true;
    info.ptr = [&](){
        m_data.samplerIndex = 0;
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::N1, info);

    info.ptr = [&](){
        m_data.samplerIndex = 1;
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::N2, info);

    info.ptr = [&](){
        m_data.samplerIndex = 2;
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::N3, info);

    info.ptr = [&](){
        m_data.samplerIndex = 3;
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::N4, info);

    const float velocity = .1f;
    info.oneTimeTouch = false;
    info.ptr = [&, velocity](){
        m_data.translate = m_data.translate.Add((Vector3{0.f, 1.f, 0.f}).Mul(velocity / m_data.scale));
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::W, info);

    info.ptr = [&, velocity](){
        m_data.translate = m_data.translate.Sub((Vector3{1.f, 0.f, 0.f}).Mul(velocity / m_data.scale));
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::A, info);

    info.ptr = [&, velocity](){
        m_data.translate = m_data.translate.Sub((Vector3{0.f, 1.f, 0.f}).Mul(velocity / m_data.scale));
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::S, info);

    info.ptr = [&, velocity](){
        m_data.translate = m_data.translate.Add((Vector3{1.f, 0.f, 0.f}).Mul(velocity / m_data.scale));
        m_constantBuffer.Update(&m_data);
    };
    inputManager.AddCallback(system::Key::D, info);
}

void SimplePass::Draw(ID3D12GraphicsCommandList* commandList, u32 frameNumber)
{
    commandList->SetDescriptorHeaps(1, engine::graphics::DescriptorHeapManager::CurrentSRVHeap.getHeapAddress());
	commandList->SetGraphicsRootSignature( *m_rootSignature );
	commandList->SetPipelineState( *m_pso );

    commandList->SetGraphicsRootConstantBufferView(0, m_rootStructure.getAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D12_INDEX_BUFFER_VIEW indexView;
    indexView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    indexView.Format = DXGI_FORMAT_R16_UINT;
    indexView.SizeInBytes = m_indexBuffer.GetBufferSize();
    commandList->IASetIndexBuffer(&indexView);

    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}


