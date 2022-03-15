// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "RTMSDF_SettingsStructCustomization.h"
#include "IDetailChildrenBuilder.h"

void FRTMSDF_SettingsStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> structPropertyHandle, IDetailChildrenBuilder& structBuilder, IPropertyTypeCustomizationUtils& structCustomizationUtils)
{
	uint32 numChildren(0);
	structPropertyHandle->GetNumChildren(numChildren);

	for(uint32 i = 0; i < numChildren; i++)
	{
		TSharedPtr<IPropertyHandle> childProperty = structPropertyHandle->GetChildHandle(i);
		if(childProperty.IsValid())
			structBuilder.AddProperty(childProperty.ToSharedRef());
	}
}
