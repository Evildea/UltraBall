// Fill out your copyright notice in the Description page of Project Settings.

#include "InformationZone.h"
#include "Components/BoxComponent.h"
#include "Ball.h"

// Sets default values
AInformationZone::AInformationZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup the Collision Zone
	TipZone = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	TipZone->OnComponentBeginOverlap.AddDynamic(this, &AInformationZone::OnOverlapBegin);
	RootComponent = TipZone;
}

// Called when the game starts or when spawned
void AInformationZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInformationZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInformationZone::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not yourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ABall* ball = Cast<ABall>(OtherActor);
		if (ball != nullptr)
		{
			//if (!WidgetInstance->GetIsVisible())
			//{
			//	WidgetInstance->AddToViewport();
			//}
		}
	}
}

