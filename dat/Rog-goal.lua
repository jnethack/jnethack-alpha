-- NetHack Rogue Rog-goal.lua	$NHDT-Date: 1652196012 2022/05/10 15:20:12 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.1 $
--	Copyright (c) 1992 by Dean Luick
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel", "noteleport")

--         1         2         3         4         5         6         7
--123456789012345678901234567890123456789012345678901234567890123456789012345
des.map([[
-----      -------.......................................|-----------------|
|...|  -----.....|.......................................|.................|
|...----...|.....|.......................................|....---------....|
|.---......---..--.................................------------.......|....|
|...............|..................................|..|...|...----........-|
|.....-----....--.................................|-..--..-|.....----S----| 
|--S---...|....|.................................|-........-|....|........| 
|.........---------.............................|-....}}....-|...|...|....| 
|....|.....S......|............................|-.....}}.....-|..--.------| 
|-----.....--.....|...........................|-...}}}}}}}}...-|....|.....--
|...........--....------S-----...............|-....}}}}}}}}....-|..........|
|............--........|...| |..............--.....}}.}}........----------S-
|.............|........|...| |..............|......}}}}}}}}......|...|.....|
|S-.---.---.---.---.---|...| ------------...--........}}.}}.....--..---....|
|.---.---.---.---.-S-..----- |....|.....|....|-....}}}}}}}}....---..S.|--..|
|...|.......|..........|...---....---...S.....|-...}}}}}}}}...-|.S..|...|..|
|...|..|....|..........|............|..--..----|-.....}}.....-|..----...-S--
|...|---....----.......|----- ......|...---|    |-....}}....-|...|..--.--..|
-----.....---.....--.---....--...--------..|     |-........-|....|.........|
    |.............|..........|.............S...   |S-------|.....|..-----..|
    ----------------------------------------  ......       ----------   ----
]]);
-- Dungeon Description
des.region(selection.area(00,00,75,20), "lit")
-- Stairs
des.levregion({ region={01,00,15,20}, region_islev=1, exclude={01,18,04,20}, type="stair-up" });
-- Doors
-- Non diggable walls
des.non_diggable(selection.area(00,00,75,20))
-- One trap to keep the gnomes at bay.
des.trap("spiked pit",37,07)
-- Objects
des.object({ id = "万能鍵", x=38, y=10, buc="blessed", spe=0, name="盗賊のマスターキー" })
des.object({ id = "缶", x=26, y=12, montype="カメレオン" })
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
des.trap()
des.trap()
des.trap()
des.trap()
des.trap()
-- Random monsters.
des.monster({ id = "暗殺者の頭領", x=38, y=10, peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ id = "レプラコーン", peaceful=0 })
des.monster({ class = "l", peaceful=0 })
des.monster({ class = "l", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ id = "番兵ナーガ", peaceful=0 })
des.monster({ class = "N", peaceful=0 })
des.monster({ class = "N", peaceful=0 })
des.monster({ class = "N", peaceful=0 })
des.monster({ id = "カメレオン", peaceful=0 })
des.monster({ id = "カメレオン", peaceful=0 })
des.monster({ id = "カメレオン", peaceful=0 })
des.monster({ id = "カメレオン", peaceful=0 })
des.monster({ id = "カメレオン", peaceful=0 })
des.monster({ id = "サメ", x=51, y=14, peaceful=0 })
des.monster({ id = "サメ", x=53, y=09, peaceful=0 })
des.monster({ id = "サメ", x=55, y=15, peaceful=0 })
des.monster({ id = "サメ", x=58, y=10, peaceful=0 })
