-- NetHack sokoban soko1-2.lua	$NHDT-Date: 1652196034 2022/05/10 15:20:34 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.6 $
--	Copyright (c) 1998-1999 by Kevin Hugo
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "premapped", "sokoban", "solidify");

des.map([[
  ------------------------
  |......................|
  |..-------------------.|
----.|    -----        |.|
|..|.--  --...|        |.|
|.....|--|....|        |.|
|.....|..|....|        |.|
--....|......--        |.|
 |.......|...|   ------|.|
 |....|..|...| --|.....|.|
 |....|--|...| |.+.....|.|
 |.......|..-- |-|.....|.|
 ----....|.--  |.+.....+.|
    ---.--.|   |-|.....|--
     |.....|   |.+.....|  
     |..|..|   --|.....|  
     -------     -------  
]]);

local place = selection.new();
place:set(16,10);
place:set(16,12);
place:set(16,14);

des.stair("down", 06,15);
des.region(selection.area(00,00,25,16),"lit");
des.non_diggable(selection.area(00,00,25,16));
des.non_passwall(selection.area(00,00,25,16));

-- Boulders
des.object("岩",04,04);
des.object("岩",02,06);
des.object("岩",03,06);
des.object("岩",04,07);
des.object("岩",05,07);
des.object("岩",02,08);
des.object("岩",05,08);
des.object("岩",03,09);
des.object("岩",04,09);
des.object("岩",03,10);
des.object("岩",05,10);
des.object("岩",06,12);
--
des.object("岩",07,14);
--
des.object("岩",11,05);
des.object("岩",12,06);
des.object("岩",10,07);
des.object("岩",11,07);
des.object("岩",10,08);
des.object("岩",12,09);
des.object("岩",11,10);

-- prevent monster generation over the (filled) holes
des.exclusion({ type = "monster-generation", region = { 05,01, 23,01 } });
-- Traps
des.trap("rolling boulder",05,01)
des.trap("hole",06,01)
des.trap("hole",07,01)
des.trap("hole",08,01)
des.trap("hole",09,01)
des.trap("hole",10,01)
des.trap("hole",11,01)
des.trap("hole",12,01)
des.trap("hole",13,01)
des.trap("hole",14,01)
des.trap("hole",15,01)
des.trap("hole",16,01)
des.trap("hole",17,01)
des.trap("hole",18,01)
des.trap("hole",19,01)
des.trap("hole",20,01)
des.trap("hole",21,01)
des.trap("hole",22,01)
des.trap("hole",23,01)

des.monster({ id = "巨大なミミック", appear_as = "obj:岩" });
des.monster({ id = "巨大なミミック", appear_as = "obj:岩" });

-- Random objects
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "%" });
des.object({ class = "=" });
des.object({ class = "/" });

-- Rewards
des.door("locked",23,12)
des.door("closed",17,10)
des.door("closed",17,12)
des.door("closed",17,14)
des.region({ region={18,09, 22,15}, lit = 1, type = "zoo", filled = 1, irregular = 1 });

local pt = selection.rndcoord(place);
if percent(25) then
   des.object({ id="軽量化の鞄", coord=pt,
		buc="not-cursed", achievement=1 });
else
   des.object({ id="反射の魔除け", coord=pt,
		buc="not-cursed", achievement=1 });
end
des.engraving({ coord = pt, type = "burn", text = "Elbereth" });
des.object({ id = "怪物を怯えさせる巻物", coord = pt, buc = "cursed" });
