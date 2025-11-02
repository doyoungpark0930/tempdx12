

class DescriptorPool
{
public:
	DescriptorPool() = default;
	ID3D12DescriptorHeap* m_descritorHeap = nullptr;
	UINT allocatedDescriptorNum = 0;
	UINT descriptorSize = 0;

	void OnInit(ID3D12Device* m_device, UINT ObjectsNum);
	void AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pOutCPUDescriptor, D3D12_GPU_DESCRIPTOR_HANDLE* pOutGPUDescriptor, UINT DescriptorCount);
	void Reset();
	~DescriptorPool();

private :
	ID3D12Device* m_device = nullptr;
	const UINT descriptorNumPerObject = 24;

	D3D12_CPU_DESCRIPTOR_HANDLE	m_cpuDescriptorHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE	m_gpuDescriptorHandle = {};

};