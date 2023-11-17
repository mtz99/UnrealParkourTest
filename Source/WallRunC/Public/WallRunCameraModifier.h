// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "WallRunCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class WALLRUNC_API UWallRunCameraModifier : public UCameraModifier
{
	GENERATED_BODY()
	
private:
	
	UPROPERTY()
	class UWallRun* WallRunComp;

	class AWRC_WallRunBase* PlayerChar;
	
	void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;

public:

	UWallRunCameraModifier(const FObjectInitializer& ObjectInitalizer);

	void Init(UWallRun* Comp);

	void UpdateWallRun();

};
