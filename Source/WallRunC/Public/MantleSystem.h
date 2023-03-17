// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "UObject/Interface.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MantleSystem.generated.h"


UINTERFACE(MinimalAPI)
//This class is used to get the unreal markup for interfaces
class UGrabbableInterface : public UInterface
{
	GENERATED_BODY()
};
//This is the implementation of the interface.
class WALLRUNC_API IGrabbableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
		TArray<FTransform> GetSockets() const;

};





UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable )
class WALLRUNC_API UMantleSystem : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Mantle Properties")
	class AWRC_WallRunBase* PlayerChar;

	

public:	
	// Sets default values for this component's properties
	UMantleSystem();

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius = 500.0f;
	float MaxDistance = 300.0f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool LedgeCheck();

	void CharMovementSwitch(bool CharState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Mantle Functions")
	void MoveChar();

	void MoveChar_Implementation();
	
};
