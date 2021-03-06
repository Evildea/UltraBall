// Fill out your copyright notice in the Description page of Project Settings.

#include "Bumper.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/SkeletalMeshComponent.h" 
#include "Components/BoxComponent.h" 
#include "Animation/AnimMontage.h"
#include "Components/AudioComponent.h"
#include "Ball.h"

// Sets default values
ABumper::ABumper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Bumper = CreateDefaultSubobject<USkeletalMeshComponent>("Bumper");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BumperMesh(TEXT("SkeletalMesh'/Game/Models/M_Bumper45.M_Bumper45'"));
	if (BumperMesh.Succeeded())
	{
		USkeletalMesh* Asset = BumperMesh.Object;
		Bumper->SetSkeletalMesh(Asset);
		Bumper->SetWorldScale3D(FVector(2.0f));
		Bumper->SetSimulatePhysics(false);
		Bumper->SetMobility(EComponentMobility::Static);
		Bumper->SetCollisionProfileName(FName("BlockAllDynamic"));
		RootComponent = Bumper;
	}

	// Setup Sound Component
	Sound = CreateDefaultSubobject<UAudioComponent>("Sound");
	Sound->SetAutoActivate(false);
	Sound->SetupAttachment(RootComponent);

	Colider = CreateDefaultSubobject<UBoxComponent>("Colider");
	Colider->SetWorldScale3D(FVector(4.1f, 0.7f, 1.1f));
	Colider->SetRelativeLocation(FVector(65.5f, 1.0f, 0.0f));
	Colider->SetWorldRotation(FRotator(0.0f, -90.000183f, 0.0f));
	Colider->OnComponentBeginOverlap.AddDynamic(this, &ABumper::OnOverlapBegin);
	Colider->SetMobility(EComponentMobility::Static);
	Colider->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AnimationObj(TEXT("AnimMontage'/Game/Models/M_Bumper45_Montage.M_Bumper45_Montage'"));
	if (AnimationObj.Succeeded())
		Animation = AnimationObj.Object;

	BouncePower = 2.0f;

}

// Called when the game starts or when spawned
void ABumper::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABumper::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		ABall* ball = Cast<ABall>(OtherActor);

		// Fire UltraBall in the direction of the Bumper.
		if (ball != nullptr)
		{
			OtherComp->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
			OtherComp->AddImpulse(Bumper->GetForwardVector() * OtherComp->GetMass() * BouncePower * 1000.0f);
			ball->BumperHit();
		}

		// Play the Bumper sound.
		if (Sound != nullptr)
		{
			if (!Sound->IsPlaying())
				Sound->Play(0.0f);
		}

		// Play the Bumper Animation.
		Bumper->PlayAnimation(Animation, false);
	}

}

