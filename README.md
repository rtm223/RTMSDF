# RTMSDF
## Unreal Engine 5 2D Signed Distance Field support
Importers for generating 2D SDFs from .svg source files and all Unreal-supported texture source files (.psd, .png, .tif etc)

Uses [MSDFGen](https://github.com/Chlumsky/msdfgen) for processing of SVG files


## Quick Start
1. Clone this repository into MyUE4Project/Plugins/RTMSDF or MYUE4Project/Plugins/RTM/RTMSDF
2. Compile for Editor as usual and launch
3. Open Project Settings > Engine > Interchange > Import Content > Content Import Settings > Pipeline Stacks > Texture
    * Add a new entry, after `DefaultTexturePipeline`, select `Interchange_RTMSDF_BitmapPipeline`.
    * If you cannot see the `Interchange_RTMSDF_BitmapPipeline` asset in the dropdown, make sure your filters allow you to see plugin content
    * Note that these settings are per-user so for other team members to use the importers you'll need to click Set As Default in the top right of the screen
5. Check Project Settings -> Plugins -> RTM SDF for the important settings
    * SVG Filename Suffix - .svg source files with this suffix will be imported and converted to MSDF, i.e. `T_MyFancyIcon_SDF.svg`
    * Bitmap Filename Suffix - bitmap files (.png, .psd, .bmp etc.) with this suffix will be imported and converted to SDF files, i.e. `T_MyLessFancyIcons_SDF.psd`
    * SVG Import settings are related to MSDFGen - see [MSDFGen](https://github.com/Chlumsky/msdfgen) for details - can be overridden for individual files
    * Bitmap Default Import Settings are for importing bitmaps and can be overridden for individual files. There is a version for single channel source files and multichannel source files
4. Create an svg, png or other image file with the approriate naming and import it to unreal

## Editing settings per-asset:
* In your imported asset, under Texture -> Advanced Settings -> Asset User Data you will be able to edit import settings for your texture
* These are identical to the settings in the project settings
* Changing a setting will not automatically update the texture - you must click Reimport to see any changes
* The SDF import process will force your Texture Compression settings - this is fine, you want uncompressed data for SDFs
* Note that by default Unreal does not differentiate between single channel PSDs and multichannel PSDs, so PSD source files are assumed to be single channel

## SVG Import Limitations
* MSDF uses skia to help parse out SVG files. Currently Skia support is Windows only
* There is a fallback parser for Mac / Linux that has not really been tested, and will likely produce worse results

## Bitmap Import Limitations
* Bitmap import will attept to detext if your source file is single channel or multichannel however
  * However UE4 doesn't handle this well and some file types (e.g. psd) will always be rgba
  * Change the compression settings to "Grayscale" for the texture to force a single channel SDF generation
* Bitmaps have 2 RGBA modes
  * Separate Channels - each channel will be trated as a separate SDF to give multiple SDFs in a single file
  * Preserve RGB - will keep the source RGB channels and produce a sdf on the alpha channel (this does not work very well right now)
