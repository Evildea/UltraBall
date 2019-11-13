// Fill out your copyright notice in the Description page of Project Settings.

#include "FinishTarget.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h" 
#include "Ball.h"

// Sets default values
AFinishTarget::AFinishTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for UltraBall
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");
	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallSimple(TEXT("StaticMesh'/Game/Models/UltraBallS.UltraBallS'"));
	UStaticMesh* SimpleAsset = UltraBallSimple.Object;
	UltraBall->SetStaticMesh(SimpleAsset);
	UltraBall->SetSimulatePhysics(true);
	UltraBall->SetRelativeScale3D(FVector(3.0f));
	UltraBall->SetNotifyRigidBodyCollision(true);
	UltraBall->OnComponentHit.AddDynamic(this, &AFinishTarget::OnHit);
	RootComponent = UltraBall;

	// Apply Dynamic Material to UltraBall
	ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("MaterialInstanceConstant'/Game/Materials/UltraBall_MI.UltraBall_MI'"));
	if (Material.Succeeded())
		UltraBall->SetMaterial(0, Material.Object);

}

// Called when the game starts or when spawned
void AFinishTarget::BeginPlay()
{
	Super::BeginPlay();
	HasFinishedLevel = false;
}

// Called every frame
void AFinishTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFinishTarget::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ABall* ball = Cast<ABall>(OtherActor);

		if (ball != nullptr)
		{
			HasFinishedLevel = true;
		}
	}
}

bool AFinishTarget::GetHasFinishedLevel()
{
	return HasFinishedLevel;
}

FName AFinishTarget::GetNextLevel()
{
	return NextLevel;
}

