# RTMSDF - Changelog
Version numbering as [**Major** . **Minor** . **Patch**]
- **Major** : Significant changes that may break backwards compatibility
- **Minor** : Additional features and changes that should not break compatibility outside of minor deprecations
- **Patch** : Bug fixes, minor edits that do not change behaviour

## Unreleased Changes
> NOTE: The following changes are live in this branch, but have not been rolled into any release yet

[No Changes]

## [1.2.0] Material Function Update
Released 2026-04-06

### Changed
- Examples folder can be safely removed from the plugin to reduce clutter (all dependencies removed)

### Added
- Basic Materials for surface and decal domains, intended for in-world icons and to demonstrate that world space can use simple thresholding
- Material Functions for various SDF manipulation
  - [Display Functions](./Docs/MaterialFunctions/Display.md) - edge masks, strokes, glows, UV & widget scaling
  - [Combination Functions](./Docs/MaterialFunctions/Combinations.md) - unions, intersections, engraves, extrusions
  - [2D Shape Functions](./Docs/MaterialFunctions/Shapes.md) - a variety of procedural SDF shapes
  - [Color Blending Functions](./Docs/MaterialFunctions/ColorBlending.md) - some handy color mixing functions that assist in applying strokes and glows etc. without producing artifacts
- Updated basic example materials with callouts to the new Material Functions
- Widget Scaling example (vertex shader to expand a widget beyond its geometry to show a wide glow)
- Color Blending example (how to fix artifacts when blending between features with different alphas)


## [1.1.1] UE5.7 Preview Support
Released 2025-09-26

Note: only 5.7 builds have been created for this patch version
Previous UE versions should use 1.1.0

## [1.1.0] Improved Examples and Material Toolkit
Released 2025-09-19

### Changed
- Project Settings - defaults for multicahnnel bitmap sources can be a single channel SDF
- Notes and clarifications to existing examples
- Updated all Example Textures to have additional meta tags (for soft texture BPFL functions)
- Updated Example materials to use the custom FWidth

### Added
- Additional Texture Processing Material Functions (texture to edge mask, UV scaling, FWidth)
- Additional BPFL functions - getters for inverted, scaled to fit, and uv scaling calculations
- Simple base materials for taking an SDF texture and reproducing an icon suitable for UI, with or without an outline / stroke
- Updated examples, with an interactive example browser (see [Examples](./Docs/Examples/Index.md))
- material library API documentation (see [Material Functions](./Docs/MaterialFunctions/Index.md))


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