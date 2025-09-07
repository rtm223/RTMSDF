# Using SDFs at Runtime
> TODO. SDF being a data format we can't use directly. Needs processing in shaders

## SDF Data
> TODO: A basic overview of what an SDF looks like, why that's useful

> TODO: A basic explainer on MSDF

> TODO: Comparing SDF to MSDF? The start is probably a pretty good example case

### Reconstructing a shape from SDF
> TODO: quick explainer on the math and then introduce the nodes
> - Process SDF Texture
> - Process MSDF Texture
> - MSDF Median
>
> Simplest form of recreating an edge, multiply by a large negative

## Examples
Check out the [Examples](../Examples/Index.md) in the plugin for more info how to use materials with SDF

## Blueprint Function Library
The plugin also comes with a [Blueprint Function Library](./BlueprintFunctionLibrary.md) that can provide 
various details about SDF textures that have been imported using the plugin, such as the SDF Format and 
UVRange (see [Generating SDFs](../Generation/Index.md) for more details on those properties)