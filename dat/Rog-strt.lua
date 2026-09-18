-- NetHack Rogue Rog-strt.lua	$NHDT-Date: 1652196012 2022/05/10 15:20:12 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.4 $
--	Copyright (c) 1992 by Dean Luick
-- NetHack may be freely redistributed.  See license for details.
--
--
--	The "start" level for the quest.
--
--	Here you meet your (besieged) class leader, Master of Thieves
--	and receive your quest assignment.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "nommap")

--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
---------------------------------.------------------------------------------
|.....|.||..........|....|......|.|.........|.......+............---.......|
|.....|..+..........+....---....S.|...-S-----.-----.|............+.+.......|
|.....+.||........---......|....|.|...|.....|.|...|.---.....------.--------|
|-----|.-------|..|........------.-----.....|.--..|...-------..............|
|.....|........------+------..........+.....|..--S---.........------.-----..
|.....|.------...............-----.}}.--------.|....-------.---....|.+...--|
|..-+--.|....|-----.--------.|...|.....+.....|.|....|.....+.+......|.--....|
|..|....|....|....+.|......|.|...-----.|.....|.--...|.....|.|......|..|....|
|..|.-----S----...|.+....-----...|...|.----..|..|.---....--.---S-----.|----|
|..|.|........|...------.|.S.....|...|....-----.+.|......|..|.......|.|....|
|---.-------..|...|....|.|.|.....|...----.|...|.|---.....|.|-.......|.---..|
...........|..S...|....---.----S----..|...|...+.|..-------.---+-....|...--+|
|---------.---------...|......|....S..|.---...|.|..|...........----.---....|
|........|.........|...+.------....|---.---...|.--+-.----.----....|.+...--+|
|........|.---+---.|----.--........|......-----......|..|..|.--+-.|.-S-.|..|
|........|.|.....|........----------.----.......---.--..|-.|....|.-----.|..|
|----....+.|.....----+---............|..|--------.+.|...SS.|....|.......|..|
|...--+-----.....|......|.------------............---...||.------+--+----..|
|..........S.....|......|.|..........S............|.....||...|.....|....|..|
-------------------------.--------------------------------------------------
]]);
-- Dungeon Description
--REGION:(00,00,75,20),lit,"ordinary"

local streets = selection.floodfill(0,12)

-- The down stairs is at one of the 4 "exits".  The others are mimics,
-- mimicking stairwells.
local place = { {33,0}, {0,12}, {25,20}, {75,05} }
shuffle(place)

des.stair({ dir = "down", coord = place[1] })
des.monster({ id = "巨大なミミック", coord = place[2], appear_as = "ter:下り階段" })
des.monster({ id = "大きなミミック", coord = place[3], appear_as = "ter:下り階段" })
des.monster({ id = "小さなミミック", coord = place[4], appear_as = "ter:下り階段" })
-- Portal arrival point
des.levregion({ region = {19,09,19,09}, type="branch" })
-- Doors (secret)
--DOOR:locked|closed|open,(xx,yy)
des.door("locked", 32, 2)
des.door("locked", 63, 9)
des.door("locked", 27,10)
des.door("locked", 31,12)
des.door("locked", 35,13)
des.door("locked", 69,15)
des.door("locked", 56,17)
des.door("locked", 57,17)
des.door("locked", 11,19)
des.door("locked", 37,19)
des.door("locked", 39, 2)
des.door("locked", 49, 5)
des.door("locked", 10, 9)
des.door("locked", 14,12)
-- Doors (regular)
des.door("closed", 52, 1)
des.door("closed",  9, 2)
des.door("closed", 20, 2)
des.door("closed", 65, 2)
des.door("closed", 67, 2)
des.door("closed",  6, 3)
des.door("closed", 21, 5)
des.door("closed", 38, 5)
des.door("closed", 69, 6)
des.door("closed",  4, 7)
des.door("closed", 39, 7)
des.door("closed", 58, 7)
des.door("closed", 60, 7)
des.door("closed", 18, 8)
des.door("closed", 20, 9)
des.door("closed", 48,10)
des.door("closed", 46,12)
des.door("closed", 62,12)
des.door("closed", 74,12)
des.door("closed", 23,14)
des.door("closed", 23,14)
des.door("closed", 50,14)
des.door("closed", 68,14)
des.door("closed", 74,14)
des.door("closed", 14,15)
des.door("closed", 63,15)
des.door("closed",  9,17)
des.door("closed", 21,17)
des.door("closed", 50,17)
des.door("closed",  6,18)
des.door("closed", 65,18)
des.door("closed", 68,18)
-- Master of Thieves
des.monster({ id = "盗賊の頭領", coord = {36, 11}, inventory = function()
   des.object({ id = "革鎧", spe = 5 });
   des.object({ id = "銀の短剣", spe = 4 });
   des.object({ id = "短剣", spe = 2, quantity = d(2,4), buc = "not-cursed" });
end })
-- The treasure of Master of Thieves
des.object("宝箱", 36, 11)
-- thug guards, room #1
des.monster("ちんぴら", 28, 10)
des.monster("ちんぴら", 29, 11)
des.monster("ちんぴら", 30, 09)
des.monster("ちんぴら", 31, 07)
-- thug guards, room #2
des.monster("ちんぴら", 31, 13)
des.monster("ちんぴら", 33, 14)
des.monster("ちんぴら", 30, 15)
--thug guards, room #3
des.monster("ちんぴら", 35, 09)
des.monster("ちんぴら", 36, 13)
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,20))
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
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
--
-- Monsters to get in the way.
--
-- West exit
des.monster({ id = "レプラコーン", x=01, y=12, peaceful=0 })
des.monster({ id = "水のニンフ", x=02, y=12, peaceful=0 })
-- North exit
des.monster({ id = "水のニンフ", x=33, y=01, peaceful=0 })
des.monster({ id = "レプラコーン", x=33, y=02, peaceful=0 })
-- East exit
des.monster({ id = "水のニンフ", x=74, y=05, peaceful=0 })
des.monster({ id = "レプラコーン", x=74, y=04, peaceful=0 })
-- South exit
des.monster({ id = "レプラコーン", x=25, y=19, peaceful=0 })
des.monster({ id = "水のニンフ", x=25, y=18, peaceful=0 })
-- Wandering the streets.
for i = 1, math.random(4,7) do
   des.monster({ id = "水のニンフ", coord = streets:rndcoord(1), peaceful=0 })
   des.monster({ id = "レプラコーン", coord = streets:rndcoord(1), peaceful=0 })
end
for i = 1, math.random(7,10) do
   des.monster({ id = "カメレオン", coord = streets:rndcoord(1), peaceful=0 })
end
