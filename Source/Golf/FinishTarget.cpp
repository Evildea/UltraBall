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

	//Setup the Scene Component
	Base = CreateDefaultSubobject<USceneComponent>("Base");
	RootComponent = Base;

	// Setup the static mesh for Outer Ring of the UltraBall
	UltraBallOuter = CreateDefaultSubobject<UStaticMeshComponent>("UltraBallOuter");
	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallComplex(TEXT("StaticMesh'/Game/Models/UltraBallC.UltraBallC'"));
	UStaticMesh* ComplexAsset = UltraBallComplex.Object;
	UltraBallOuter->SetStaticMesh(ComplexAsset);
	UltraBallOuter->SetSimulatePhysics(true);
	UltraBallOuter->SetRelativeScale3D(FVector(3.0f));
	UltraBallOuter->SetNotifyRigidBodyCollision(true);
	UltraBallOuter->OnComponentHit.AddDynamic(this, &AFinishTarget::OnHit);
	UltraBallOuter->SetupAttachment(Base);

	// Setup the static mesh for the Inner Ring of the UltraBall
	UltraBallInner = CreateDefaultSubobject<UStaticMeshComponent>("UltraBallInner");
	UltraBallInner->SetStaticMesh(ComplexAsset);
	UltraBallInner->SetSimulatePhysics(false);
	UltraBallInner->SetRelativeScale3D(FVector(2.0f));
	UltraBallInner->SetNotifyRigidBodyCollision(false);
	UltraBallInner->SetCollisionProfileName(FName("NoCollision"));
	UltraBallInner->SetupAttachment(Base);

	// Apply Dynamic Material to UltraBall
	ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/Materials/Wireframe.Wireframe'"));
	if (Material.Succeeded())
	{
		UltraBallOuter->SetMaterial(0, Material.Object);
		UltraBallInner->SetMaterial(0, Material.Object);
	}

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

	// Cause the Inner Ring to Constantly Rotate.
	FRotator Rotation = UltraBallInner->GetComponentRotation();
	Rotation.Yaw += 50.0f * DeltaTime;
	Rotation.Roll += 50.0f * DeltaTime;
	UltraBallInner->SetRelativeRotation(Rotation);

	// Update the Inner Ring's position to match the Outer Rings position.
	UltraBallInner->SetWorldLocation(UltraBallOuter->GetComponentLocation());
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

