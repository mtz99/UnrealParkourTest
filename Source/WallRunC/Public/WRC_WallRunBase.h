// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

	

	UPROPERTY()
		float XRoll;
	UPROPERTY()
		float YPitch;
	UPROPERTY()
		float ZYaw;
	

	int JumpsLeft = 0;

	int MaxJumps = 2;

	float RightAxis = 0.0;

	float ForwardAxis = 0.0;

	

	//Player controller's original rotation
	FRotator PlayerCOriginalRotation;

	UFUNCTION()
	void ResetPlayerCRotation();


private:
	
	enum PlayerState {
		STATE_IDLE,
		STATE_JUMPONCE,
		STATE_DOUBLEJUMP,
		STATE_WALLRUN,
		STATE_NOJUMPSLEFT
	};

	PlayerState currentState = STATE_IDLE;

	UPROPERTY(EditAnywhere, Category = "WallRunComp")
	class UWallRun* WallRunComp;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	

	

public:	
	
	
	
	bool changeState(PlayerState input);
	void Update(ACharacter& player);
	
	
	
	// Called every frame
	void Tick(float DeltaTime) override;

#if 0
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#endif
	

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	bool CheckWallRun(FVector ImpactNormal);
	
	void InputAxisMoveForward(float Val);

	void InputAxisMoveRight(float Val);

	void Falling();

	void Landed
	(
		const FHitResult& Hit
	);

	void InputActionJump();

	void ResetJump(int jumps);

	void EndWallRun(bool FallReason);
	

	void Normalize(FVector Input, float Tolerance, FVector& Output) const;

	FVector FindLaunchVelocity() const;


	FVector2D GetHorizontalVelocity() const;

	void SetHorizontalVelocity(FVector2D HorizontalVelocity);

	void ClampHorizontalVelocity();
	
};
