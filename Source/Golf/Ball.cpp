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

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("MaterialInstanceConstant'/Game/Materials/UltraBall_MI.UltraBall_MI'"));
	if (Material.Succeeded())
	{
		UltraBall->SetMaterial(0, Material.Object);
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
	CurrentZoomAmount = 0.0f;
	CurrentCharge = 0.0f;
	MaxChargePossibleAtFullChargeUp = 3.0f;
	TimeNeededToReachFullChargeUp = 1.0f;
	ChargeUpTimePassed = 0.0f;
	CurrentStateOfBall = Idle;
	MaxNumberOfShotsAllowedInTheAir = 1;
	NumberOfAirShotsTaken = 0;
	MaxDistanceOffGroundConsideredAir = 100.0f;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	inTheAir = false;
	inTheAirBurnOut = 0.0f;
	Squishiness = 0.11f;
	CurrentSideOfBallDown = 0;

	// Generate Spheres
	GenerateSphere(1, sphere1, "sphere1", FVector(0.0, 58.0, 0.0));
	GenerateSphere(2, sphere2, "sphere2", FVector(0.0, -58.0, 0.0));
	GenerateSphere(3, sphere3, "sphere3", FVector(43.0, -25.0, 30.0));
	GenerateSphere(4, sphere4, "sphere4", FVector(-37.0, 25.0, -30.0));
	GenerateSphere(5, sphere5, "sphere5", FVector(-37.0, 25.0, 30.0));
	GenerateSphere(6, sphere6, "sphere6", FVector(12.0, 25.0, 48.0));
	GenerateSphere(7, sphere7, "sphere7", FVector(49.0, 25.0, 0.0));
	GenerateSphere(8, sphere8, "sphere8", FVector(-51.0, -25.0, 0.0));
	GenerateSphere(9, sphere9, "sphere9", FVector(-12.0, -25.0, 48.0));
	GenerateSphere(10, sphere10, "sphere10", FVector(12.0, 25.0, -48.0));
	GenerateSphere(11, sphere11, "sphere11", FVector(-12.0, -25.0, -48.0));
	GenerateSphere(12, sphere12, "sphere12", FVector(37.0, -25.0, -30.0));

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

	// This section charges the CurrentCharge of the ball if the Left Mouse button is down.
	if (CurrentStateOfBall == Charging)
	{
		// Charge up the CurrentCharge.
		ChargeUpTimePassed += DeltaTime;
		CurrentCharge = (MaxChargePossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * ChargeUpTimePassed;
		if (CurrentCharge > MaxChargePossibleAtFullChargeUp)
			CurrentCharge = MaxChargePossibleAtFullChargeUp;
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
	if (inTheAir && NumberOfAirShotsTaken == MaxNumberOfShotsAllowedInTheAir && CurrentStateOfBall != Charging)
	{
		inTheAirBurnOut += (DeltaTime * 2);
		if (inTheAirBurnOut > 1.0f)
			inTheAirBurnOut = 1.0f;
	}
	else
	{
		inTheAirBurnOut -= (DeltaTime * 2);
		if (inTheAirBurnOut < 0.0f)
			inTheAirBurnOut = 0.0f;
	}

	// Get which way the ball is facing
	CurrentSideOfBallDown = 0;
	for (int i = 1; i < 12; i++)
		GetSideFacing(i);

	// Apply the correct level of squishyness depending on what way the ball is facing
	FVector	TargetSquishAmount;
	switch (CurrentSideOfBallDown)
	{
	case 0:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0);
		break;
	case 1:
		TargetSquishAmount = FVector(1.0, 1.0 - Squishiness, 1.0);
		break;
	case 2:
		TargetSquishAmount = FVector(1.0, 1.0 - Squishiness, 1.0);
		break;
	case 3:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0 - Squishiness);
		break;
	case 4:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case 5:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case 6:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case 7:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case 8:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	case 9:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case 10:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case 11:
		TargetSquishAmount = FVector(1.0, 1.0, 1.0 - Squishiness);
		break;
	case 12:
		TargetSquishAmount = FVector(1.0 - Squishiness, 1.0, 1.0);
		break;
	}

	// Squish the ball
	FVector CurrentScale = GetActorScale3D();
	CurrentScale.X = ClampIt(CurrentScale.X, TargetSquishAmount.X, DeltaTime);
	CurrentScale.Y = ClampIt(CurrentScale.Y, TargetSquishAmount.Y, DeltaTime);
	CurrentScale.Z = ClampIt(CurrentScale.Z, TargetSquishAmount.Z, DeltaTime);
	UltraBall->SetWorldScale3D(CurrentScale);

	// This section determines the transparency of the ball.
	FVector distance = Camera->GetComponentLocation() - GetActorLocation();
	float transparency = 1.0 - ((1.0 / 400.0) * distance.Size());
	if (transparency < 0.0) { transparency = 0.0; }
	if (transparency > 0.85) { transparency = 1.0; }
		UltraBall->SetScalarParameterValueOnMaterials("Alpha", transparency);

	// This Snaps the Ball to the center of the DeadZone if it's gravity is frozen.
	if (!UltraBall->IsGravityEnabled() && !isInCentreOfGravityFreeze)
	{
		FVector location = LocationOfGravityFreeze - GetActorLocation();
		FVector size = LocationOfGravityFreeze - GetActorLocation();

		// Move UltraBall Towards the Center of the DeadZone / LaunchZone
		location = location.GetSafeNormal(1.0) * 800;
		UltraBall->SetAllPhysicsLinearVelocity(location, false);

		// Check if UltraBall is near the Center of the DeadZone / LaunchZone
		if (size.Size() < 10.0)
		{
			// Freeze the ability of UltraBall to move towards the Center of the DeadZone / LaunchZone.
			isInCentreOfGravityFreeze = true;
			SetActorLocation(LocationOfGravityFreeze);
			UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f), false);

			// If UltraBall is a LaunchZone propel it in the correct direction.
			if (CurrentLauncherType == LaunchZone)
			{
				FVector offset = LocationOfLauncherDirection - UltraBall->GetComponentLocation();
				offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentLauncherPower * 10000.0f;
				UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
				UltraBall->AddForce(offset);
				UltraBall->SetEnableGravity(true);
				UltraBall->SetAllPhysicsAngularVelocity(AngularVelocity, false);
			}
		}
	}

	// This section determines the colour of the ball. If it's charging it becomes reder. If it's out of charges it becomes blacker.
	Pointlight->SetIntensity(((1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge) * 9000.0f);
	UltraBall->SetScalarParameterValueOnMaterials("Power", (1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge);
	UltraBall->SetScalarParameterValueOnMaterials("InAir", inTheAirBurnOut);

	// DEBUG MESSAGES
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Charge CurrentCharge is %f"), CurrentCharge));
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
	GEngine->AddOnScreenDebugMessage(11, 5.f, FColor::Red, FString::Printf(TEXT("burn out %f"), inTheAirBurnOut));

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
	CurrentZoomAmount -= (CurrentZoomAmount / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::ZoomOut()
{
	CurrentZoomAmount += (CurrentZoomAmount / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::Fire()
{
	// If the ball is Idle then start charging the ball.
	if (CurrentStateOfBall == Idle && NumberOfAirShotsTaken < MaxNumberOfShotsAllowedInTheAir)
	{
		CurrentStateOfBall = Charging;
		ChargeUpTimePassed = 0.0f;
		if (inTheAir && UltraBall->IsGravityEnabled())
			NumberOfAirShotsTaken++;
		return;
	}
}

void ABall::EndFire()
{
	// If the ball is charging then fire the ball.
	if (CurrentStateOfBall == Charging)
	{
		FVector offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		offset = offset.GetSafeNormal(1.0f) * CurrentCharge * 10000000.0f;
		UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
		UltraBall->AddForce(offset);
		if (!UltraBall->IsGravityEnabled())
		{
			UltraBall->SetEnableGravity(true);
			UltraBall->SetAllPhysicsAngularVelocity(AngularVelocity, false);
		}

		CurrentCharge = 0.0f;
		ChargeUpTimePassed = 0.0f;
		Pointlight->SetIntensity(0.0f);
		CurrentStateOfBall = Idle;
	}
}

void ABall::CancelFire()
{
	if (CurrentStateOfBall != Idle)
	{
		CurrentStateOfBall = Idle;
		CurrentCharge = 0.0f;
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

void ABall::ZoneEnter(FVector a_zoneLocation, FVector a_ZonelaunchDirection, float a_zonePower, int a_zoneType)
{
	LocationOfGravityFreeze = a_zoneLocation;
	LocationOfLauncherDirection = a_ZonelaunchDirection;
	AngularVelocity = UltraBall->GetPhysicsAngularVelocity();
	UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	UltraBall->SetAllPhysicsAngularVelocity(FVector(0.0f, 0.0f, 0.0f), false);
	UltraBall->SetEnableGravity(false);
	NumberOfAirShotsTaken = 0;
	isInCentreOfGravityFreeze = false;
	CurrentLauncherPower = a_zonePower;

	if (a_zoneType == 0)
		CurrentLauncherType = DeadZone;
	if (a_zoneType == 1)
		CurrentLauncherType = LaunchZone;
}

void ABall::UpdateComponents()
{
	if (CurrentZoomAmount < MinZoomInLength) { CurrentZoomAmount = MinZoomInLength; }
	if (CurrentZoomAmount > MaxZoomOutLength) { CurrentZoomAmount = MaxZoomOutLength; }
	SpringArm->TargetArmLength = CurrentZoomAmount;
}

void ABall::GenerateSphere(int a_number, USphereComponent * &a_sphere, FName a_name, FVector a_location)
{
	a_sphere = CreateDefaultSubobject<USphereComponent>(a_name);
	a_sphere->SetRelativeLocation(a_location);
	a_sphere->SetWorldScale3D(FVector(0.5f));
	a_sphere->SetupAttachment(RootComponent);
	a_sphere->SetSimulatePhysics(false);
	a_sphere->SetEnableGravity(false);

	CurrentSideOfBallDownList[a_number] = a_sphere;
}

void ABall::GetSideFacing(int a_side)
{
	Start = GetActorLocation();
	End = CurrentSideOfBallDownList[a_side]->GetComponentLocation();
	GetWorld()->LineTraceSingleByChannel(Result, Start, End, CollisionChannel, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam);
	if (Result.GetActor() != NULL)
		CurrentSideOfBallDown = a_side;
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

