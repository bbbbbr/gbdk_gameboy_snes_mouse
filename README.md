# Example for using the SNES/Super Nintendo Mouse on the Game Boy (in C/GBDK) attached directly to the link port


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
