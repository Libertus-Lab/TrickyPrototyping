// Copyright (c) 2022 Artyom "Tricky Fat Cat" Volkov (tricky.fat.cat@gmail.com)


#include "Actors/FloatingActorBase.h"

#include "Components/TimelineComponent.h"


AFloatingActorBase::AFloatingActorBase()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformRoot = CreateDefaultSubobject<USceneComponent>("PlatformRoot");
	SetRootComponent(PlatformRoot);

	MovementTimeline = CreateDefaultSubobject<UTimelineComponent>("MovementTimeline");
}

void AFloatingActorBase::BeginPlay()
{
	if (MovementTimeline && MovementAnimationCurve)
	{
		FOnTimelineFloat MovementProgress{};
		MovementProgress.BindUFunction(this, FName("MovePlatform"));
		MovementTimeline->AddInterpFloat(MovementAnimationCurve, MovementProgress);

		FOnTimelineEvent MovementFinished{};
		MovementFinished.BindUFunction(this, FName("ContinueMovement"));
		MovementTimeline->SetTimelineFinishedFunc(MovementFinished);

		CalculateTimelinePlayRate();
	}

	if (WaitDuration <= 0.f)
	{
		bStopAtPoints = false;
	}

	if (bAutoStart)
	{
		StartMovement();
	}

	Super::BeginPlay();
}

void AFloatingActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ConstructActor();
}

void AFloatingActorBase::ConstructActor()
{
	FillPointIndexes();

	if (PointsIndexes.Num() > 0)
	{
		if (!IndexIsValid(StartPointIndex))
		{
			StartPointIndex = 0;
			// TODO Print error.
		}

		CurrentPointIndex = StartPointIndex;
		NextPointIndex = StartPointIndex;
		MovePlatform(0.0f);

		if (bStopAtPoints && bStopAtCertainPoints && CustomStopsIndexes.Num() > 0)
		{
			RemoveInvalidCustomIndexes();
			auto Iterator = [](const int32& LHS, const int32& RHS) { return LHS < RHS; };
			CustomStopsIndexes.Sort(Iterator);
		}
	}
	else
	{
		// TODO Print error
	}
}

void AFloatingActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloatingActorBase::StartMovement()
{
	if (State == EPlatformState::Moving || PointsIndexes.Num() == 0) return; // TODO Print error if Num == 0

	if (CurrentPointIndex == NextPointIndex)
	{
		CalculateNextPointIndex();
	}

	CalculateTravelTime();
	CalculateTimelinePlayRate();

	if (bStopAtPoints && bWaitAtStart)
	{
		StartStopWaitTimer();
		return;
	}

	MovementTimeline->PlayFromStart();
	SetState(EPlatformState::Moving);
}

void AFloatingActorBase::StopMovement()
{
	if (State == EPlatformState::Idle) return;

	MovementTimeline->Stop();
	SetState(EPlatformState::Idle);
}

void AFloatingActorBase::ResumeMovement()
{
	if (State != EPlatformState::Idle) return;

	MovementTimeline->Play();
	SetState(EPlatformState::Moving);
}

void AFloatingActorBase::MoveToPoint(const int32 PointIndex)
{
	if (!IndexIsValid(PointIndex)) return;

	NextPointIndex = PointIndex;
	CalculateTravelTime();
	StartMovement();
}

void AFloatingActorBase::SetSpeed(const float Value)
{
	if (Value < 0.f) return;

	Speed = Value;
	CalculateTravelTime();
}

bool AFloatingActorBase::IndexIsValid(const int32 Index) const
{
	return Index >= 0 && Index < PointsIndexes.Num();
}

void AFloatingActorBase::CalculateTravelTime()
{
}

void AFloatingActorBase::FillPointIndexes()
{
	PointsIndexes.Empty();
}

void AFloatingActorBase::RemoveInvalidCustomIndexes()
{
}

void AFloatingActorBase::MovePlatform(const float Progress)
{
}

void AFloatingActorBase::ContinueMovement()
{
	if (State == EPlatformState::Idle) return;

	OnPointReached.Broadcast(CurrentPointIndex);
	CalculateNextPointIndex();
	CalculateTravelTime();

	if (bStopAtPoints)
	{
		if (bStopAtPoints && CustomStopsIndexes.Contains(CurrentPointIndex))
		{
			StartStopWaitTimer();
		}
		else if (!bStopAtCertainPoints)
		{
			StartStopWaitTimer();
		}
		else
		{
			MovementTimeline->PlayFromStart();
		}
	}
	else
	{
		MovementTimeline->PlayFromStart();
	}
}

void AFloatingActorBase::CalculateNextPointIndex()
{
	auto CalculateNextIndex = [&]() { NextPointIndex = bIsReversed ? CurrentPointIndex - 1 : CurrentPointIndex + 1; };

	CurrentPointIndex = NextPointIndex;
	CalculateNextIndex();
	const bool bIndexIsValid = IndexIsValid(NextPointIndex);

	switch (MovementMode)
	{
		case EPlatformMovementMode::Loop:
			if (!bIndexIsValid)
			{
				CurrentPointIndex = bIsReversed ? PointsIndexes.Num() - 1 : 0;
				CalculateNextIndex();
			}
			break;

		case EPlatformMovementMode::PingPong:
			if (!bIndexIsValid)
			{
				bIsReversed = !bIsReversed;
				CalculateNextIndex();
			}
			break;

		default:
			break;
	}
}

void AFloatingActorBase::StartStopWaitTimer()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	if (!TimerManager.IsTimerActive(WaitTimerHandle))
	{
		OnWaitStarted.Broadcast(CurrentPointIndex);
		SetState(EPlatformState::Waiting);
		TimerManager.SetTimer(WaitTimerHandle,
		                      this,
		                      &AFloatingActorBase::FinishStopTimer,
		                      WaitDuration,
		                      false);
	}
}

void AFloatingActorBase::FinishStopTimer()
{
	OnWaitFinished.Broadcast(CurrentPointIndex);
	SetState(EPlatformState::Moving);
	MovementTimeline->PlayFromStart();
}

void AFloatingActorBase::SetState(const EPlatformState NewState)
{
	if (State == NewState) return;

	State = NewState;
	OnStateChanged.Broadcast(State);
}

void AFloatingActorBase::CalculateTimelinePlayRate()
{
	if (TravelTime <= 0.f)
	{
		TravelTime = 1.f;
		// TODO Print error;
	}

	MovementTimeline->SetPlayRate(1 / TravelTime);
}
