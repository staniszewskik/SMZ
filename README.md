# SMZ
Very much WIP! Download the zip file from the latest release, unpack it and run SMZ.exe to check it out.

Controls:
- **W A S D** and the **mouse** to walk and look around
- **Spacebar** to jump (if you are touching the ground or a wall)
- **Esc** to exit the program
- **R** to restart
- **V** to toggle VSync off/on
- **Numpad 8**, **2**, **4** and **6** to change the sun's direction
- **F** to force a shadow redraw
- **I** and **O** to adjust the FOV
- **[** and **]** to adjust the mouse sensitivity
- **F12** to take a screenshot (creates a screenshot.bmp file in the resources/ folder)

 

Note - you can edit the resources/levels/level0.levelgeo file to modify the generated level:
- changing the string after the "sed" changes the level generation seed
- adding a "sun \<red\> \<green\> \<blue\> \<pitch\> \<yaw\>" line overrides the default sun color and direction
- adding an "amb \<red\> \<green\> \<blue\>" line overrides the default ambient light color
- adding an "acc \<red\> \<green\> \<blue\>" line overrides the generated accent color

(colors are values from 0.0 to 1.0, pitch and yaw are in degrees)
