#pragma once

#include "Common.h"
#include <map>
#include "CharacterState.h"
#include "Character.h"
#include "CharacterStateBlackboard.h"

namespace SSB
{
	typedef std::string StateManagerName;

	class CharacterStateManager : public Common
	{
	private:
		std::map<StateName, CharacterState*> m_StateMap;
		std::map<Character*, CharacterState*> m_CharacterStateMap;
		std::map<Character*, CharacterState*> m_ReservedState;

	private:
		void RegisterReservedState(Blackboard* blackboard, Character* character, CharacterState* compareState);
		void PrepareForTransfer(Blackboard* blackboard, Character* character);

	public:
		void Initialize_RegisterState(StateName name, CharacterState* state);
		void RegisterCharacter(Character* character, StateName initialStateName);

	public:
		bool Init() override;
		bool Frame() override;
		bool Render() override;
		bool Release() override;

	private:
		CharacterStateBlackboardManager _blackboardManager;
	};

	const StateManagerName kPlayerStateManager = "PlayerStateManager";
	const StateManagerName kEnemyNPCMobStateManager = "EnemyNPCMobStateManager";
	const StateManagerName kFieldBossStateManager = "FieldBossStateManager";
	const StateManagerName kBossMobStateManager = "BossMobStateManager";
}
