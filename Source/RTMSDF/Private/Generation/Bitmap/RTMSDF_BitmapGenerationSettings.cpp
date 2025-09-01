// Copyright (c) Richard Meredith AB. All Rights Reserved

#include "Generation/Bitmap/RTMSDF_BitmapGenerationSettings.h"
#include "Module/RTMSDF.h"

void FRTMSDF_BitmapGenerationSettings::SetChannelBehavior(ERTMSDF_Channels channel, ERTMSDF_BitmapChannelBehavior behavior)
{
	// TODO - consider bitmask support
	switch(channel)
	{
		case ERTMSDF_Channels::Red:
			RedChannel = behavior;
			break;
		case ERTMSDF_Channels::Green:
			GreenChannel = behavior;
			break;
		case ERTMSDF_Channels::Blue:
			BlueChannel = behavior;
			break;
		case ERTMSDF_Channels::Alpha:
			AlphaChannel = behavior;
			break;

		default:
			const int enumIntValue = static_cast<int>(channel);
			const auto* uenumPtr = StaticEnum<ERTMSDF_Channels>();
			const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
			ensureAlwaysMsgf(false, TEXT("Unsupported channel requested ('%s' - %d)- skipping"), *enumName, enumIntValue);
	}
}

ERTMSDF_BitmapChannelBehavior FRTMSDF_BitmapGenerationSettings::GetChannelBehavior(ERTMSDF_Channels channel) const
{
	switch(Format)
	{
		case ERTMSDF_SDFFormat::SingleChannel: return channel == SDFChannel ? ERTMSDF_BitmapChannelBehavior::SDF : ERTMSDF_BitmapChannelBehavior::Discard;
		case ERTMSDF_SDFFormat::SeparateChannels: return GetSeparatedChannelBehavior(channel);

		default:
			static_assert(static_cast<int>(ERTMSDF_SDFFormat::MAX) == 5);
			const int enumIntValue = static_cast<int>(Format);
			const auto* uenumPtr = StaticEnum<ERTMSDF_SDFFormat>();
			const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
			ensureAlwaysMsgf(false, TEXT("Unsupported Format requested ('%s' - %d)- skipping"), *enumName, enumIntValue);

			return ERTMSDF_BitmapChannelBehavior::Discard;
	}
}

ERTMSDF_BitmapChannelBehavior FRTMSDF_BitmapGenerationSettings::GetSeparatedChannelBehavior(ERTMSDF_Channels channel) const
{
	check(Format == ERTMSDF_SDFFormat::SeparateChannels)

	switch(channel)
	{
		case ERTMSDF_Channels::Red: return RedChannel;
		case ERTMSDF_Channels::Green: return GreenChannel;
		case ERTMSDF_Channels::Blue: return BlueChannel;
		case ERTMSDF_Channels::Alpha: return AlphaChannel;
		case ERTMSDF_Channels::None: return ERTMSDF_BitmapChannelBehavior::Discard;

		default:
			const int enumIntValue = static_cast<int>(channel);
			const auto* uenumPtr = StaticEnum<ERTMSDF_Channels>();
			const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
			ensureAlwaysMsgf(false, TEXT("Unsupported channel requested ('%s' - %d)- skipping"), *enumName, enumIntValue);

			return ERTMSDF_BitmapChannelBehavior::Discard;
	}
}

ERTMSDF_Channels FRTMSDF_BitmapGenerationSettings::GetChannelMapping(ERTMSDF_Channels sdfChannel) const
{
	switch(Format)
	{
		case ERTMSDF_SDFFormat::SingleChannel: return SDFChannel;
		case ERTMSDF_SDFFormat::SeparateChannels: return sdfChannel;

		default:
			static_assert(static_cast<int>(ERTMSDF_SDFFormat::MAX) == 5);
			const int enumIntValue = static_cast<int>(Format);
			const auto* uenumPtr = StaticEnum<ERTMSDF_SDFFormat>();
			const FString enumName = uenumPtr->GetNameStringByValue(enumIntValue);
			ensureAlwaysMsgf(false, TEXT("Unsupported Format requested ('%s' - %d)- skipping"), *enumName, enumIntValue);

			return ERTMSDF_Channels::None;
	}
}

bool FRTMSDF_BitmapGenerationSettings::LooksLikePreserveRGB() const
{
	return GetChannelBehavior(ERTMSDF_Channels::Red) == ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Green) == ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Blue) == ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Alpha) == ERTMSDF_BitmapChannelBehavior::SDF;
}

bool FRTMSDF_BitmapGenerationSettings::CanScaleSDFTexture() const
{
	return GetChannelBehavior(ERTMSDF_Channels::Red) != ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Green) != ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Blue) != ERTMSDF_BitmapChannelBehavior::SourceData
		&& GetChannelBehavior(ERTMSDF_Channels::Alpha) != ERTMSDF_BitmapChannelBehavior::SourceData;
}

void FRTMSDF_BitmapGenerationSettings::FixUpVersioning()
{
	if(VersionNumber >= CurrentVersionNumber)
		return;

	UE_LOG(RTMSDF, Log, TEXT("%s : Fixing up asset version number %d -> %d"), ANSI_TO_TCHAR(__FUNCTION__), VersionNumber, CurrentVersionNumber);

	if(VersionNumber < 1)
	{
		if(!bIsInProjectSettings)
			bScaleToFitDistance = false;

		if(NumSDFChannels == 1)
		{
			Format = ERTMSDF_SDFFormat::SingleChannel;
			// Nothing else to change as the SDF channel field has remained the same
		}
		else if(NumSourceChannels > 1)
		{
			if(RGBAMode == ERTMSDF_RGBAMode::PreserveRGB)
			{
				Format = ERTMSDF_SDFFormat::SeparateChannels;
				RedChannel = ERTMSDF_BitmapChannelBehavior::SourceData;
				BlueChannel = ERTMSDF_BitmapChannelBehavior::SourceData;
				GreenChannel = ERTMSDF_BitmapChannelBehavior::SourceData;
				AlphaChannel = ERTMSDF_BitmapChannelBehavior::SDF;
			}
			else if(RGBAMode == ERTMSDF_RGBAMode::SeparateChannels)
			{
				Format = ERTMSDF_SDFFormat::SeparateChannels;
				RedChannel = 0 != (SDFChannels & static_cast<uint8>(ERTMSDF_Channels::Red)) ? ERTMSDF_BitmapChannelBehavior::SDF : ERTMSDF_BitmapChannelBehavior::Discard;
				BlueChannel = 0 != (SDFChannels & static_cast<uint8>(ERTMSDF_Channels::Green)) ? ERTMSDF_BitmapChannelBehavior::SDF : ERTMSDF_BitmapChannelBehavior::Discard;
				GreenChannel = 0 != (SDFChannels & static_cast<uint8>(ERTMSDF_Channels::Blue)) ? ERTMSDF_BitmapChannelBehavior::SDF : ERTMSDF_BitmapChannelBehavior::Discard;
				AlphaChannel = 0 != (SDFChannels & static_cast<uint8>(ERTMSDF_Channels::Alpha)) ? ERTMSDF_BitmapChannelBehavior::SDF : ERTMSDF_BitmapChannelBehavior::Discard;
			}
			else
			{
				checkNoEntry();
			}
		}
	}

	VersionNumber = CurrentVersionNumber;
	return;
}
