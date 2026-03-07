// Copyright Ragna-TH Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ROBTTask_UseSkill.generated.h"

class AROMonsterBase;

/**
 * UROBTTask_UseSkill
 * BT Task: Select a skill from the monster's skill list, check cooldown,
 * face the target, begin cast if applicable, and apply the skill effect.
 */
UCLASS(meta = (DisplayName = "RO: Use Monster Skill"))
class RAGNAROKUE_API UROBTTask_UseSkill : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UROBTTask_UseSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Execution phases for skill usage. */
	enum class ESkillPhase : uint8
	{
		SelectSkill,
		FaceTarget,
		Casting,
		ApplyEffect,
		Done
	};

	/** Per-instance memory for this task. */
	struct FSkillTaskMemory
	{
		ESkillPhase Phase = ESkillPhase::SelectSkill;
		int32 SelectedSkillIndex = -1;
		float CastTimeRemaining = 0.0f;
	};

	/**
	 * Select the best available skill from the monster's skill list.
	 * Considers cooldowns, HP threshold, range, and use chance.
	 * @return Index into SkillList, or -1 if no skill available.
	 */
	int32 SelectSkill(const AROMonsterBase* Monster, const AActor* Target) const;

	/**
	 * Apply the selected skill's effect.
	 * Handles damage, status effects, healing, and summon skills.
	 */
	void ApplySkillEffect(AROMonsterBase* Monster, AActor* Target, int32 SkillIndex, class AAIController* AIController) const;
};
