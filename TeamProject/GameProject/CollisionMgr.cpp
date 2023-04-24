#include "CollisionMgr.h"
#include "Player.h"

CollisionMgr::CollisionMgr()
{

}
CollisionMgr::~CollisionMgr()
{

}

	//
	// �� ��ü�� CollisionMgr�� ������ �ڽ��� ���ϴ� ����Ʈ�� �浹�˻縦 �ش޶�� ��û �� �ǵ������ ��� �ޱ�
	//

bool	CollisionMgr::ChkPlayerAttackToNpcList(T_BOX* box)
{
	for (auto iter : m_NpcList)
	{
		T_BOX* npcBox = iter.first;
		if (TCollision::ChkOBBToOBB(*box, *npcBox))
		{
			// NPC�� �ǰ� �̺�Ʈ Ȱ��ȭ?
			// �ƴϸ� ���� ���� ������ true false�� üũ

			return true;
		}
	}
	return false;
}

bool	CollisionMgr::ChkCharacterToStaticObjList(T_BOX* box)
{
	for (auto iter : m_StaticObjectList)
	{
		T_BOX* staticBox = iter.first;
		if (TCollision::ChkOBBToOBB(*box, *staticBox))
		{
			return true;
		}
	}
	return false;
}

std::vector<Character*> CollisionMgr::GetHitCharacterList(T_BOX* attackBox)
{
	std::vector<Character*> ret;
	for (auto iter : m_NpcList)
	{
		T_BOX* npcBox = iter.first;
		if (TCollision::ChkOBBToOBB(*attackBox, *npcBox))
		{
			ret.push_back(iter.second);
		}
	}
	return ret;
}

bool CollisionMgr::IsCollide(T_BOX* box)
{
	// NPC���� �浹 �� ������ ����
	//for (auto iter : m_NpcList)
	//{
	//	T_BOX* npcBox = iter.first;
	//	if (npcBox != box)
	//	{
	//		if (TCollision::ChkOBBToOBB(*box, *npcBox))
	//		{
	//			return true;
	//		}
	//	}
	//}

	for (auto iter : m_MapCollisionList)
	{
		if (TCollision::ChkOBBToOBB(*box, iter))
		{
			return true;
		}
	}

	return false;
}

bool CollisionMgr::IsCollideTrigger(T_BOX* box)
{
	for (auto iter : m_MapTriggerList)
	{
		if (TCollision::ChkOBBToOBB(*box, iter.second))
		{
			return true;
		}
	}

	return false;
}

void CollisionMgr::AddMapCollisionBox(T_BOX tBox)
{
	m_MapCollisionList.push_back(tBox);
}

void CollisionMgr::AddMapTriggerBox(std::wstring szName, T_BOX tBox)
{
	m_MapTriggerList.insert(std::make_pair(szName, tBox));
}

void CollisionMgr::DeleteTriggerBox(std::wstring szName)
{
	auto iter = m_MapTriggerList.find(szName);
	if (iter != m_MapTriggerList.end())
		m_MapTriggerList.erase(iter);
}

std::vector<T_BOX>& CollisionMgr::GetMapCollisionList()
{
	return m_MapCollisionList;
}

std::map<std::wstring, T_BOX>& CollisionMgr::GetMapTriggerList()
{
	return m_MapTriggerList;
}

void	CollisionMgr::AddStaticObjectBox(T_BOX* box, Character* pChar)
{
	if (m_StaticObjectList.find(box) == m_StaticObjectList.end())
	{
		m_StaticObjectList.insert(std::make_pair(box, pChar));
	}
}

void	CollisionMgr::AddNpcBox(T_BOX* box, Character* pChar)
{
	if (m_NpcList.find(box) == m_NpcList.end())
	{
		m_NpcList.insert(std::make_pair(box, pChar));
	}
}

void	CollisionMgr::DeleteBox(T_BOX* box)
{
	//test.erase(box);
	//int a = 4;
}