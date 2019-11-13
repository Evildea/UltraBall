// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/SphereComponent.h" 
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

	UPROPERTY(VisibleAnywhere)
	UStaticMesh* SimpleAsset;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMesh* ComplexAsset;

	// Visible Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* UltraBall;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent* Pointlight;

	UPROPERTY(EditAnywhere, Category = "Designer")
	class UAudioComponent* Sound;

	// Callable functions
	UFUNCTION(BlueprintCallable)
	bool getGamePauseState() { return isGamePaused; }

	UFUNCTION(BlueprintCallable)
	void setGamePauseState(bool isPaused) { isGamePaused = isPaused; }

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
	void CameraLock();

	UFUNCTION()
	void CameraUnLock();

	UFUNCTION(BlueprintCallable)
	FString GetParString();

	UFUNCTION(BlueprintCallable)
	FString GetFinishParString();

	UFUNCTION(BlueprintCallable)
	float GetCharge();

	UFUNCTION(BlueprintCallable)
	bool GetBurnedOutStatus();

	UFUNCTION()
	void Pause();

	UFUNCTION()
	void ZoneEnter(int ZoneType, FVector CenterOfGravity, FVector LaunchDirection, float LaunchPower);

	UFUNCTION()
	void BumperHit();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

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
	float MaxChargePossibleAtFullChargeUp;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "2000.0", UIMin = "1.0", UIMax = "2000.0"))
	float SpeedAtWhichMeshTransitionsBackToComplex;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "500.0", UIMin = "1.0", UIMax = "500.0"))
	float MaxDistanceOffGroundConsideredAir;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxNumberOfShotsAllowedInTheAir;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxParAllowed;

	UPROPERTY(EditAnywhere, Category = "Designer")
	bool IsInDebugMode;

private:

	enum FireStates {Idle, Charging};
	enum ChargeStates {HaveCharges, HaveNoCharges};
	enum LocationStates {OnTheGround, InTheAir};
	enum ZoneStates {InDeadZone, InLaunchZone, InNoZone};

	FireStates CurrentFireState;
	ChargeStates CurrentChargeState;
	LocationStates CurrentLocationState;
	ZoneStates CurrentZoneState;

	float CurrentZoomAmount;
	float CurrentChargeUpTimePassed;
	float CurrentCharge;
	float inAirBlackenAmount;
	float TimeSinceMeshChange;
	float TimeSinceLastInZone;
	float TimeSinceAttemptedFire;
	bool StartTimerSinceLastInZone;
	bool IsPowerIterating;

	bool isCameraLocked;
	float CameraZoomAmountLock;
	FVector CameraLocationLock;
	FRotator CameraAngleLock;

	int CurrentPar;
	int CurrentShotsTakenInTheAir;

	bool isGamePaused;

	FVector CenterOfGravity;
	FVector LaunchDirection;
	float LaunchPower;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

	void ZoneTick(float DeltaTime);
	void MaterialTick(float DeltaTime);
	void LocationTick();
	void DebugTick();
	void MeshChangeTick(float DeltaTime);
	float SnapToCenterOfGravityTick();


	void ResetChargeState();
	void SetMesh(UStaticMesh* MeshToUse);

};