#include "EngineGfx/RenderContext.h"
#include "EngineGfx/dx12/Buffers.h"
#include "EngineCommon/util/Logger.h"
#include "EngineCommon/system/InputManager.h"
#include <format>

namespace engine::graphics
{
	void LoadPipeline(ID3D12Device* dev)
	{
		LogScope("Graphics");
		PopulateRootSignatures(dev);
		PopulateShaders();
		PopulatePSO(dev);
		PopulateDescriptorHeaps();
	}

	void RenderContext::Initialize(SwapChainSettings set)
	{
		LogScope("RenderContext");
		m_device.Initialize();
		engine::util::PrintInfo("device initialized");
		m_graphicsQueue.Init(m_device.GetDevice(), {});
		engine::util::PrintInfo("queue initialized");
		m_graphicsCommandList.Initialize(m_device);
		engine::util::PrintInfo("list initialized");
		m_device.CreateFence(&m_fence);
		u32 value = m_fence->GetCompletedValue();

		DescriptorHeapManager::CreateRTVHeap(engine::config::NumFrames);
		DescriptorHeapManager::CreateDSVHeap(1);
		DescriptorHeapManager::CreateSamplerHeap(engine::config::NumFrames);
		engine::util::PrintInfo("heaps created");
		LoadPipeline(m_device.GetDevice());
		{

			// Create an event handle to use for frame synchronization.
			if (m_fenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

		}
        ResetSwapChain(set);

		auto& inputManager = engine::system::InputManager::GetInputManager();
		system::CallbackInfo callbackInfo;
		callbackInfo.ptr = [&]() {
			ShaderManager::Clear();
			PopulateShaders();
			PSO::allPSO.clear();
			PopulatePSO(m_device.GetDevice());
		};
		inputManager.AddCallback(system::Key::R, callbackInfo);
	}

	void RenderContext::SetupViewport(SwapChainSettings& set)
	{
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width    = static_cast<float>(set.width);
		m_viewport.Height   = static_cast<float>(set.height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissorRect= { 0, 0, set.width, set.height};
	}

	void RenderContext::ResetSwapChain(SwapChainSettings set)
	{
		if (m_graphicsQueue.GetQueue() != nullptr)
			m_swapChain.Init(set, m_device.GetFactory(), m_graphicsQueue.GetQueue());
		SetupViewport(set);

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ResourceDescription desc;
		desc.createState = ResourceState::DEPTH_WRITE;
		desc.descriptor.descriptor = DescriptorFlags::DepthStencil;
		desc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.flags = ResourceFlags::DEPTH_STENCIL;
		desc.format = DXGI_FORMAT_R24G8_TYPELESS;
		desc.width = set.width;
		desc.height = set.height;
		m_dsvBuffer.init(Device::device->GetDevice(), desc, &optClear);
	}

	void RenderContext::FlushCommandQueue()
	{
		u64 value = m_fence->GetCompletedValue();
		m_swapChain.m_fence[m_currentFrame] = ++m_currentFence;
		ThrowIfFailed(m_graphicsQueue->Signal(m_fence, m_currentFence));

		// Wait until the previous frame is finished.
		if (m_swapChain.m_fence[m_currentFrame]!=0 && value < m_swapChain.m_fence[m_currentFrame] )
		{
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_swapChain.m_fence[m_currentFrame], m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
			CloseHandle(m_fenceEvent);
		}
		m_currentFrame = (m_currentFrame + 1) % (engine::config::NumFrames);
	}

	void RenderContext::NextFrame()
	{
		FlushCommandQueue();
	}

	void RenderContext::ResetCommandAllocator()
	{
		m_graphicsCommandList.Reset(0);

	}

	void RenderContext::StartFrame()
	{
		m_graphicsCommandList.Reset(m_currentFrame);
		m_graphicsCommandList->RSSetViewports(1, &m_viewport);
		m_graphicsCommandList->RSSetScissorRects(1, &m_scissorRect);
		m_currentFrame = m_swapChain.ChangeState(m_graphicsCommandList.GetList(), ResourceState::RENDER_TARGET);
		auto rtvHandle = m_swapChain.GetView(m_currentFrame);
		auto dsvHandle = m_dsvBuffer.dsv;
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_graphicsCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_graphicsCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		m_graphicsCommandList->OMSetRenderTargets(1, &rtvHandle.HandleCPU, true, &dsvHandle.HandleCPU);
	}

	void RenderContext::EndFrame()
	{
		m_swapChain.ChangeState(m_graphicsCommandList.GetList(), ResourceState::PRESENT);
		ThrowIfFailed(m_graphicsCommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_graphicsCommandList.GetList()};
		m_graphicsQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// Present the frame.
		m_swapChain->Present(0, 0);
		FlushCommandQueue();
		ThrowIfFailed(m_device.GetDevice()->GetDeviceRemovedReason());
	}

};
