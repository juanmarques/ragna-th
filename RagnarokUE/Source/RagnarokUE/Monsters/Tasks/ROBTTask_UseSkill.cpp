// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROBTTask_UseSkill.h"
#include "RagnarokUE/Monsters/ROMonsterBase.h"
#include "RagnarokUE/Monsters/ROMonsterAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"

UROBTTask_UseSkill::UROBTTask_UseSkill()
{
	NodeName = "RO: Use Monster Skill";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UROBTTask_UseSkill, BlackboardKey), AActor::StaticClass());
}

uint16 UROBTTask_UseSkill::GetInstanceMemorySize() const
{
	return sizeof(FSkillTaskMemory);
}

EBTNodeResult::Type UROBTTask_UseSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FSkillTaskMemory* Memory = reinterpret_cast<FSkillTaskMemory*>(NodeMemory);
	if (!Memory)
	{
		return EBTNodeResult::Failed;
	}

	Memory->Phase = ESkillPhase::SelectSkill;
	Memory->SelectedSkillIndex = -1;
	Memory->CastTimeRemaining = 0.0f;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster || Monster->bIsDead || Monster->SkillList.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor));

	// Select a skill
	const int32 SkillIdx = SelectSkill(Monster, Target);
	if (SkillIdx < 0)
	{
		// No skill available (all on cooldown or out of range)
		return EBTNodeResult::Failed;
	}

	Memory->SelectedSkillIndex = SkillIdx;
	const FROMonsterSkillEntry& Skill = Monster->SkillList[SkillIdx];

	// Face the target if we have one
	if (Target)
	{
		const FVector Direction = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
		if (!Direction.IsNearlyZero())
		{
			Monster->SetActorRotation(Direction.Rotation());
		}
	}

	// Check if skill has cast time
	if (Skill.CastTime > 0.0f)
	{
		Memory->Phase = ESkillPhase::Casting;
		Memory->CastTimeRemaining = Skill.CastTime;

		// Stop movement during cast
		AIController->StopMovement();

		return EBTNodeResult::InProgress;
	}

	// Instant skill - apply immediately
	Memory->Phase = ESkillPhase::ApplyEffect;
	ApplySkillEffect(Monster, Target, SkillIdx, AIController);

	// Put skill on cooldown
	Monster->SetSkillCooldown(Skill.SkillID, Skill.Cooldown);

	return EBTNodeResult::Succeeded;
}

void UROBTTask_UseSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FSkillTaskMemory* Memory = reinterpret_cast<FSkillTaskMemory*>(NodeMemory);
	if (!Memory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AROMonsterBase* Monster = Cast<AROMonsterBase>(AIController->GetPawn());
	if (!Monster || Monster->bIsDead)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Memory->SelectedSkillIndex < 0 || !Monster->SkillList.IsValidIndex(Memory->SelectedSkillIndex))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	switch (Memory->Phase)
	{
	case ESkillPhase::Casting:
	{
		Memory->CastTimeRemaining -= DeltaSeconds;
		if (Memory->CastTimeRemaining <= 0.0f)
		{
			// Cast complete, apply effect
			Memory->Phase = ESkillPhase::ApplyEffect;

			UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
			AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(AROMonsterAIController::BB_TargetActor)) : nullptr;

			const FROMonsterSkillEntry& Skill = Monster->SkillList[Memory->SelectedSkillIndex];
			ApplySkillEffect(Monster, Target, Memory->SelectedSkillIndex, AIController);

			// Put skill on cooldown
			Monster->SetSkillCooldown(Skill.SkillID, Skill.Cooldown);

			Memory->Phase = ESkillPhase::Done;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		break;
	}

	case ESkillPhase::Done:
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		break;
	}

	default:
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		break;
	}
}

EBTNodeResult::Type UROBTTask_UseSkill::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Cast interrupted
	return EBTNodeResult::Aborted;
}

int32 UROBTTask_UseSkill::SelectSkill(const AROMonsterBase* Monster, const AActor* Target) const
{
	if (!Monster || Monster->SkillList.Num() == 0)
	{
		return -1;
	}

	const float MonsterHPPercent = (Monster->MaxHP > 0)
		? (static_cast<float>(Monster->HP) / static_cast<float>(Monster->MaxHP)) * 100.0f
		: 100.0f;

	const float DistToTarget = Target
		? FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation())
		: 0.0f;

	// Collect eligible skills
	TArray<int32> EligibleSkills;

	for (int32 i = 0; i < Monster->SkillList.Num(); ++i)
	{
		const FROMonsterSkillEntry& Skill = Monster->SkillList[i];

		// Check cooldown
		if (!Monster->IsSkillReady(Skill.SkillID))
		{
			continue;
		}

		// Check HP threshold (skill only usable below this HP %)
		if (MonsterHPPercent > Skill.HPThresholdPercent)
		{
			continue;
		}

		// Check range (if we have a target)
		if (Target && Skill.Range > 0.0f && DistToTarget > Skill.Range)
		{
			continue;
		}

		// Roll use chance
		const float Roll = FMath::FRandRange(0.0f, 100.0f);
		if (Roll <= Skill.UseChance)
		{
			EligibleSkills.Add(i);
		}
	}

	if (EligibleSkills.Num() == 0)
	{
		return -1;
	}

	// Pick a random eligible skill
	return EligibleSkills[FMath::RandRange(0, EligibleSkills.Num() - 1)];
}

void UROBTTask_UseSkill::ApplySkillEffect(AROMonsterBase* Monster, AActor* Target, int32 SkillIndex,
	AAIController* AIController) const
{
	if (!Monster || !Monster->SkillList.IsValidIndex(SkillIndex))
	{
		return;
	}

	const FROMonsterSkillEntry& Skill = Monster->SkillList[SkillIndex];

	UE_LOG(LogTemp, Log, TEXT("Monster %s uses Skill ID %d (Lv %d)"),
		*Monster->MonsterName.ToString(), Skill.SkillID, Skill.SkillLevel);

	// Apply skill effect based on type
	// The skill system integration will expand this based on the Skills subsystem
	// For now, apply damage skills as direct damage to the target

	if (Target && IsValid(Target))
	{
		// Calculate skill damage: base ATK * skill level modifier
		// This is a simplified formula; the full skill system will handle specifics
		const float BaseDamage = static_cast<float>(Monster->ATK) * static_cast<float>(Skill.SkillLevel);
		const float SkillDamage = BaseDamage * 1.5f; // Skill multiplier placeholder

		FDamageEvent DamageEvent;
		Target->TakeDamage(SkillDamage, DamageEvent, AIController, Monster);
	}

	// Self-targeted skills (heal, buff, summon) would be handled here
	// For heal: Monster->HP = FMath::Min(Monster->HP + HealAmount, Monster->MaxHP);
	// For summon: Spawn additional monsters via spawn manager
	// For status effect: Apply to target via status effect system
}

FString UROBTTask_UseSkill::GetStaticDescription() const
{
	return TEXT("Select and use a monster skill.\nChecks cooldown, range, HP threshold, and use chance.\nHandles cast time and applies effect.");
}
