#include "PlayerStateService.h"
#include "Input.h"
#include "Player.h"
#include "CollisionMgr.h"

namespace SSB
{
	bool PlayerIdleState::IsTransfer()
	{
		bool transfer = false;

		if (m_pCharacter->IsDead())
		{
			transfer = true;
			SetNextTransferName(kPlayerDead);
		}
		else
		{
			if (I_Input.GetKey('A') == KEY_HOLD ||
				I_Input.GetKey('D') == KEY_HOLD ||
				I_Input.GetKey('W') == KEY_HOLD ||
				I_Input.GetKey('S') == KEY_HOLD)
			{
				transfer = true;
				SetNextTransferName(kPlayerMove);
			}

			if (I_Input.GetKey(VK_LBUTTON) == KEY_PUSH)
			{
				transfer = true;
				SetNextTransferName(kPlayerAttack);
			}
		}

		return transfer;
	}
	bool PlayerMoveState::IsTransfer()
	{
		bool transfer = false;

		if (m_pCharacter->IsDead())
		{
			transfer = true;
			SetNextTransferName(kPlayerDead);
		}
		else
		{
			if (I_Input.GetKey(VK_LBUTTON) == KEY_PUSH)
			{
				transfer = true;
				SetNextTransferName(kPlayerAttack);
			}

			if (!transfer)
			{
				if (I_Input.GetKey('A') != KEY_HOLD &&
					I_Input.GetKey('D') != KEY_HOLD &&
					I_Input.GetKey('W') != KEY_HOLD &&
					I_Input.GetKey('S') != KEY_HOLD)
				{
					transfer = true;
					SetNextTransferName(kPlayerIdle);
				}
			}
		}

		return transfer;
	}
	void PlayerMoveState::Run()
	{
        CharacterState::Run();
		Player* player = static_cast<Player*>(m_pCharacter);

		bool moveChar = false;
		XMVECTOR desiredCharDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		if (I_Input.GetKey('A') == KEY_HOLD)
		{
			desiredCharDir += (player->m_pMainCamera->m_vRight);
			moveChar = true;
		}
		if (I_Input.GetKey('D') == KEY_HOLD)
		{
			desiredCharDir += -(player->m_pMainCamera->m_vRight);
			moveChar = true;
		}
		if (I_Input.GetKey('W') == KEY_HOLD)
		{
			desiredCharDir += (player->m_pMainCamera->m_vLook);
			moveChar = true;
		}
		if (I_Input.GetKey('S') == KEY_HOLD)
		{
			desiredCharDir += -(player->m_pMainCamera->m_vLook);
			moveChar = true;
		}

		if (moveChar == true)
		{
			XMMATRIX world = XMLoadFloat4x4(&player->m_matWorld);
			player->MoveChar(desiredCharDir, world);
		}
	}
	bool PlayerAttackState::IsTransfer()
	{
		bool transfer = false;

		if (m_pCharacter->IsDead())
		{
			transfer = true;
			SetNextTransferName(kPlayerDead);
		}
		else
		{
			// ���� Ÿ�̸� > 0, isAttack != true, �ִϸ��̼��� �� �˸����� ������ �߰��� �ʿ� ����
			if (IsPassedRequireCoolTime(m_pCharacter->GetStateElapseTime()))
			{
			if (I_Input.GetKey(VK_LBUTTON) != KEY_PUSH &&
					I_Input.GetKey(VK_LBUTTON) != KEY_HOLD)
				{
					transfer = true;
					SetNextTransferName(kPlayerIdle);
				}
			}
		}

		return transfer;
	}
	void PlayerAttackState::Run()
	{
        CharacterState::Run();
		// ���õ� ������ �ִϸ��̼� ����� �����ͼ� ���� �ڽ��� ������Ѽ� 
		// �浹 ó���� �� �� �ְԲ� �ؾ���
        if (I_Collision.ChkPlayerAttackToNpcList(&m_pCharacter->m_AttackBox))
        {
			auto list = I_Collision.GetHitCharacterList(&m_pCharacter->m_AttackBox);
			for (auto obj : list)
			{
				if (obj != m_pCharacter)
				{
					obj->Damage(m_pCharacter->m_Damage, m_pCharacter->m_fStateTImeStamp);
				}
			}
        }
	}
	bool PlayerDeadState::IsTransfer()
	{
		return false;
	}
}
