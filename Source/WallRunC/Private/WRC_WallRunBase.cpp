// Fill out your copyright notice in the Description page of Project Settings.

#include "WRC_WallRunBase.h"

#include "WallRunC/WallRunCProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"

#include "WallRunC/Public/WallRun.h"
#include "WallRunC/Public/MantleSystem.h"

#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "iostream"







//PRAGMA_DISABLE_OPTIMIZATION
// Sets default values
AWRC_WallRunBase::AWRC_WallRunBase(const FObjectInitializer& ObjectInitalizer)
	:Super(ObjectInitalizer.SetDefaultSubobjectClass<UCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AWRC_WallRunBase::OnComponentHit);
	

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// SceneComponent for the camera rotation
	CameraRotateLayer = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRotationLayer"));
	CameraRotateLayer->SetupAttachment(GetCapsuleComponent());

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(CameraRotateLayer);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	//Create a wall run component
	WallRunComp = CreateDefaultSubobject<UWallRun>(TEXT("WallRunComponent"));

	//Create ledge ignore array.
	LedgeToIgnore = nullptr;

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

	
}



void AWRC_WallRunBase::ResetPlayerCRotation()
{
	CameraRotateLayer->SetRelativeRotation(FQuat::Identity);
}

// Called when the game starts or when spawned
void AWRC_WallRunBase::BeginPlay()
{
	Super::BeginPlay();
	
	ResetJump(MaxJumps);

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	if (MantleComp == nullptr) {
		MantleComp = NewObject<UMantleSystem>(this);
		MantleComp->RegisterComponent();
		AddOwnedComponent(MantleComp);
	}
	
	

	// Set the original player controller rotation
	PlayerCOriginalRotation.Roll = GetControlRotation().Roll;
	PlayerCOriginalRotation.Pitch = GetControlRotation().Pitch;
	PlayerCOriginalRotation.Yaw = GetControlRotation().Yaw;
	
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

void AWRC_WallRunBase::OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//Character decides what state you're in.
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		FVector ImpactNormal = Hit.ImpactNormal;

		WallRunComp->SetEWallRun(ImpactNormal);
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, ImpactNormal.ToString()); }
		if (CheckWallRun(ImpactNormal) && changeState(EPlayerState::STATE_WALLRUN)) {
			WallRunComp->BeginWallRun();
		}

		/*//For now also do a mantle check after jumping, but consider decoupling this check from this function.
		if (CheckMantle() && changeState(EPlayerState::STATE_MANTLE)) {
			MantleComp->MoveChar();
		}*/
	}
}

bool AWRC_WallRunBase::CheckWallRun(FVector ImpactNormal)
{
	if (WallRunComp->CanSurfaceWallBeRan(ImpactNormal)) {
		if (GetCharacterMovement()->IsFalling())
		{	
			if (WallRunComp->AreRequiredKeysDown()) {
				return true;
			}
		}
	}
	return false;
}

bool AWRC_WallRunBase::CheckMantle()
{
	return MantleComp->LedgeCheck();
}

void AWRC_WallRunBase::SetIdle()
{
	changeState(EPlayerState::STATE_IDLE);
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

void AWRC_WallRunBase::Falling()
{
	changeState(EPlayerState::STATE_NOJUMPSLEFT);
}

void AWRC_WallRunBase::Landed(const FHitResult& Hit)
{
	ResetJump(MaxJumps);
	changeState(EPlayerState::STATE_IDLE);
}

void AWRC_WallRunBase::InputActionJump()
{
	if (changeState(EPlayerState::STATE_JUMPONCE)||changeState(EPlayerState::STATE_DOUBLEJUMP)) {
		JumpsLeft -= 1;
		LaunchCharacter(FindLaunchVelocity(), false, true);
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Forward %f"), ForwardAxis)); }
	}

	//Make sure the movechar isn't called twice!!!
	if (CheckMantle() && changeState(EPlayerState::STATE_MANTLE) && LedgeToIgnore == nullptr) {
		MantleComp->MoveChar();
		LedgeToIgnore = MantleComp->ReturnLedge(); //Figure out when to reset this!
	}
}

void AWRC_WallRunBase::EndWallRun(bool FallReason) 
{
	if (FallReason) {
		Falling();
	}
	else {
		ResetJump(MaxJumps - 1);
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
	
#if 0
	if (launchbool) {
		WallRun.FindLaunchVelocity();

		LaunchDirection = FVector::CrossProduct(WallRunDirection, WallRunSideLocalZ);

		else if (GetCharacterMovement()->IsFalling())
		{
			LaunchDirection = (GetActorRightVector() * RightAxis) + (GetActorForwardVector() * ForwardAxis);
		}
	}
#endif

	LaunchDirection.Z += 1;
	LaunchDirection = LaunchDirection * GetCharacterMovement()->JumpZVelocity;
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::SanitizeFloat(LaunchDirection.X)); }
	return LaunchDirection;
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


void AWRC_WallRunBase::ClampHorizontalVelocity()
{
	if (GetCharacterMovement()->IsFalling()) {
		FVector2D CharVelocity;
		float CharTime;
		
		CharVelocity = GetHorizontalVelocity();
		CharTime = CharVelocity.Size() / GetCharacterMovement()->GetMaxSpeed();

		if (CharTime > 1.0) {
			SetHorizontalVelocity(GetHorizontalVelocity()/CharTime);
		}
	}
}

bool AWRC_WallRunBase::changeState(EPlayerState input)
{
	bool changeValid = false;

	switch (input) {
		//If there's no jumps left (or falling).	
		case EPlayerState::STATE_NOJUMPSLEFT:
			changeValid = currentState == EPlayerState::STATE_DOUBLEJUMP || currentState == EPlayerState::STATE_WALLRUN;
			break;
		//Jump from idle, or set the character to "have jumped once" after falling off wall.
		case EPlayerState::STATE_JUMPONCE:
			changeValid = currentState == EPlayerState::STATE_IDLE;
			break;
		//Jump a second time.
		case EPlayerState::STATE_DOUBLEJUMP:
			changeValid = currentState == EPlayerState::STATE_JUMPONCE;
			break;
		//Go to idle state from jumping once, double jump, or wall run, or no jumps left.
		case EPlayerState::STATE_IDLE:
			changeValid = currentState == EPlayerState::STATE_JUMPONCE || currentState == EPlayerState::STATE_DOUBLEJUMP || currentState == EPlayerState::STATE_WALLRUN || currentState == EPlayerState::STATE_NOJUMPSLEFT || currentState == EPlayerState::STATE_MANTLE;
			break;
		//Go to wall run state.
		case EPlayerState::STATE_WALLRUN:
			changeValid = currentState == EPlayerState::STATE_JUMPONCE;
			break;
		//Go to mantle state.
		case EPlayerState::STATE_MANTLE:
			changeValid = currentState == EPlayerState::STATE_JUMPONCE || currentState == EPlayerState::STATE_DOUBLEJUMP || currentState == EPlayerState::STATE_WALLRUN;
			break;
	}
	
	if (changeValid)
	{
		//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString::Printf(TEXT("Current state, %d->, %d"), (int)currentState, (int)input)); }
		currentState = input;
	}
	
	return changeValid;
}

// Called every frame
void AWRC_WallRunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ClampHorizontalVelocity();

}

#if 0
// Called to bind functionality to input
void AWRC_WallRunBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
#endif

//PRAGMA_ENABLE_OPTIMIZATION