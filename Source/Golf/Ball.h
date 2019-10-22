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
	UStaticMeshComponent* UltraBall;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent* Pointlight;

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
	float MaxPowerPossibleAtFullChargeUp;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "500.0", UIMin = "1.0", UIMax = "500.0"))
	float MaxDistanceOffGroundConsideredAir;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxNumberOfShotsAllowedInTheAir;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.10", ClampMax = "0.30", UIMin = "0.10", UIMax = "0.30"))
	float Squishiness;

	// Sphere Coliders
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* sphere1;
	class USphereComponent* sphere2;
	class USphereComponent* sphere3;
	class USphereComponent* sphere4;
	class USphereComponent* sphere5;
	class USphereComponent* sphere6;
	class USphereComponent* sphere7;
	class USphereComponent* sphere8;
	class USphereComponent* sphere9;
	class USphereComponent* sphere10;
	class USphereComponent* sphere11;
	class USphereComponent* sphere12;

private:

	enum BallState {Idle, Charging};
	enum BallSide { none, side1, side2, side3, side4, side5, side6, side7, side8, side9, side10, side11, side12 };

	BallState CurrentBallState;
	BallSide CurrentSideTouched;
	FVector TargetSquishAmount;
	float ZoomLength;
	float Power;
	float ChargeUpTimePassed;
	int NumberOfAirShotsTaken;
	bool inTheAir;
	float inTheAirBurnOut;

	FHitResult Result;
	FVector Start;
	FVector End;
	ECollisionChannel CollisionChannel;
	FVector AngularVelocity;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

	void GenerateSphere(class USphereComponent* &a_sphere, FName a_name, FVector a_location);
	void GetSideFacing(class USphereComponent* &a_sphere, BallSide a_side);
	float ClampIt(float X1, float X2, float DeltaTime);
	

};