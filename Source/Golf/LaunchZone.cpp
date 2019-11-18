// Fill out your copyright notice in the Description page of Project Settings.

#include "LaunchZone.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/ArrowComponent.h"
#include "Components/PointLightComponent.h" 
#include "Components/AudioComponent.h"
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
	LaunchZone->SetMobility(EComponentMobility::Static);
	RootComponent = LaunchZone;

	// Setup Sound Component
	Sound = CreateDefaultSubobject<UAudioComponent>("Sound");
	Sound->SetAutoActivate(false);
	Sound->SetupAttachment(RootComponent);

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

	// Setup Point Light
	Pointlight = CreateDefaultSubobject<UPointLightComponent>("light");
	Pointlight->SetMobility(EComponentMobility::Static);
	Pointlight->SetAttenuationRadius(300.0f);
	Pointlight->SetSourceRadius(125.0f);
	Pointlight->SetIntensity(5000.0f);
	Pointlight->SetLightColor(FLinearColor(1.0f, 1.0f, 0.006995f, 1.0f));
	Pointlight->SetupAttachment(RootComponent);

	EjectPower = 2.0f;

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
		
		if (ball != nullptr)
		{
			//ball->ZoneEnter(1, GetActorLocation(), Arrow->GetComponentLocation(), EjectPower);
		}

		if (Sound != nullptr)
		{
			if (!Sound->IsPlaying())
				Sound->Play(0.0f);
		}
	}
}

