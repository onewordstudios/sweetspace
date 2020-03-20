# SweetSpace

Linter Status [![Build Status](https://dev.azure.com/onewordstudios/sweetspace/_apis/build/status/sweetspace-lint?branchName=master)](https://dev.azure.com/onewordstudios/sweetspace/_build/latest?definitionId=2&branchName=master)

Compilation Status [![Build Status](https://dev.azure.com/onewordstudios/sweetspace/_apis/build/status/sweetspace-compile?branchName=master)](https://dev.azure.com/onewordstudios/sweetspace/_build/latest?definitionId=4&branchName=master)

![Sweetspace game logo](http://onewordstudios.fun/sweetspace/sweetspace_hero.png "Sweetspace - game logo")

Cosmic radiation is tearing through your ship, and you’re the only donuts who can stop it! Donut waste time as you roll and patch your way through these bite-sized holes in your ship. You’ll need to communicate and cooperate to achieve your dream of a sweet, sweet space.

A game developed by [onewordstudios](https://onewordstudios.fun) for CS 4152 at Cornell University.

Coming August 2020 to Android and iOS.

## Current Progress

### Technical Prototype

The primary objective for the Technical Prototype was the solve the largest technical hurdle of our game's development - networking. Our goal was to have a fixed number of players (3) be able to join together into the same ship and see each other's movements. This we achieved. We also managed to add primitive support for doors, which require two players to be up against in order to open. Much of the physics is placeholder and will be improved, as will the UI, network interpolation, and general game balance. Still, it seems to work.

Our game currently supports rolling around the ship in a group of three. While doing so, you can fix breaches of your color by rolling over it back and forth three times. You can also open locked doors by having two players both touch the door. You cannot interact with breaches of other colors - you'll need the correct colored player to come over and fix those. Tilt or use the arrow keys to roll left and right. Tap or click to jump (doesn't really have a purpose yet).

**MATCHMAKING INSTRUCTIONS**

The matchmaking is currently very fragile and will be improved greatly in future releases. For now, follow these steps. First, our free-tier Heroku Dyno sleeps after a period of inactivity, so go to [https://status.onewordstudios.fun/](https://status.onewordstudios.fun/) to make sure the server is up and running first. Next, one player should choose "Host" inside the app and give the other two players the game code. The other two players will need to click "Join" and enter the code into the textbox in the bottom right. On mobile, the onscreen keyboard may cover up the textbox; sorry. We've been fighting with the scene graph for a while now and this is the best we've gotten so far. You'll need to delete the placeholder text in there first and then enter the game code. When finished (tap outside to close the keyboard so you can see the textbox again), make sure the code is correct and then tap the "Join" button **once**. If you tap multiple times, the app may softlock. If you enter the wrong game code, the app *will* softlock. These are all things that we'll fix later, but for now are quite fragile. When all three players have joined, the game should automatically start.

There is no disconnect detection; everyone should just close the app when done. There is also no conflict resolution yet, so in the relatively unlikely but possible event that a player gets de-sync-ed, you'll have to restart the game.

### Gameplay Prototype

Our programming objective for the Gameplay Prototype was to have a way to test and balance the core movement mechanic of tilting one's phone to move their avatar around the ship. Our prototype successfully implemented the portions of our game that could not be modeled in our nondigital prototype into a CUGL project and provided us a way to tweak the physics constants. Only one sized level is available, and the prototype is currently only single-player, though we began laying the groundwork for multiplayer support.

Our game currently supports rolling around the ship as a single player and resolving issues. Tilt your phone left and right to roll; the further you tilt, the faster you roll. Tap on the screen when your donut is above a green circle to "fix" it. If using the desktop build, roll with the left / right arrow keys and click to emulate tapping.

## onewordstudios Team

<a href="https://onewordstudios.fun"><img src="https://xingmichael.com/School/flourish/studioSmall.png" alt="onewordstudios logo" width="200"/></a>

**Demi Chang** - Head Chef (Design Lead)

**Aashna Saxena** - Plant Master (Programmer)

**Samuel Sorenson** - AI Hackerman (Programmer)

**Michael Xing** - Repository Despot (Software Lead)

**Jeffrey Yao** - Donut Man (Programmer)

**Wenjia Zhang** - Project Overlord (Project Lead)

## Acknowledgements

Thanks to the [Game Design Initiative at Cornell University](https://gdiac.cis.cornell.edu/) for providing the Cornell University Game Library (CUGL) used for this project, as well as [Box2D](https://box2d.org/) for the physics engine.

Continuous Integration provided by [Azure DevOps](https://azure.microsoft.com/en-us/services/devops/).

## Contributions

As this is a class project, we are currently not accepting external contributions.

## Copyright

Copyright © 2020 onewordstudios
All Rights Reserved

As students ourselves, we are happy to make this repository public as a hopefully useful learning tool. We do still reserve all intellectual property rights. Thanks for visiting our repo!
