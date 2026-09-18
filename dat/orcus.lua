-- NetHack gehennom orcus.lua	$NHDT-Date: 1652196033 2022/05/10 15:20:33 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992 by M. Stephenson and Izchak Miller
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style="mazegrid", bg ="-" });

des.level_flags("mazelevel", "shortsighted")

local tmpbounds = selection.match("-");
local bnds = tmpbounds:bounds();
local bounds2 = selection.fillrect(bnds.lx, bnds.ly + 1, bnds.hx - 2, bnds.hy - 1);

-- A ghost town
local orcus1 = des.map({ halign = "right", valign = "center", map = [[
.|....|....|....|..............|....|........
.|....|....|....|..............|....|........
.|....|....|....|--...-+-------|.............
.|....|....|....|..............+.............
.|.........|....|..............|....|........
.--+-...-+----+--....-------...--------.-+---
.....................|.....|.................
.....................|.....|.................
.--+----....-+---....|.....|...----------+---
.|....|....|....|....---+---...|......|......
.|.........|....|..............|......|......
.----...---------.....-----....+......|......
.|........................|....|......|......
.----------+-...--+--|....|....----------+---
.|....|..............|....+....|.............
.|....+.......|......|....|....|.............
.|....|.......|......|....|....|.............
]], contents = function(rm)
   des.mazewalk(00,06,"west")
   -- Entire main area
   des.region(selection.area(01,00,44,16),"unlit")
   des.stair("down", 33,15)
   -- Wall "ruins"
   des.object("岩",19,02)
   des.object("岩",20,02)
   des.object("岩",21,02)
   des.object("岩",36,02)
   des.object("岩",36,03)
   des.object("岩",06,04)
   des.object("岩",05,05)
   des.object("岩",06,05)
   des.object("岩",07,05)
   des.object("岩",39,05)
   des.object("岩",08,08)
   des.object("岩",09,08)
   des.object("岩",10,08)
   des.object("岩",11,08)
   des.object("岩",06,10)
   des.object("岩",05,11)
   des.object("岩",06,11)
   des.object("岩",07,11)
   des.object("岩",21,11)
   des.object("岩",21,12)
   des.object("岩",13,13)
   des.object("岩",14,13)
   des.object("岩",15,13)
   des.object("岩",14,14)
   -- Doors
   des.door("closed",23,02)
   des.door("open",31,03)
   des.door("nodoor",03,05)
   des.door("closed",09,05)
   des.door("closed",14,05)
   des.door("closed",41,05)
   des.door("open",03,08)
   des.door("nodoor",13,08)
   des.door("open",41,08)
   des.door("closed",24,09)
   des.door("closed",31,11)
   des.door("open",11,13)
   des.door("closed",18,13)
   des.door("closed",41,13)
   des.door("open",26,14)
   des.door("closed",06,15)
   -- Special rooms
   des.altar({ x=24,y=07,align="noalign",type="sanctum" })
   des.region({ region={22,12,25,16},lit=0,type="morgue",filled=1 })
   des.region({ region={32,09,37,12},lit=1,type="shop",filled=1 })
   des.region({ region={12,00,15,04},lit=1,type="shop",filled=1 })
   -- Some traps.
   des.trap("spiked pit")
   des.trap("sleep gas")
   des.trap("anti magic")
   des.trap("fire")
   des.trap("fire")
   des.trap("fire")
   des.trap("magic")
   des.trap("magic")
   -- Some random objects
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   des.object()
   -- An object that's worth most of a wish
   -- (this is part of the compensation for the reduced wishes at the Castle)
   if math.random(0, 1) == 1 then
      des.object("魔法のマーカ")
   else
      des.object("魔法のランプ")
   end
   -- The resident nasty
   des.monster("オーケス",33,15)
   -- And its preferred companions
   des.monster("人間のゾンビ",32,15)
   des.monster("影",32,14)
   des.monster("影",32,16)
   des.monster("吸血鬼",35,16)
   des.monster("吸血鬼",35,14)
   des.monster("吸血鬼の貴族",36,14)
   des.monster("吸血鬼の貴族",36,15)
   -- Randomly placed companions
   des.monster("骸骨")
   des.monster("骸骨")
   des.monster("骸骨")
   des.monster("骸骨")
   des.monster("骸骨")
   des.monster("影")
   des.monster("影")
   des.monster("影")
   des.monster("影")
   des.monster("巨人のゾンビ")
   des.monster("巨人のゾンビ")
   des.monster("巨人のゾンビ")
   des.monster("エティンのゾンビ")
   des.monster("エティンのゾンビ")
   des.monster("エティンのゾンビ")
   des.monster("人間のゾンビ")
   des.monster("人間のゾンビ")
   des.monster("人間のゾンビ")
   des.monster("吸血鬼")
   des.monster("吸血鬼")
   des.monster("吸血鬼")
   des.monster("吸血鬼の貴族")
   des.monster("吸血鬼の貴族")
   -- A few more for the party
   des.monster()
   des.monster()
   des.monster()
   des.monster()
   des.monster()
end });

des.levregion({ region={01,00,12,20}, region_islev=1, exclude={20,01,70,20}, exclude_islev=1, type="stair-up" });
des.levregion({ region={01,00,12,20}, region_islev=1, exclude={20,01,70,20}, exclude_islev=1, type="branch" });
des.teleport_region({ region={01,00,12,20}, region_islev=1, exclude={20,01,70,20}, exclude_islev=1 });

local protected = bounds2:negate() | orcus1;
hell_tweaks(protected);
