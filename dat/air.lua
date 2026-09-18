-- NetHack endgame air.lua	$NHDT-Date: 1652196019 2022/05/10 15:20:19 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1992,1993 by Izchak Miller, David Cohrs,
--                      and Timo Hakulinen
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport", "hardfloor", "shortsighted", "stormy")
-- The following messages are somewhat obtuse, to make then
-- equally meaningful if the player can see or not.
des.message("What a strange feeling!")
des.message("You notice that there is no gravity here.")
-- The player lands, upon arrival, in the
-- lower-left area.  The location of the
-- portal to the next level is randomly chosen.
-- This map has no visible outer boundary, and
-- is all "air".
des.map([[
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
]]);
-- Use up and down regions to partition the level into three parts;
-- teleportation can't cross from one part into another.
-- The up region is where you'll arrive after activating the portal from
-- the preceding level; the exit portal is placed inside the down region.
des.teleport_region({ region = {01,00,24,20}, region_islev = 1, exclude = {25,00,79,20}, exclude_islev=1, dir="up" })
des.teleport_region({ region = {56,00,79,20}, region_islev = 1, exclude = {01,00,55,20}, exclude_islev=1, dir="down" })

-- des.portal(levregion(57,01,78,19),(0,0,0,0),"fire")
des.levregion({ region = {57,01,78,19}, region_islev=1, type="portal", name="fire" });
des.region(selection.area(00,00,75,19),"lit")
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })
des.monster({ id = "風の精", peaceful = 0 })

des.monster({ id = "さまよう目", peaceful = 0 })
des.monster({ id = "さまよう目", peaceful = 0 })
des.monster({ id = "さまよう目", peaceful = 0 })

des.monster({ id = "黄色い光", peaceful = 0 })
des.monster({ id = "黄色い光", peaceful = 0 })
des.monster({ id = "黄色い光", peaceful = 0 })

des.monster("コウアトル")

des.monster("D")
des.monster("D")
des.monster("D")
des.monster("D")
des.monster("D")

des.monster("E")
des.monster("E")
des.monster("E")
des.monster("J")
des.monster("J")

des.monster({ id = "風のジン", peaceful = 0 })
des.monster({ id = "風のジン", peaceful = 0 })
des.monster({ id = "風のジン", peaceful = 0 })

des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "霧の雲", peaceful = 0 })
des.monster({ id = "エネルギーの渦", peaceful = 0 })
des.monster({ id = "エネルギーの渦", peaceful = 0 })
des.monster({ id = "エネルギーの渦", peaceful = 0 })
des.monster({ id = "エネルギーの渦", peaceful = 0 })
des.monster({ id = "エネルギーの渦", peaceful = 0 })
des.monster({ id = "蒸気の渦", peaceful = 0 })
des.monster({ id = "蒸気の渦", peaceful = 0 })
des.monster({ id = "蒸気の渦", peaceful = 0 })
des.monster({ id = "蒸気の渦", peaceful = 0 })
des.monster({ id = "蒸気の渦", peaceful = 0 })

