# Importing SDFs from Bitmaps
The bitmap importer uses a custom algorithm that generates a distance field by detecting the distance to the edge of the shapes in the source bitmap file. This is superior all other algorithms I've encountered, which tend to find the distance to the nearest pixel on the other side of the shape. By detecting edges, the algorithm
- Is a more accurate representation of the shape
- Can account for feathered / anti-aliased edges on the source shape, rather than thresholded pixels

## Source Formats

RTMSDF can import from all bitmap filetypes that Unreal supports, including
- PNG
- TGA
- TIFF
- JPG
- PSD

> NOTE: Currently only 8-bit per channel source files are supported, other bit depths will fail

## Bitmap SDF formats
Bitmap SDFs can either be **Single Channel** or **Separate Channel** formats

### Single Channel
**Single Channel** SDFs are a greyscale texture holding 1 SDF. This is likely to be the most common kind of SDF you generate and will be the default if you import an single channel source file.

> NOTE: Unreal Engine currently treats all .psd as RGBA, even if they are in Greyscale mode. For now, the plugin assumes a PSD should be treated as single channel. If this is not the intent, you can change the SDF `Format` setting and reimport. Future reimports will respect that choice

### Separate Channels
**Separate Channels** SDFs allow you to specify how to treat each channel of the source data. Per-channel options are
- **SDF** - Produces an SDF in this channel
- **Source Data** - Copies the source data of that channel into the texture. This can be useful if you wish to have non-SDF data in a channel (such as a gradient or other masks for use in materials), or wish to keep the RGB data and encode an SDF into the Alpha channel
- **Discard** This channel will be left empty (all 0s in the case of R,G,B and all 1s in the case of Alpha)

## Other Generation Settings
See [Generating SDFs](./Index.md) for other generation settings