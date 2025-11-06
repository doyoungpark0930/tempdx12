#include "pch.h"
#include "CommonStructs.h"
#include "DescriptorPool.h"
#include "Renderer.h"
#include "DXUtil.h"
#include "DXHelper.h"
#include "SrvManager.h"
#include "CbvManager.h"
#include "GeometryGenerator.h"
#include "Animation.h"
#include "Model.h"
#include <iostream>
using namespace DirectX::SimpleMath;

Renderer* Model::m_renderer = nullptr;
DescriptorPool* Model::m_descriptorPool = nullptr;
SrvManager* Model::m_srvManager = nullptr;
CbvManager* Model::m_cbvManager = nullptr;

Model::Model()
{
	m_device = m_renderer->GetDevice();
	UpdateMembers();
	descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

void Model::UpdateMembers()
{
	m_commandList = m_renderer->GetCommandList();
	m_commandAllocator = m_renderer->GetCommandAllocator();
	m_commandQueue = m_renderer->GetCommandQueue();
	m_pipelineState = m_renderer->GetPipeline();
	m_fence = m_renderer->GetFence();
	m_fenceEvent = m_renderer->GetFenceEvent();
	m_fenceValue = m_renderer->GetFenceValue();
}


D3D12_VERTEX_BUFFER_VIEW Model::CreateVertexBuffer(Vertex* vertices, UINT vertexCount)
{
	const UINT vertexBufferSize = sizeof(Vertex) * vertexCount;

	UINT verticesOffset = 0;
	if (FAILED(
		SetDataToUploadBuffer(
			&(m_renderer->m_vsCur),
			m_renderer->m_vsBegin,
			m_renderer->m_vsEnd,
			vertices, sizeof(Vertex), vertexCount,
			sizeof(float),
			verticesOffset
		)))__debugbreak();

	if (FAILED(m_commandAllocator->Reset())) __debugbreak();

	if (FAILED(m_commandList->Reset(m_commandAllocator, m_pipelineState))) __debugbreak();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->m_vsBufferPool, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(m_renderer->m_vsBufferPool, verticesOffset, m_renderer->m_vsUploadBufferPool, verticesOffset, vertexBufferSize);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->m_vsBufferPool, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_commandList->ResourceBarrier(1, &barrier);

	if (FAILED(m_commandList->Close())) __debugbreak();

	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForPreviousFrame();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	vertexBufferView.BufferLocation = m_renderer->m_vsBufferPool->GetGPUVirtualAddress() + verticesOffset;
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;

	return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW Model::CreateIndexBuffer(UINT* indices, UINT indiceCount)
{
	const UINT indicesBufferSize = sizeof(UINT) * indiceCount;

	UINT indicesOffset = 0;
	if (FAILED(
		SetDataToUploadBuffer(
			&(m_renderer->m_indexCur),
			m_renderer->m_indexBegin,
			m_renderer->m_indexEnd,
			indices, sizeof(UINT), indiceCount,
			sizeof(UINT),
			indicesOffset
		)))__debugbreak();

	if (FAILED(m_commandAllocator->Reset())) __debugbreak();

	if (FAILED(m_commandList->Reset(m_commandAllocator, m_pipelineState))) __debugbreak();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->m_indexBufferPool, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->CopyBufferRegion(m_renderer->m_indexBufferPool, indicesOffset, m_renderer->m_indexUploadBufferPool, indicesOffset, indicesBufferSize);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderer->m_indexBufferPool, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	m_commandList->ResourceBarrier(1, &barrier);

	if (FAILED(m_commandList->Close())) __debugbreak();

	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForPreviousFrame();

	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	indexBufferView.BufferLocation = m_renderer->m_indexBufferPool->GetGPUVirtualAddress() + indicesOffset;
	indexBufferView.SizeInBytes = indicesBufferSize;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	return indexBufferView;
}
void Model::CreateModel(MeshDataInfo meshesInfo, bool useNormalMap)
{
	rootNode = meshesInfo.rootNode;
	m_materialNum = meshesInfo.materialNum;
	//m_TriGroupList = new TRI_GROUP_PER_MTL[m_materialNum]; //디버깅중 누수맘에안들어서 일단 주석

	m_vertexBufferView = CreateVertexBuffer(meshesInfo.vertices, meshesInfo.verticesNum);
	/*(for (int i = 0; i < m_materialNum; i++)
	{
		m_TriGroupList[i].triCount = meshesInfo.Materials[i].face_cnt;
		m_TriGroupList[i].IndexBufferView = CreateIndexBuffer(meshesInfo.Materials[i].index, m_TriGroupList[i].triCount * 3);
		//뭐이쯤해놨으니 대충, createtexture여기서 하고 잘넣고 draw에서 잘처리해서 하쇼
		if (meshesInfo.Materials[i].albedoTexFilename)
			CreateTextureFromName(meshesInfo.meshes[i].albedoTexFilename, m_Meshes[i].srvContainer, &m_Meshes[i].srvNum);
		if (meshesInfo.Materials[i].normalTexFilename)
			..

	}*/

	if (meshesInfo.m_animations)
	{
		existAnimation = true;
		boneMatricesContainer = m_cbvManager->AllocAnimationMatrices();
		m_FinalBoneMatrices = meshesInfo.finalBoneMatrices;
		m_matricesNum = meshesInfo.matricesNum;
		m_animations = meshesInfo.m_animations; //Model에서 animation데이터 해제하기 위함
	}
	m_useNormalMap = useNormalMap;



	SafeDeleteArray(&meshesInfo.vertices);
	for (int i = 0; i < m_materialNum; i++)
	{
		SafeDeleteArray(&meshesInfo.Materials[i].index);
	}

}
void Model::CreateTextureFromName(char* textureFilename, SRV_CONTAINER* srvContainer, UINT* srvCnt)
{
	if (textureFilename)
	{
		wchar_t PathName[256];
		MultiByteToWideChar(CP_UTF8, 0, textureFilename, -1, PathName, 256);

		srvContainer[(*srvCnt)++] = m_srvManager->CreateTexture(PathName);
	}
}

void Model::Draw(const Matrix* pMatrix)
{

	//GlobalConstant
	CBV_CONTAINER globalContainer = m_cbvManager->GetGlobalContainer();

	CD3DX12_CPU_DESCRIPTOR_HANDLE globalCpuHandle(m_descriptorPool->m_descritorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE globalGpuHandle(m_descriptorPool->m_descritorHeap->GetGPUDescriptorHandleForHeapStart());
	m_device->CopyDescriptorsSimple(1, globalCpuHandle, globalContainer.CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_commandList->SetGraphicsRootDescriptorTable(0, globalGpuHandle);


	//ModelConstant
	CBV_CONTAINER* cbvContainer = m_cbvManager->GetAllocContainer();
	MODEL_CONSTANT* pModelConstant = (MODEL_CONSTANT*)cbvContainer->pSystemMemAddr;

	//Model Update
	{
		pModelConstant->Model = pMatrix->Transpose();

		Matrix NormalMatrix = *pMatrix;
		NormalMatrix.Translation(Vector3(0.0f));
		NormalMatrix = NormalMatrix.Invert().Transpose();
		pModelConstant->NormalModel = NormalMatrix.Transpose();
		pModelConstant->useNormalMap = m_useNormalMap;
	}

	//AnimationUpdate
	if (existAnimation)
	{
		SkinnedConstants* pSkinnedConstant = (SkinnedConstants*)boneMatricesContainer->pSystemMemAddr;
		memcpy(pSkinnedConstant->boneTransforms, m_FinalBoneMatrices, sizeof(Matrix) * m_matricesNum);

	}

	//AllocTable 및 디스크립터 복사
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};

	UINT requiredDescriptorCount = 1 + 1 + m_materialNum * texNum; //model행렬(1)+ animationMatrices(1) + srv의 개수
	m_descriptorPool->AllocDescriptorTable(&cpuDescriptorTable, &gpuDescriptorTable, requiredDescriptorCount);

	// modelCBV
	m_device->CopyDescriptorsSimple(1, cpuDescriptorTable, cbvContainer->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cpuDescriptorTable.Offset(1, descriptorSize);
	if (existAnimation)
	{
		m_device->CopyDescriptorsSimple(1, cpuDescriptorTable, boneMatricesContainer->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cpuDescriptorTable.Offset(1, descriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorTable);
		gpuDescriptorTable.Offset(2, descriptorSize);
	}
	else
	{
		m_commandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorTable);
		gpuDescriptorTable.Offset(1, descriptorSize);
	}

	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// modelCBV / mtl[0]{albeo,ao,metallic..} / mtl[1]{albedo, ao, metallic..} /..
	// modelCBV / m_FinalBoneMatrices / mtl[0]{albeo,ao,metallic..} / mtl[1]{albedo, ao, metallic..} /..


	for (int i = 0; i < m_materialNum; i++)
	{
		TRI_GROUP_PER_MTL* pTriGroup = m_TriGroupList + i;
		m_commandList->IASetIndexBuffer(&pTriGroup->IndexBufferView);
		m_commandList->DrawIndexedInstanced(pTriGroup->triCount * 3, 1, 0, 0, 0);
	}





}

void Model::WaitForPreviousFrame()
{
	// Signal and increment the fence value.
	const UINT64 fence = *m_fenceValue;
	if (FAILED(m_commandQueue->Signal(m_fence, fence))) __debugbreak();
	(*m_fenceValue)++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		if (FAILED(m_fence->SetEventOnCompletion(fence, m_fenceEvent))) __debugbreak();
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

}

void deleteNode(maxNode* node)
{
	if (node == nullptr)
		return;

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		deleteNode(node->mChildren[i]);
		node->mChildren[i] = nullptr;
	}
	delete node;
}
Model::~Model()
{
	//WaitForPreviousFrame(); //gpu올라오기 전에 종료해버린다면 의미가 잇을수도

	//참고 : 나중에 이 Model을 안 쓸거면 srvContainer의 Resource를 VRAM에서 내려도 되는데,
	//그렇다면 srvManager소멸자에서 해제가아니라, 여기 소멸자에서 resource해제해야함
	//for (int i = 0; i < meshNum; i++)
	//{
		//SafeDeleteArray(&m_Meshes[i].srvContainer);
	//}
	deleteNode(rootNode);
	//SafeDeleteArray(&m_Meshes);
	SafeDeleteArray(&m_animations);

}