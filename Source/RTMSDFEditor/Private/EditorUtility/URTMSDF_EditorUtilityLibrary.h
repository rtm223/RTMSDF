// Copyright (c) Richard Meredith AB. All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "URTMSDF_EditorUtilityLibrary.generated.h"

UCLASS()
class RTMSDFEDITOR_API UURTMSDF_EditorUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category="RTM|SDF|EditorUtility", meta=(BlueprintInternalUseOnly, DeterminesOutputType="blueprintClass", AutoCreateRefTerm="referenceAsset"))
	static TArray<UClass*> FindBlueprintClasses(const FString& searchLocation , const UClass* blueprintClass);

	UFUNCTION(BlueprintCallable, Category="RTM|SDF|EditorUtility", meta=(BlueprintInternalUseOnly, DefaultToSelf="Widget", AutoCreateRefTerm="size"))
	static void SetUtilityWidgetWindowSize(UEditorUtilityWidget* widget, const FVector2D& size);
#endif
};
