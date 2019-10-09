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

	//UPROPERTY(VisibleAnywhere)
	//UStaticMeshComponent* Arrow;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	// Callable functions
	UFUNCTION(BlueprintCallable)
	void ZoomIn();

	UFUNCTION(BlueprintCallable)
	void ZoomOut();

	UFUNCTION()
	void Fire();

	UFUNCTION()
	void CancelFire();

	UFUNCTION()
	void LookUp(float value);

	UFUNCTION()
	void LookLeft(float value);

	//UFUNCTION()
	//void ArrowLockUnlock();

	// Designer Functionality
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MaxZoomOutLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "100.0", ClampMax = "10000.0", UIMin = "100.0", UIMax = "10000.0"))
	float MinZoomInLength;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float ZoomInSpeed;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.1", ClampMax = "5.0", UIMin = "0.1", UIMax = "5.0"))
	float ChargeUpSpeed;

	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
	float ChargeUpPower;

private:

	float ZoomLength;
	float Power;
	//bool ArrowLock;

	bool isCharging;
	bool isDisableFireRelease;

	// Forces the components such as the arrow and spring arm to update
	void UpdateComponents();

};
