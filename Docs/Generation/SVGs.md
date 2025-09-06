# Importing SDFs from SVGs
> TODO 
## MSDF Overview
> TODO 
## Source Formats
> TODO 

## SDV SDF Formats
Bitmap SDFs can be **Single Channel**,  **Single Channel Pseudo**, **Multichannel** (MSDF) or **Multichannel Plus Alpha** formats

### Single Channel
**Single Channel** SDFs are a greyscale texture holding 1 true SDF

### Single Channel Pseudo
**Single Channel Pseudo** SDFs are a greyscale texture holding 1 SDF, that has corner features more akin to MSDF fields. This format is effectively an MSDF that has been "baked" down to a single channel. It is unlikely that this is the optimal format for any real use case, as the baking process effectively strips the benefits that MSDF has in handling sharp corners better than a true SDF.

### Multichannel (MSDF)
**Multichannel** SDFs hold an MSDF texture in the RGB channels. This can be converted to SDF at runtime using the median of the three channels. See the `MSDF Channel Median` and `Process MSDF Texture` material expressions, under `RTM > SDF > Textures` in the material editor. It is unlikely that this optimal format for any use case, (see comments in **Multichannel Plus Alpha**)

### Multichannel Plus Alpha
**Multichannel Plus Alpha** SDFs hold an MSDF in RGB and a true SDF in alpha. This is probably the most common format for imported SVGs because it allows the higher-precision corners of MSDF for the edge of your shape, but has the softer true SDF if you wish to do effects (glows, soft shadows, pulsing outlines etc.) in the distance field

> NOTE: As the **Multichannel** format has an empty alpha channel, using that channel for a true SDF has no impact on memory footprint and is effectively free

## SVG Generation Settings
The following settings are exposed for fine tuning of MSDF texture generation. It is honestly quite unlikely that you will need to use them often, as the default settings are pretty good. They are exposed for handling edge cases and potentially for allowing better defaults for certain art styles etc.
### Edge Coloring Mode
> TODO 
### Max Corner Angle
> TODO 
### Error Correction Mode
> TODO 
### Error Correction Deviation
> TODO 
### Min Error Improvement
> TODO 
## Other Generation Settings
See [Generating SDFs](./Index.md) for other generation settings