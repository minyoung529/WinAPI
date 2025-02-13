#include "pch.h"
#include "CommandQueue.h"
#include "Engine.h"

CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
	::CloseHandle(m_fenceEvent);
}

void CommandQueue::Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain)
{
	m_swapChain = swapChain;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAlloc));

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_cmdList));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_rescmdAlloc));

	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_rescmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_rescmdList));

	// cmdList 명령 목록
	m_cmdList->Close();

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CommandQueue::WaitSync()
{
	m_fenceValue++;

	m_cmdQueue->Signal(m_fence.Get(), m_fenceValue);

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void CommandQueue::RenderBegin(const D3D12_VIEWPORT* vp, const D3D12_RECT* rect)
{
	m_cmdAlloc->Reset();
	m_cmdList->Reset(m_cmdAlloc.Get(), nullptr);
	
	int8 backIndex = m_swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		g_Engine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_PRESENT, // 화면 출력
		D3D12_RESOURCE_STATE_RENDER_TARGET); // 외주 결과물

	m_cmdList->SetGraphicsRootSignature(ROOT_SIGNATURE.Get());
	
	CONST_BUFFER(CONSTANT_BUFFER_TYPE::TRANSFORM)->Clear();
	CONST_BUFFER(CONSTANT_BUFFER_TYPE::MATERIAL)->Clear();

	g_Engine->GetTableDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = g_Engine->GetTableDescHeap()->GetDescriptorHeap().Get();
	m_cmdList->SetDescriptorHeaps(1, &descHeap);

	m_cmdList->ResourceBarrier(1, &barrier);
	// Set the viewport and scissor rect. This needs to be reset whenever the command list is reset.
	m_cmdList->RSSetViewports(1, vp);
	m_cmdList->RSSetScissorRects(1, rect);
	// Specify the buffers we are going to render to.
}

void CommandQueue::RenderEnd()
{
	int8 backIndex = m_swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		g_Engine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // 외주 결과물
		D3D12_RESOURCE_STATE_PRESENT); // 화면 출력
	m_cmdList->ResourceBarrier(1, &barrier);
	m_cmdList->Close();
	// 커맨드 리스트 수행
	ID3D12CommandList* cmdListArr[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);
	m_swapChain->Present();
	// Wait until frame commands are complete. This waiting is inefficient and is
	// done for simplicity. Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	WaitSync();
	m_swapChain->SwapIndex();
}

void CommandQueue::FlushResourceCommandQueue()
{
	m_rescmdList->Close();

	ID3D12CommandList* cmdListArr[] = { m_rescmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	m_rescmdAlloc->Reset();
	m_rescmdList->Reset(m_rescmdAlloc.Get(), nullptr);
}
