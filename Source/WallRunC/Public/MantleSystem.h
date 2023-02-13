// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MantleSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLRUNC_API UMantleSystem : public UActorComponent
{
	GENERATED_BODY()

private:
	class AWRC_WallRunBase* PlayerChar;

	

public:	
	// Sets default values for this component's properties
	UMantleSystem();

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius = 500.0f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void LedgeCheck(UStaticMeshComponent GrabbableSurface);

	void CharMovementSwitch(bool CharState);

	void MoveChar();
	
};
