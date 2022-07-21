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



PRAGMA_DISABLE_OPTIMIZATION
// Sets default values
AWRC_WallRunBase::AWRC_WallRunBase(const FObjectInitializer& ObjectInitalizer)
	:Super(ObjectInitalizer.SetDefaultSubobjectClass<UCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#if 0
	// Set size for collision capsule
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(55.f, 96.0f);

	CapsuleComp->SetSimulatePhysics(true);
	CapsuleComp->SetNotifyRigidBodyCollision(true);
	CapsuleComp->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	CapsuleComp->OnComponentHit.AddDynamic(this, &AWRC_WallRunBase::OnComponentHit);
#endif

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AWRC_WallRunBase::OnComponentHit);

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


#if 0
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
#endif

	// Set the original player controller rotation
	PlayerCOriginalRotation.Roll = GetControlRotation().Roll;
	PlayerCOriginalRotation.Pitch = GetControlRotation().Pitch;
	PlayerCOriginalRotation.Yaw = GetControlRotation().Yaw;
}

void AWRC_WallRunBase::TimelineProgress(float Value)
{
	float CamRollMultiplier;
	if (eWallRun == left)
		CamRollMultiplier = -1.0;
	else
		CamRollMultiplier = 1.0;


	FRotator NewActorRotation;
	NewActorRotation.Roll = Value * CamRollMultiplier;

	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::SanitizeFloat(NewActorRotation.Roll)); } //Debug for cam rotation.
	NewActorRotation.Pitch = YPitch;
	NewActorRotation.Yaw = ZYaw;

	//Sets the player's controller to new rotation
	if (Controller != nullptr)
		Controller->SetControlRotation(NewActorRotation);
}

void AWRC_WallRunBase::ResetPlayerCRotation()
{
	FOnTimelineEvent clearBind;
	PlayerCOriginalRotation.Pitch = GetControlRotation().Pitch;
	PlayerCOriginalRotation.Yaw = GetControlRotation().Yaw;
	Controller->SetControlRotation(PlayerCOriginalRotation);
	CurveTimeline.SetTimelineFinishedFunc(clearBind);
}

// Called when the game starts or when spawned
void AWRC_WallRunBase::BeginPlay()
{
	Super::BeginPlay();
	
	ResetJump(MaxJumps);

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	WallRunDirection.X = 0.0;
	WallRunDirection.Y = 0.0;
	WallRunDirection.Z = 0.0;
	
	//Create timeline to handle camera tilt.
	FOnTimelineFloat TimelineProgress;
	TimelineProgress.BindDynamic(this, &AWRC_WallRunBase::TimelineProgress);
	CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
	CurveTimeline.SetLooping(false);
	
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
	ForwardAxis = Val;
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Forward %f"), ForwardAxis)); }
	if (Val != 0.0f) {
		AddMovementInput(FirstPersonCameraComponent->GetForwardVector(), ForwardAxis);
	}
}

void AWRC_WallRunBase::InputAxisMoveRight(float Val)
{
	RightAxis = Val;
	
	if (Val != 0.0f) {
		AddMovementInput(FirstPersonCameraComponent->GetRightVector(), RightAxis);
	}
}

void AWRC_WallRunBase::Landed(const FHitResult& Hit)
{
	ResetJump(MaxJumps);
}

void AWRC_WallRunBase::InputActionJump()
{
	if (!WallRunningBool && JumpsLeft <= 0) {
		return;
	}

	if (!WallRunningBool && JumpsLeft > 0) {
		JumpsLeft -= 1;
	}

	LaunchCharacter(FindLaunchVelocity(), false, true);

	if (WallRunningBool) {
		EndWallRun(jumped);
	}

}

void AWRC_WallRunBase::ResetJump(int jumps)
{
	if (jumps > MaxJumps) {
		JumpsLeft = 2;
	}
	else if (jumps <= 0) {
		JumpsLeft = 0;
	}
	else {
		JumpsLeft = jumps;
	}
}

void AWRC_WallRunBase::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		FVector ImpactNormal = Hit.ImpactNormal;
		FRDASVals returnVals;
		if (!WallRunningBool) {
			if (CanSurfaceWallBeRan(ImpactNormal)) {
				if (GetCharacterMovement()->IsFalling())
				{
					FindRunDirectionAndSide(ImpactNormal, returnVals);


					//Don't forget to reset these values back to original state once you land on the ground.

					
 					WallRunDirection = returnVals.Direction;
					eWallRun = returnVals.Side;

					if (AreRequiredKeysDown()) {
						BeginWallRun();
					}
				}
			}
		}
	}
}

void AWRC_WallRunBase::BeginWallRun()
{
	FVector planeNormal;
	planeNormal.Z = 1.0;
	GetCharacterMovement()->AirControl = 1.0f;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->SetPlaneConstraintNormal(planeNormal);
	WallRunningBool = true;
	BeginCameraTilt();
	UpdateWallRunBool = true;

}

void AWRC_WallRunBase::EndWallRun(StopReason reason)
{
	FVector PlaneNorm;
	PlaneNorm.X = 0.0;
	PlaneNorm.Y = 0.0;
	PlaneNorm.Z = 0.0;
	if (reason == fell) {
		ResetJump(1);
	}
	else {
		ResetJump(MaxJumps - 1);
	}
	GetCharacterMovement()->AirControl = 0.05;
	GetCharacterMovement()->GravityScale = 1.0;
	GetCharacterMovement()->SetPlaneConstraintNormal(PlaneNorm);
	WallRunningBool = false;

	EndCameraTilt();

	UpdateWallRunBool = false;

	//Binded function to reset player rotation
	FOnTimelineEvent ResetDelegate;
	ResetDelegate.BindDynamic(this, &AWRC_WallRunBase::ResetPlayerCRotation);

	CurveTimeline.SetTimelineFinishedFunc(ResetDelegate);
}





void AWRC_WallRunBase::BeginCameraTilt()
{
	if (CurveFloat) {
		XRoll = GetControlRotation().Roll;
		YPitch = GetControlRotation().Pitch;
		ZYaw = GetControlRotation().Yaw;

		CurveTimeline.PlayFromStart();
	}
}


void AWRC_WallRunBase::EndCameraTilt()
{
	if (CurveFloat) {
		XRoll = GetControlRotation().Roll;
		YPitch = GetControlRotation().Pitch;
		ZYaw = GetControlRotation().Yaw;

		CurveTimeline.Reverse(); 
		//After reversing rotation, send a signal to say it's done and then manually set rotation back to inital vals.
	}
}

void AWRC_WallRunBase::FindRunDirectionAndSide(FVector WallNormal, FRDASVals& returnVals) const
{
	WallRunSide localSide;
	FVector localVector;
	FVector rightVector = GetActorRightVector();

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

bool AWRC_WallRunBase::CanSurfaceWallBeRan(FVector SurfaceNormal) const
{
	float SurfaceAngleCheck;
	if (SurfaceNormal.Z < -0.05) {
		return false;
	}

	FVector NormalizedSNVector = SurfaceNormal.GetSafeNormal2D(0.0001);

#if 0 //Old surface normal code
	if (SurfaceNormal.FVector::Normalize(0.0001)) {
		NormalizedSNVector = SurfaceNormal;
	}
	else {
		NormalizedSNVector.X = 0.0;
		NormalizedSNVector.Y = 0.0;
		NormalizedSNVector.Z = 0.0;
	}
#endif


	SurfaceAngleCheck = FMath::RadiansToDegrees(FMath::Acos(NormalizedSNVector.DotProduct(NormalizedSNVector, SurfaceNormal)));
	float MaxFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	if (SurfaceAngleCheck < MaxFloorAngle) {
		return true;
	}
	return false;
	
}

void AWRC_WallRunBase::Normalize(FVector Input, float Tolerance, FVector& Output) const 
{
	Input.GetSafeNormal(Tolerance);
	Input.Normalize(Tolerance);
	Output = Input;
}


FVector AWRC_WallRunBase::FindLaunchVelocity() const
{
	FVector LaunchDirection;
	LaunchDirection.X = 0.0;
	LaunchDirection.Y = 0.0;
	LaunchDirection.Z = 0.0;
	
	if (WallRunningBool) {
		FVector WallRunSideLocalZ;
		
		WallRunSideLocalZ.X = 0.0;
		WallRunSideLocalZ.Y = 0.0;
		
		if (eWallRun == left) {
			WallRunSideLocalZ.Z = 1.0;
		}
		else {
			WallRunSideLocalZ.Z = -1.0;
		}

		LaunchDirection = FVector::CrossProduct(WallRunDirection, WallRunSideLocalZ);
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		LaunchDirection = (GetActorRightVector() * RightAxis) + (GetActorForwardVector() * ForwardAxis);
	}

	LaunchDirection.Z += 1;
	LaunchDirection = LaunchDirection * GetCharacterMovement()->JumpZVelocity;
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::SanitizeFloat(LaunchDirection.X)); }
	return LaunchDirection;
}

bool AWRC_WallRunBase::AreRequiredKeysDown() const
{
	if (ForwardAxis <= 0.1) {
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
	return FVector2D(GetCharacterMovement()->Velocity);
}

void AWRC_WallRunBase::SetHorizontalVelocity(FVector2D HorizontalVelocity)
{
	GetCharacterMovement()->Velocity.X = HorizontalVelocity.X;
	GetCharacterMovement()->Velocity.Y = HorizontalVelocity.Y;
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
		return;
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
		return;
	}

	FindRunDirectionAndSide(OutHit.ImpactNormal, FRDASVals);

	if (!FRDASVals.Side == eWallRun) {
		EndWallRun(fell);
		return;
	}

	WallRunDirection = FRDASVals.Direction;
	
	GetCharacterMovement()->Velocity.X = WallRunDirection.X * GetCharacterMovement()->GetMaxSpeed();
	GetCharacterMovement()->Velocity.Y = WallRunDirection.Y * GetCharacterMovement()->GetMaxSpeed();
	GetCharacterMovement()->Velocity.Z = 0.0;
}

void AWRC_WallRunBase::ClampHorizontalVelocity()
{
	if (GetCharacterMovement()->IsFalling()) {
		FVector2D CharVelocity;
		float CharTime;
		
		CharVelocity = GetHorizontalVelocity();
		CharTime = CharVelocity.Size() / GetCharacterMovement()->GetModifiedMaxSpeed();

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

	CurveTimeline.TickTimeline(DeltaTime);
		
}

#if 0
// Called to bind functionality to input
void AWRC_WallRunBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
#endif

PRAGMA_ENABLE_OPTIMIZATION