// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h" 
#include "Components/PointLightComponent.h" 
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h" 
#include "Materials/MaterialInstanceDynamic.h" 
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/BodySetup.h"
#include "TimerManager.h"

ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for UltraBall
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");

	// Find the Simple and Complex models used by UltraBall. These models have different coliders applied.
	// UltraBallS uses a sphere colider. UltraBallC uses a Dodecahedron colider.
	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallSimple(TEXT("StaticMesh'/Game/Models/UltraBallS.UltraBallS'"));
	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallComplex(TEXT("StaticMesh'/Game/Models/UltraBallC.UltraBallC'"));

	// Assign the two meshes to Asset pointers that can be accessed later in the code.
	SimpleAsset = UltraBallSimple.Object;
	ComplexAsset = UltraBallComplex.Object;

	// Apply the Static Mesh and Setup the required bindings.
	UltraBall->SetStaticMesh(ComplexAsset);
	UltraBall->SetSimulatePhysics(true);
	UltraBall->SetNotifyRigidBodyCollision(true);
	UltraBall->OnComponentHit.AddDynamic(this, &ABall::OnHit);
	UltraBall->SetAngularDamping(2.0f);
	RootComponent = UltraBall;

	// Apply the Dynamic Material to UltraBall.
	ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("MaterialInstanceConstant'/Game/Textures/MaterialInstance/UltraBall_MI.UltraBall_MI'"));
	if (Material.Succeeded())
		UltraBall->SetMaterial(0, Material.Object);

	// Setup the six Predictor rings used to show where the UltraBall will go when fired.
	PredictorRing01 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing01");
	SetupRing(PredictorRing01);
	PredictorArray.Add(PredictorRing01);

	PredictorRing02 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing02");
	SetupRing(PredictorRing02);
	PredictorArray.Add(PredictorRing02);

	PredictorRing03 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing03");
	SetupRing(PredictorRing03);
	PredictorArray.Add(PredictorRing03);

	PredictorRing04 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing04");
	SetupRing(PredictorRing04);
	PredictorArray.Add(PredictorRing04);

	PredictorRing05 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing05");
	SetupRing(PredictorRing05);
	PredictorArray.Add(PredictorRing05);

	PredictorRing06 = CreateDefaultSubobject<UStaticMeshComponent>("PredictorRing06");
	SetupRing(PredictorRing06);
	PredictorArray.Add(PredictorRing06);

	// Setup the Sound Component that is called when the ball colides with the floor.
	Sound = CreateDefaultSubobject<UAudioComponent>("Sound");
	Sound->SetAutoActivate(false);
	Sound->SetupAttachment(RootComponent);

	// Setup the Spring Arm for the Camera.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("springarm");
	SpringArm->bAbsoluteRotation = true;
	SpringArm->SetupAttachment(RootComponent);

	// Setup the Camera used by the Viewport.
	Camera = CreateDefaultSubobject<UCameraComponent>("camera");
	Camera->SetupAttachment(SpringArm);

	// Setup the Point Light that represents the reddish glow.
	Pointlight = CreateDefaultSubobject<UPointLightComponent>("light");
	Pointlight->SetAttenuationRadius(300.0f);
	Pointlight->SetSourceRadius(40.0f);
	Pointlight->SetIntensity(0.0f);
	Pointlight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f));
	Pointlight->SetupAttachment(RootComponent);

	// Set up the inital values that can be modified by the Designer.
	MinZoomInLength = 200.0f;							// The Min Zoom Out Distance of the Camera
	MaxZoomOutLength = 1000.0f;							// The Max Zoom Out Distance of the Camera
	ZoomInSpeed = 93.0f;								// The Speed at which the Camera can Zoom in and Out
	MaxChargePossibleAtFullChargeUp = 6.0f;				// The Maximum Charge the Ball can Reach
	SpeedAtWhichMeshTransitionsBackToComplex = 300.0f;	// The Speed/Velocity at which to Swap Back to a Complex Mesh from a Simple Mesh.
	MaxParAllowed = 20;									// The Maximum Allowed Par to win the Level
	isLastLevel = false;								// Whether this UltraBall is on the last level.

	// Update the Camera based on their inital values.
	UpdateComponents();

	// Tell the game controller to possess this player.
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();

	// Setup default values.
	CurrentFireState = Idle;
	CurrentZoneState = InNoZone;
	CurrentLocationState = OnTheGround;
	CurrentChargeState = HaveCharges;
	CurrentZoomAmount = 0.0f;
	CurrentPar = 0;
	CurrentCharge = 0.0f;
	isCameraLocked = false;
	CameraZoomAmountLock = 0.0f;
	isMeshChangeAllowed = false;
	hasAttemptedShotWhileMoving = false;
	hasPlayedSoundOnTheGroundBefore = false;
	isFailLevelAllowed = true;
	BlackeningAmount = 0.0f;
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if UltraBall is in the Air or on the ground and reactivate the ability to play the bounce sound and reactivate charges.
	ECollisionChannel CollisionChannel = ECC_Visibility;
	FCollisionQueryParams CollisionParameters;
	FHitResult Result;
	CollisionParameters.AddIgnoredActor(this);
	FVector EndLocation = GetActorLocation();
	EndLocation.Z -= 100.0f;

	GetWorld()->LineTraceSingleByChannel(Result, GetActorLocation(), EndLocation, CollisionChannel, CollisionParameters, FCollisionResponseParams::DefaultResponseParam);
	if (Result.GetActor() == NULL)
		hasPlayedSoundOnTheGroundBefore = false;
	else
	{
		if (CurrentChargeState == HaveNoCharges)
		{
			CurrentChargeState = HaveCharges;
			EndBlackening();
		}
	}
	
	// Hide each Predictor Ring at the start of the tick.
	for (int i = 0; i < PredictorArray.Num(); i++)
		PredictorArray[i]->SetVisibility(false);

		// This section predicts what direction the shot will go roughly. It's only activated when the player attempts to fire.
	if (CurrentFireState == Charging)
	{

		// Determine what way to fire.
		FVector offset;
		if (isCameraLocked)
			offset = GetActorLocation() - CameraLocationLock;
		else
			offset = GetActorLocation() - Camera->GetComponentLocation();
		float ChargeAmount = CurrentCharge * MaxChargePossibleAtFullChargeUp;
		offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * ChargeAmount * 10.0f;

		// Setup the predictor.
		FPredictProjectilePathParams Predictor;
		FPredictProjectilePathResult ProjectileResult;
		ECollisionChannel CollisionChannel = ECC_Visibility;

		// Setup the initial variables used by the predictor.
		Predictor.StartLocation = GetActorLocation();
		Predictor.LaunchVelocity = offset;
		Predictor.bTraceComplex = true;
		Predictor.ProjectileRadius = 30.0f;
		Predictor.TraceChannel = CollisionChannel;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		Predictor.ActorsToIgnore = ActorsToIgnore;
		Predictor.SimFrequency = 12.0f;
		Predictor.MaxSimTime = 2.0f;

		// Project the Path.
		UGameplayStatics::PredictProjectilePath(GetWorld(), Predictor, ProjectileResult);

		// Get the Location Data.
		TArray<FPredictProjectilePathPointData> Locations;
		Locations = ProjectileResult.PathData;
		
		// Update Each Predictor Ring according to the location data.
		for (int i = 0; i < PredictorArray.Num(); i++)
			SetRing(PredictorArray[i], Locations[1 + (i * 2)].Location);
	}

	// Change to a Sphere Mesh Colider if UltraBall is moving too fast and a Dodecahedron Mesh Colider if it's moving too slow.
	if (isMeshChangeAllowed)
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

	// Update the Dynamic Material and Lights.
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
	UltraBall->SetScalarParameterValueOnMaterials("Blackening", BlackeningAmount);

	// If in a Gravity Zone
	if (CurrentZoneState == InGravityZone)
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
	}

	// If in a Launcher Zone
	if (CurrentZoneState == InLaunchZone)
	{
		// Move the UltraBall Towards the Center of Gravity
		FVector Offset = CenterOfGravity - GetActorLocation();
		FVector DistanceToCenter = CenterOfGravity - GetActorLocation();

		if (DistanceToCenter.Size() < 10.0)
		{
			SetActorLocation(CenterOfGravity);
			CurrentZoneState = InNoZone;
			UltraBall->SetEnableGravity(true);
			UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
			UltraBall->AddImpulse(LaunchDirection * LaunchPower);
		}
		else
		{
			Offset = Offset.GetSafeNormal(1.0) * 800;
			UltraBall->SetAllPhysicsLinearVelocity(Offset, false);
		}
	}

}

// Called to bind functionality to input
void ABall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup all bindings used for controlling UltraBall.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABall::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABall::EndFire);
	PlayerInputComponent->BindAction("CancelFire", IE_Pressed, this, &ABall::CancelFire);
	PlayerInputComponent->BindAxis("LookUp", this, &ABall::LookUp);
	PlayerInputComponent->BindAxis("LookLeft", this, &ABall::LookLeft);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ABall::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ABall::ZoomOut);
	PlayerInputComponent->BindAction("CameraLock", IE_Pressed, this, &ABall::CameraLock);
	PlayerInputComponent->BindAction("CameraLock", IE_Released, this, &ABall::CameraUnLock);

}

void ABall::setCurrentCharge(float CurrentCharge)
{
	// Set the current charge to be applied to UltraBall.
	this->CurrentCharge = CurrentCharge;

	// Update the Dynamic Material and the internal light.
	float ReddishGlow = (1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge;
	UltraBall->SetScalarParameterValueOnMaterials("Power", ReddishGlow);
	Pointlight->SetIntensity(ReddishGlow * 9000.0f);
}

void ABall::setCurrentBlackening(float CurrentBlackening)
{
	BlackeningAmount = CurrentBlackening;
}

void ABall::ZoomIn()
{
	// Zoom the Camera in.
	CurrentZoomAmount -= (CurrentZoomAmount / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::ZoomOut()
{
	// Zoom the Camera out.
	CurrentZoomAmount += (CurrentZoomAmount / (100.0f - ZoomInSpeed));
	UpdateComponents();
}

void ABall::Fire()
{
	// If UltraBall still has charges then allow the charging of UltraBall.
	if (CurrentChargeState == HaveCharges && CurrentPar != MaxParAllowed)
	{
		StartCharging();
		CurrentFireState = Charging;
	}
	else
	{
		// If UltraBall doesn't have chrges, then set "hasAttemptedShotWhileMoving" to true. This will draw a "X" to the screen until the timer has expired
		// to inform the player that they attempted an illegal move.
		hasAttemptedShotWhileMoving = true;
		FTimerHandle AttemptedShotTimer;
		GetWorldTimerManager().SetTimer(AttemptedShotTimer, this, &ABall::hasAttemptedShotWhileMovingTimerExpired, 1.0f);
	}
}

void ABall::EndFire()
{
	if (CurrentFireState == Charging)
	{
		// Start Blackening Process.
		if (CurrentZoneState == InNoZone)
		{
			CurrentChargeState = HaveNoCharges;
			StartBlackening();
		}

		// Charge back to an Idle Charge State.
		CurrentFireState = Idle;
		CurrentZoneState = InNoZone;
		UltraBall->SetEnableGravity(true);

		// Load the Simple Mesh or the Complex mesh depending on the Charge going to be applied.
		// If the Charge is low use the Complex mesh otherwise use the Simple mesh.
		if (CurrentCharge > 0.1f)
			UltraBall->SetStaticMesh(SimpleAsset);
		else
			UltraBall->SetStaticMesh(ComplexAsset);

		// Set a timer so a mesh change can't happen again too soon.
		isMeshChangeAllowed = false;
		FTimerHandle MeshChangeTimer;
		GetWorldTimerManager().SetTimer(MeshChangeTimer, this, &ABall::MeshChangeTimerExpired, 1.0f);

		// Calculate the launch direction for UltraBall.
		FVector LaunchDirection;
		if (isCameraLocked)
			LaunchDirection = UltraBall->GetComponentLocation() - CameraLocationLock;
		else
			LaunchDirection = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();

		// Apply the charge to UltraBall as a Impulse.
		float ChargeAmount = CurrentCharge * MaxChargePossibleAtFullChargeUp;
		LaunchDirection = LaunchDirection.GetSafeNormal(1.0f) * UltraBall->GetMass() * ChargeAmount * 1000.0f;
		UltraBall->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 0.0f));
		UltraBall->AddImpulse(LaunchDirection);

		// Increase the Par.
		CurrentPar++;

		// Call the Blueprint EndCharging Event.
		EndCharging();
	}
}

void ABall::CancelFire()
{
	// Only proceed if the player is Charging UltraBall.
	if (CurrentFireState == Charging)
	{
		// Cancel the Charging.
		CurrentFireState = Idle;

		// Call the Blueprint EndCharging Event.
		EndCharging();
	}
}

void ABall::LookUp(float value)
{
	// Restrict how far up and down the Camera can look to stop control reversing when flipping over the axis.
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	if (cameraRotation.Pitch < -70)
		cameraRotation.Pitch = -70.0f;
	if (cameraRotation.Pitch > 36)
		cameraRotation.Pitch = 36.0f;

	// Apply the new Pitch.
	cameraRotation.Pitch += value;
	SpringArm->SetWorldRotation(cameraRotation);

}

void ABall::LookLeft(float value)
{
	// Apply the new Yaw.
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	cameraRotation.Yaw += value;
	SpringArm->SetWorldRotation(cameraRotation);
}

void ABall::CameraLock()
{
	// Lock the current direcion for shooting based on the camera and allow free movement of the camera.
	isCameraLocked = true;
	CameraAngleLock = SpringArm->GetComponentRotation();
	CameraLocationLock = Camera->GetComponentLocation();
}

void ABall::CameraUnLock()
{
	// Return the camera back to the locked position.
	isCameraLocked = false;
	SpringArm->SetRelativeRotation(CameraAngleLock);
}

void ABall::BumperHit()
{
	isMeshChangeAllowed = false;
	FTimerHandle MeshChangeTimer;
	GetWorldTimerManager().SetTimer(MeshChangeTimer, this, &ABall::MeshChangeTimerExpired, 1.0f);

	isFailLevelAllowed = false;
	FTimerHandle FailLevelTimer;
	GetWorldTimerManager().SetTimer(FailLevelTimer, this, &ABall::FailLevelTimerExpired, 1.0f);
}

void ABall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// This section attempts to play a sound if UltraBall colides with the ground.
	if (Sound != nullptr)
	{
		if (GetVelocity().Size() > 50.0f)
		{

			FVector Location = GetActorLocation();
			Location.Z -= 100.0f;
			PlaySoundOnImpact(Location, true);

			Location = GetActorLocation();
			Location.Z += 100.0f;
			PlaySoundOnImpact(Location, false);

			Location = GetActorLocation();
			Location.X += 150.0f;
			PlaySoundOnImpact(Location, false);

			Location = GetActorLocation();
			Location.X -= 150.0f;
			PlaySoundOnImpact(Location, false);

			Location = GetActorLocation();
			Location.Y += 150.0f;
			PlaySoundOnImpact(Location, false);

			Location = GetActorLocation();
			Location.Y -= 150.0f;
			PlaySoundOnImpact(Location, false);

		}
	}
}

void ABall::ZoneEnter(int ZoneType, FVector CenterOfGravity, FVector LaunchDirection, float LaunchPower)
{

	// Set Local Variables
	this->CenterOfGravity = CenterOfGravity;
	this->LaunchDirection = LaunchDirection;
	this->LaunchPower = LaunchPower * UltraBall->GetMass() * 1000.0f;

	// Gravity Zone Enter
	if (ZoneType == 0)
		CurrentZoneState = InGravityZone;

	// Gravity Launcher Enter
	if (ZoneType == 1)
		CurrentZoneState = InLaunchZone;

	// Disable All Gravity.
	UltraBall->SetEnableGravity(false);
	UltraBall->SetAllPhysicsLinearVelocity(FVector(0.0f), false);
	// UltraBall->SetAllPhysicsAngularVelocity(FVector(0.0f), false);

}

void ABall::UpdateComponents()
{
	// Updated the Spring Arms length to match the new Zoom settings.
	if (CurrentZoomAmount < MinZoomInLength) { CurrentZoomAmount = MinZoomInLength; }
	if (CurrentZoomAmount > MaxZoomOutLength) { CurrentZoomAmount = MaxZoomOutLength; }
	SpringArm->TargetArmLength = CurrentZoomAmount;
}

void ABall::SetMesh(UStaticMesh* MeshToUse)
{
	// Record the current physics.
	FVector LinearVelocity = UltraBall->GetPhysicsLinearVelocity();
	FVector AngularVelocity = UltraBall->GetPhysicsAngularVelocity();

	// Change the mesh.
	UltraBall->SetStaticMesh(MeshToUse);

	// Apply the previous physics to the new mesh.
	UltraBall->SetPhysicsLinearVelocity(LinearVelocity);
	UltraBall->SetPhysicsAngularVelocity(AngularVelocity);
}

void ABall::SetupRing(UStaticMeshComponent *Mesh)
{
	// Configure the Predictor Ring.
	ConstructorHelpers::FObjectFinder<UStaticMesh> Predictor(TEXT("StaticMesh'/Game/Models/M_aim_guide.M_aim_guide'"));
	Mesh->SetStaticMesh(Predictor.Object);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetVisibility(false);
	Mesh->SetRelativeScale3D(FVector(0.5f));
}

void ABall::SetRing(UStaticMeshComponent *Mesh, FVector Location)
{
	// Set the Predictor Rings World Location.
	Mesh->SetWorldLocation(Location);
	Mesh->SetWorldRotation(SpringArm->GetComponentRotation());
	Mesh->SetVisibility(true);
}

void ABall::PlaySoundOnImpact(FVector EndLocation, bool isGroundLevel)
{
	// Initialise Paramaters for a Ray Trace.
	FVector StartLocation = GetActorLocation();
	ECollisionChannel CollisionChannel = ECC_Visibility;
	FCollisionQueryParams CollisionParameters;
	FHitResult Result;
	CollisionParameters.AddIgnoredActor(this);

	// Setup Parameters for the Ray Trace.
	GetWorld()->LineTraceSingleByChannel(Result, StartLocation, EndLocation, CollisionChannel, CollisionParameters, FCollisionResponseParams::DefaultResponseParam);
	if (isGroundLevel)
	{
		if (Result.GetActor() != NULL && !hasPlayedSoundOnTheGroundBefore)
		{
			// Play the bounce sound.
			Sound->Play();
			Sound->SetVolumeMultiplier(0.001f * GetVelocity().Size());
			hasPlayedSoundOnTheGroundBefore = true;
		}
	}
	else
	{
		if (Result.GetActor() != NULL)
		{
			// Play the bounce sound.
			Sound->Play();
			Sound->SetVolumeMultiplier(0.001f * GetVelocity().Size());
			hasPlayedSoundOnTheGroundBefore = true;
		}
	}

}

