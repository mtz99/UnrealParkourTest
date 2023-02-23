// Fill out your copyright notice in the Description page of Project Settings.

#include <Kismet/KismetSystemLibrary.h>
#include "MantleSystem.h"
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

void UMantleSystem::LedgeCheck(UStaticMeshComponent GrabbableSurface)
{
	//Local variables
	FHitResult OutHit;
	FVector TraceEnd = PlayerChar->GetActorLocation() + TraceRadius;
	TArray <AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PlayerChar->GetParentActor());


	//Create predefined array of all socket names from grabbable surface and array which will store all sockets.
	const TArray<FName> AllSocketNames = GrabbableSurface.GetAllSocketNames();
	const FName SocketNameType = GrabbableSurface.GetFName();
	
	TArray<const UStaticMeshSocket*> AllSockets;
	AllSockets.SetNum(AllSocketNames.Num());
	
	for (int32 SocketIdx = 0; SocketIdx <= AllSocketNames.Num(); ++SocketIdx)
	{
		AllSockets[SocketIdx] = GrabbableSurface.GetSocketByName(AllSocketNames[SocketIdx]);
	}
	
	
	const bool Hit = UKismetSystemLibrary::SphereTraceSingleByProfile(GetWorld(), PlayerChar->GetActorLocation(), TraceEnd, TraceRadius, SocketNameType, 
		true, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true,
		FLinearColor::Gray, FLinearColor::Blue, 60.0f);
	if (Hit) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Hit ? "True" : "False");
	}
	


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

