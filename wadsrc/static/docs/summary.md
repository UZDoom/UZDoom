# UZDoom ZScript Source Code Documentation

This is the documentation for the internal ZScript code within the UZDoom
engine, distributed with the engine via `uzdoom.pk3` - that's the stuff that
you, as a modder, need to interact with!

These pages therefore serve as a reference for all of the API surface in the
entire engine, or at least the ZScript bits.

Make sure to check the search bar up at the top of the page as it's often the
most efficient way of finding what you need!

Here's a couple of particularly important items:

- The **[Builtin Types](#builtins)** - Types that are built in to the ZScript
  language, representing the basic building blocks of the rest of the code
- **[`Actor`]** - The base class for practically anything visually in-world
- **[`Thinker`]** - The base class of `Actor`, representing things that tick in
  the game loop
- **[`Inventory`]** - The base class for `Actor`s that can be in another's
  inventory
- **[`EventHandler`]** - A flexible system for registering your own code so
  that it can react to "events" in the game engine

## Other Documentation

This site is API documentation for ZScript itself, and does not necessarily
comprise everything you might need to know about the engine. You will still
want to use the [ZDoom Wiki](https://zdoom.org/wiki/Main_Page), especially for
tutorials and stuff outside the scope of ZScript.

## Contributing

This site doesn't work like a wiki - it's totally generated from the UZDoom
GitHub repository with a tool called `zscdoc`. As such, contributing here
requires contributing to the repository, by adding "doc-comments" into the
source code.

We haven't set up a guide yet, but in the future one should be available on the
GitHub wiki or some other more developer-facing documentation location.
