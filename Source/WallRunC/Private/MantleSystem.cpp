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
	TArray<FHitResult> OutHit;
	TArray <AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PlayerChar->GetParentActor());

	//Add profile name to this!!!
	const bool Hit = UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(), PlayerChar->GetActorLocation(), PlayerChar->GetActorLocation(), TraceRadius, " ", false, ActorsToIgnore,
		EDrawDebugTrace::None, OutHit, true);
	if (Hit) {
		TArray<FTransform> Sockets;
		for (int i = 0; i < OutHit.Num(); i++) {
			if (OutHit[i].Actor->Implements<UGrabbableInterface>()) {
				Sockets = IGrabbableInterface::Execute_GetSockets(OutHit[i].Actor.Get());

				for (int j = 0; j <= Sockets.Num(); j++) {
					float CalcDistance = Sockets[j].GetLocation().Size() - PlayerChar->GetActorLocation().Size();
					if (CalcDistance < MaxDistance) {
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

void UMantleSystem::MoveChar()
{

}

