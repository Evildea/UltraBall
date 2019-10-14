// Fill out your copyright notice in the Description page of Project Settings.

#include "Bumper.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h" 

// Sets default values
ABumper::ABumper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Bumper = CreateDefaultSubobject<UStaticMeshComponent>("Bumper");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BumperMesh(TEXT("StaticMesh'/Game/Models/Bumper.Bumper'"));
	if (BumperMesh.Succeeded())
	{
		UStaticMesh* Asset = BumperMesh.Object;
		Bumper->SetStaticMesh(Asset);
		Bumper->SetSimulatePhysics(false);
		Bumper->SetWorldScale3D(FVector(2.0f));
		RootComponent = Bumper;
	}

	Colider = CreateAbstractDefaultSubobject<UBoxComponent>("Colider");
	Colider->SetWorldScale3D(FVector(3.75f, 0.25f, 1.0f));
	Colider->SetRelativeLocation(FVector(70.5f, -0.5f, 0.0f));
	Colider->SetWorldRotation(FRotator(0.0f, -90.000183f, 0.0f));
	Colider->SetupAttachment(RootComponent);
	Colider->OnComponentBeginOverlap.AddDynamic(this, &ABumper::OnOverlapBegin);
	
	BouncePower = 30.0f;

}

// Called when the game starts or when spawned
void ABumper::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABumper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABumper::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Detected"));
		OtherComp->AddForce(Bumper->GetForwardVector() * OtherComp->GetMass() * BouncePower * 10000.0f);
	}

}

