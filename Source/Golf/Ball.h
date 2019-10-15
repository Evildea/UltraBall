// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Ball.generated.h"

UCLASS()
class GOLF_API ABall : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Visible Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Dodecahedron;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	// Callable functions
	UFUNCTION()
	void ZoomIn();

	UFUNCTION()
	void ZoomOut();

	UFUNCTION()
	void Fire();

	UFUNCTION()
	void EndFire();

	UFUNCTION()
	void CancelFire();

	UFUNCTION()
	void LookUp(float value);

	UFUNCTION()
	void LookLeft(float value);

	UFUNCTION()
	void DeadZoneFreeze();

	// Designer Functionality
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MaxZoomOutLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MinZoomInLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float ZoomInSpeed;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.01", ClampMax = "10.0", UIMin = "0.01", UIMax = "10.0"))
	float TimeNeededToReachFullChargeUp;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.01", ClampMax = "30.0", UIMin = "0.01", UIMax = "30.0"))
	float FullChargeUpPower;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "500.0", UIMin = "1.0", UIMax = "500.0"))
	float MaxDistanceOffGroundConsideredAir;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxNumberOfShotsAllowedInTheAir;

private:

	enum BallState {Idle, Charging, CancelCharging};

	BallState CurrentBallState;
	float ZoomLength;
	float Power;
	float ChargeUpTimePassed;
	int NumberOfAirShotsTaken;
	bool inTheAir;

	FHitResult Result;
	FVector Start;
	FVector End;
	ECollisionChannel CollisionChannel;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

};