// Fill out your copyright notice in the Description page of Project Settings.

#include "MantleSystem.h"
#include <Kismet/KismetSystemLibrary.h>

#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include <WallRunC/Public/WRC_WallRunBase.h>







// Sets default values for this component's properties
UMantleSystem::UMantleSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMantleSystem::BeginPlay()
{
	Super::BeginPlay();

	PlayerChar = Cast<AWRC_WallRunBase>(this->GetOwner());
	
}


// Called every frame
void UMantleSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UMantleSystem::LedgeCheck()
{
	//Local variables
	TArray<FHitResult> OutHits;
	TArray <AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PlayerChar->GetParentActor());

	const bool Hit = UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), PlayerChar->GetActorLocation(), PlayerChar->GetActorLocation(), TraceRadius, "Climbable", false, ActorsToIgnore,
		EDrawDebugTrace::None, OutHits, true);
	if (Hit) {
		TArray<FTransform> Sockets;
		for (auto& OutHit:OutHits) {
			if (OutHit.Actor->Implements<UGrabbableInterface>()) {
				Sockets = IGrabbableInterface::Execute_GetSockets(OutHit.Actor.Get());

				for (auto& Socket:Sockets) {
					float CalcX = Socket.GetLocation().X - PlayerChar->GetActorLocation().X;
					float CalcY = Socket.GetLocation().Y - PlayerChar->GetActorLocation().Y;
					float CalcZ = Socket.GetLocation().Z - PlayerChar->GetActorLocation().Z;
					
					float CalcDistance = sqrt(pow(CalcX,2) + pow(CalcY,2) + pow(CalcZ,2));
					
					if (CalcDistance < MaxDistance) {
						ChosenSocket = Socket;
						
						return true;
					}
				}
			}
		}
	}

	return false;
#if 0
	const bool Hit = UKismetSystemLibrary::SphereTraceSingleByProfile(GetWorld(), PlayerChar->GetActorLocation(), TraceEnd, TraceRadius, SocketNameType, 
		true, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true,
		FLinearColor::Gray, FLinearColor::Blue, 60.0f);
	if (Hit) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Hit ? "True" : "False");
	}

#endif
	


}

AActor* UMantleSystem::ReturnLedge()
{
	//Local variables
	TArray<FHitResult> OutHits;
	AActor* returnValue;
	TArray <AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PlayerChar->GetParentActor());

	const bool Hit = UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), PlayerChar->GetActorLocation(), PlayerChar->GetActorLocation(), TraceRadius, "Climbable", false, ActorsToIgnore,
		EDrawDebugTrace::None, OutHits, true);
	if (Hit) {

		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Ledge return successful!"))); }
		return (returnValue = OutHits[0].GetActor());
	}

	return 0;
}

void UMantleSystem::CharMovementSwitch(bool CharState)
{
	if (CharState == false)
	{
		FVector planeNormal;
		planeNormal.Z = 1.0;
		PlayerChar->GetCharacterMovement()->AirControl = 1.0f;
		PlayerChar->GetCharacterMovement()->GravityScale = 0.0f;
	}
	else
	{
		PlayerChar->ResetJump(0);
		PlayerChar->GetCharacterMovement()->AirControl = 0.05;
		PlayerChar->GetCharacterMovement()->GravityScale = 1.0;

	}
}

void UMantleSystem::MoveChar_Implementation()
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("It's mantlin time!"))); }

	//The following call below isn't needed as the char movement comp is handling the transition, but fix the animation system.
	
	//When moving has finished:
	PlayerChar->SetIdle();
}

