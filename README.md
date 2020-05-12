# SweetSpace

Linter Status [![Build Status](https://dev.azure.com/onewordstudios/sweetspace/_apis/build/status/sweetspace-lint?branchName=master)](https://dev.azure.com/onewordstudios/sweetspace/_build/latest?definitionId=2&branchName=master)

Compilation Status [![Build Status](https://dev.azure.com/onewordstudios/sweetspace/_apis/build/status/sweetspace-compile?branchName=master)](https://dev.azure.com/onewordstudios/sweetspace/_build/latest?definitionId=4&branchName=master)

![Sweetspace game logo](http://onewordstudios.fun/sweetspace/sweetspace_hero.png "Sweetspace - game logo")

Cosmic radiation is tearing through your ship, and you’re the only donuts who can stop it! Donut waste time as you roll and patch your way through these bite-sized holes in your ship. You’ll need to communicate and cooperate to achieve your dream of a sweet, sweet space.

A game developed by [onewordstudios](https://onewordstudios.fun) for CS 4152 at Cornell University.

Coming August 2020 to Android and iOS.

## Current Progress

### Golden Master

This release was our "fix-the-tutorial" release.

While other balance and polish changes were made, the biggest goal was to adapt the tutorial according to player feedback. Starting from the easiest level in the Level Select screen will now walk the players through a series of levels that individually introduce and then slowly layer in new mechanics, hopefully making the game more learnable.

Numerous other small polish and UI changes can also be seen (and heard) throughout the game.

Controls are the same as before. Tilt (or use arrow keys) to move left and right. Tap (or click / press space) to jump. Roll over breaches of your own color repeatedly to fix them, get two players onto a door to open them, get two players to both jump on buttons to fix them, and when told to roll in a direction, get everyone doing that. Keep the ship health out of the red until the end of the level to win.

### Beta Release

With the core gameplay in place, this release was about tutorialization and polish.

The most notable improvement is the tutorials that now show when players start on easy mode. These tutorials are very much still a work in progress, and are a bit buggy, but represent our vision for how we plan to teach players to play our game. Each tutorial level introduces a single new mechanic, and our goal is to visually guide players into solving these new challenges, easing them into our game.

Numerous UI changes have happened, including a pause menu and the ability to continue to the next level after winning, as well as a restart button after losing. This allows players to actually play a continuous session spanning multiple levels, instead of being forced to restart after every level.

Apart from that, the remaining changes have all been visual and gameplay polish. Lots of balancing was done internally, though we anticipate external playtesting still being very necessary. There are still some bugs and placeholder assets here and there, though all of them are slowly but surely being phased out over the next few weeks.

Controls are the same as before. Tilt (or use arrow keys) to move left and right. Tap (or click / press space) to jump. Roll over breaches of your own color repeatedly to fix them, get two players onto a door to open them, get two players to both jump on buttons to fix them, and when told to roll in a direction, get everyone doing that. Keep the ship health out of the red until the end of the level to win.

### Pre-Beta Release

We said we got all core gameplay features implemented last release. We lied.

There are now four main gameplay elements in our game, all implemented with varying levels of polish at the moment. The breaches and doors work as they always have. The new type of challenge is called an "engine malfunction" and manifests as two buttons that show up on the ship. Each button displays the location of the other. Two players will need to go to both buttons and jump on them together to clear the task, forcing players to stop clumping into one large group. Finally, the renamed "stabilizer failure" makes a return, where all players must roll in the same direction for a few seconds to clear the task. If players fail, the broken ship stabilizer spins out of control and flings players all over the ship. In gameplay terms, everyone's position gets randomized.

There are now three levels of difficulty, chosen by the host at the start of the game. Many bugs regarding ship health and the timer have been fixed, such that they actually sync up correctly across players. The team can now actually win and lose levels in a meaningful sense.

Controls are the same as before. Tilt (or use arrow keys) to move left and right. Tap (or click / press space) to jump. Roll over breaches of your own color repeatedly to fix them, get two players onto a door to open them, get two players to both jump on buttons to fix them, and when told to roll in a direction, get everyone doing that. Keep the ship health out of the red until the end of the level to win.

### Alpha Release

The focus of this release was to get all the core gameplay features implemented, as well as getting some finalized UI elements into the game and further polishing the networking.

On the gameplay end, the third and final type of challenge is now implemented. We're tentatively calling this a "stabilizer malfunction," and it requires all the players on a ship to roll in the same direction at the same time. However, only one player will get the notification, so they'll need to let everyone else know.

Additionally, ship health has been tweaked and a rudimentary round timer now shows up in the lower left corner. With that said, the networking has not been fully adapted to work with this yet, so you'll notice players desync-ing on this data. There's not much gameplay purpose to these yet; that'll be a focus for next sprint.

The main menu UI has been drastically improved and now resembles how we hope the final version to look. Lots of animations and assets have also been updated throughout the game. When connecting on the main menu, there's some better error handling (such as when entering an incorrect room ID, it'll just clear the ID), but it's not perfect yet.

Most importantly, our game supports anywhere from 3 to 6 players now. The host can click the "Start" button once enough people have joined. The ship is also much bigger to accommodate.

To play, our game still centers around rolling around the ship in a group. While doing so, you can fix breaches of your color by rolling over it back and forth three times. You can also open locked doors by having two players both touch the door. You cannot roll through or fix breaches of other colors - you'll need the correct colored player to come over and fix those. Instead, tap or click to jump over those breaches. Tilt or use the arrow keys to roll left and right. From time to time, one player will get a notification that everyone needs to roll in that direction. Do so. Apart from that, the game plays as it always has.

### Pre-Alpha Release

This is kind of a grab-bag release, with a bunch of small, behind-the-scenes changes that aren't super visible. Some changes to the networking API that were unfortunately not backwards compatible mean that the technical prototype will no longer connect to our server.

The most visible change is that players now collide with breaches of other colors, and thus need to jump over them. As we need all major challenges and game features implemented by pre-beta, this is a significant step towards making the jump feature necessary. The major purpose of this release, therefore, is to get an opportunity to playtest how this obstacle changes the dynamic of gameplay.  In theory, it should no longer be possible to just hold the phone sideways and roll forever, with both doors and breaches getting in your way now.

The networking is a bit more robust, with rudimentary discrepancy resolution built-in (as well as a myriad of backend changes). It's still not completely perfect, and you will occasionally see stuff flicker. The matchmaking screen has been drastically improved, in that the keyboard no longer covers up the input box. The instructions are otherwise identical to what's listed below.

Our game still supports rolling around the ship in a group of three. While doing so, you can fix breaches of your color by rolling over it back and forth three times. You can also open locked doors by having two players both touch the door. You cannot roll through or fix breaches of other colors - you'll need the correct colored player to come over and fix those. Instead, tap or click to jump over those breaches. Tilt or use the arrow keys to roll left and right.

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
