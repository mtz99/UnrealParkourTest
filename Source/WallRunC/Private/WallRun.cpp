// Fill out your copyright notice in the Description page of Project Settings.

#include "WallRun.h"
#include "WallRunC/WallRunCProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "WallRunC/Public/WRC_WallRunBase.h"

#include "Components/TimelineComponent.h"

#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>



//PRAGMA_DISABLE_OPTIMIZATION

UWallRun::UWallRun(const FObjectInitializer& ObjectInitalizer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UWallRun::BeginPlay()
{
	Super::BeginPlay();

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

	//A multiplier of some type needs to be applied.
	/*float CamRollMultiplier;
	if (eWallRun == left)
		CamRollMultiplier = -1.0;
	else
		CamRollMultiplier = 1.0;

	Value = Value * CamRollMultiplier;*/
	
	FQuat NewActorRotation(PlayerChar->GetActorForwardVector(), FMath::DegreesToRadians((Value - prevRotatorValue)));

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, NewActorRotation.ToString()); } //Debug for cam rotation.
	
	if (PlayerChar->Controller != nullptr) {
		//Add in input rotation
		PlayerChar->GetCapsuleComponent()->AddLocalRotation(FRotator(NewActorRotation));
		//Set the control rotation using the capsules rotation
		PlayerChar->Controller->SetControlRotation(PlayerChar->GetCapsuleComponent()->GetComponentRotation());
	}

	
	
	prevRotatorValue = Value;

}

void UWallRun::InputActionJump()
{
	if (WallRunningBool) {
		EndWallRun(jumped);
	}
}

void UWallRun::SetEWallRun(FVector ImpactNormal)
{
	FRDASVals returnVals;
	FindRunDirectionAndSide(ImpactNormal, returnVals);
	
	//Don't forget to reset these values back to original state once you land on the ground.
	WallRunDirection = returnVals.Direction;
	eWallRun = returnVals.Side;
}


//Camera tilt could either be state mgmt issue or unknown flag I don't know of, or maybe I need to recreate timeline everytime?
//Maybe there's a reset function for timeline.

void UWallRun::BeginWallRun()
{
	//Clearing all reset delegates.
	CurveTimeline.SetTimelineFinishedFunc(FOnTimelineEvent());

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Begin Wall Run."))); }
	FVector planeNormal;
	planeNormal.Z = 1.0;
	PlayerChar->GetCharacterMovement()->AirControl = 1.0f;
	PlayerChar->GetCharacterMovement()->GravityScale = 0.0f;
	//PlayerChar->GetCharacterMovement()->SetPlaneConstraintNormal(planeNormal);
	WallRunningBool = true;
	BeginCameraTilt();
}

void UWallRun::EndWallRun(StopReason reason)
{
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("End Wall Run."))); }

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
	//PlayerChar->GetCharacterMovement()->SetPlaneConstraintNormal(PlaneNorm);
	
	EndCameraTilt();
	
	//Resetting all set variables to original state.
	WallRunningBool = false;
	WallRunDirection = FVector::ZeroVector;
	eWallRun = left;

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
	CollisionParams.AddIgnoredActor(PlayerChar);
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




	bool TraceHit = GetWorld()->LineTraceSingleByChannel(OutHit, PlayerChar->GetActorLocation(), TraceEnd, ECC_WorldStatic, CollisionParams);
	//Trace debug stuff
	//DrawDebugLine(GetWorld(), PlayerChar->GetActorLocation(), TraceEnd, TraceHit ? FColor::Green : FColor::Red, false, 100.0f);
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TraceHit ? "True" : "False"); }



	if (!TraceHit) {
		EndWallRun(fell);
		return;
	}

	FindRunDirectionAndSide(OutHit.ImpactNormal, FRDASVals);

	if (FRDASVals.Side != eWallRun) {
		EndWallRun(fell);
		return;
	}

	WallRunDirection = FRDASVals.Direction;

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("WallRunDirection:%f, %f,"), WallRunDirection.X, WallRunDirection.Y)); }

#if 0
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("WallRunVelocity:%f, %f, %f,"),
			PlayerChar->GetCharacterMovement()->Velocity.X,
			PlayerChar->GetCharacterMovement()->Velocity.Y,
			PlayerChar->GetCharacterMovement()->Velocity.Z));
	}
#endif

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

		//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, CurveTimeline.IsPlaying() ? "True" : "False"); }

		CurveTimeline.PlayFromStart();
	}
}


void UWallRun::EndCameraTilt()
{
	if (CurveFloat) {
		PlayerChar->XRoll = PlayerChar->GetControlRotation().Roll;
		PlayerChar->YPitch = PlayerChar->GetControlRotation().Pitch;
		PlayerChar->ZYaw = PlayerChar->GetControlRotation().Yaw;

		CurveTimeline.ReverseFromEnd();
		
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

	if (WallRunningBool) {
		UpdateWallRun();
	}

	CurveTimeline.TickTimeline(DeltaTime);
}

//PRAGMA_ENABLE_OPTIMIZATION