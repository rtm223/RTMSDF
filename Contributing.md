# Contributing

## Contributor's Agreement

There is no expectation for you to to provide any bug fixes, patches, or
upgrades to this project. However, if you choose to make changes directly to the project 
(i.e. as a pull request), without imposing a separate written license agreement for such
changes, then you hereby grant the following license to Richard Meredith AB: a non-exclusive,
royalty-free perpetual license to install, use, modify, prepare derivative works, incorporate
into other computer software, distribute, and sublicense such enhancements or derivative works
 thereof, in binary and source code form

## Open Source Development
This repository is intended to provide a free and easily-obtainable version of the core
functionality of my SDF generators. As such it is not really intended as an open-source,
collaborative project. As such, I am happy to accept pull requests for bug fixes, support
for alternate versions of Unreal, support for additional platforms, corrections to typos,
clarifications/corrections to documentation and other minor fixes, but am unlikely to 
accept larger features or more opinionated changes

## Raising Issues
If you discover issues with the functionality, feel free to 
[raise an issue](https://github.com/rtm223/RTMSDF/issues)  on github if there is not one already
there. Please include as much detail as possible and where possible / relevant please attach
source assets to reproduce (for example when a particular file is not importing correctly)

## Pull Requests
Please send pull reqests to the main branch

### Code Style
Follow the existing code style for code for whichever part of the source you are modifying

The core modules (**RTMSDF** and **RTMSDFEditor**) use a variant of the Epic Games code 
standard for Unreal Engine, with notable exceptions such as
- `camelCase` for locally scoped variables and function parameters
- Prefered use of `const` and `auto` keywords
  - Note use of `auto&`, `const auto&`, `auto*`, `const auto*` rather than plain `auto`
- Code that is not relevant to UHT is in `namespace RTM::SDF`
- Ommitting braces for single line code blocks

msdfgen, dropXML and skia (in the **ChlumskyMSDFGen** module) all use their own code styles. You'll just have to fumble your way 
through that, just try and match the existing code as best as possible

### Documentating Changes
If your pull request consists of a bug fix, or other notable user-facing change, please
update the [Changelog](CHANGELOG.md) in the unreleased changes section