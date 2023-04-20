#include "Effect.h"

bool	Effect::Init()
{
	return true;
}

bool	Effect::Frame()
{
	// Emitter�� SpawnCount, DestroyTime������ �ڽ��� ������ ����
	// ������ InitDelay�� �������� Ȯ���ؾ���
	// Effect�� ����Ʈ�� Emitter�� ��� �������� ���� ����
	TMatrix world;
	D3DXMatrixTranslation(&world, m_vPos.x, m_vPos.y, m_vPos.z);
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->m_matParent = world;
		pEmitter->Frame();
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
	}
	return true;
}

void	Effect::Reset()
{
	for (auto pEmitter : m_pEmitterList)
	{
		pEmitter->Reset();
	}
}