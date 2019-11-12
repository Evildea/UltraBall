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

	UFUNCTION()
	void Pause();

	UFUNCTION()
	void ZoneEnter(FVector a_zoneLocation, FVector a_ZonelaunchDirection, float a_zonePower, int a_zoneType);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void BumperHit();

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

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.10", ClampMax = "0.30", UIMin = "0.10", UIMax = "0.30"))
	float Squishiness;

	UPROPERTY(EditAnywhere, Category = "Designer")
	int MaxParAllowed;

	// Sphere Coliders
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere1;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere2;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere3;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere4;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere5;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere6;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere7;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere8;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere9;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere10;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere11;
	UPROPERTY(EditAnywhere)
	USphereComponent* sphere12;

private:

	enum BallState {Idle, Charging};
	enum LauncherType {DeadZone, LaunchZone};

	BallState					CurrentStateOfBall;
	LauncherType				CurrentLauncherType;
	float						CurrentLauncherPower;
	int							CurrentSideOfBallDown;
	FVector						LocationOfGravityFreeze;
	FVector						LocationOfLauncherDirection;
	float						CurrentZoomAmount;
	float						CurrentCharge;
	float						ChargeUpTimePassed;
	int							NumberOfAirShotsTaken;
	bool						inTheAir;
	bool						hasSoundPlayed;
	bool						isInCentreOfGravityFreeze;
	float						inTheAirBurnOut;
	USphereComponent*			CurrentSideOfBallDownList[12];
	int							CurrentPar;
	bool						isCameraLocked;
	float						CameraZoomAmountLock;
	FRotator					CameraAngleLock;
	FVector						CameraLocationLock;
	bool						isGamePaused;
	float						TimeSinceMeshChange;

	FHitResult Result;
	FVector Start;
	FVector End;
	ECollisionChannel CollisionChannel;
	FVector AngularVelocity;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

	void GenerateSphere(int a_number, class USphereComponent* &a_sphere, FName a_name, FVector a_location);
	void GetSideFacing(int a_side);
	float ClampIt(float X1, float X2, float DeltaTime);

	void ZoneTick();
	void SquishTick(float DeltaTime);
	void MaterialTick(float DeltaTime);
	void inTheAirCheckTick();
	void ChargesRemainingCheckTick(float DeltaTime);

};