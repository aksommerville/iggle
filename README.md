# Reddin Iggle

Requires [Egg](https://github.com/aksommerville/egg) to build.

2025-02-28

Entry for [Code for a Cause](https://itch.io/jam/code-for-a-cause), theme "KEEP MOVING FORWARD".

Help the eagle put all the things in their place by flapping his wings with the space bar.

## Agenda

- Fri 2025-02-28
- - [x] Throwaway implementation to confirm the concept is tractable. ...CONFIRMED
- - [x] Global mode switch: game,hello,farewell
- - [x] Enough graphics to get started with. 32x32 tiles.
- - [x] Real sprites framework.
- - [x] Load maps.
- - [x] Flight, collisions, carrying. Everything we have in throwaway.
- - [x] Level termination.
- Sat 2025-03-01
- - [x] When one pumpkin sits on another just by the toes, you land on the lower one and things go a little weird.
- - - I think the best bet would be when you pick up the pumpkin, force nearby sprites within Iggle's vertical range away.
- - [x] Hazards. ...no hazards
- - [x] Scorekeeping.
- - [x] Proper graphics.
- - [x] Make grabbing a little more tolerant. Landing on a pumpkin without grabbing it should be rare.
- - [x] Allow certain pumpkin tiles to resist transform (the ones with text)
- - [x] Hello and Farewell splashes.
- - [x] We should allow pumpkins to stack on the goal. It's hard to avoid, and hard to explain why that's not "on goal".
- - [x] Cheat hero's hit box down at the top when flying, he hits the ceiling too soon. ...no reason to do it only while flying, much easier to shorten him always.
- - [x] Need a bigger font.
- - [x] Proper maps.
- - - Bonk-to-drop is not obvious. Ensure the first level or two have just one pumpkin, and then train the user on bonking.
- - - 3-row apertures are surprisingly difficult to pass through.
- - - 45-degree slopes, both ceiling and floor, are pretty annoying.
- - - ...6 maps and it seems a pretty nice spread of difficulty. I can just barely complete all 6 in under 2 minutes. Expect 5-10 for a new player?
- Sun 2025-03-02
- - [x] Press Start to reset level.
- - [x] Music.
- - [x] Sound effects.
- - [x] Title banner has got to go. Try something hand-drawn.
- - [x] Itch page.
- - [x] Have an arguably complete game by EOD. I don't want to do a lot during the week.
- - [x] Submit to jam? We can keep revising once submitted, I believe.
- - [x] Swap maps 5 and 6; 5 is more difficult.
- - [x] Ending is kind of abrupt.
- - - [x] Something more "You Win!" than "Game Over", make it positive.
- - - [x] Can we style the last map to make it feel more like a finale? ...the new Level 7 is really tricky, I think that's good enough.
- Mon 2025-03-03
- Tue 2025-03-04
- - [x] Dust clouds on landing.
- - [x] "On goal" indicator.
- - [x] Can we do another level or two?
- Wed 2025-03-05
- Thu 2025-03-06
- Fri 2025-03-07

Stretch goals, whenever:
- [x] Minicomic. ...nah, I can't think up a story line.
- [x] Ridiculously excessive background graphics. Thinking, clouds moving and parting, trees blowing in the wind, lights going on and off in the distant city...
- - Got sun, moon, and stars. We could still do clouds, terrain, what-have-you...
- [x] Hand-drawn graphics for the foreground? Might look cool... ...nah
- [x] Even if not hand-drawn, a rotating sprite for the wing would probly be a good touch. ...nah, I like it as is
- [x] Replace the green terrain tiles until we run out of tilesheet.
- - ...there's still some tilesheet left, but I think this is as far as it goes.
- [x] When we're done replacing graphics, update the screencaps on Itch.
- [x] Anti-alias edge of sun, the rotating one. Transparent pixels save black and get used for interpolation.

## High Scores

- 2:37.305 AK 2024-03-02
- 2:32.776 AK 2024-03-03
- 2:31.547 AK 2024-03-03
- 2:30.632 AK 2024-03-03
- 2:20.501 AK 2024-03-03 -- Almost flawless. Tho there might be better strategies.
- 2:14.246 AK 2024-03-05 -- After a small change to make Up more sensitive.

- 10:07.359 Dean 2024-03-03

- All the above scores are before level 8. 8 is super simple, shouldn't add much, but still.
