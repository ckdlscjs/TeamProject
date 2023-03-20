#include "CollisionMgr.h"
#include "Player.h"

CollisionMgr::CollisionMgr()
{

}
CollisionMgr::~CollisionMgr()
{

}

bool	CollisionMgr::Frame()
{
	Player* player = &Player::GetInstance();

	// static mesh -> player check
	// �浹�� �÷��̾��� ���� �ݴ������� �̵��ӵ���ŭ �ڷ� 
	for (auto iter : m_StaticObjectList)
	{
		T_BOX box = *iter.first;
		if (TCollision::ChkOBBToOBB(box, player->m_ColliderBox))
		{
			player->m_vPos = player->m_vPos + (-player->m_vDirection * player->m_fSpeed * g_fSecondPerFrame);
			player->UpdateMatrix();
			//player->UpdateBox();
			//player->UpdateBuffer();
		}
	}

	// player attack -> npc check
	// �浹�� npc���� ������ ���� ������ ���� npc�� ���� �����̳� ���� ���ݿ� ���� ������ ����
	// ��Ÿ�� ���� ������ ������ ���� �浹 npc�� ������ ���� Ÿ���̸� ���� ���� ���� ����
	// �Ƹ� ���� �������� �ҵ�?

	// npc attack -> player check
	// npc ����(�ٰŸ�) �÷��̾�� ������ �� ��������
	// �÷��̾ �������� �޾����� break�ص� �ɵ�

	// 

	return true;
}
void	CollisionMgr::AddBox(T_BOX* box, Character* pChar)
{
	if (test.find(box) == test.end())
	{
		test.insert(std::make_pair(box, pChar));

	}
	int a = 4;
}

void	CollisionMgr::AddStaticObjectBox(T_BOX* box, Character* pChar)
{
	if (m_StaticObjectList.find(box) == m_StaticObjectList.end())
	{
		m_StaticObjectList.insert(std::make_pair(box, pChar));

	}
}

void	CollisionMgr::DeleteBox(T_BOX* box)
{
	test.erase(box);
	int a = 4;
}