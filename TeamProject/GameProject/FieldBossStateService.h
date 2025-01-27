#pragma once

#include "CharacterState.h"

namespace SSB
{
	const StateName kFieldBossMobIdle = "Idle";
	const StateName kFieldBossMobMove = "Move";
	const StateName kFieldBossMobAttack = "Attack";
	const StateName kFieldBossMobAttackStart = "AttackStart";
	const StateName kFieldBossMobSkillCasting = "SkillCasting";
	const StateName kFieldBossMobSkillFire = "SkillFire";
	const StateName kFieldBossMobDead = "Dead";
	const StateName kFieldBossMobAirborne = "Airborne";
	const StateName kFieldBossMobPound = "Pound";

	const unsigned int FieldBossMobDeadTypePriority = 0;
	const unsigned int FieldBossMobAirborneTypePriority = 1;
	const unsigned int FieldBossMobSkillTypePriority = 2;
	const unsigned int FieldBossMobAttackTypePriority = 3;
	const unsigned int FieldBossMobMoveTypePriority = 4;

	class FieldBossIdleState : public CharacterState
	{
	public:
		void StateDecision() override;
		void Action() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossAirBorneState : public CharacterState
	{
	private:
		// If change this value, consider PlayerStateService::PlayerUltimateSkillState::Action::elem->m_vPos.y
		float _airborneHeight = 3;

	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossPoundState : public CharacterState, public MinimumTransferCoolTimeRequireInterface
	{
	private:
		float _transferRequireTime;

	public:
		FieldBossPoundState(float transferRequireTime);

	public:
		void StateDecision() override;
		void Action() override;
		float GetTransferRequireTime() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossMoveState : public CharacterState
	{
	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossAttackStartState : public CharacterState, public MinimumTransferCoolTimeRequireInterface, public DamageTypeStateInterface
	{
	private:
		float _transferRequireTime;
		bool _isAttacked = false;

	public:
		FieldBossAttackStartState(float transferRequireTime);

	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		float GetTransferRequireTime() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossAttackState : public CharacterState, public MinimumTransferCoolTimeRequireInterface, public DamageTypeStateInterface
	{
	private:
		float _transferRequireTime;
		bool _isAttacked = false;

	public:
		FieldBossAttackState(float transferRequireTime);

	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		float GetTransferRequireTime() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossSkillCastingState : public CharacterState, public MinimumTransferCoolTimeRequireInterface, public DamageTypeStateInterface
	{
	private:
		float _transferRequireTime;

	public:
		FieldBossSkillCastingState(float transferRequireTime);

	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		float GetTransferRequireTime() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossSkillFireState : public CharacterState, public MinimumTransferCoolTimeRequireInterface, public DamageTypeStateInterface
	{
	private:
		float _transferRequireTime;
		float _attackTimeStamp = 0.0f;
		const float _kAttackInterval = 0.3f;

	public:
		FieldBossSkillFireState(float transferRequireTime);

	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		float GetTransferRequireTime() override;
		std::vector<std::string> GetLinkedList() override;
	};

	class FieldBossDeadState : public CharacterState
	{
	public:
		void StateDecision() override;
		void Action() override;
		StateTransferPriority GetPriority() override;
		std::vector<std::string> GetLinkedList() override;
	};
}

