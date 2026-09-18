-- NetHack Samurai Sam-strt.lua	$NHDT-Date: 1695932714 2023/09/28 20:25:14 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991-92 by M. Stephenson, P. Winner
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Lord Sato
--	and receive your quest assignment.
--
--      The throne room designation produces random atmospheric
--      messages (until the room is entered) but this one doesn't
--      actually contain any throne.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor")

des.map([[
..............................................................PP............
...............................................................PP...........
..........---------------------------------------------------...PPP.........
..........|......|.........|...|..............|...|.........|....PPPPP......
......... |......|.........S...|..............|...S.........|.....PPPP......
..........|......|.........|---|..............|---|.........|.....PPP.......
..........+......|.........+...-------++-------...+.........|......PP.......
..........+......|.........|......................|.........|......PP.......
......... |......---------------------++--------------------|........PP.....
..........|.................................................|.........PP....
..........|.................................................|...........PP..
..........----------------------------------------...-------|............PP.
..........................................|.................|.............PP
.............. ................. .........|.................|..............P
............. } ............... } ........|.................|...............
.............. ........PP....... .........|.................|...............
.....................PPP..................|.................|...............
......................PP..................-------------------...............
............................................................................
............................................................................
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,19), "lit")
des.region({ region={18,03, 26,07}, lit=1, type="throne", filled=2 })
-- Portal arrival zone
des.levregion({ region = {62,12,70,17}, type="branch" })
-- Stairs
des.stair("down", 29,04)
-- Doors
des.door("locked",10,06)
des.door("locked",10,07)
des.door("closed",27,04)
des.door("closed",27,06)
des.door("closed",38,06)
des.door("locked",38,08)
des.door("closed",39,06)
des.door("locked",39,08)
des.door("closed",50,04)
des.door("closed",50,06)
-- Lord Sato
des.monster({ id = "大名佐藤", coord = {20, 04}, inventory = function()
   des.object({ id = "鉄片の鎧", spe = 5, eroded=-1, buc="not-cursed" });
   des.object({ id = "刀", spe = 4, eroded=-1, buc="not-cursed" });
end })
-- The treasure of Lord Sato
des.object("宝箱", 20, 04)
-- roshi guards for the audience chamber
des.monster("浪士", 18, 04)
des.monster("浪士", 18, 05)
des.monster("浪士", 18, 06)
des.monster("浪士", 18, 07)
des.monster("浪士", 26, 04)
des.monster("浪士", 26, 05)
des.monster("浪士", 26, 06)
des.monster("浪士", 26, 07)
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Random traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
-- Monsters on siege duty.
des.monster({ id = "忍者", x=64, y=00, peaceful=0 })
des.monster("狼", 65, 01)
des.monster({ id = "忍者", x=67, y=02, peaceful=0 })
des.monster({ id = "忍者", x=69, y=05, peaceful=0 })
des.monster({ id = "忍者", x=69, y=06, peaceful=0 })
des.monster("狼", 69, 07)
des.monster({ id = "忍者", x=70, y=06, peaceful=0 })
des.monster({ id = "忍者", x=70, y=07, peaceful=0 })
des.monster({ id = "忍者", x=72, y=01, peaceful=0 })
des.monster("狼", 75, 09)
des.monster({ id = "忍者", x=73, y=05, peaceful=0 })
des.monster({ id = "忍者", x=68, y=02, peaceful=0 })
des.monster("ストーカー")
