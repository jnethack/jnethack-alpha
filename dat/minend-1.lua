-- NetHack mines minend-1.lua	$NHDT-Date: 1652196029 2022/05/10 15:20:29 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.3 $
--	Copyright (c) 1989-95 by Jean-Christophe Collet
--	Copyright (c) 1991-95 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
-- Mine end level variant 1
-- "Mimic of the Mines"

des.level_init({ style = "solidfill", fg = " " });

des.level_flags("mazelevel");

des.map([[
------------------------------------------------------------------   ------
|                        |.......|     |.......-...|       |.....|.       |
|    ---------        ----.......-------...........|       ---...-S-      |
|    |.......|        |..........................-S-      --.......|      |
|    |......-------   ---........................|.       |.......--      |
|    |..--........-----..........................|.       -.-..----       |
|    --..--.-----........-.....................---        --..--          |
|     --..--..| -----------..................---.----------..--           |
|      |...--.|    |..S...S..............---................--            |
|     ----..-----  ------------........--- ------------...---             |
|     |.........--            ----------              ---...-- -----      |
|    --.....---..--                           --------  --...---...--     |
| ----..-..-- --..---------------------      --......--  ---........|     |
|--....-----   --..-..................---    |........|    |.......--     |
|.......|       --......................S..  --......--    ---..----      |
|--.--.--        ----.................---     ------..------...--         |
| |....S..          |...............-..|         ..S...........|          |
--------            --------------------           ------------------------
]]);

-- Dungeon Description
local place = { {08,16},{13,07},{21,08},{41,14},{50,04},{50,16},{66,01} }
shuffle(place)

-- make the entry chamber a real room; it affects monster arrival
des.region({ region={26,01,32,01}, lit=0, type="ordinary", irregular=1, arrival_room=true })
des.region(selection.area(20,08,21,08),"unlit")
des.region(selection.area(23,08,25,08),"unlit");
-- Secret doors
des.door("locked",07,16)
des.door("locked",22,08)
des.door("locked",26,08)
des.door("locked",40,14)
des.door("locked",50,03)
des.door("locked",51,16)
des.door("locked",66,02)
-- Stairs
des.stair("up", 36,04)
-- Non diggable walls
des.non_diggable(selection.area(00,00,74,17))
-- Niches
-- Note: place[6] empty
des.object("ダイヤモンド",place[7])
des.object("エメラルド",place[7])
des.object("紫のガラス",place[7])
des.monster({ class="m", coord=place[7], appear_as="obj:幸せの石" })
des.object("白いガラス",place[1])
des.object("エメラルド",place[1])
des.object("アメジスト",place[1])
des.monster({ class="m", coord=place[1], appear_as="obj:重し" })
des.object("ダイヤモンド",place[2])
des.object("緑のガラス",place[2])
des.object("アメジスト",place[2])
des.monster({ class="m", coord=place[2], appear_as="obj:火打ち石" })
des.object("白いガラス",place[3])
des.object("エメラルド",place[3])
des.object("紫のガラス",place[3])
des.monster({ class="m", coord=place[3], appear_as="obj:試金石" })
des.object("赤いガラス",place[4])
des.object("ルビー",place[4])
des.object("重し",place[4])
des.object("ルビー",place[5])
des.object("赤いガラス",place[5])
des.object({ id="幸せの石", coord=place[5], buc="not-cursed", achievement=1 })
-- Random objects
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
-- Random monsters
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
