// Fill out your copyright notice in the Description page of Project Settings.

#include "Ball.h"
#include "UObject/ConstructorHelpers.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h" 
#include "Camera/CameraComponent.h" 
#include "GameFramework/SpringArmComponent.h" 

// Sets default values
ABall::ABall()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup static mesh for ball
	Dodecahedron = CreateDefaultSubobject<UStaticMeshComponent>("Dodecahedron");
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DodecaMesh(TEXT("StaticMesh'/Game/Models/Dodeca.Dodeca'"));
	if (DodecaMesh.Succeeded())
	{
		UStaticMesh* Asset = DodecaMesh.Object;
		Dodecahedron->SetStaticMesh(Asset);
		Dodecahedron->SetSimulatePhysics(true);
		RootComponent = Dodecahedron;
	}

	// Setup static mesh for the arrow
	//Arrow = CreateDefaultSubobject<UStaticMeshComponent>("Arrow");
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMesh(TEXT("StaticMesh'/Game/Models/Arrow.Arrow'"));
	//if (ArrowMesh.Succeeded())
	//{
	//	UStaticMesh* Asset2 = ArrowMesh.Object;
	//	Arrow->SetStaticMesh(Asset2);
	//	Arrow->SetSimulatePhysics(false);
	//	Arrow->SetCollisionProfileName(FName("NoCollision"));
	//	Arrow->AddRelativeLocation(FVector(70.0f, 0.0f, 0.0f));
	//	Arrow->SetupAttachment(Dodecahedron);
	//	Arrow->bAbsoluteRotation = true;
	//	Arrow->CastShadow = false;
	//}

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
	ChargeUpSpeed = 1.0f;
	ChargeUpPower = 1.0f;
	//ArrowLock = false;
	isCharging = false;
	isDisableFireRelease = false;

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
	if (isCharging && !isDisableFireRelease)
	{
		// Charge up the Power.
		Power += (ChargeUpSpeed * DeltaTime);
		if (Power > 100.0f)
			Power = 100.0f;

		

		// Rotate the arrow based on the direction that the Dodecahedron is facing when powering up.
		//Arrow->SetVisibility(true, false);

		/*FVector offset = Dodecahedron->GetComponentLocation() - Camera->GetComponentLocation();
		offset = offset.GetSafeNormal(1.0f) * 200.0f;
		Arrow->SetWorldLocation(GetActorLocation() + offset);

		FRotator rotation = Camera->GetComponentRotation();
		rotation.Pitch = 0.0f;
		Arrow->SetRelativeRotation(rotation);*/
	}
	//else
	//	Arrow->SetVisibility(false, false);

}

// Called to bind functionality to input
void ABall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Start Charge Up and End Charge Up for firing.
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABall::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABall::Fire);

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
	isCharging = !isCharging;

	if (!isDisableFireRelease)
	{
		if (isCharging)
		{
			UE_LOG(LogTemp, Warning, TEXT("Start Charge Up"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("End Charge Up and Fire"));

			FVector offset = Dodecahedron->GetComponentLocation() - Camera->GetComponentLocation();
			offset = offset.GetSafeNormal(1.0f) * Power * (100000000.0f * ChargeUpPower);
			Dodecahedron->ComponentVelocity = FVector(0.0f, 0.0f, 0.0f);
			Dodecahedron->AddForce(offset);
			Power = 0.0f;
		}
	}

	if (!isCharging)
		isDisableFireRelease = false;
}

void ABall::CancelFire()
{
	UE_LOG(LogTemp, Error, TEXT("Charge Up Cancelled"));
	isDisableFireRelease = true;
}

void ABall::LookUp(float value)
{
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	cameraRotation.Pitch += value;
	SpringArm->SetWorldRotation(cameraRotation);
}

void ABall::LookLeft(float value)
{
	FRotator cameraRotation = SpringArm->GetComponentRotation();
	cameraRotation.Yaw += value;
	SpringArm->SetWorldRotation(cameraRotation);
}

//void ABall::ArrowLockUnlock()
//{
//	ArrowLock = !ArrowLock;
//	UE_LOG(LogTemp, Warning, TEXT("ArrowLock"));
//}

void ABall::UpdateComponents()
{
	if (ZoomLength < MinZoomInLength) { ZoomLength = MinZoomInLength; }
	if (ZoomLength > MaxZoomOutLength) { ZoomLength = MaxZoomOutLength; }
	//Arrow->SetRelativeScale3D(FVector(ZoomLength / 10.0f, 15.0f, 15.0f));
	SpringArm->TargetArmLength = ZoomLength;
}

