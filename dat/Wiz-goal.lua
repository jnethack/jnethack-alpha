-- NetHack Wizard Wiz-goal.lua	$NHDT-Date: 1652196018 2022/05/10 15:20:18 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.2 $
--	Copyright (c) 1992 by David Cohrs
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

des.map([[
                                                                            
                                                                            
                                                                            
                   -------------                 -------------              
                   |...........|                 |...........|              
            -------|...........-------------------...........|              
            |......S...........|..|..|..|..|..|..|...........|              
            |......|...........|..|..|..|..|..|..|...........|              
            |......|...........-F+-F+-F+-F+-F+-F+-...........|              
            --S----|...........S.................+...........|              
            |......|...........-F+-F+-F+-F+-F+-F+-...........|              
            |......|...........|..|..|..|..|..|..|...........|              
            |......|...........|..|..|..|..|..|..|...........|              
            -------|...........-------------------...........|              
                   |...........|                 |...........|              
                   -------------                 -------------              
                                                                            
                                                                            
                                                                            
                                                                            
]]);
-- Dungeon Description
des.region({ region={13,10,18,12}, lit=0, type="temple", filled=2 })
des.region(selection.area(13,06,18,08), "lit")
des.region(selection.area(20,04,30,14), "unlit")
des.region(selection.area(32,06,33,07), "unlit")
des.region(selection.area(35,06,36,07), "unlit")
des.region(selection.area(38,06,39,07), "unlit")
des.region(selection.area(41,06,42,07), "unlit")
des.region(selection.area(44,06,45,07), "unlit")
des.region(selection.area(47,06,48,07), "unlit")
des.region(selection.area(32,09,48,09), "unlit")
des.region(selection.area(32,11,33,12), "unlit")
des.region(selection.area(35,11,36,12), "unlit")
des.region(selection.area(38,11,39,12), "unlit")
des.region(selection.area(41,11,42,12), "unlit")
des.region(selection.area(44,11,45,12), "unlit")
des.region(selection.area(47,11,48,12), "unlit")
des.region(selection.area(50,04,60,14), "lit")
-- Doors
des.door("locked",19,06)
des.door("locked",14,09)
des.door("locked",31,09)
des.door("locked",33,08)
des.door("locked",36,08)
des.door("locked",39,08)
des.door("locked",42,08)
des.door("locked",45,08)
des.door("locked",48,08)
des.door("locked",33,10)
des.door("locked",36,10)
des.door("locked",39,10)
des.door("locked",42,10)
des.door("locked",45,10)
des.door("locked",48,10)
des.door("locked",49,09)
-- Stairs
des.stair("up", 55,05)
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,19))
-- The altar.  This is not a shrine.
des.altar({ coord={16,11}, aligned="noncoaligned", type="altar" })
-- Objects
des.object({ id = "遠視の魔除け", x=16, y=11, buc="blessed", spe=0, name="エチオピアの目" })
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
des.object()
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
des.monster("暗きもの", 16, 11)
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "B",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster({ class = "i",random, peaceful = 0 })
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster("吸血こうもり")
des.monster({ class = "i",random, peaceful = 0 })
-- Captive Monsters in the dungeon
des.monster({ id = "盗賊", x=35, y=06, peaceful=1, name="Pug" })
des.monster({ id = "アウルベア", x=47, y=06, peaceful=1, asleep=1 })
des.monster({ id = "魔法使い", x=32, y=11, peaceful=1, asleep=1, name="Newt" })
des.monster({ id = "灰色エルフ", x=44, y=11, peaceful=1 })
des.monster({ id = "丘の巨人", x=47, y=11, peaceful=1, asleep=1 })
des.monster({ id = "ノームの魔法使い", x=38, y=06, peaceful=1 })
des.monster({ id = "囚人", x=35, y=11, peaceful=1 })
des.monster({ id = "囚人", x=41, y=11, peaceful=1, asleep=1 })
