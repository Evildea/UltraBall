// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h" 
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h" 

#include "Materials/MaterialInstanceDynamic.h" 
#include "Components/PointLightComponent.h" 

#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Components/SphereComponent.h" 

// Sets default values
ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for ball
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DodecaMesh(TEXT("StaticMesh'/Game/Models/Ultraball.Ultraball'"));
	if (DodecaMesh.Succeeded())
	{
		UStaticMesh* Asset = DodecaMesh.Object;
		UltraBall->SetStaticMesh(Asset);
		UltraBall->SetSimulatePhysics(true);
		RootComponent = UltraBall;
	}

	// Setup Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("springarm");
	SpringArm->bAbsoluteRotation = true;
	SpringArm->SetupAttachment(RootComponent);

	// Setup Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("camera");
	Camera->SetupAttachment(SpringArm);

	// Setup Point Light
	Pointlight = CreateDefaultSubobject<UPointLightComponent>("light");
	Pointlight->SetAttenuationRadius(300.0f);
	Pointlight->SetSourceRadius(40.0f);
	Pointlight->SetIntensity(0.0f);
	Pointlight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
	Pointlight->SetupAttachment(RootComponent);

	// Set up inital values that can be modified by the Designer.
	MinZoomInLength = 200.0f;
	MaxZoomOutLength = 1000.0f;
	ZoomInSpeed = 93.0f;

	// Set up initial values that can't be modified by the Designer.
	ZoomLength = 0.0f;
	Power = 0.0f;
	MaxPowerPossibleAtFullChargeUp = 3.0f;
	TimeNeededToReachFullChargeUp = 1.0f;
	ChargeUpTimePassed = 0.0f;
	CurrentBallState = Idle;
	MaxNumberOfShotsAllowedInTheAir = 1;
	NumberOfAirShotsTaken = 0;
	MaxDistanceOffGroundConsideredAir = 100.0f;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	inTheAir = false;
	inTheAirBurnOut = 0.0f;
	TargetSquishAmount = FVector(1.0f, 1.0f, 1.0f);
	Squishiness = 0.11f;

	// Generate Spheres
	GenerateSphere(sphere1, "sphere1", FVector(0.0, 58.0, 0.0));
	GenerateSphere(sphere2, "sphere2", FVector(0.0, -58.0, 0.0));
	GenerateSphere(sphere3, "sphere3", FVector(43.0, -25.0, 30.0));
	GenerateSphere(sphere4, "sphere4", FVector(-37.0, 25.0, -30.0));
	GenerateSphere(sphere5, "sphere5", FVector(-37.0, 25.0, 30.0));
	GenerateSphere(sphere6, "sphere6", FVector(12.0, 25.0, 48.0));
	GenerateSphere(sphere7, "sphere7", FVector(49.0, 25.0, 0.0));
	GenerateSphere(sphere8, "sphere8", FVector(-51.0, -25.0, 0.0));
	GenerateSphere(sphere9, "sphere9", FVector(-12.0, -25.0, 48.0));
	GenerateSphere(sphere10, "sphere10", FVector(12.0, 25.0, -48.0));
	GenerateSphere(sphere11, "sphere11", FVector(-12.0, -25.0, -48.0));
	GenerateSphere(sphere12, "sphere12", FVector(37.0, -25.0, -30.0));

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

	// This section charges the power of the ball if the Left Mouse button is down.
	if (CurrentBallState == Charging)
	{
		// Charge up the Power.
		ChargeUpTimePassed += DeltaTime;
		Power = (MaxPowerPossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * ChargeUpTimePassed;
		if (Power > MaxPowerPossibleAtFullChargeUp)
			Power = MaxPowerPossibleAtFullChargeUp;
	}
	
	// This section determines whether the ball is currently in the air.
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

	// This section determines how black to make the ball when out of charges in the air.
	if (inTheAir && NumberOfAirShotsTaken == MaxNumberOfShotsAllowedInTheAir && CurrentBallState != Charging)
	{
		inTheAirBurnOut += DeltaTime;
		if (inTheAirBurnOut > 1.0f)
			inTheAirBurnOut = 1.0f;
	}
	else
	{
		inTheAirBurnOut -= DeltaTime;
		if (inTheAirBurnOut < 0.0f)
			inTheAirBurnOut = 0.0f;
	}

	// This section determines the colour of the ball. If it's charging it becomes reder. If it's out of charges it becomes blacker.
	Pointlight->SetIntensity(((1.0f / MaxPowerPossibleAtFullChargeUp) * Power) * 9000.0f);
	UltraBall->SetScalarParameterValueOnMaterials("Power", (1.0f / MaxPowerPossibleAtFullChargeUp) * Power);
	UltraBall->SetScalarParameterValueOnMaterials("InAir", inTheAirBurnOut);

	// DEBUG MESSAGES
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Charge Power is %f"), Power));
	if (inTheAir)
	{
		inTheAirBurnOut += DeltaTime;
		if (inTheAirBurnOut > 1.0f)
			inTheAirBurnOut = 1.0f;
	}
	else
	{
		inTheAirBurnOut -= DeltaTime;
		if (inTheAirBurnOut < 0.0f)
			inTheAirBurnOut = 0.0f;
	}

	// -------------------------------------------------------------
	// This section determines how much to squish the ball.

	CurrentSideTouched = none;
	GetSideFacing(sphere1, side1);
	GetSideFacing(sphere2, side2);
	GetSideFacing(sphere3, side3);
	GetSideFacing(sphere4, side4);
	GetSideFacing(sphere5, side5);
	GetSideFacing(sphere6, side6);
	GetSideFacing(sphere7, side7);
	GetSideFacing(sphere8, side8);
	GetSideFacing(sphere9, side9);
	GetSideFacing(sphere10, side10);
	GetSideFacing(sphere11, side11);
	GetSideFacing(sphere12, side12);

	switch (CurrentSideTouched)
	{
	case ABall::none:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0);
		break;
	case ABall::side1:
		TargetSquishAmount = FVector(1.0, 1.0 - Squishiness, 1.0);
		break;
	case ABall::side2:
		TargetSquishAmount = FVector(1.0, 1.0 - Squishiness, 1.0);
		break;
	case ABall::side3:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0 - Squishiness);
		break;
	case ABall::side4:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case ABall::side5:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case ABall::side6:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case ABall::side7:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case ABall::side8:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case ABall::side9:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case ABall::side10:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case ABall::side11:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case ABall::side12:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	}

	FVector CurrentScale = GetActorScale3D();
	CurrentScale.X = ClampIt(CurrentScale.X, TargetSquishAmount.X, DeltaTime);
	CurrentScale.Y = ClampIt(CurrentScale.Y, TargetSquishAmount.Y, DeltaTime);
	CurrentScale.Z = ClampIt(CurrentScale.Z, TargetSquishAmount.Z, DeltaTime);
	UltraBall->SetWorldScale3D(CurrentScale);

	// This section determines the colour of the ball. If it's charging it becomes reder. If it's out of charges it becomes blacker.
	Pointlight->SetIntensity(((1.0f / MaxPowerPossibleAtFullChargeUp) * Power) * 9000.0f);
	UltraBall->SetScalarParameterValueOnMaterials("Power", (1.0f / MaxPowerPossibleAtFullChargeUp) * Power);
	UltraBall->SetScalarParameterValueOnMaterials("InAir", inTheAirBurnOut);

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
	// If the ball is Idle then start charging the ball.
	if (CurrentBallState == Idle && NumberOfAirShotsTaken < MaxNumberOfShotsAllowedInTheAir)
	{
		CurrentBallState = Charging;
		ChargeUpTimePassed = 0.0f;
		if (inTheAir && UltraBall->IsGravityEnabled())
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
		if (!UltraBall->IsGravityEnabled())
		{
			UltraBall->SetEnableGravity(true);
			UltraBall->SetAllPhysicsAngularVelocity(AngularVelocity, false);
		}

		Power = 0.0f;
		ChargeUpTimePassed = 0.0f;
		Pointlight->SetIntensity(0.0f);
		CurrentBallState = Idle;
	}
}

void ABall::CancelFire()
{
	if (CurrentBallState != Idle)
	{
		CurrentBallState = Idle;
		Power = 0.0f;
		ChargeUpTimePassed = 0.0f;
		Pointlight->SetIntensity(0.0f);
		if (inTheAir && UltraBall->IsGravityEnabled())
			NumberOfAirShotsTaken--;
	}
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
	AngularVelocity = UltraBall->GetPhysicsAngularVelocity();
	UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	UltraBall->SetAllPhysicsAngularVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	UltraBall->SetEnableGravity(false);
	NumberOfAirShotsTaken = 0;
}

void ABall::UpdateComponents()
{
	if (ZoomLength < MinZoomInLength) { ZoomLength = MinZoomInLength; }
	if (ZoomLength > MaxZoomOutLength) { ZoomLength = MaxZoomOutLength; }
	SpringArm->TargetArmLength = ZoomLength;
}

void ABall::GenerateSphere(USphereComponent * &a_sphere, FName a_name, FVector a_location)
{
	a_sphere = CreateDefaultSubobject<USphereComponent>(a_name);
	a_sphere->SetRelativeLocation(a_location);
	a_sphere->SetWorldScale3D(FVector(0.5f));
	a_sphere->SetupAttachment(RootComponent);
}

void ABall::GetSideFacing(USphereComponent *& a_sphere, BallSide a_side)
{
	Start = GetActorLocation();
	End = a_sphere->GetComponentLocation();
	GetWorld()->LineTraceSingleByChannel(Result, Start, End, CollisionChannel, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam);
	if (Result.GetActor() != NULL)
	{
		CurrentSideTouched = a_side;
	}
}

float ABall::ClampIt(float X1, float X2, float DeltaTime)
{
	if (X1 < X2)
	{
		X1 += (DeltaTime / 3);
		if (X1 > X2)
			X1 = X2;
	}
	else
	{
		X1 -= (DeltaTime / 3);
		if (X1 < X2)
			X1 = X2;
	}

	return X1;
}

