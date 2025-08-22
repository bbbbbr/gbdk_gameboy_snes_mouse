# GBDK/C Example for SNES Mouse on the Game Boy Link Port

Example for using the SNES/Super Nintendo Mouse (or Gamepad/Controller) on 
the Game Boy (in C/GBDK) attached directly to the link port.

https://github.com/user-attachments/assets/db09934d-4106-4748-8b14-7fa4eb37c3af

There are two implementations, a interrupt based polling method which is more
efficient and a simplistic one with blocking waits that is much slower.

There are separate implementation files for the mouse vs gamepad.

# Nintendo SNES Mouse
When an inverter is used on the clock out, this setup has been tested as
working on the following consoles:
- Game Boy: DMG, ~~MGB~~, GBC, SGB2, AGS101
- GB Boy Colour
- Analogue Pocket

And not working on the following (maybe offset by one bit):
- MGB (had seemed to work, but not upon later testing)
- FPGBC
- Modretro Chromatic


# Hyperkin Hyper Click SNES Mouse
When an inverter is used on the clock out, this setup has been tested as
working on ALL of the following consoles:
- Game Boy: DMG, MGB, GBC, SGB2, AGS101
- GB Boy Colour
- Analogue Pocket
- FPGBC
- Modretro Chromatic

Note: The Hyperkin mouse seems to be about 2x as sensitive even on the slowest 
setting (button on bottom toggles speed), making it harder to draw curves.
Implementing sub-pixel down scaling might help.


# SNES Controller Port

```
 SNES CONTROLLER                                GAME BOY
 *PORT*                                        LINK *PORT*
   _
  / \
 | 7 | GND         -   GB_Link.GND       [6]
 | 6 |
 | 5 |
 |---|                                                    _______
 | 4 | Data  (out)  -----------> GB_Link.S-IN      [3]    /       \
 | 3 | Latch (in)  <-----------  GB_Link.S-OUT(*2) [2]   | 5  3  1 |
 | 2 | Clock (in)  <- INVERT <-  GB_Link.S-CLK(*1) [5]   | 6  4  2 |
 | 1 | 5v    (in)  <-----------  GB_Link.5v        [1]   |_________|
 |___|

*1: GB_Link.S-CLK works *much* better with an inverter on it
    and works with OEM consoles (FPGA clones vary).
    Without an Inverter it only works on DMG models.
    Tested with a SN74AHCT14N

*2: The problem with using GB_Link.S-OUT to trigger the controller
    protocol latch is that the clock runs during that triggering
    the sensitivity adjustment each time the mouse is polled. In 
    practice this doesn't seem to make a big difference.
    GB_Link.4(S-Data) might work as an alternative, controlled
    via bit 4 (d-pad select) of the joypad register. 

```

# Resources
The documentation at this link was very useful and well written:
https://www.repairfaq.org/REPAIR/F_SNES.html
