// Fill out your copyright notice in the Description page of Project Settings.

#include "WallRunC/Public/WRC_WallRunBase.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

#include "WallRunCameraModifier.h"
#include "WallRunC/Public/WallRun.h"

UWallRunCameraModifier::UWallRunCameraModifier(const FObjectInitializer& ObjectInitalizer)
{
	
}

//Create an init function and pass in the wall run comp value and set it there
void UWallRunCameraModifier::Init(UWallRun* Comp)
{
	WallRunComp = Comp; 
	PlayerChar = Cast<AWRC_WallRunBase>(WallRunComp->GetOwner());
}

/*Move all this to the camera modifier : https://www.youtube.com/watch?v=BOtItHPL39k */
void UWallRunCameraModifier::UpdateWallRun()
{
	//Local variables
	FHitResult OutHit;
	FVector TraceEnd;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PlayerChar);
	UWallRun::FRDASVals FRDASVals;


	if (!WallRunComp->AreRequiredKeysDown()) {

		WallRunComp->EndWallRun(UWallRun::StopReason::fell);
		return;
	}

	float multiplyVal;
	/*FVector WallRunSideLocalZ;*/
	if (WallRunComp->eWallRun == UWallRun::WallRunSide::left) {
		multiplyVal = -200.0f;
	}
	else {
		multiplyVal = 200.0f;

	}

	TraceEnd = PlayerChar->GetActorLocation() + (FVector::CrossProduct(WallRunComp->WallRunDirection, FVector::UpVector) * multiplyVal);




	bool TraceHit = GetWorld()->LineTraceSingleByChannel(OutHit, PlayerChar->GetActorLocation(), TraceEnd, ECC_WorldStatic, CollisionParams);
	//Trace debug stuff
	//DrawDebugLine(GetWorld(), PlayerChar->GetActorLocation(), TraceEnd, TraceHit ? FColor::Green : FColor::Red, false, 100.0f);
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TraceHit ? "True" : "False"); }



	if (!TraceHit) {
		WallRunComp->EndWallRun(UWallRun::StopReason::fell);
		return;
	}

	WallRunComp->FindRunDirectionAndSide(OutHit.ImpactNormal, FRDASVals);

	if (FRDASVals.Side != WallRunComp->eWallRun) {
		WallRunComp->EndWallRun(UWallRun::StopReason::fell);
		return;
	}

	WallRunComp->WallRunDirection = FRDASVals.Direction;

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("WallRunDirection:%f, %f,"), WallRunDirection.X, WallRunDirection.Y)); }


	
	

	PlayerChar->GetCharacterMovement()->Velocity.X = WallRunComp->WallRunDirection.X * PlayerChar->GetCharacterMovement()->GetMaxSpeed();
	PlayerChar->GetCharacterMovement()->Velocity.Y = WallRunComp->WallRunDirection.Y * PlayerChar->GetCharacterMovement()->GetMaxSpeed();
	PlayerChar->GetCharacterMovement()->Velocity.Z = 0.0;
}

void UWallRunCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	ViewRotation.Yaw += 20;
}