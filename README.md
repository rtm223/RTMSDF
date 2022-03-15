# RTMSDF
UE4 SDF Importers / Generators for SVG and Bitmaps

**NOTE**: This module is very much work in progress / preview and likely to change
Uses [MSDFGen](https://github.com/Chlumsky/msdfgen) for processing of SVG files


## Quick Start
1. Clone this repository into MyUE4Project/Plugins/RTMSDF or MYUE4Project/Plugins/RTM/RTMSDF
2. Compile for Editor as usual and launch
3. Check Project Settings -> Plugins -> RTM SDF for the important settings
  * SVG Filename Suffix - .svg source files with this suffix will be imported and converted to MSDF, i.e. T_MyFancyIcon_SDF.svg
  * Bitmap Filename Suffix - bitmap files (.png, .psd, .bmp etc.) with this suffix will be imported and converted to SDF files, i.e. T_MyLessFancyIcons_SDF.psd
  * SVG Import settings are related to MSDFGen - see [MSDFGen](https://github.com/Chlumsky/msdfgen) for details and - can be overridden for individual files
  * Bitmap Default Import Settings are for importing bitmaps and can be overridden for individual files
4. Create an svg file with the approriate naming and import it to unreal

## Editing settings per-asset:
* In your imported asset, under Texture -> Advanced Settings -> Asset User Data you will be able to edit import settings for your texture
* These are identical to the settings in the project settings
* Changing a setting will not automatically update the texture - you must click Reimport to see any changes

## SVG Import Limitations
* MSDFGen only reads a single path from the SVG file - combine paths to create more complex shapes
* Overlapping shapes are not supported - I am not sure if that's on my end or on MSDFGen's end

## Bitmap Import Limitations
* Bitmap import will attept to detext if your source file is single channel or multichannel however
  * However UE4 doesn't handle this well and some file types (e.g. psd) will always be rgba
  * Change the compression settings to "Grayscale" for the texture to force a single channel SDF generation
* Bitmaps have 2 RGBA modes
  * Separate Channels - each channel will be trated as a separate SDF to give multiple SDFs in a single file
  * Preserve RGB - will keep the source RGB channels and produce a sdf on the alpha channel (this does not work very well right now)
