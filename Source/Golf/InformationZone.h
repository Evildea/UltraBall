// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InformationZone.generated.h"

UCLASS()
class GOLF_API AInformationZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInformationZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* TipZone;

	//UPROPERTY(EditAnywhere, Category = "Designer")
	//TSubclassOf<class UUserWidget> WidgetTemplate;

	//UPROPERTY(VisibleAnywhere)
	//class UUserWidget* WidgetInstance;

	UPROPERTY(EditAnywhere, Category = "Designer")
	UTextProperty* Title;

	UPROPERTY(EditAnywhere, Category = "Designer")
	UTextProperty* Content;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
