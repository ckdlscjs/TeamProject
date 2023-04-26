#include "Effect.h"

bool	Effect::Init(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	m_pd3dDevice = pd3dDevice;
	m_pImmediateContext = pContext;

	m_isDone = false;

	return true;
}

bool	Effect::Frame()
{
	// Emitter�� SpawnCount, DestroyTime������ �ڽ��� ������ ����
	// ������ InitDelay�� �������� Ȯ���ؾ���
	// Effect�� ����Ʈ�� Emitter�� ��� �������� ���� ����
	TMatrix matTrans;
	D3DXMatrixTranslation(&matTrans, m_vPos.x, m_vPos.y, m_vPos.z);

	auto iter = m_pEmitterList.begin();
	while(iter != m_pEmitterList.end())
	{
		Emitter* pEmitter = (*iter);
		if (pEmitter->m_isDone)
		{
			iter = m_pEmitterList.erase(iter);
			pEmitter->Release();
			delete pEmitter;
		}
		else
		{
			pEmitter->m_matParentTrans = matTrans;
			pEmitter->Frame();
			iter++;
		}
	}
	if (m_pEmitterList.size() == 0)
	{
		m_isDone = true;
	}
	return true;
}

bool	Effect::Render()
{
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->Render();
	}
	return true;
}

bool	Effect::Release()
{
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->Release();
		delete pEmitter;
		pEmitter = nullptr;
	}
	m_pEmitterList.clear();
	return true;
}

void	Effect::SetCamera(Camera* pCamera)
{
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->SetCamera(pCamera);
	}
}

void	Effect::Reset()
{
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->Reset();
	}
}