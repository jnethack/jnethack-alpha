-- NetHack mines minend-2.lua	$NHDT-Date: 1652196029 2022/05/10 15:20:29 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Mine end level variant 2
-- "Gnome King's Wine Cellar"

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

des.map([[
---------------------------------------------------------------------------
|...................................................|                     |
|.|---------S--.--|...|--------------------------|..|                     |
|.||---|   |.||-| |...|..........................|..|                     |
|.||...| |-|.|.|---...|.............................|                ..   |
|.||...|-|.....|....|-|..........................|..|.               ..   |
|.||.....|-S|..|....|............................|..|..                   |
|.||--|..|..|..|-|..|----------------------------|..|-.                   |
|.|   |..|..|....|..................................|...                  |
|.|   |..|..|----|..-----------------------------|..|....                 |
|.|---|..|--|.......|----------------------------|..|.....                |
|...........|----.--|......................|     |..|.......              |
|-----------|...|.| |------------------|.|.|-----|..|.....|..             |
|-----------|.{.|.|--------------------|.|..........|.....|....           |
|...............|.S......................|-------------..-----...         |
|.--------------|.|--------------------|.|.........................       |
|.................|                    |.....................|........    |
---------------------------------------------------------------------------
]]);

if percent(50) then
   des.terrain({55,14},"-")
   des.terrain({56,14},"-")
   des.terrain({61,15},"|")
   des.terrain({52,5}, "S")
   des.door("locked", 52,5)
end
if percent(50) then
   des.terrain({18,1}, "|")
   des.terrain(selection.area(7,12, 8,13), ".")
end
if percent(50) then
   des.terrain({49,4}, "|")
   des.terrain({21,5}, ".")
end
if percent(50) then
   if percent(50) then
      des.terrain({22,1}, "|")
   else
      des.terrain({50,7}, "-")
      des.terrain({51,7}, "-")
   end
end

-- uncontrolled arrival (via trap door, level teleport) will be in the central
-- portion of level to prevent ending up stuck in the treasure area, whether
-- arriving from above or below (despite this being bottom of Mines branch,
-- hero might arrive from below by invoking Wiz role's Eye of the Aethiopica)
des.teleport_region({ region={23,03,48,16}, region_islev=1 })

-- Dungeon Description
des.feature("fountain", {14,13})
des.region(selection.area(23,03,48,06),"lit")
des.region(selection.area(21,06,22,06),"lit")
des.region(selection.area(14,04,14,04),"unlit")
des.region(selection.area(10,05,14,08),"unlit")
des.region(selection.area(10,09,11,09),"unlit")
des.region(selection.area(15,08,16,08),"unlit")
-- Secret doors
des.door("locked",12,02)
des.door("locked",11,06)
-- Stairs
des.stair("up", 36,04)
-- Non diggable walls
des.non_diggable(selection.area(00,00,52,17))
des.non_diggable(selection.area(53,00,74,00))
des.non_diggable(selection.area(53,17,74,17))
des.non_diggable(selection.area(74,01,74,16))
des.non_diggable(selection.area(53,07,55,07))
des.non_diggable(selection.area(53,14,61,14))
-- The Gnome King's wine cellar.
-- the Trespassers sign is a long-running joke
des.engraving({12,03}, "engrave",
	      "You are now entering the Gnome King's wine cellar.")
des.engraving({12,04}, "engrave", "Trespassers will be persecuted!")
des.object("酔っぱらいの薬", 10, 07)
des.object("酔っぱらいの薬", 10, 07)
des.object("!", 10, 07)
des.object("酔っぱらいの薬", 10, 08)
des.object("酔っぱらいの薬", 10, 08)
des.object("!", 10, 08)
des.object("酔っぱらいの薬", 10, 09)
des.object("酔っぱらいの薬", 10, 09)
des.object("物体を探す薬", 10, 09)
-- Objects
-- The Treasure chamber...
des.object("ダイヤモンド", 69, 04)
des.object("*", 69, 04)
des.object("ダイヤモンド", 69, 04)
des.object("*", 69, 04)
des.object("エメラルド", 70, 04)
des.object("*", 70, 04)
des.object("エメラルド", 70, 04)
des.object("*", 70, 04)
des.object("エメラルド", 69, 05)
des.object("*", 69, 05)
des.object("ルビー", 69, 05)
des.object("*", 69, 05)
des.object("ルビー", 70, 05)
des.object("アメジスト", 70, 05)
des.object("*", 70, 05)
des.object("アメジスト", 70, 05)
des.object({ id="幸せの石", x=70, y=05,
	     buc="not-cursed", achievement=1 });
-- Scattered gems...
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("*")
des.object("(")
des.object("(")
des.object()
des.object()
des.object()
-- Random traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
-- Random monsters.
des.monster("ノームの王")
des.monster("ノームの貴族")
des.monster("ノームの貴族")
des.monster("ノームの貴族")
des.monster("ノームの魔法使い")
des.monster("ノームの魔法使い")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ノーム")
des.monster("ホビット")
des.monster("ホビット")
des.monster("ドワーフ")
des.monster("ドワーフ")
des.monster("ドワーフ")
des.monster("h")
