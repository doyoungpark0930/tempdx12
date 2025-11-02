#include "pch.h"
#include "DescriptorPool.h"



void DescriptorPool:: OnInit(ID3D12Device* pDevice, UINT ObjectsNum)
{
	m_device = pDevice;

	descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = ObjectsNum * descriptorNumPerObject;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (FAILED(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_descritorHeap)))) __debugbreak();

	m_cpuDescriptorHandle = m_descritorHeap->GetCPUDescriptorHandleForHeapStart();
	m_gpuDescriptorHandle = m_descritorHeap->GetGPUDescriptorHandleForHeapStart();

	allocatedDescriptorNum = 1; //첫번째 디스크립터는 globalConstant라서 디폴트가 1임
}
void DescriptorPool::AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGPUDescriptor, UINT DescriptorCount)
{
	*pOutCPUDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cpuDescriptorHandle, allocatedDescriptorNum, descriptorSize);
	*pOutGPUDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_gpuDescriptorHandle, allocatedDescriptorNum, descriptorSize);
	allocatedDescriptorNum += DescriptorCount;
}
void DescriptorPool::Reset()
{
	allocatedDescriptorNum = 1; //globalConstant자리로 1 띄움
}

DescriptorPool::~DescriptorPool()
{
	if (m_descritorHeap)
	{
		m_descritorHeap->Release();
		m_descritorHeap = nullptr;
	}
}