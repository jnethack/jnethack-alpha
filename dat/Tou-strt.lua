-- NetHack Tourist Tou-strt.lua	$NHDT-Date: 1652196016 2022/05/10 15:20:16 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1991,92 by M. Stephenson, P. Winner
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Twoflower
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor")
des.map([[
.......}}....---------..-------------------------------------------------...
........}}...|.......|..|.-------------------------------------------...|...
.........}}..|.......|..|.|......|......|.............|......|......|...|...
..........}}.|.......|..|.|......+......+.............+......+..\...|...|...
...........}}}..........|.|......|......|.............|......|......|...|...
.............}}.........|.|----S-|--S---|S----------S-|---S--|------|...|...
..............}}}.......|...............................................|...
................}}}.....----S------++--S----------S----------S-----------...
..................}}...........    ..    ...................................
......-------......}}}}........}}}}..}}}}..}}}}..}}}}.......................
......|.....|.......}}}}}}..}}}}   ..   }}}}..}}}}..}}}.....................
......|.....+...........}}}}}}........................}}}..}}}}..}}}..}}}...
......|.....|...........................................}}}}..}}}..}}}}.}}}}
......-------...............................................................
............................................................................
...-------......-------.....................................................
...|.....|......|.....|.....................................................
...|.....+......+.....|.....................................................
...|.....|......|.....|.....................................................
...-------......-------.....................................................
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,19), "lit")
des.region({ region={14,01, 20,03}, lit=0, type="morgue", filled=1 })
des.region(selection.area(07,10,11,12), "unlit")
des.region(selection.area(04,16,08,18), "unlit")
des.region(selection.area(17,16,21,18), "unlit")
des.region(selection.area(27,02,32,04), "unlit")
des.region(selection.area(34,02,39,04), "unlit")
des.region(selection.area(41,02,53,04), "unlit")
des.region(selection.area(55,02,60,04), "unlit")
des.region(selection.area(62,02,67,04), "lit")
-- Stairs
des.stair("down", 66,03)
-- Portal arrival point
des.levregion({ region = {68,14,68,14}, type="branch" })
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- Doors
des.door("locked",31,05)
des.door("locked",36,05)
des.door("locked",41,05)
des.door("locked",52,05)
des.door("locked",58,05)
des.door("locked",28,07)
des.door("locked",39,07)
des.door("locked",50,07)
des.door("locked",61,07)
des.door("closed",33,03)
des.door("closed",40,03)
des.door("closed",54,03)
des.door("closed",61,03)
des.door("open",12,11)
des.door("open",09,17)
des.door("open",16,17)
des.door("locked",35,07)
des.door("locked",36,07)
-- Monsters on siege duty.
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("巨大グモ")
des.monster("s")
des.monster("s")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("森のケンタウロス")
des.monster("C")
-- Twoflower
des.monster({ id = "ツーフラワー", coord = {64, 03}, inventory = function()
   des.object({ id = "散歩用の靴", spe = 3 });
   des.object({ id = "アロハシャツ", spe = 3 });
end })
-- The treasure of Twoflower
des.object("宝箱", 64, 03)
-- guides for the audience chamber
des.monster("ガイド", 29, 03)
des.monster("ガイド", 32, 04)
des.monster("ガイド", 35, 02)
des.monster("ガイド", 38, 03)
des.monster("ガイド", 45, 03)
des.monster("ガイド", 48, 02)
des.monster("ガイド", 49, 04)
des.monster("ガイド", 51, 03)
des.monster("ガイド", 57, 03)
des.monster("ガイド", 62, 04)
des.monster("ガイド", 66, 04)
-- path guards
des.monster("見張り", 35, 08)
des.monster("見張り", 36, 08)
-- river monsters
des.monster("巨大ウナギ", 62, 12)
des.monster("ピラニア", 47, 10)
des.monster("ピラニア", 29, 11)
des.monster("クラーケン", 34, 09)
des.monster("クラーケン", 37, 09)
-- Random traps
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
