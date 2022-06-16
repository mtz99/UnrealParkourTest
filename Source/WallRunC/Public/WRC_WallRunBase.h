// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WRC_WallRunBase.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UCurveFloat;


#define OnWall(execute), (true, false)


UCLASS()
class WALLRUNC_API AWRC_WallRunBase : public ACharacter
{
	GENERATED_BODY()

	/** Capsule Component: Used for collision*/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		UCapsuleComponent* CapsuleComp;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USceneComponent* FP_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FirstPersonCameraComponent;

	/** Character movement component **/
	UCharacterMovementComponent* CharacterMovementComponent;




public:
	// Sets default values for this character's properties
	AWRC_WallRunBase(const FObjectInitializer& ObjectInitalizer);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AWallRunCProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		uint8 bUsingMotionControllers : 1;




	FVector WallRunDirection;

	bool WallRunningBool = false;

	bool UpdateWallRunBool = false;

	int JumpsLeft = 0;

	int MaxJumps = 2;

	float RightAxis = 0.0;

	float ForwardAxis = 0.0;

	enum WallRunSide{left, right};

	enum StopReason {fell, jumped};

	WallRunSide eWallRun = left;


	//Blueprint timeline in C++
	UFUNCTION()
	void TimelineProgress(float Value);
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	//Blueprint timeline in C++
	FTimeline CurveTimeline;
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

	UPROPERTY()
	FVector StartLoc;
	UPROPERTY()
	FVector EndLoc;
	UPROPERTY()
	float ZOffset;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if 0
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#endif
	
	virtual void InputAxisMoveForward(float Val);

	virtual void InputAxisMoveRight(float Val);

	virtual void Landed
	(
		const FHitResult& Hit
	);

	virtual void InputActionJump();

	virtual void ResetJump(int jumps);

	UFUNCTION()
	virtual void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void BeginWallRun();

	virtual void EndWallRun(StopReason reason);

	virtual void BeginCameraTilt();

	virtual void EndCameraTilt();



	struct FRDASVals {
		FVector Direction;
		WallRunSide Side;
	};
	
	virtual void FindRunDirectionAndSide(FVector WallNormal, FRDASVals& returnVals) const;

	virtual bool CanSurfaceWallBeRan(FVector SurfaceNormal) const;

	virtual void Normalize(FVector Input, float Tolerance, FVector& Output) const;

	virtual FVector FindLaunchVelocity() const;
	
	virtual bool AreRequiredKeysDown() const;

	virtual FVector2D GetHorizontalVelocity() const;

	virtual void SetHorizontalVelocity(FVector2D HorizontalVelocity);

	virtual void UpdateWallRun();

	virtual void ClampHorizontalVelocity();




	

	
	
};
