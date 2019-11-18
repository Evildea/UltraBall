// Fill out your copyright notice in the Description page of Project Settings.

#include "DeadZone.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/PointLightComponent.h" 
#include "Components/AudioComponent.h"
#include "Ball.h"

// Sets default values
ADeadZone::ADeadZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Collision Sphere Component.
	DeadZone = CreateDefaultSubobject<USphereComponent>("DeadZone");
	DeadZone->SetRelativeScale3D(FVector(3.25f));
	DeadZone->OnComponentBeginOverlap.AddDynamic(this, &ADeadZone::OnOverlapBegin);
	DeadZone->SetMobility(EComponentMobility::Static);
	RootComponent = DeadZone;

	// Setup Sound Component
	Sound = CreateDefaultSubobject<UAudioComponent>("Sound");
	Sound->SetAutoActivate(false);
	Sound->SetupAttachment(RootComponent);

	// Setup Particle System.
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("ParticleSystem");
	ParticleSystem->SetRelativeScale3D(FVector(0.25f));
	ParticleSystem->SetupAttachment(RootComponent);

	// Link Particle System to its respective Particle Spawner.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(TEXT("ParticleSystem'/Game/ParticleSystems/Pt_Deadzone.Pt_Deadzone'"));
	if (Particle.Succeeded())
		ParticleSystem->SetTemplate(Particle.Object);

	// Setup Point Light
	Pointlight = CreateDefaultSubobject<UPointLightComponent>("light");
	Pointlight->SetMobility(EComponentMobility::Static);
	Pointlight->SetAttenuationRadius(300.0f);
	Pointlight->SetSourceRadius(125.0f);
	Pointlight->SetIntensity(5000.0f);
	Pointlight->SetLightColor(FLinearColor(0.462077f, 0.158961f, 0.806952f, 1.0f));
	Pointlight->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ADeadZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADeadZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADeadZone::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ABall* ball = Cast<ABall>(OtherActor);

		if (ball != nullptr)
		{
			//ball->ZoneEnter(0, GetActorLocation(), FVector(0.0f), 0);
		}

		if (Sound != nullptr)
		{
			if (!Sound->IsPlaying())
				Sound->Play(0.0f);
		}
	}
}

