# Blueprint Function Library
`URTMSDF_FunctionLibrary` in module `RTMSDF`

The runtime blueprint function library provides access to various data from SDF-encoded textures. See functions under category `RTM > SDF` in the blueprint editor for more details

The functions are:
```cpp
static bool IsSDFTexture(const UTexture2D* texture);
static ERTMSDF_SDFFormat GetSDFFormat(const UTexture2D* texture);
static FIntPoint GetSourceDimensions(const UTexture2D* texture);
static bool IsSingleChannelSDFTexture(const UTexture2D* texture);
static bool IsMSDFTexture(const UTexture2D* texture);
static float GetSDFUVRange(const UTexture2D* texture);

static bool IsSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);
static ERTMSDF_SDFFormat GetSDFFormatFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);
static FIntPoint GetSourceDimensionsFromSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);
static bool IsSingleChannelSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);
static bool IsMSDFSoftTexture(const TSoftObjectPtr<UTexture2D>& softTexture);
static float GetSDFUVRangeFromSoftTexure(const TSoftObjectPtr<UTexture2D>& softTexture);
```


> TODO: Document these functions as a blueprint library 