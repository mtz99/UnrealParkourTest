// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRun.generated.h"

/**
 * 
 */
UCLASS()
class WALLRUNC_API UWallRun : public UActorComponent
{
	GENERATED_BODY()
private:
	class AWRC_WallRunBase* PlayerChar;


	FVector WallRunDirection;

	bool WallRunningBool = false;

	enum WallRunSide { left, right };

	enum StopReason { fell, jumped };

	WallRunSide eWallRun = left;

	float prevRotatorValue = 0.0f;

public:
	
	UWallRun(const FObjectInitializer& ObjectInitalizer);

	virtual void BeginPlay() override;


	//Blueprint timeline in C++
	UFUNCTION()
	void TimelineProgress(float Value);
	
	FTimeline CurveTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

	

	// Called every frame
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InputActionJump();
	
	void SetEWallRun(FVector ImpactNormal);
	
	void BeginWallRun();

	void EndWallRun(StopReason reason);

	UFUNCTION()
	void ResetPlayerCRotation();

	void UpdateWallRun();

	void BeginCameraTilt();

	void EndCameraTilt();

	UFUNCTION()
	void ResetTimeline();
	
	struct FRDASVals {
		FVector Direction;
		WallRunSide Side;
	};



	void FindRunDirectionAndSide(FVector WallNormal, FRDASVals& returnVals) const;

	bool CanSurfaceWallBeRan(FVector SurfaceNormal) const;

	FVector FindLaunchVelocity() const;

	bool AreRequiredKeysDown() const;
};
