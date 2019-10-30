// Fill out your copyright notice in the Description page of Project Settings.

#include "LaunchZone.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/ArrowComponent.h"
#include "Ball.h"

// Sets default values
ALaunchZone::ALaunchZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Collision Sphere Component.
	LaunchZone = CreateDefaultSubobject<USphereComponent>("LaunchZone");
	LaunchZone->SetRelativeScale3D(FVector(3.25f));
	LaunchZone->OnComponentBeginOverlap.AddDynamic(this, &ALaunchZone::OnOverlapBegin);
	RootComponent = LaunchZone;

	// Setup Particle System.
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("ParticleSystem");
	ParticleSystem->SetRelativeScale3D(FVector(0.25f));
	ParticleSystem->SetupAttachment(RootComponent);

	// Link Particle System to its respective Particle Spawner.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(TEXT("ParticleSystem'/Game/ParticleSystems/Pt_Launchzone.Pt_Launchzone'"));
	if (Particle.Succeeded())
		ParticleSystem->SetTemplate(Particle.Object);

	// Setup Arrow Component.
	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetRelativeLocation(FVector(40.0f, 0.0f, 0.0f));
	Arrow->SetHiddenInGame(false);
	Arrow->SetArrowColor(FColor(241,248,7,255));
	Arrow->SetupAttachment(LaunchZone);

	EjectPower = 30.0f;

}

// Called when the game starts or when spawned
void ALaunchZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALaunchZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALaunchZone::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ABall* ball = Cast<ABall>(OtherActor);
		ball->ZoneEnter(GetActorLocation(), Arrow->GetComponentLocation(), EjectPower, 1);
	}
}

