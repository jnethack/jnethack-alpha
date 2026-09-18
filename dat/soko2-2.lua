-- NetHack sokoban soko2-2.lua	$NHDT-Date: 1652196035 2022/05/10 15:20:35 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "premapped", "sokoban", "solidify");

des.map([[
  --------            
--|.|....|            
|........|----------  
|.-...-..|.|.......|  
|...-......|.......|  
|.-....|...|.......|  
|....-.--.-|.......|  
|..........|.......|  
|.--...|...|.......---
|....-.|---|.......+.|
--|....|------------.|
  |................+.|
  --------------------
]]);
des.stair("down", 06,11)
des.stair("up", 15,06)
des.door("locked",19,09)
des.door("locked",19,11)
des.region(selection.area(00,00,21,12), "lit");
des.non_diggable(selection.area(00,00,21,12));
des.non_passwall(selection.area(00,00,21,12));

-- Boulders
des.object("岩",04,02)
des.object("岩",04,03)
des.object("岩",05,03)
des.object("岩",07,03)
des.object("岩",08,03)
des.object("岩",02,04)
des.object("岩",03,04)
des.object("岩",05,05)
des.object("岩",06,06)
des.object("岩",09,06)
des.object("岩",03,07)
des.object("岩",04,07)
des.object("岩",07,07)
des.object("岩",06,09)
des.object("岩",05,10)
des.object("岩",05,11)

-- prevent monster generation over the (filled) holes
des.exclusion({ type = "monster-generation", region = { 06,11, 18,11 } });
-- Traps
des.trap("rolling boulder",07,11)
des.trap("hole",08,11)
des.trap("hole",09,11)
des.trap("hole",10,11)
des.trap("hole",11,11)
des.trap("hole",12,11)
des.trap("hole",13,11)
des.trap("hole",14,11)
des.trap("hole",15,11)
des.trap("hole",16,11)
des.trap("hole",17,11)
des.trap("hole",18,11)

-- Random objects
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "=" });
des.object({ class = "/" });
