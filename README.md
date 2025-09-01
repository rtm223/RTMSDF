# RTMSDF v1.0.0 [BETA]
## 2D Signed Distance Field support for Unreal Engine 5
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

## New for version 1.0.0
### Bitmap Importing
* Interchange Support (plus legacy support for projects not using Interchange)
* 4-50x speed increase on importing
* Low level API for runtime importing
* Support for wrapped / tiling SDFs

### SVG Importing
* Support for multiple shapes, different kinds of SVG shapes, overlapping shapes

### All SDFs
* Automatic rescaling of textures to fit distance field (no need to author assets with margins)
* Support for non-square SDFs
* Blueprint library to query SDF features (distance field range, source asset size, at runtime)
* [Untested] Support for Linux / Mac (though with limitations on SVGs, ee below)
* Much nicer thumbnails
* Many many many bugfixes for stability and handling edge cases

## Editing settings per-asset:
* In your imported asset, under Texture -> Advanced Settings -> Asset User Data you will be able to edit import settings for your texture
* These are identical to the settings in the project settings
* Changing a setting will not automatically update the texture - you must click Reimport to see any changes
* The SDF import process will force your Texture Compression settings - this is fine, you want uncompressed data for SDFs
* Note that by default Unreal does not differentiate between single channel PSDs and multichannel PSDs, so PSD source files are assumed to be single channel

## SVG Import Limitations
* MSDF uses skia to help parse out SVG files. Currently Skia support is Windows only
* There is a fallback parser for Mac / Linux that has not really been tested, and will likely produce worse results
