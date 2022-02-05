// Copyright (c) 2021 Artyom "Tricky Fat Cat" Volkov (tricky.fat.cat@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "Components/TriggerComponents/BaseBoxTriggerComponent.h"
#include "InteractionBoxComponent.generated.h"

/**
 * A box trigger which adds an actor to an interaction queue
 */
UCLASS(ClassGroup=(Interaction), BlueprintType, meta=(BlueprintSpawnableComponent))
class TRICKYPROTOTYPING_API UInteractionBoxComponent : public UBaseBoxTriggerComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintGetter, Category="Trigger")
	bool GetIsNormalTrigger() const { return bIsNormalTrigger; }

	UFUNCTION(BlueprintSetter, Category="Trigger")
	void SetIsNormalTrigger(const bool Value) { bIsNormalTrigger = Value; }

	UPROPERTY(BlueprintReadWrite, Category="Trigger")
	bool bRequireLineOfSight = false;
private:
	UPROPERTY(VisibleAnywhere,
		BlueprintGetter=GetIsNormalTrigger,
		BlueprintSetter=SetIsNormalTrigger,
		Category="Trigger",
		meta=(AllowPrivateAccess="true"))
	bool bIsNormalTrigger = false;

	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	                            AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp,
	                            int32 OtherBodyIndex,
	                            bool bFromSweep,
	                            const FHitResult& SweepResult) override;

	virtual void OnEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                          AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp,
	                          int32 OtherBodyIndex) override;
};
