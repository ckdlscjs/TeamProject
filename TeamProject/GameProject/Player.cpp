#include "Player.h"
#include "Input.h"
#include "CollisionMgr.h"

Player::Player()
{

}

Player::~Player()
{
	if (m_pTrail)
	{
		m_pTrail->Release();
		delete m_pTrail;
		m_pTrail = nullptr;
	}
	Release();
}

bool    Player::Init()
{
	Character::Init();

	m_AttackBox.CreateOBBBox(1, 1, 1);

	m_pTrail = new TrailEffect();
	m_pTrail->Create(m_pd3dDevice, m_pImmediateContext, L"../../data/shader/TrailEffect.txt", L"../../data/swoosh.png");
	m_pTrail->Init();

	return true;
}

bool    Player::Frame()
{
	Character::Frame();
	m_pTrail->Frame();

	//
	// ��� �迭? ť? ����Ʈ?�� �÷��̾� ���� ���̴� ������ ���� �ѱ��
	// ���İ� �� ������ ����� ~ ������ ���� 
	// �ȼ� ���̴����� �ȼ� �׸���
	// 
	// �ν��Ͻ��� ����ϴ� ���
	// �ν��Ͻ� �����͸� ������۷� ����� ���� �������ϱ�
	// �÷��̾� ���� ���̴��� ������ �ؾ��ҵ�
	// https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=sorkelf&logNo=40171221589
	// https://www.braynzarsoft.net/viewtutorial/q16390-33-instancing-with-indexed-primitives
	// 
	// float4x4 mTransform : TRANSFORM; // ��İ�
	// float4 mesh : MESHCOLOR; // �÷��� 
	// �� �ΰ��� ������۷� ������ ������ �ѱ� ���� �迭�� �ε����� �ؿ� SV_VertexID���� ����� ����
	// 
	// �� ���� ������ �÷��̾� �ؿ� �� �ؿ� �Ž��� ���̴��� �����ؾ� �ϴµ� 
	// ������� �Ҵ��� 5���� �ϸ� �ȴٰ� ġ�� �ȼ� ���̴��� �ȹٲ㵵 �ɰ� ����
	// ���ؽ� ���̴��� �����ϸ� ���ٵ� 
	// ���� �ϴ� �Լ��� SetVertexShader(Shader* shader)�� ����ϸ� �ɵ� �𵨾ȿ� ������� ����ϸ� ��� �Ž��� ����ġ��
	// 
	// 
	// uint  iVertex : SV_VertexID; // �ý������� �� ���ؽ� ��ǲ ����ü �������ٿ� �߰����ָ� ��
	// 
	// m_pSprite->PreDraw(g_pImmediateContext);
	// {
	// 	ID3D11Buffer* vb[2] = { m_pSprite->m_dxobj.g_pVertexBuffer.Get(),	m_pSprite->m_pVBInstance.Get() };
	// 	UINT stride[2] = { sizeof(PNCT_VERTEX), sizeof(TInstatnce) };
	// 	UINT offset[2] = { 0, 0 };
	// 	g_pImmediateContext->IASetVertexBuffers(0, 2, vb, stride, offset);
	// 	g_pImmediateContext->UpdateSubresource(
	// 		m_pSprite->m_dxobj.g_pConstantBuffer.Get(), 0, NULL, &m_pSprite->m_cbData, 0, 0);
	// }
	// 	g_pImmediateContext->DrawIndexedInstanced(m_pSprite->m_dxobj.m_iNumIndex, (UINT)m_pSprite->m_pInstance.size(), 0, 0, 0);
	// (��ü�� ���� �ε��� ����, �ν��Ͻ� �������� ����, 0, 0, 0)
	// 
	//

	return true;
}

bool	Player::Render()
{
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &_toViewSpaceTransformBuffer);
	m_pImmediateContext->VSSetConstantBuffers(1, 1, &_objectToWorldTransformBuffer);
	m_pModel->Render();
	//{
	//	m_pModel->_currentAnimation->Render();
	//
	//	m_pImmediateContext->PSSetShader(m_pModel->_ps->m_pPS, NULL, 0);
	//
	//	for (auto material : m_pModel->_materials)
	//	{
	//		material.second->Render();
	//	}
	//
	//	for (auto mesh : m_pModel->_meshes)
	//	{
	//		mesh.second->Render();
	//	}
	//}

	return true;
}