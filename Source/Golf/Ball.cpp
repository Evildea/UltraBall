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
#include "TimerManager.h"

ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for UltraBall
	UltraBall = CreateDefaultSubobject<UStaticMeshComponent>("UltraBall");

	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallSimple(TEXT("StaticMesh'/Game/Models/UltraBallS.UltraBallS'"));
	ConstructorHelpers::FObjectFinder<UStaticMesh> UltraBallComplex(TEXT("StaticMesh'/Game/Models/UltraBallC.UltraBallC'"));

	SimpleAsset = UltraBallSimple.Object;
	ComplexAsset = UltraBallComplex.Object;

	UltraBall->SetStaticMesh(ComplexAsset);
	UltraBall->SetSimulatePhysics(true);
	UltraBall->SetNotifyRigidBodyCollision(true);
	UltraBall->OnComponentHit.AddDynamic(this, &ABall::OnHit);
	UltraBall->SetAngularDamping(2.0f);
	RootComponent = UltraBall;

	// Apply Dynamic Material to UltraBall
	ConstructorHelpers::FObjectFinder<UMaterialInstance> Material(TEXT("MaterialInstanceConstant'/Game/Materials/UltraBall_MI.UltraBall_MI'"));
	if (Material.Succeeded())
		UltraBall->SetMaterial(0, Material.Object);

	// Setup the Predictor rings
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
	MinZoomInLength = 200.0f;							// The Min Zoom Out Distance of the Camera
	MaxZoomOutLength = 1000.0f;							// The Max Zoom Out Distance of the Camera
	ZoomInSpeed = 93.0f;								// The Speed at which the Camera can Zoom in and Out
	MaxChargePossibleAtFullChargeUp = 6.0f;				// The Maximum Charge the Ball can Reach
	SpeedAtWhichMeshTransitionsBackToComplex = 300.0f;	// The Speed/Velocity at which to Swap Back to a Complex Mesh from a Simple Mesh.
	MaxParAllowed = 20;									// The Maximum Allowed Par to win the Level

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
	CurrentZoomAmount = 0.0f;
	CurrentPar = 0;
	CurrentCharge = 0.0f;
	isCameraLocked = false;
	CameraZoomAmountLock = 0.0f;
	isGamePaused = false;
	isMeshChangeAllowed = false;
	hasAttemptedShotWhileMoving = false;
}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hide each Predictor Ring
	for (int i = 0; i < PredictorArray.Num(); i++)
		PredictorArray[i]->SetVisibility(false);

		//This section predicts what direction the shot will go.
	if (CurrentFireState == Charging)
	{

		FVector offset;
		if (isCameraLocked)
			offset = GetActorLocation() - CameraLocationLock;
		else
			offset = GetActorLocation() - Camera->GetComponentLocation();

		offset = offset.GetSafeNormal(1.0f) * UltraBall->GetMass() * CurrentCharge * 27.0f;
		FPredictProjectilePathParams Predictor;
		FPredictProjectilePathResult ProjectileResult;
		ECollisionChannel CollisionChannel = ECC_Visibility;

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

		// Project the Path
		UGameplayStatics::PredictProjectilePath(GetWorld(), Predictor, ProjectileResult);

		// Get the Location Data
		TArray<FPredictProjectilePathPointData> Locations;
		Locations = ProjectileResult.PathData;
		
		// Update Each Predictor Ring
		for (int i = 0; i < PredictorArray.Num(); i++)
			SetRing(PredictorArray[i], Locations[1 + (i * 2)].Location);
	}

	// Change to a Sphere Mesh Colider if UltraBall is moving fast and a Dodecahedron Mesh Colider if it's moving slow.
	MeshChangeTick(DeltaTime);

	// Update the Dynamic Material and Lights
	MaterialTick(DeltaTime);

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
	PlayerInputComponent->BindAction("CameraLock", IE_Pressed, this, &ABall::CameraLock);
	PlayerInputComponent->BindAction("CameraLock", IE_Released, this, &ABall::CameraUnLock);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ABall::Pause);

}

void ABall::setCurrentCharge(float CurrentCharge)
{
	this->CurrentCharge = CurrentCharge;
	float ReddishGlow = (1.0f / MaxChargePossibleAtFullChargeUp) * CurrentCharge;
	UltraBall->SetScalarParameterValueOnMaterials("Power", ReddishGlow);
	Pointlight->SetIntensity(ReddishGlow * 9000.0f);
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
	// Start Charging UltraBall if UltraBall isn't moving.
	if (UltraBall->GetPhysicsLinearVelocity().Size() <= 5.0f && UltraBall->GetPhysicsAngularVelocity().Size() <= 5.0f)
	{
		StartCharging();
		CurrentFireState = Charging;
	}
	else
	{
		hasAttemptedShotWhileMoving = true;
		FTimerHandle AttemptedShotTimer;
		GetWorldTimerManager().SetTimer(AttemptedShotTimer, this, &ABall::hasAttemptedShotWhileMovingTimerExpired, 1.0f);
	}
}

void ABall::EndFire()
{
	// Cancel Charging UltraBall if UltraBall is current Charging.
	if (CurrentFireState == Charging)
	{
		// Charge back to an Idle Charge State.
		CurrentFireState = Idle;

		// Load the Simple Mesh to stop UltraBall from flying around randomly and set the timer for allowing a swap back to the complex mesh to at least 1 second.
		UltraBall->SetStaticMesh(SimpleAsset);
		isMeshChangeAllowed = false;
		FTimerHandle MeshChangeTimer;
		GetWorldTimerManager().SetTimer(MeshChangeTimer, this, &ABall::MeshChangeTimerExpired, 1.0f);

		// Calculate the launch direction for UltraBall.
		FVector LaunchDirection;
		if (isCameraLocked)
			LaunchDirection = UltraBall->GetComponentLocation() - CameraLocationLock;
		else
			LaunchDirection = UltraBall->GetComponentLocation() - Camera->GetComponentLocation();

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
	if (CurrentFireState == Charging)
	{
		CurrentFireState = Idle;
		EndCharging();
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
	CameraAngleLock = SpringArm->GetComponentRotation();
	CameraLocationLock = Camera->GetComponentLocation();
}

void ABall::CameraUnLock()
{
	isCameraLocked = false;
	SpringArm->SetRelativeRotation(CameraAngleLock);
}

FString ABall::GetParString()
{
	return FString::Printf(TEXT("par: %d/%d"), CurrentPar, MaxParAllowed);
}

FString ABall::GetFinishParString()
{
	if (GetIfOutsidePar())
	{
		if (CurrentPar - MaxParAllowed == 1)
			return FString::Printf(TEXT("the maximum par allowed is %d. you are %d shot over par."), MaxParAllowed, CurrentPar - MaxParAllowed);
		else
			return FString::Printf(TEXT("the maximum par allowed is %d. you are %d shots over par."), MaxParAllowed, CurrentPar - MaxParAllowed);
	}
	else
		return FString::Printf(TEXT("completed in %d out of %d shots"), CurrentPar, MaxParAllowed);
}

bool ABall::GetIfOutsidePar()
{
	return CurrentPar > MaxParAllowed;
}

float ABall::GetCharge()
{
	return CurrentCharge;
}

void ABall::Pause()
{
	isGamePaused = !isGamePaused;
}

void ABall::BumperHit()
{
	isMeshChangeAllowed = false;
}

void ABall::MeshChangeTimerExpired()
{
	isMeshChangeAllowed = true;
}

void ABall::hasAttemptedShotWhileMovingTimerExpired()
{
	hasAttemptedShotWhileMoving = false;
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
			EndLocation.Z -= 50;

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

bool ABall::GetHasAttemptedShotWhileMoving()
{
	return hasAttemptedShotWhileMoving;
}

void ABall::UpdateComponents()
{
	if (CurrentZoomAmount < MinZoomInLength) { CurrentZoomAmount = MinZoomInLength; }
	if (CurrentZoomAmount > MaxZoomOutLength) { CurrentZoomAmount = MaxZoomOutLength; }
	SpringArm->TargetArmLength = CurrentZoomAmount;
}

void ABall::MaterialTick(float DeltaTime)
{
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

void ABall::MeshChangeTick(float DeltaTime)
{
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
			{
				SetMesh(ComplexAsset);
			}
		}
	}
}

void ABall::SetMesh(UStaticMesh* MeshToUse)
{
	FVector LinearVelocity = UltraBall->GetPhysicsLinearVelocity();
	FVector AngularVelocity = UltraBall->GetPhysicsAngularVelocity();

	UltraBall->SetStaticMesh(MeshToUse);
	UltraBall->SetPhysicsLinearVelocity(LinearVelocity);
	UltraBall->SetPhysicsAngularVelocity(AngularVelocity);
}

void ABall::SetupRing(UStaticMeshComponent *Mesh)
{
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
	Mesh->SetWorldLocation(Location);
	Mesh->SetWorldRotation(SpringArm->GetComponentRotation());
	Mesh->SetVisibility(true);
}

