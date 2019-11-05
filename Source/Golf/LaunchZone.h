// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchZone.generated.h"

UCLASS()
class GOLF_API ALaunchZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
		class USphereComponent* LaunchZone;

	UPROPERTY(VisibleAnywhere)
		class UParticleSystemComponent* ParticleSystem;

	UPROPERTY(VisibleAnywhere)
		class UArrowComponent* Arrow;

	UPROPERTY(VisibleAnywhere)
		class UPointLightComponent* Pointlight;

	UPROPERTY(EditAnywhere, Category = "Designer")
		class UAudioComponent* Sound;
	
	// Designer Functionality
	UPROPERTY(EditAnywhere, Category = "Designer", meta = (ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
		float EjectPower;

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
