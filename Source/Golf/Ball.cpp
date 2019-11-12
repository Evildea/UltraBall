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
#include "PhysicalMaterials/PhysicalMaterial.h" 

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

	// Apply Physics Material to UltraBall
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> PhysicsMaterial(TEXT("PhysicalMaterial'/Game/Materials/UltraBallPhysics.UltraBallPhysics'"));
	if (PhysicsMaterial.Succeeded())
		UltraBall->SetPhysMaterialOverride(PhysicsMaterial.Object);

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

	// Set up inital values that can be modified by the Designer.
	MinZoomInLength = 200.0f;	// The Min Zoom Out Distance of the Camera
	MaxZoomOutLength = 1000.0f;	// The Max Zoom Out Distance of the Camera
	ZoomInSpeed = 93.0f;	// The Speed at which the Camera can Zoom in and Out
	MaxChargePossibleAtFullChargeUp = 2.0f;		// The Maximum Charge the Ball can Reach
	TimeNeededToReachFullChargeUp = 1.0f;		// The Time Needed to Reach Full Charge Up
	MaxNumberOfShotsAllowedInTheAir = 1;		// The Maximum Amount of Shots the Player is allowed to take in the Air
	SpeedAtWhichMeshTransitionsBackToComplex = 300.0f;
	MaxParAllowed = 20;		// The Maximum Allowed Par to win the Level
	MaxDistanceOffGroundConsideredAir = 100.0f;
	IsInDebugMode = false;

	// Update the Camera based on their inital values.
	UpdateComponents();

	// Tell the game controller to possess this player.
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();

	CurrentFireState = Idle;
	CurrentChargeState = HaveCharges;
	CurrentLocationState = OnTheGround;
	CurrentZoneState = InNoZone;

	CurrentChargeUpTimePassed = 0.0f;
	CurrentZoomAmount = 0.0f;
	CurrentPar = 0;
	CurrentCharge = 0.0f;
	inAirBlackenAmount = 0.0f;
	TimeSinceMeshChange = 0.0f;
	TimeSinceLastInZone = 0.0f;
	StartTimerSinceLastInZone = false;
	isCameraLocked = false;
	CameraZoomAmountLock = 0.0f;

	CurrentShotsTakenInTheAir = 0;

	isGamePaused = false;
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update for CHARGING STATE
	if (CurrentFireState == Charging)
	{
		// Calculate how much time has passed since the start of the charge up.
		CurrentChargeUpTimePassed += DeltaTime;

		// Calculate the current amount of charge to be applied to UltraBall.
		CurrentCharge = (MaxChargePossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * CurrentChargeUpTimePassed;
		if (CurrentCharge > MaxChargePossibleAtFullChargeUp)
			CurrentCharge = MaxChargePossibleAtFullChargeUp;
	
	

		//This section predicts what direction the shot will go.
		//FVector offset;
		//if (isCameraLocked)
		//	offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		//else
		//	offset = UltraBall->GetComponentLocation() - CameraLocationLock;


		//offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 12.5f;
		//FPredictProjectilePathParams Predictor;
		//FPredictProjectilePathResult ProjectileResult;
		//ECollisionChannel CollisionChannel = ECC_Visibility;

		//Predictor.StartLocation = GetActorLocation();
		//Predictor.LaunchVelocity = offset;
		//Predictor.bTraceComplex = true;
		//Predictor.ProjectileRadius = 30.0f;
		//Predictor.TraceChannel = CollisionChannel;

		//TArray<AActor*> ActorsToIgnore;
		//ActorsToIgnore.Add(this);

		//Predictor.ActorsToIgnore = ActorsToIgnore;
		//Predictor.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		//Predictor.SimFrequency = 10.0f;
		//Predictor.MaxSimTime = 1.2f;

		//UGameplayStatics::PredictProjectilePath(GetWorld(), Predictor, ProjectileResult);

	
	}

	// Change to a Sphere Mesh Colider if UltraBall is moving fast and a Dodecahedron Mesh Colider if it's moving slow.
	MeshChangeTick(DeltaTime);

	// Update for LOCATION STATE
	LocationTick();

	// Update for ZONE STATE
	ZoneTick(DeltaTime);

	// Update the Dynamic Material and Lights
	MaterialTick(DeltaTime);

	// Update Debug
	DebugTick();






	
	//// This section charges the CurrentCharge of the ball if the Left Mouse button is down.
	//if (CurrentStateOfBall == Charging)
	//{
	//	// Charge up the CurrentCharge.
	//	ChargeUpTimePassed += DeltaTime;
	//	CurrentCharge = (MaxChargePossibleAtFullChargeUp / TimeNeededToReachFullChargeUp) * ChargeUpTimePassed;
	//	if (CurrentCharge > MaxChargePossibleAtFullChargeUp)
	//		CurrentCharge = MaxChargePossibleAtFullChargeUp;

	//	//This section predicts what direction the shot will go.
	//	FVector offset;
	//	if (isCameraLocked)
	//		offset = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
	//	else
	//		offset = UltraBall->GetComponentLocation() - CameraLocationLock;

	//	offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 12.5f;
	//	FPredictProjectilePathParams Predictor;
	//	FPredictProjectilePathResult ProjectileResult;

	//	Predictor.StartLocation = GetActorLocation();
	//	Predictor.LaunchVelocity = offset;
	//	Predictor.bTraceComplex = true;
	//	Predictor.ProjectileRadius = 30.0f;
	//	Predictor.TraceChannel = CollisionChannel;

	//	TArray<AActor*> ActorsToIgnore;
	//	ActorsToIgnore.Add(this);

	//	Predictor.ActorsToIgnore = ActorsToIgnore;
	//	Predictor.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	//	Predictor.SimFrequency = 10.0f;
	//	Predictor.MaxSimTime = 1.2f;

	//	UGameplayStatics::PredictProjectilePath(GetWorld(), Predictor, ProjectileResult);
	//}

	//// This section checks whether the UltraBall is inside a DeadZone or LaunchZone.
	//ZoneTick();

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
	// Only Fire if in a Idle State and you have't reached the Maxmimum number of Shots.
	if (CurrentFireState == Idle && CurrentShotsTakenInTheAir != MaxNumberOfShotsAllowedInTheAir)
	{
		// Change to the CHARGING STATE
		CurrentFireState = Charging;

		// Reset the Charge State.
		ResetChargeState();
	}
	
}

void ABall::EndFire()
{

	if (CurrentFireState == Charging)
	{
		// Charge back to an Idle Charge State.
		CurrentFireState = Idle;

		// Load the Simple Mesh to stop UltraBall from flying around randomly.
		UltraBall->SetStaticMesh(SimpleAsset);

		// Calculate the launch direction for UltraBall.
		FVector LaunchDirection = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();
		LaunchDirection = LaunchDirection.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 1000.0f;
		UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
		UltraBall->AddImpulse(LaunchDirection);

		// If In the Air up the Amount of Air Shots Taken
		if (CurrentLocationState == InTheAir && CurrentZoneState == InNoZone)
			CurrentShotsTakenInTheAir++;

		// Remove Any Zone Captures.
		CurrentZoneState = InNoZone;
		UltraBall->SetEnableGravity(true);

		// Reset the Charge State.
		ResetChargeState();

		// Increase the Par.
		CurrentPar++;

	}
}

void ABall::CancelFire()
{
	if (CurrentFireState == Charging)
	{
		CurrentFireState = Idle;
		ResetChargeState();
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
	SpringArm->SetRelativeRotation(CameraAngleLock);
}

void ABall::CameraUnLock()
{
	isCameraLocked = false;
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

void ABall::ZoneEnter(int ZoneType, FVector CenterOfGravity, FVector LaunchDirection, float LaunchPower)
{
	if (StartTimerSinceLastInZone == false)
	{

		// Set Local Variables
		this->CenterOfGravity = CenterOfGravity;
		this->LaunchDirection = LaunchDirection;
		this->LaunchPower = LaunchPower;

		// Reset the Shots already taken.
		CurrentShotsTakenInTheAir = 0;

		// Dead Zone Enter
		if (ZoneType == 0)
		{
			CurrentZoneState = InDeadZone;

			// Disable All Gravity.
			UltraBall->SetEnableGravity(false);
			UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f), false);
			UltraBall->SetAllPhysicsAngularVelocity(FVector(0.0f), false);

			// Reset the Timer
			StartTimerSinceLastInZone = true;

		}

		// Launch Enter
		if (ZoneType == 1)
		{
			CurrentZoneState = InLaunchZone;

			// Disable All Gravity.
			UltraBall->SetEnableGravity(false);
			UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f), false);
			UltraBall->SetAllPhysicsAngularVelocity(FVector(0.0f), false);

			// Reset the Timer
			StartTimerSinceLastInZone = true;

		}
	}
}

void ABall::BumperHit()
{
	TimeSinceMeshChange = 0.0f;
}

void ABall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Sound != nullptr && !Sound->IsPlaying())
	{
		if (GetVelocity().Size() > 50.0f)
		{
			// Initialise Paramaters for a Ray Trace.
			FVector StartLocation = GetActorLocation();
			FVector EndLocation = GetActorLocation();
			ECollisionChannel CollisionChannel = ECC_Visibility;
			FCollisionQueryParams CollisionParameters;
			FHitResult Result;

			// Setup Parameters for the Ray Trace.
			CollisionParameters.AddIgnoredActor(this);
			EndLocation.Z -= MaxDistanceOffGroundConsideredAir / 2;

			// Perform Ray Trace and Setup LOCATION STATE.
			GetWorld()->LineTraceSingleByChannel(Result, StartLocation, EndLocation, CollisionChannel, CollisionParameters, FCollisionResponseParams::DefaultResponseParam);
			if (Result.GetActor() != NULL)
			{
				Sound->Play(0.0f);
				Sound->SetVolumeMultiplier((0.5f / 50.0f) * GetVelocity().Size());
			}
		}
	}
}

void ABall::UpdateComponents()
{
	if (CurrentZoomAmount < MinZoomInLength) { CurrentZoomAmount = MinZoomInLength; }
	if (CurrentZoomAmount > MaxZoomOutLength) { CurrentZoomAmount = MaxZoomOutLength; }
	SpringArm->TargetArmLength = CurrentZoomAmount;
}

void ABall::ZoneTick(float DeltaTime)
{

	// Don't activate a Zone if you've recently been in a Zone.
	if (StartTimerSinceLastInZone)
	{
		TimeSinceLastInZone += DeltaTime;
		if (TimeSinceLastInZone > 0.3f)
		{
			StartTimerSinceLastInZone = false;
			TimeSinceLastInZone = 0.0f;
		}
	}

	// If in a Dead Zone
	if (CurrentZoneState == InDeadZone)
	{
		// Snap UltraBall to the Center of Gravity.
		SnapToCenterOfGravityTick();
	}
	
	// If in a Launch Zone
	if (CurrentZoneState == InLaunchZone)
	{
		// Snap UltraBall to the Center of Gravity.
		float Distance = SnapToCenterOfGravityTick();

		// Launch the 
		if (Distance < 10.0f)
		{
			FVector offset = LaunchDirection - GetActorLocation();
			offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * LaunchPower * 1000.0f;
			UltraBall->AddImpulse(offset);
			UltraBall->SetEnableGravity(true);
			TimeSinceMeshChange = 0.0f;

			CurrentZoneState = InNoZone;
		}

	}
}

void ABall::MaterialTick(float DeltaTime)
{
	// Add a Reddish Glow to UltraBall when it's Charging.
	if (CurrentFireState == Charging)
	{
		float ReddishGlow = (1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge;
		UltraBall->SetScalarParameterValueOnMaterials("Power", ReddishGlow);
		Pointlight->SetIntensity(ReddishGlow * 9000.0f);
	}
	else
		UltraBall->SetScalarParameterValueOnMaterials("Power", 0.0f);
	
	// Blacken UltraBall when he's out of Shots.
	if (CurrentShotsTakenInTheAir == MaxNumberOfShotsAllowedInTheAir)
	{
		inAirBlackenAmount += (DeltaTime * 2);
		if (inAirBlackenAmount > 1.0f)
			inAirBlackenAmount = 1.0f;
	}
	else
	{
		inAirBlackenAmount -= (DeltaTime * 2);
		if (inAirBlackenAmount < 0.0f)
			inAirBlackenAmount = 0.0f;
	}
	UltraBall->SetScalarParameterValueOnMaterials("BurnOut", inAirBlackenAmount);
	
	// Make UltraBall Transparent if the Camera is too close.
	FVector CameraDistance = Camera->GetComponentLocation() - GetActorLocation();
	if (CameraDistance.Size() < 60.0f)
		UltraBall->SetVisibility(false);
	else
	{
		UltraBall->SetVisibility(true);
		float transparency = 1.0f - ((1.0f / CurrentZoomAmount) * (CameraDistance.Size() - 100.0f));
		if (transparency < 0.5f) { transparency = 0.0f; }
		if (transparency >= 0.5f) { transparency = -((0.5 - transparency) * 2); }
		if (transparency > 0.8f) { transparency = 1.0f; }
		UltraBall->SetScalarParameterValueOnMaterials("Alpha", transparency);
	}
}

void ABall::LocationTick()
{
	// Initialise Paramaters for a Ray Trace.
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = GetActorLocation();
	ECollisionChannel CollisionChannel = ECC_Visibility;
	FCollisionQueryParams CollisionParameters;
	FHitResult Result;

	// Setup Parameters for the Ray Trace.
	CollisionParameters.AddIgnoredActor(this);
	EndLocation.Z -= MaxDistanceOffGroundConsideredAir;

	// Perform Ray Trace and Setup LOCATION STATE.
	GetWorld()->LineTraceSingleByChannel(Result, StartLocation, EndLocation, CollisionChannel, CollisionParameters, FCollisionResponseParams::DefaultResponseParam);
	if (Result.GetActor() == NULL)
		CurrentLocationState = InTheAir;
	else
	{
		CurrentLocationState = OnTheGround;
		CurrentShotsTakenInTheAir = 0;
	}
}

void ABall::DebugTick()
{
	if (IsInDebugMode)
	{
		if (CurrentLocationState == OnTheGround)
			GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("UltraBall is on the ground...")));
		else
			GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("UltraBall is in the air...")));

		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Red, FString::Printf(TEXT("Shots taken in the air: %d"), CurrentShotsTakenInTheAir));

		switch (CurrentZoneState)
		{
			case InDeadZone:
				GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("In Dead Zone")));
				break;
			case InLaunchZone:
				GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("In Launch Zone")));
				break;
			case InNoZone:
				GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("In No Zone")));
				break;
		}

		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, FString::Printf(TEXT("Last in zone: %f"), TimeSinceLastInZone));
	}
}

void ABall::MeshChangeTick(float DeltaTime)
{
	if (TimeSinceMeshChange < 100.0f)
		TimeSinceMeshChange += DeltaTime;
	if (TimeSinceMeshChange > 1.0f && CurrentZoneState == InNoZone)
	{
		if (UltraBall->GetPhysicsLinearVelocity().Size() >= SpeedAtWhichMeshTransitionsBackToComplex)
		{

			if (UltraBall->GetStaticMesh() != SimpleAsset)
				SetMesh(SimpleAsset);
		}
		else
		{
			if (UltraBall->GetStaticMesh() != ComplexAsset)
				SetMesh(ComplexAsset);
		}
	}
}

float ABall::SnapToCenterOfGravityTick()
{
	// Move the UltraBall Towards the Center of Gravity
	FVector Offset = CenterOfGravity - GetActorLocation();
	FVector DistanceToCenter = CenterOfGravity - GetActorLocation();

	if (DistanceToCenter.Size() < 10.0)
	{
		UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f), false);
		SetActorLocation(CenterOfGravity);
	}
	else
	{
		Offset = Offset.GetSafeNormal(1.0) * 800;
		UltraBall->SetAllPhysicsLinearVelocity(Offset, false);
	}

	return DistanceToCenter.Size();
}

void ABall::ResetChargeState()
{
	TimeSinceMeshChange = 0.0f;
	CurrentCharge = 0.0f;
	CurrentChargeUpTimePassed = 0.0f;
	Pointlight->SetIntensity(0.0f);
}

void ABall::SetMesh(UStaticMesh* MeshToUse)
{
	FVector LinearVelocity = UltraBall->GetPhysicsLinearVelocity();
	FVector AngularVelocity = UltraBall->GetPhysicsAngularVelocity();

	UltraBall->SetStaticMesh(MeshToUse);

	UltraBall->SetPhysicsLinearVelocity(LinearVelocity);
	UltraBall->SetPhysicsAngularVelocity(AngularVelocity);
}

