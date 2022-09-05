// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WRC_WallRunBase.h"
#include "WallRun.generated.h"

/**
 * 
 */
UCLASS()
class WALLRUNC_API UWallRun : public UActorComponent
{
private:
	AWRC_WallRunBase* PlayerChar;


	FVector WallRunDirection;

	bool WallRunningBool = false;

	bool UpdateWallRunBool = false;

	enum WallRunSide { left, right };

	enum StopReason { fell, jumped };

	WallRunSide eWallRun = left;

public:
	GENERATED_BODY()

	virtual void BeginPlay() override;


	//Blueprint timeline in C++
	UFUNCTION()
	void TimelineProgress(float Value);
	
	FTimeline CurveTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void InputActionJump();
	
	virtual void BeginWallRun();

	virtual void EndWallRun(StopReason reason);

	virtual void UpdateWallRun();

	virtual void BeginCameraTilt();

	virtual void EndCameraTilt();

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
