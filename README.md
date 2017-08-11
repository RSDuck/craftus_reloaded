# Craftus 3D Reloaded

It's been a while since I started developing this version of Craftus. I started to do a rewrite, because the old Craftus was beyond fixable. Probably 15% is copied from the old Craftus the remaining part is rewritten.

## Features

* An infinte world(in theory)
* An awesome flat world
* One single block which can be placed down
* Saving and loading the world

## Known bugs

* I experienced once, that blocks, which are newly placed in a specific chunk weren't rendered anymore, but blocks placed in another chunk were, but I couldn't reproduce this bug. Maybe it's related to old savefiles
* Making a screenshot with Luma Rosalina what menu leads to crash

## Credits
* People who made 3DS homebrew possible
    * Especially smea for 3dscraft which is good place for ~~stealing ideas~~ inspiration
    * Contributors to the 3dbrew.org wiki
    * Finc for citro3d
* XSShepp, for the textures, taken from his [resourcepack](http://www.minecraftforum.net/forums/mapping-and-modding/resource-packs/1242533-pixel-perfection-now-with-polar-bears-1-11) which is licensed under [CC BY-SA](https://creativecommons.org/licenses/by-sa/4.0/)
* Tommaso Checchi, for his [culling algorithm](https://tomcc.github.io/2014/08/31/visibility-1.html), orginally developed for the use in MC PE
* All people who worked on the libraries Craftus is depending on(see the dependencies folder)