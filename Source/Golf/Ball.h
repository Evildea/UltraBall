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

	// Simple Mesh Asset of UltraBall with Sphere Colider.
	UPROPERTY(VisibleAnywhere)
	UStaticMesh* SimpleAsset;
	
	// Complex Mesh Asset of UltraBall with Dodecahedron Colider.
	UPROPERTY(VisibleAnywhere)
	UStaticMesh* ComplexAsset;

	// Current Mesh of UltraBall.
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* UltraBall;

	// Camera Controlled by the Player.
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	// Spring Arm that ensures the Camera doesn't crash into the walls and floor.
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	// Spot light that lights up red when Charge is being applied.
	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent* Pointlight;

	// Sound to play when UltraBall colides with the ground.
	UPROPERTY(EditAnywhere, Category = "Designer")
	class UAudioComponent* Sound;

	// Predictor Rings - These are used to draw where UltraBall will fire if the charge is applied.
	UPROPERTY()
	UStaticMeshComponent* PredictorRing01;

	UPROPERTY()
	UStaticMeshComponent* PredictorRing02;

	UPROPERTY()
	UStaticMeshComponent* PredictorRing03;

	UPROPERTY()
	UStaticMeshComponent* PredictorRing04;

	UPROPERTY()
	UStaticMeshComponent* PredictorRing05;

	UPROPERTY()
	UStaticMeshComponent* PredictorRing06;

	UPROPERTY()
	TArray<UStaticMeshComponent*> PredictorArray;

	// Set the Current Charge.
	UFUNCTION(BlueprintCallable)
	void setCurrentCharge(float CurrentCharge);

	// Set the Current Blackening.
	UFUNCTION(BlueprintCallable)
	void setCurrentBlackening(float CurrentBlackening);

	// Player Controller Function: Zoom In.
	UFUNCTION()
	void ZoomIn();

	// Player Controller Function: Zoom Out.
	UFUNCTION()
	void ZoomOut();

	// Player Controller Function: Start Applying Charge.
	UFUNCTION()
	void Fire();

	// Player Controller Function: Stop Applying Charge.
	UFUNCTION()
	void EndFire();

	// Player Controller Function: CAncel Applying Charge.
	UFUNCTION()
	void CancelFire();

	// Player Controller Function: Mouse Look Up and Down.
	UFUNCTION()
	void LookUp(float value);

	// Player Controller Function: Mouse Look Left to Right.
	UFUNCTION()
	void LookLeft(float value);

	// Player Controller Function: Space bar unattach the firing direction from the camera.
	UFUNCTION()
	void CameraLock();

	// Player Controller Function: Space bar reattach the firing direction to the camera.
	UFUNCTION()
	void CameraUnLock();

	// Widget: Return the current par and Max Par. This is used by the HUD Widget.
	UFUNCTION(BlueprintPure)
	int GetCurrentPar() { return CurrentPar; }

	UFUNCTION(BlueprintPure)
	int GetMaxPar() { return MaxParAllowed; }

	// Widget: Returns whether this is the last level.
	UFUNCTION(BlueprintPure)
	bool GetLastLevel() { return isLastLevel; }

	// Returns Whether the Player has run out of shots.
	UFUNCTION(BlueprintPure)
	bool GetIfOutOfShots() { return isFailLevelAllowed ? CurrentPar >= MaxParAllowed : false; }

	// Widget: Decrement the Par by one.
	UFUNCTION(BlueprintCallable)
	void DecrementFromPar(int Amount) { CurrentPar -= Amount; }

	// Widget: Return the current Charge. This is used by Blueprints.
	UFUNCTION(BlueprintPure)
	float GetCharge() { return CurrentCharge; }

	// Widget: Return if the player attempted an illegal shot. This is used by the HUD Widget.
	UFUNCTION(BlueprintPure)
	bool GetHasAttemptedShotWhileMoving() { return hasAttemptedShotWhileMoving; }

	// Called when UltraBall hits the Bumper.
	UFUNCTION()
	void BumperHit();

	// Called when UltraBall colides with the ground.
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Called by Blueprints when Charging has started. 
	UFUNCTION(BlueprintImplementableEvent)
	void StartCharging();

	// Called by Blueprints when Charging has ended.
	UFUNCTION(BlueprintImplementableEvent)
	void EndCharging();

	// Called by Blueprints when Blackening has started. 
	UFUNCTION(BlueprintImplementableEvent)
	void StartBlackening();

	// Called by Blueprints when Blackening has ended.
	UFUNCTION(BlueprintImplementableEvent)
	void EndBlackening();

	// Designer: Maximum Zoom Out Length of the Camera.
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MaxZoomOutLength;

	// Designer: Minimum Zoom In Length of the Camera.
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MinZoomInLength;

	// Designer: Maximum Zoom Speed of the Camera.
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float ZoomInSpeed;

	// Designer: Maximum Charge the UltraBall can reach when fully charged.
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.01", ClampMax = "30.0", UIMin = "0.01", UIMax = "30.0"))
	float MaxChargePossibleAtFullChargeUp;

	// Designer: At what speed the UltraBall should transition between Simple and Complex meshes.
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "2000.0", UIMin = "1.0", UIMax = "2000.0"))
	float SpeedAtWhichMeshTransitionsBackToComplex;

	// Designer: The maximum amount of Par for this level.
	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxParAllowed;

	// Designer: Whether this is the last level.
	UPROPERTY(EditAnywhere, Category = "Designer")
	bool isLastLevel;

	UFUNCTION(BlueprintCallable)
	void ZoneEnter(int ZoneType, FVector CenterOfGravity, FVector LaunchDirection, float LaunchPower);

private:

	// This enumerator determines what state UltraBall is in.
	enum FireStates { Idle, Charging };
	enum ChargeStates { HaveCharges, HaveNoCharges };
	enum LocationStates { OnTheGround, InTheAir };
	enum ZoneStates { InGravityZone, InLaunchZone, InNoZone };

	FireStates CurrentFireState;
	ChargeStates CurrentChargeState;
	LocationStates CurrentLocationState;
	ZoneStates CurrentZoneState;

	// Various temporary variables used for controlling UltraBall.
	float CurrentZoomAmount;
	float CurrentCharge;
	float CameraZoomAmountLock;
	float LaunchPower;
	float BlackeningAmount;
	bool isMeshChangeAllowed;
	bool isCameraLocked;
	bool hasAttemptedShotWhileMoving;
	bool hasPlayedSoundOnTheGroundBefore;
	bool isFailLevelAllowed;
	FVector CameraLocationLock;
	FRotator CameraAngleLock;
	FVector CenterOfGravity;
	FVector LaunchDirection;

	int CurrentPar;

	// Forces the components such as the arrow and spring arm to update.
	void UpdateComponents();

	// This function is called when changing to a new mesh.
	void SetMesh(UStaticMesh* MeshToUse);

	// This function is called when generating a predictor ring.
	void SetupRing(UStaticMeshComponent *Mesh);

	// This function sets the location of a predictor ring.
	void SetRing(UStaticMeshComponent *Mesh, FVector Location);

	// Timer: Allow Mesh changing again.
	FORCEINLINE void MeshChangeTimerExpired() { isMeshChangeAllowed = true; }

	// Timer: Allow level failing again.
	FORCEINLINE void FailLevelTimerExpired() { isFailLevelAllowed = true; }

	// Timer: Stop showing the "X" after the player attempted an illegal shot.
	FORCEINLINE void hasAttemptedShotWhileMovingTimerExpired() { hasAttemptedShotWhileMoving = false; }

	// Play a sound upon impact with the ground or a wall in front of UltraBall.
	void PlaySoundOnImpact(FVector EndLocation, bool isGroundLevel);

};