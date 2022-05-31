// Fill out your copyright notice in the Description page of Project Settings.

#include "WRC_WallRunBase.h"
#include "WallRunC/WallRunCProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "iostream"

// Sets default values
AWRC_WallRunBase::AWRC_WallRunBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

void AWRC_WallRunBase::TimelineProgress(float Value)
{
	FVector NewLocation = FMath::Lerp(StartLoc, EndLoc, Value);
	SetActorLocation(NewLocation);

	
}

// Called when the game starts or when spawned
void AWRC_WallRunBase::BeginPlay()
{
	Super::BeginPlay();
	
	ResetJump(MaxJumps);

	CharacterMovementComponent->SetPlaneConstraintEnabled(true);

#if 0
	//Timeline demo
	if (CurveFloat) {
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetLooping(true);

		StartLoc = EndLoc = GetActorLocation();
		EndLoc.Z += ZOffset;

		CurveTimeline.PlayFromStart();
	}
#endif

}

void AWRC_WallRunBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AWRC_WallRunBase::InputActionJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AWRC_WallRunBase::StopJumping);

#if 0
	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWallRunCCharacter::OnFire);
#endif

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AWRC_WallRunBase::InputAxisMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWRC_WallRunBase::InputAxisMoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AWRC_WallRunBase::InputAxisMoveForward(float Val)
{
	if (Val != 0.0f) {
		ForwardAxis = Val;
		AddMovementInput(FirstPersonCameraComponent->GetForwardVector(), ForwardAxis);
	}
}

void AWRC_WallRunBase::InputAxisMoveRight(float Val)
{
	if (Val != 0.0f) {
		RightAxis = Val;
		AddMovementInput(FirstPersonCameraComponent->GetRightVector(), RightAxis);
	}
}

void AWRC_WallRunBase::OnLanded(FVector2D Hit)
{
	ResetJump(MaxJumps);
}

void AWRC_WallRunBase::InputActionJump()
{
	if (!WallRunningBool && JumpsLeft <= 0) {
		return;
	}

	if (JumpsLeft > 0) {
		JumpsLeft -= 1;
	}

	LaunchCharacter(FindLaunchVelocity(), false, true);

	if (WallRunningBool) {
		EndWallRun(jumped);
	}
}

void AWRC_WallRunBase::ResetJump(int jumps)
{
	JumpsLeft = std::max(0, MaxJumps);
}

void AWRC_WallRunBase::OnComponentHit(FVector HitImpactNormal)
{
	FRDASVals returnVals;
	if (!WallRunningBool) {
		if (CanSurfaceWallBeRan(HitImpactNormal)) {
			if(CharacterMovementComponent->IsFalling())
			{
				FindRunDirectionAndSide(HitImpactNormal, returnVals);

				WallRunDirection = returnVals.Direction;
				eWallRun = returnVals.Side;

				if (AreRequiredKeysDown()) {
					BeginWallRun();
				}
			}
		}
	}
}

void AWRC_WallRunBase::BeginWallRun()
{
	FVector planeNormal;
	planeNormal.Z = 1.0;
	CharacterMovementComponent->AirControl = 1.0f;
	CharacterMovementComponent->GravityScale = 0.0f;
	CharacterMovementComponent->SetPlaneConstraintNormal(planeNormal);
	WallRunningBool = false;
	BeginCameraTilt();
	UpdateWallRunBool = true;

}

void AWRC_WallRunBase::EndWallRun(StopReason reason)
{
	FVector PlaneNorm;
	PlaneNorm.Z = 0.0;
	MaxJumps -= 1;
	if (reason == jumped) {
		ResetJump(MaxJumps);
	}
	else if (reason == fell) {
		ResetJump(1);
	}
	CharacterMovementComponent->AirControl = 0.05;
	CharacterMovementComponent->GravityScale = 1.0;
	CharacterMovementComponent->SetPlaneConstraintNormal(PlaneNorm);
	WallRunningBool = false;

	EndCameraTilt();

	UpdateWallRunBool = false;
}

void AWRC_WallRunBase::BeginCameraTilt()
{
}

void AWRC_WallRunBase::EndCameraTilt()
{
}

void AWRC_WallRunBase::FindRunDirectionAndSide(FVector WallNormal, FRDASVals& returnVals) const
{
	WallRunSide localSide;
	FVector localVector;


	if (FVector::DotProduct(WallNormal, GetActorRightVector()) > 0) {
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

bool AWRC_WallRunBase::CanSurfaceWallBeRan(FVector SurfaceNormal) const
{
	FVector NormalizedSNVector;
	float SurfaceAngleCheck;
	float MaxFloorAngle;
	if (SurfaceNormal.Z < -0.05) {
		return false;
	}
	if (SurfaceNormal.FVector::Normalize(0.0001)) {
		NormalizedSNVector = SurfaceNormal;
	}
	else {
		NormalizedSNVector.X = 0.0;
		NormalizedSNVector.Y = 0.0;
		NormalizedSNVector.Z = 0.0;
	}

	SurfaceAngleCheck = (acos(FVector::DotProduct(NormalizedSNVector, SurfaceNormal)) * 180) / 3.14159;
	MaxFloorAngle = CharacterMovementComponent->GetWalkableFloorAngle();
	if (SurfaceAngleCheck < MaxFloorAngle) {
		return true;
	}
	return false;
	
}

FVector AWRC_WallRunBase::FindLaunchVelocity() const
{
	FVector LaunchDirection;
	
	if (WallRunningBool) {
		FVector WallRunSideLocalZ;
		if (eWallRun == left) {
			WallRunSideLocalZ.Z = 1.0;
		}
		else {
			WallRunSideLocalZ.Z = -1.0;
		}

		LaunchDirection = FVector::CrossProduct(WallRunDirection, WallRunSideLocalZ);
	}
	else if (CharacterMovementComponent->IsFalling())
	{
		LaunchDirection = (GetActorRightVector() * RightAxis) + (GetActorForwardVector() * ForwardAxis);
	}

	LaunchDirection.Z += 1;
	LaunchDirection *= CharacterMovementComponent->JumpZVelocity;
	return LaunchDirection;
}

bool AWRC_WallRunBase::AreRequiredKeysDown() const
{
	if (ForwardAxis < 0.1) {
		return false;
	}
	
	if (eWallRun == left) {
		return RightAxis > 0.1;
	}
	else {
		return RightAxis < -0.1;
	}
}

FVector2D AWRC_WallRunBase::GetHorizontalVelocity() const
{
	return FVector2D(CharacterMovementComponent->Velocity);
}

void AWRC_WallRunBase::SetHorizontalVelocity(FVector2D HorizontalVelocity)
{
	CharacterMovementComponent->Velocity.X = HorizontalVelocity.X;
	CharacterMovementComponent->Velocity.Y = HorizontalVelocity.Y;
}

void AWRC_WallRunBase::UpdateWallRun()
{
	//Local variables
	FHitResult OutHit;
	FVector TraceEnd;
	FCollisionQueryParams CollisionParams;
	FRDASVals FRDASVals;

	if (!AreRequiredKeysDown()) {
		EndWallRun(fell);
	}
	
	

	FVector WallRunSideLocalZ;
	if (eWallRun == left) {
		WallRunSideLocalZ.Z = -1.0;
	}
	else {
		WallRunSideLocalZ.Z = 1.0;
	}

	TraceEnd = GetActorLocation() + (FVector::CrossProduct(WallRunDirection, WallRunSideLocalZ) * 200);

	if (!ActorLineTraceSingle(OutHit, GetActorLocation(), TraceEnd, ECC_WorldStatic, CollisionParams)) {
		EndWallRun(fell);
	}

	FindRunDirectionAndSide(OutHit.ImpactNormal, FRDASVals);

	if (!FRDASVals.Side == eWallRun) {
		EndWallRun(fell);
	}

	WallRunDirection = FRDASVals.Direction;
	
	CharacterMovementComponent->Velocity.X = WallRunDirection.X * CharacterMovementComponent->GetMaxSpeed();
	CharacterMovementComponent->Velocity.Y = WallRunDirection.Y * CharacterMovementComponent->GetMaxSpeed();
	CharacterMovementComponent->Velocity.Z = 0.0;
}

void AWRC_WallRunBase::ClampHorizontalVelocity()
{
	if (CharacterMovementComponent->IsFalling()) {
		FVector2D CharVelocity;
		float CharTime;
		
		CharVelocity = GetHorizontalVelocity();
		CharTime = CharVelocity.Size() / CharacterMovementComponent->GetMaxSpeed();

		if (CharTime > 1.0) {
			SetHorizontalVelocity(GetHorizontalVelocity()/CharTime);
		}
	}
}

// Called every frame
void AWRC_WallRunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ClampHorizontalVelocity();
	
	if (UpdateWallRunBool) {
		UpdateWallRun();
	}
		
}

#if 0
// Called to bind functionality to input
void AWRC_WallRunBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
#endif

