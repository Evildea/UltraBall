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

	// Predictor Paths
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing01;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing02;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing03;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing04;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing05;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PredictorRing06;

	UPROPERTY()
	TArray<UStaticMeshComponent*> PredictorArray;

	// Callable functions
	UFUNCTION(BlueprintPure)
	bool getGamePauseState() { return isGamePaused; }

	UFUNCTION(BlueprintCallable)
	void setGamePauseState(bool isPaused) { isGamePaused = isPaused; }

	UFUNCTION(BlueprintCallable)
	void setCurrentCharge(float CurrentCharge);

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

	UFUNCTION()
	void Pause();

	UFUNCTION()
	void BumperHit();

	void MeshChangeTimerExpired();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent)
	void StartCharging();

	UFUNCTION(BlueprintImplementableEvent)
	void EndCharging();

	// Designer Functionality
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MaxZoomOutLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MinZoomInLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float ZoomInSpeed;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.01", ClampMax = "30.0", UIMin = "0.01", UIMax = "30.0"))
	float MaxChargePossibleAtFullChargeUp;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "2000.0", UIMin = "1.0", UIMax = "2000.0"))
	float SpeedAtWhichMeshTransitionsBackToComplex;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxParAllowed;

	bool GetBurnedOutStatus();

private:

	enum FireStates {Idle, Charging};
	FireStates CurrentFireState;

	float CurrentZoomAmount;
	float CurrentCharge;
	bool isMeshChangeAllowed;
	bool isCameraLocked;
	bool isGamePaused;
	float CameraZoomAmountLock;
	FVector CameraLocationLock;
	FRotator CameraAngleLock;
	int CurrentPar;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

	void MaterialTick(float DeltaTime);
	void MeshChangeTick(float DeltaTime);

	void SetMesh(UStaticMesh* MeshToUse);
	void SetupRing(UStaticMeshComponent *Mesh);
	void SetRing(UStaticMeshComponent *Mesh, FVector Location);

};