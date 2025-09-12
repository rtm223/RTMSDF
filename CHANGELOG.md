# RTMSDF - Changelog
Version numbering as [**Major** . **Minor** . **Patch**]
- **Major** : Significant changes that may break backwards compatibility
- **Minor** : Additional features and changes that should not break compatibility outside of minor deprecations
- **Patch** : Bug fixes, minor edits that do not change behaviour

## Unreleased Changes
> NOTE: The following changes are live in this branch, but have not been rolled into any release yet

### Changed
- Project Settings - defaults for multicahnnel bitmap sources can be a single channel SDF
- Notes and clarifications to existing examples
- Updated all Example Textures to have additional meta tags

### Added
- Additional Texture Processing Material Functions (texture to edge mask, UV scaling)
- Additional BPFL functions - getters for for inverted, scaled to fit, and uv scaling calculations
- Simple base materials for taking an SDF texture and reproducing an icon suitable for UI

## [1.0.0] Initial Release
Released 2025-09-08

### Added
#### Bitmap Importing
- Interchange Support (plus legacy support for projects not using Interchange)
- 4-50x speed increase on importing
- Support for wrapped / tiling SDFs
. Material Nodes for processing an SDF texture to -1 to +1 range
- Runtime generation of SDFs (low level API, requires user-implemented specifics)

#### SVG Importing
- Support for multiple shapes
- Support for different kinds of SVG shapes (path, rect, circle, ellipse, polygon)
- Support for overlapping shapes
. Material Nodes for processing an MSDF texture to -1 to +1 range
- **[UNTESTED]** Probably partial support on Win / Mac

#### All SDFs
- Automatic rescaling of textures to fit distance field (no need to author assets with margins)
- Support for non-square source files / SDFs
- Asset tagging to label specific properties of SDFs (distance field range, source asset dimensions, at runtime)
- Blueprint library to query SDF features (distance field range, source asset dimensions, at runtime)

### Documentation
- Installation instructions
- Overview of Generation / Import settings
- Overview of Runtime usage of SDFs
- Indexing of the Examples folder in the plugin
- Stubs for Blueprint and Material APIs (to be filled in in future versions)

### Changed
- Unreal Engine version support changed to 5.4+
  - Tested in 5.4, 5.5 and 5.6 engine releases
  - Should compile for earlier versions, possibly with some tweaks
  - NOTE: There appears to be some incompatibility with `ue5-main` as of 2025-09-01
- Both importers now override user-selected `TextureCompression` settings for consistent results
- Updated concepts of "Inverted" / "Non-Inverted" SDF and made consistent 
  - Non-inverted means that values below 0.5 are inside the shape, above 0.5 are outside the shape
  - Inverted means that values below 0.5 are outside the shape, above 0.5 are inside the shape
  - Both types of importer follow this convention
  - NOTE: Vanilla MSDFGen uses the opposite convention, but most SDF math sources assume outside to be positive
- Fancy new thumbnails styling

### Performance
- 4-50x speed increase on importing bitmaps

### Fixed
- Many many many bugfixes for stability and handling edge cases

## [0.1.0] Preview (2022-03-15)
### Added 
- Importer for bitmap source files (.psd, .png, etc.) to SDF
- Importer for svg files to MSDF (using [MSDFGen](https://github.com/Chlumsky/msdfgen))
- Thumbnail support for SDF files