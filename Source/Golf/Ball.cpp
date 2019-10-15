// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h" 
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h" 

#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values
ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for ball
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DodecaMesh(TEXT("StaticMesh'/Game/Models/Dodeca.Dodeca'"));
	if (DodecaMesh.Succeeded())
	{
		UStaticMesh* Asset = DodecaMesh.Object;
		UltraBall->SetStaticMesh(Asset);
		UltraBall->SetSimulatePhysics(true);
		UltraBall->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));
		RootComponent = UltraBall;
	}

	// Setup Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("springarm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bAbsoluteRotation = true;

	// Setup Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("camera");
	Camera->SetupAttachment(SpringArm);

	// Set up inital values that can be modified by the Designer.
	MinZoomInLength = 200.0f;
	MaxZoomOutLength = 1000.0f;
	ZoomInSpeed = 93.0f;

	// Set up initial values that can't be modified by the Designer.
	ZoomLength = 0.0f;
	Power = 0.0f;
	MaxPowerPossibleAtFullChargeUp = 1.0f;
	TimeNeededToReachFullChargeUp = 1.0f;
	ChargeUpTimePassed = 0.0f;
	CurrentBallState = Idle;
	MaxNumberOfShotsAllowedInTheAir = 1;
	NumberOfAirShotsTaken = 0;
	MaxDistanceOffGroundConsideredAir = 100.0f;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	inTheAir = false;

	// Update all Components based on their inital values.
	UpdateComponents();

	// Tell the game controller to possess this player.
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Charge Up the Power if the Left Mouse is down and the Charge Up hasn't been cancelled.
	if (CurrentBallState == Charging)
	{
		// Charge up the Power.
		ChargeUpTimePassed += DeltaTime;

		Power = (MaxPowerPossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * ChargeUpTimePassed;

		if (Power > MaxPowerPossibleAtFullChargeUp)
			Power = MaxPowerPossibleAtFullChargeUp;
	}
	
	// This whether the ball is currently in the air.
	Start = GetActorLocation();
	End = GetActorLocation();
	End.Z -= MaxDistanceOffGroundConsideredAir;
	GetWorld()->LineTraceSingleByChannel(Result, Start, End, CollisionChannel, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam);

	if (Result.GetActor() != NULL)
	{
		inTheAir = false;
		NumberOfAirShotsTaken = 0;
	}
	else
		inTheAir = true;

	// DEBUG MESSAGES
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Charge Power is %f"), Power));
	if (inTheAir)
	{
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("In the air")));
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Blue, FString::Printf(TEXT("Remaining In-air Shots: %d"), MaxNumberOfShotsAllowedInTheAir - NumberOfAirShotsTaken));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("On the ground")));
		GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Blue, FString::Printf(TEXT("In-air shots irrelevant")));
	}

	switch (CurrentBallState)
	{
	case ABall::Idle:
		GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::White, FString::Printf(TEXT("Idle")));
		break;
	case ABall::Charging:
		GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::White, FString::Printf(TEXT("Charging")));
		break;
	case ABall::CancelCharging:
		GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::White, FString::Printf(TEXT("CancelCharging")));
		break;
	default:
		break;
	}

}

// Called to bind functionality to input
void ABall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Start Charge Up and End Charge Up for firing.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABall::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABall::EndFire);

	PlayerInputComponent->BindAction("CancelFire", IE_Pressed, this, &ABall::CancelFire);

	PlayerInputComponent->BindAxis("LookUp", this, &ABall::LookUp);
	PlayerInputComponent->BindAxis("LookLeft", this, &ABall::LookLeft);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ABall::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ABall::ZoomOut);

}

void ABall::ZoomIn()
{
	ZoomLength -= (ZoomLength / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::ZoomOut()
{
	ZoomLength += (ZoomLength / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::Fire()
{
	// If a charge is cancelled.
	if (CurrentBallState == CancelCharging)
	{
		CurrentBallState = Idle;
		ChargeUpTimePassed = 0.0f;
		Power = 0.0f;
		if (inTheAir)
			NumberOfAirShotsTaken--;
		return;
	}

	// If the ball is Idle then start charging the ball.
	if (CurrentBallState == Idle && NumberOfAirShotsTaken < MaxNumberOfShotsAllowedInTheAir)
	{
		CurrentBallState = Charging;
		ChargeUpTimePassed = 0.0f;
		if (inTheAir)
			NumberOfAirShotsTaken++;
		return;
	}
}

void ABall::EndFire()
{
	// If the ball is charging then fire the ball.
	if (CurrentBallState == Charging)
	{
		FVector offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		offset = offset.GetSafeNormal(1.0f) * Power * 10000000.0f;
		UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
		UltraBall->AddForce(offset);
		UltraBall->SetEnableGravity(true);

		ChargeUpTimePassed = 0.0f;
		Power = 0.0f;
		CurrentBallState = Idle;
	}
}

void ABall::CancelFire()
{
	if (CurrentBallState != Idle)
		CurrentBallState = CancelCharging;
}

void ABall::LookUp(float value)
{
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	if (cameraRotation.Pitch < -70)
		cameraRotation.Pitch = -70.0f;
	if (cameraRotation.Pitch > 36)
		cameraRotation.Pitch = 36.0f;

	cameraRotation.Pitch += value;
	SpringArm->SetWorldRotation(cameraRotation);

}

void ABall::LookLeft(float value)
{
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	cameraRotation.Yaw += value;
	SpringArm->SetWorldRotation(cameraRotation);
}

void ABall::DeadZoneFreeze()
{
	UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
	UltraBall->SetEnableGravity(false);
	NumberOfAirShotsTaken = 0;
}

void ABall::UpdateComponents()
{
	if (ZoomLength < MinZoomInLength) { ZoomLength = MinZoomInLength; }
	if (ZoomLength > MaxZoomOutLength) { ZoomLength = MaxZoomOutLength; }
	SpringArm->TargetArmLength = ZoomLength;
}

