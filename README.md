# GBDK/C Example for SNES Mouse on the Game Boy Link Port

Example for using the SNES/Super Nintendo Mouse (or Gamepad/Controller) on 
the Game Boy (in C/GBDK) attached directly to the link port.

<video width="640" height="360" controls>
  <source src="info/gameboy_snes_mouse.mp4" type="video/mp4">
</video>

This is a simplistic implementation with blocking waits for serial transfers
to finish. In actual use it would be better to use an interrupt to handle
completion of each serial transfer.


SNES Controller Port

```
  SNES CONTROLLER                             GAME BOY
  *PORT*                                     LINK *PORT*
   _
  / \
 | 7 | GND         -   GB_Link.GND   [6]
 | 6 |
 | 5 |
 |---|                                           _______
 | 4 | Data  (out)  -> GB_Link.S-IN      [3]    /       \
 | 3 | Latch (in)  <-  GB_Link.S-OUT     [2]   | 5  3  1 |
 | 2 | Clock (in)  <-  GB_Link.S-CLK(*1) [5]   | 6  4  2 |
 | 1 | 5v    (in)  <-  GB_Link.5v        [1]   |_________|
 |___|

*1: GB_Link.S-CLK should probably have an inverter on it, works on DMG (only) without one

```
