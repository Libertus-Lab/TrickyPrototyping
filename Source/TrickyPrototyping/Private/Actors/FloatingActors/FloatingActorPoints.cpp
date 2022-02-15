// Copyright (c) 2022 Artyom "Tricky Fat Cat" Volkov (tricky.fat.cat@gmail.com)


#include "Actors/FloatingActors/FloatingActorPoints.h"

void AFloatingActorPoints::CalculateTravelTime()
{
	Super::CalculateTravelTime();
}

void AFloatingActorPoints::FillPointIndexes()
{
	Super::FillPointIndexes();

	if (TargetActors.Num() == 0 || TargetActors.Num() == 1)
	{
		// TODO Print error
		return;
	}

	auto FillIndexes = [&]()
	{
		for (int32 i = 0; i < TargetActors.Num(); i++)
		{
			if (!TargetActors[i])
			{
				// TODO Print error
				continue;
			}
			PointsIndexes.Add(i);
		}
	};

	switch (MovementMode)
	{
		case EFloatingActorMovementMode::Manual:
			FillIndexes();
			break;

		default:
			if (bStopAtPoints && bStopAtCertainPoints)
			{
				if (CustomStopsIndexes.Num() < 2)
				{
					return; // TODO Print error;
				}

				PointsIndexes = CustomStopsIndexes;
				SortPointsIndexes();

				return;
			}

			FillIndexes();
			break;
	}
}

void AFloatingActorPoints::RemoveInvalidCustomIndexes()
{
	Super::RemoveInvalidCustomIndexes();

	for (int32 i = 0; i < CustomStopsIndexes.Num(); i++)
	{
		const int32 Index = CustomStopsIndexes[i];

		if (Index < 0 || Index >= TargetActors.Num())
		{
			CustomStopsIndexes.Remove(Index);
			continue;
		}

		if (!TargetActors[Index])
		{
			// TODO Print error
			CustomStopsIndexes.Remove(Index);
		}
	}
}

void AFloatingActorPoints::MovePlatform(const float Progress)
{
	Super::MovePlatform(Progress);
}
