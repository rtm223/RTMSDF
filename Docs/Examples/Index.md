# SDF Examples

The plugin contains various examples of how to work with SDF textures, which you can find in the Content Browser under `All/Plugins/Signed Distance Fields [RTMSDF] Content/Examples/`. If you cannot see this directory check the content browser settings that `Show Plugin Content` is checked. There are also a small selection of SDF textures, used in these examples

![](Images/Examples_ContentBrowser.png)

## Using the Examples
Most of the examples in the plugin are materials, with small amounts of annotation inside. These each all paired with a material instance to easily preview parameter changes, e.g.
- `M_RTMSDF_EGBasic_01_SDFIcon_Simple` - Material (prefix `M_`)
- `MI_RTMSDF_EGBasic_01_SDFIcon_Simple` - Material Instance (prefix `MI_`)

## Examples
### Textures
A small collection of textures that have been imported as SDFs. The source assets for these are included in `{PluginDir}/SourceAssets/Examples/Textures`

These textures are all from [Game-icons.net](https://game-icons.net/), created by Lorc and Delapouite

### Basic
These examples show some of the basic ways of using SDFs, including
- Rendering a flat icon from SDF data
- Adding strokes / outlines
- Glow / bloom effects
- Drop shadows
- Morphing between shapes

These examples are intentionally simple, to show a few examples of how to start working with your SDF assets, and give an overview of the math used to turn SDFs into graphical effects