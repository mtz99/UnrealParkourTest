// Fill out your copyright notice in the Description page of Project Settings.

#include "WallRunC/WallRunCProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"

#include "WallRunC/Public/WRC_WallRunBase.h"

#include "Components/TimelineComponent.h"
#include "WallRun.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>




void UWallRun::BeginPlay()
{
	Super::BeginPlay();

	//AWRC_WallRunBase* PlayerChar = UGameplayStatics::GetActorOfClass(GetWorld(), AWRC_WallRunBase::StaticClass());
	PlayerChar = Cast<AWRC_WallRunBase>(this->GetOwner());

	WallRunDirection.X = 0.0;
	WallRunDirection.Y = 0.0;
	WallRunDirection.Z = 0.0;

	//Create timeline to handle camera tilt.
	FOnTimelineFloat TimelineProgress;
	TimelineProgress.BindDynamic(this, &UWallRun::TimelineProgress);
	CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
	CurveTimeline.SetLooping(false);
	
}

void UWallRun::TimelineProgress(float Value)
{
	float CamRollMultiplier;
	if (eWallRun == left)
		CamRollMultiplier = -1.0;
	else
		CamRollMultiplier = 1.0;


	FRotator NewActorRotation;
	NewActorRotation.Roll = Value * CamRollMultiplier;

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::SanitizeFloat(NewActorRotation.Roll)); } //Debug for cam rotation.
	NewActorRotation.Pitch = PlayerChar->YPitch;
	NewActorRotation.Yaw = PlayerChar->ZYaw;

	//Sets the player's controller to new rotation
	if (PlayerChar->Controller != nullptr)
		PlayerChar->Controller->SetControlRotation(NewActorRotation);
}

void UWallRun::InputActionJump()
{
	if (WallRunningBool) {
		EndWallRun(jumped);
	}
}


void UWallRun::BeginWallRun(FVector ImpactNormal)
{
	FRDASVals returnVals;
	FindRunDirectionAndSide(ImpactNormal, returnVals);
	
	//Don't forget to reset these values back to original state once you land on the ground.
	WallRunDirection = returnVals.Direction;
	eWallRun = returnVals.Side;
	
	
	FVector planeNormal;
	planeNormal.Z = 1.0;
	PlayerChar->GetCharacterMovement()->AirControl = 1.0f;
	PlayerChar->GetCharacterMovement()->GravityScale = 0.0f;
	PlayerChar->GetCharacterMovement()->SetPlaneConstraintNormal(planeNormal);
	WallRunningBool = true;
	BeginCameraTilt();
	UpdateWallRunBool = true;

}

void UWallRun::EndWallRun(StopReason reason)
{
	FVector PlaneNorm;
	PlaneNorm.X = 0.0;
	PlaneNorm.Y = 0.0;
	PlaneNorm.Z = 0.0;
	if (reason == fell) {
		PlayerChar->ResetJump(1);
	}
	else {
		PlayerChar->ResetJump(PlayerChar->MaxJumps - 1);
	}
	PlayerChar->GetCharacterMovement()->AirControl = 0.05;
	PlayerChar->GetCharacterMovement()->GravityScale = 1.0;
	PlayerChar->GetCharacterMovement()->SetPlaneConstraintNormal(PlaneNorm);
	WallRunningBool = false;

	EndCameraTilt();

	UpdateWallRunBool = false;

	//Calls function to change state machine to falling (NOJUMPSLEFT).
	PlayerChar->Falling();

	//Binded function to reset player rotation
	FOnTimelineEvent ResetDelegate;
	
	ResetDelegate.BindDynamic(this, &UWallRun::ResetPlayerCRotation);

	CurveTimeline.SetTimelineFinishedFunc(ResetDelegate);
}

void UWallRun::ResetPlayerCRotation() 
{
	PlayerChar->ResetPlayerCRotation();
}


void UWallRun::UpdateWallRun()
{
	//Local variables
	FHitResult OutHit;
	FVector TraceEnd;
	FCollisionQueryParams CollisionParams;
	FRDASVals FRDASVals;


	if (!AreRequiredKeysDown()) {
		EndWallRun(fell);
		return;
	}

	float multiplyVal;
	/*FVector WallRunSideLocalZ;*/
	if (eWallRun == left) {
		multiplyVal = -200.0f;
	}
	else {
		multiplyVal = 200.0f;

	}
	TraceEnd = PlayerChar->GetActorLocation() + (FVector::CrossProduct(WallRunDirection, FVector::UpVector) * multiplyVal);




	bool TraceHit = PlayerChar->ActorLineTraceSingle(OutHit, PlayerChar->GetActorLocation(), TraceEnd, ECC_WorldStatic, CollisionParams);
	//Trace debug stuff
	//DrawDebugLine(GetWorld(), GetActorLocation(), TraceEnd, FColor::Red, false, 100.0f);
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TraceHit ? "True" : "False"); }



	if (!TraceHit) {
		//Appears that trace hit always hits, but doesn't detect when it's not hitting an object.
		EndWallRun(fell);
		return;
	}

	FindRunDirectionAndSide(OutHit.ImpactNormal, FRDASVals);

	if (!FRDASVals.Side == eWallRun) {
		EndWallRun(fell);
		return;
	}

	WallRunDirection = FRDASVals.Direction;

	PlayerChar->GetCharacterMovement()->Velocity.X = WallRunDirection.X * PlayerChar->GetCharacterMovement()->GetMaxSpeed();
	PlayerChar->GetCharacterMovement()->Velocity.Y = WallRunDirection.Y * PlayerChar->GetCharacterMovement()->GetMaxSpeed();
	PlayerChar->GetCharacterMovement()->Velocity.Z = 0.0;


}



void UWallRun::BeginCameraTilt()
{
	if (CurveFloat) {
		PlayerChar->XRoll = PlayerChar->GetControlRotation().Roll;
		PlayerChar->YPitch = PlayerChar->GetControlRotation().Pitch;
		PlayerChar->ZYaw = PlayerChar->GetControlRotation().Yaw;

		CurveTimeline.PlayFromStart();
	}
}


void UWallRun::EndCameraTilt()
{
	if (CurveFloat) {
		PlayerChar->XRoll = PlayerChar->GetControlRotation().Roll;
		PlayerChar->YPitch = PlayerChar->GetControlRotation().Pitch;
		PlayerChar->ZYaw = PlayerChar->GetControlRotation().Yaw;

		CurveTimeline.Reverse();
		//Within the timeline's reverse function, a signal is sent out to say when the curve is done playing.
	}
}

void UWallRun::ResetTimeline()
{
	FOnTimelineEvent clearBind;
	CurveTimeline.SetTimelineFinishedFunc(clearBind);
	PlayerChar->ResetPlayerCRotation();
}

void UWallRun::FindRunDirectionAndSide(FVector WallNormal, FRDASVals& returnVals) const
{
	WallRunSide localSide;
	FVector localVector;
	FVector rightVector = PlayerChar->GetActorRightVector();

	localVector.X = 0.0;
	localVector.Y = 0.0;

	if (FVector::DotProduct(WallNormal.GetSafeNormal2D(), rightVector.GetSafeNormal2D()) > 0) {
		localSide = right;
		localVector.Z = 1;
	}
	else
	{
		localSide = left;
		localVector.Z = -1;
	}

	returnVals.Side = localSide;
	returnVals.Direction = FVector::CrossProduct(WallNormal, localVector);
}

bool UWallRun::CanSurfaceWallBeRan(FVector SurfaceNormal) const
{
	float SurfaceAngleCheck;
	if (SurfaceNormal.Z < -0.05) {
		return false;
	}

	FVector NormalizedSNVector = SurfaceNormal.GetSafeNormal2D(0.0001);


	SurfaceAngleCheck = FMath::RadiansToDegrees(FMath::Acos(NormalizedSNVector.DotProduct(NormalizedSNVector, SurfaceNormal)));
	float MaxFloorAngle = PlayerChar->GetCharacterMovement()->GetWalkableFloorAngle();
	if (SurfaceAngleCheck < MaxFloorAngle) {
		return true;
	}
	return false;

}

FVector UWallRun::FindLaunchVelocity() const {

	FVector WallRunSideLocalZ;
	if (WallRunningBool) {
		

		WallRunSideLocalZ.X = 0.0;
		WallRunSideLocalZ.Y = 0.0;

		if (eWallRun == left) {
			WallRunSideLocalZ.Z = 1.0;
		}
		else {
			WallRunSideLocalZ.Z = -1.0;
		}
	}
	
	return WallRunSideLocalZ;
}

bool UWallRun::AreRequiredKeysDown() const
{
	if (PlayerChar->ForwardAxis <= 0.1) {
		return false;
	}

	if (eWallRun == left) {
		return PlayerChar->RightAxis > 0.1;
	}
	else {
		return PlayerChar->RightAxis < -0.1;
	}
}


void UWallRun::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UpdateWallRunBool) {
		UpdateWallRun();
	}

	CurveTimeline.TickTimeline(DeltaTime);
}