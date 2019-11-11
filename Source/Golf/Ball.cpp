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
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "PhysicsEngine/BodySetup.h"

ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for UltraBall
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");

	static ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallSimple(TEXT("StaticMesh'/Game/Models/UltraBallS.UltraBallS'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallComplex(TEXT("StaticMesh'/Game/Models/UltraBallC.UltraBallC'"));

	SimpleAsset = UltraBallSimple.Object;
	ComplexAsset = UltraBallComplex.Object;

	UltraBall->SetStaticMesh(ComplexAsset);

	UltraBall->SetSimulatePhysics(true);
	UltraBall->SetNotifyRigidBodyCollision(true);
	UltraBall->OnComponentHit.AddDynamic(this, &ABall::OnHit);
	UltraBall->SetAngularDamping(2.0f);
	RootComponent = UltraBall;

	// Apply Dynamic Material to UltraBall
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("MaterialInstanceConstant'/Game/Materials/UltraBall_MI.UltraBall_MI'"));
	if (Material.Succeeded())
		UltraBall->SetMaterial(0, Material.Object);

	// Setup Sound Component
	Sound = CreateDefaultSubobject<UAudioComponent>("Sound");
	Sound->SetAutoActivate(false);
	Sound->SetupAttachment(RootComponent);

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

	// Generate Spheres that are used for Collision Detection
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

	// Set up inital values that can be modified by the Designer.
	MinZoomInLength = 200.0f;	// The Min Zoom Out Distance of the Camera
	MaxZoomOutLength = 1000.0f;	// The Max Zoom Out Distance of the Camera
	ZoomInSpeed = 93.0f;	// The Speed at which the Camera can Zoom in and Out
	MaxChargePossibleAtFullChargeUp = 2.0f;		// The Maximum Charge the Ball can Reach
	TimeNeededToReachFullChargeUp = 1.0f;		// The Time Needed to Reach Full Charge Up
	CurrentStateOfBall = Idle;		// The Current State of the User's Interaction with UltraBall
	MaxNumberOfShotsAllowedInTheAir = 1;		// The Maximum Amount of Shots the Player is allowed to take in the Air
	MaxDistanceOffGroundConsideredAir = 100.0f;	// The Maximum Distance that is considered Off the Ground
	Squishiness = 0.11f;	// How Squishy UltraBall is
	SpeedAtWhichMeshTransitionsBackToComplex = 300.0f;
	MaxParAllowed = 20;		// The Maximum Allowed Par to win the Level

	// Update the Camera based on their inital values.
	UpdateComponents();

	// Tell the game controller to possess this player.
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();
	isCameraLocked = true;
	inTheAir = false;
	inTheAirBurnOut = 0.0f;
	CurrentSideOfBallDown = 0;
	ChargeUpTimePassed = 0.0f;
	CurrentZoomAmount = 0.0f;
	CurrentCharge = 0.0f;
	CollisionChannel = ECC_Visibility;
	hasSoundPlayed = false;
	CurrentPar = 0;
	isGamePaused = false;
	NumberOfAirShotsTaken = 0;
	TimeSinceMeshChange = 0.0f;
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// This counter purely exists to stop mesh changes from happening too fast.
	if (TimeSinceMeshChange < 100.0f)
		TimeSinceMeshChange += DeltaTime;

	// This section charges the CurrentCharge of the ball if the Left Mouse button is down.
	if (CurrentStateOfBall == Charging)
	{
		// Charge up the CurrentCharge.
		ChargeUpTimePassed += DeltaTime;
		CurrentCharge = (MaxChargePossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * ChargeUpTimePassed;
		if (CurrentCharge > MaxChargePossibleAtFullChargeUp)
			CurrentCharge = MaxChargePossibleAtFullChargeUp;

		//This section predicts what direction the shot will go.
		FVector offset;
		if (isCameraLocked)
			offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		else
			offset = UltraBall->GetComponentLocation() - CameraLocationLock;

		offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 12.5f;
		FPredictProjectilePathParams Predictor;
		FPredictProjectilePathResult ProjectileResult;

		Predictor.StartLocation = GetActorLocation();
		Predictor.LaunchVelocity = offset;
		Predictor.bTraceComplex = true;
		Predictor.ProjectileRadius = 30.0f;
		Predictor.TraceChannel = CollisionChannel;

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		Predictor.ActorsToIgnore = ActorsToIgnore;
		Predictor.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		Predictor.SimFrequency = 10.0f;
		Predictor.MaxSimTime = 1.2f;

		UGameplayStatics::PredictProjectilePath(GetWorld(), Predictor, ProjectileResult);
	}
	
	// This section checks and updates whether UltraBall is in the air.
	inTheAirCheckTick();

	// This section determines how many changes you have left when flying through the air.
	ChargesRemainingCheckTick(DeltaTime);

	// This section squishes the UltraBall based on what side is facing down.
	SquishTick(DeltaTime);

	// This section checks whether the UltraBall is inside a DeadZone or LaunchZone.
	ZoneTick();

	// This section applies material changes based on the status of UltraBall.
	MaterialTick(DeltaTime);

	if (TimeSinceMeshChange > 1.0f && UltraBall->IsGravityEnabled())
	{
		if (UltraBall->GetPhysicsLinearVelocity().Size() >= SpeedAtWhichMeshTransitionsBackToComplex)
		{

			if (UltraBall->GetStaticMesh() != SimpleAsset)
			{
			
				FVector LinearVelocity = UltraBall->GetPhysicsLinearVelocity();
				FVector AngularVelocity = UltraBall->GetPhysicsAngularVelocity();

				UltraBall->SetStaticMesh(SimpleAsset);

				UltraBall->SetPhysicsLinearVelocity(LinearVelocity);
				UltraBall->SetPhysicsAngularVelocity(AngularVelocity);
			}
		}
		else
		{
			if (UltraBall->GetStaticMesh() != ComplexAsset)
			{
			
				FVector LinearVelocity = UltraBall->GetPhysicsLinearVelocity();
				FVector AngularVelocity = UltraBall->GetPhysicsAngularVelocity();

				UltraBall->SetStaticMesh(ComplexAsset);

				UltraBall->SetPhysicsLinearVelocity(LinearVelocity);
				UltraBall->SetPhysicsAngularVelocity(AngularVelocity);
			}
		}
	}

	//if (UltraBall->GetStaticMesh() == SimpleAsset)
	//GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("Simple %f"), GetVelocity().Size()));

	//if (UltraBall->GetStaticMesh() == ComplexAsset)
	//	GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("Complex %f"), GetVelocity().Size()));

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
	PlayerInputComponent->BindAction("CameraLock", IE_Pressed, this, &ABall::CameraUnLock);
	PlayerInputComponent->BindAction("CameraLock", IE_Released, this, &ABall::CameraLock);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ABall::Pause);

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
		UltraBall->SetStaticMesh(SimpleAsset);
		TimeSinceMeshChange = 0.0f;

		FVector offset;
		if (isCameraLocked)
			offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		else
			offset = UltraBall->GetComponentLocation() - CameraLocationLock;

		offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 1000.0f;
		UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
		UltraBall->AddImpulse(offset);
		if (!UltraBall->IsGravityEnabled())
		{
			UltraBall->SetEnableGravity(true);
			UltraBall->SetAllPhysicsAngularVelocity(AngularVelocity, false);
		}

		CurrentCharge = 0.0f;
		ChargeUpTimePassed = 0.0f;
		Pointlight->SetIntensity(0.0f);
		CurrentStateOfBall = Idle;
		CurrentPar++;

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

void ABall::CameraLock()
{
	isCameraLocked = true;
	CurrentZoomAmount = CameraZoomAmountLock;
	SpringArm->SetRelativeRotation(CameraAngleLock);
}

void ABall::CameraUnLock()
{
	isCameraLocked = false;
	CameraZoomAmountLock = CurrentZoomAmount;
	CameraAngleLock = SpringArm->GetComponentRotation();
	CameraLocationLock = Camera->GetComponentLocation();
}

FString ABall::GetParString()
{
	return FString::Printf(TEXT("Par: %d/%d"), CurrentPar, MaxParAllowed);
}

void ABall::Pause()
{
	isGamePaused = !isGamePaused;
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

void ABall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Sound != nullptr && !hasSoundPlayed)
	{
		Sound->Play(0.0f);
		hasSoundPlayed = true;
	}
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

void ABall::ZoneTick()
{
	// This checks if Gravity is disabled for the UltraBall and it's not yet in the centre of the Zone.
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
				offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentLauncherPower * 1000.0f;
				UltraBall->AddImpulse(offset);
				UltraBall->SetEnableGravity(true);
				UltraBall->SetAllPhysicsAngularVelocity(AngularVelocity, false);
				TimeSinceMeshChange = 0.0f;
			}
		}
	}
}

void ABall::SquishTick(float DeltaTime)
{
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
}

void ABall::MaterialTick(float DeltaTime)
{
	// BurnOut: This section determines how black to make the ball when it runs out of charges in the air.
	UltraBall->SetScalarParameterValueOnMaterials("BurnOut", inTheAirBurnOut);

	// Power: This section determines how red to make UltraBall if it's charging.
	float PowerAmount = (1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge;
	Pointlight->SetIntensity(PowerAmount * 9000.0f);
	UltraBall->SetScalarParameterValueOnMaterials("Power", PowerAmount);

	// Alpha: This section determines how transparent to make UltraBall if the camera is too close.
	FVector distance = Camera->GetComponentLocation() - GetActorLocation();
	if (distance.Size() < 60.0f)
		UltraBall->SetVisibility(false);
	else
	{
		UltraBall->SetVisibility(true);
		float transparency = 1.0f - ((1.0f / CurrentZoomAmount) * (distance.Size() - 100.0f));
		if (transparency < 0.5f) { transparency = 0.0f; }
		if (transparency >= 0.5f) { transparency = -((0.5 - transparency) * 2); }
		if (transparency > 0.8f) { transparency = 1.0f; }
		UltraBall->SetScalarParameterValueOnMaterials("Alpha", transparency);
	}
}

void ABall::inTheAirCheckTick()
{
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
	{
		inTheAir = true;
		hasSoundPlayed = false;
	}

	End = GetActorLocation();
	End.Z -= (MaxDistanceOffGroundConsideredAir / 2);
	GetWorld()->LineTraceSingleByChannel(Result, Start, End, CollisionChannel, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam);
	if (Result.GetActor() == NULL)
	{
		hasSoundPlayed = false;
	}
}

void ABall::ChargesRemainingCheckTick(float DeltaTime)
{
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
}

