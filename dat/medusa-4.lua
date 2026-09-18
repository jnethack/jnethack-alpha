-- NetHack medusa medusa-4.lua	$NHDT-Date: 1716152274 2024/05/19 20:57:54 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.8 $
--	Copyright (c) 1989 by Jean-Christophe Collet
--	Copyright (c) 1990, 1991 by M. Stephenson
-- NetHack may be freely redistributed.  See license for details.
--
des.level_init({ style = "solidfill", fg = " " });
des.level_flags("noteleport", "mazelevel")
--
-- Here the Medusa rules some slithery monsters from her 'palace', with
-- a yellow dragon nesting in the backyard.
--
des.map([[
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
}}}}}}}}}}}}}}........}}}}}}}}}}}}}}}}}}}}}}}..}}}.....}}}}}}}}}}}----|}}}}}
}}}}}}..----------F-.....}}}}}}}}}}}}}}}}..---...}}}}....T.}}}}}}}....|}}}}}
}}}.....|...F......S}}}}....}}}}}}}...}}.....|}}.}}}}}}}......}}}}|......}}}
}}}.....+...|..{...|}}}}}}}}}}}}.....}}}}|...|}}}}}}}}}}}.}}}}}}}}----.}}}}}
}}......|...|......|}}}}}}}}}......}}}}}}|.......}}}}}}}}}}}}}..}}}}}...}}}}
}}|-+--F|-+--....|F|-|}}}}}....}}}....}}}-----}}.....}}}}}}}......}}}}.}}}}}
}}|...}}|...|....|}}}|}}}}}}}..}}}}}}}}}}}}}}}}}}}}....}}}}}}}}....T.}}}}}}}
}}|...}}F...+....F}}}}}}}..}}}}}}}}}}}}}}...}}}}}}}}}}}}}}}}}}}}}}....}}..}}
}}|...}}|...|....|}}}|}....}}}}}}....}}}...}}}}}...}}}}}}}}}}}}}}}}}.....}}}
}}--+--F|-+--....-F|-|....}}}}}}}}}}.T...}}}}....---}}}}}}}}}}}}}}}}}}}}}}}}
}}......|...|......|}}}}}.}}}}}}}}}....}}}}}}}.....|}}}}}}}}}.}}}}}}}}}}}}}}
}}}}....+...|..{...|.}}}}}}}}}}}}}}}}}}}}}}}}}}.|..|}}}}}}}......}}}}...}}}}
}}}}}}..|...F......|...}}}}}}}}}}..---}}}}}}}}}}--.-}}}}}....}}}}}}....}}}}}
}}}}}}}}-----S----F|....}}}}}}}}}|...|}}}}}}}}}}}}...}}}}}}...}}}}}}..}}}}}}
}}}}}}}}}..............T...}}}}}.|.......}}}}}}}}}}}}}}..}...}.}}}}....}}}}}
}}}}}}}}}}....}}}}...}...}}}}}.......|.}}}}}}}}}}}}}}.......}}}}}}}}}...}}}}
}}}}}}}}}}..}}}}}}}}}}.}}}}}}}}}}-..--.}}}}}}}}..}}}}}}..T...}}}..}}}}}}}}}}
}}}}}}}}}...}}}}}}}}}}}}}}}}}}}}}}}...}}}}}}}....}}}}}}}.}}}..}}}...}}}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}.}}}}}}....}}}}}}}}}}}}}}}}}}}...}}}}}}
}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
]]);

--
-- place handling is similar to medusa-3.lua except that there are 4
-- downstairs-eligible rooms rather than 3, and only 2 of them are used
local place = selection.new();
-- each of these spots are inside a distinct room
place:set(04,08);
place:set(10,04);
place:set(10,08);
place:set(10,12);

-- location of Medusa and downstairs and Perseus's statue
local medloc = place:rndcoord(1,1);
-- specific location for some other statue in a different downstairs-eligible
-- room, to prevent object detection from becoming a trivial way to pinpoint
-- Medusa's location
-- [usefulness depends on future STATUE->dknown changes in nethack's core]
local altloc = place:rndcoord(1,1);

--
des.region(selection.area(00,00,74,19),"lit")
-- fixup_special hack: The first "room" region in Medusa levels gets filled
-- with some leaderboard statues, so this needs to be a room; setting
-- irregular=1 will force this
des.region({ region={13,03, 18,13}, lit=1, type="ordinary", irregular=1 })
--
des.teleport_region({ region = {64,01,74,17}, dir="down" });
des.teleport_region({ region = {02,02,18,13}, dir="up" });
--
des.levregion({ region = {67,01,74,20}, type="stair-up" });

-- place the downstairs at the same spot where Medusa will be placed
des.stair("down", medloc)
--
des.door("locked",04,06)
des.door("locked",04,10)
des.door("locked",08,04)
des.door("locked",08,12)
des.door("locked",10,06)
des.door("locked",10,10)
des.door("locked",12,08)
--
des.levregion({ region = {27,00,79,20}, type="branch" });
--
des.non_diggable(selection.area(01,01,22,14));
--
des.object("水晶玉", 07,08)
--
-- same spot as Medusa plus downstairs
des.object({ id="像", coord=medloc, buc="uncursed",
                      montype="騎士", historic=1, male=1,name="Perseus",
                      contents = function()
                         if percent(75) then
                            des.object({ id = "反射の盾", buc="cursed", spe=0 })
                         end
                         if percent(25) then
                            des.object({ id = "浮遊の靴", spe=0 })
                         end
                         if percent(50) then
                            des.object({ id = "シミター", buc="blessed", spe=2 })
                         end
                         if percent(50) then
                            des.object("袋")
                         end
                      end
});
--
-- first random statue is in one of the designated stair rooms but not the
-- one with Medusa plus downstairs
des.object({ id = "像", coord=altloc, contents=0 })
des.object({ id = "像", contents=0 })
des.object({ id = "像", contents=0 })
des.object({ id = "像", contents=0 })
des.object({ id = "像", contents=0 })
des.object({ id = "像", contents=0 })
des.object({ id = "像", contents=0 })
for i=1,8 do
   des.object()
end
--
for i=1,7 do
   des.trap()
end
--
-- place Medusa before placing other monsters so that they won't be able to
-- unintentionally steal her spot on the downstairs
des.monster({ id = "メデューサ", coord=medloc, asleep=1 })
des.monster("クラーケン", 07,07)
--
-- the nesting dragon
des.monster({ id = "黄色ドラゴン", x=05, y=04, asleep=1 })
if percent(50) then
   des.monster({ id = "黄色ドラゴンの子供", x=04,y=04, asleep=1 })
end
if percent(25) then
   des.monster({ id = "黄色ドラゴンの子供", x=04, y=05, asleep=1 })
end
des.object({ id = "卵", x=05, y=04, montype="黄色ドラゴン" });
if percent(50) then
   des.object({ id = "卵", x=05, y=04, montype="黄色ドラゴン" });
end
if percent(25) then
   des.object({ id = "卵", x=05, y=04, montype="黄色ドラゴン" });
end
--
des.monster("巨大ウナギ")
des.monster("巨大ウナギ")
des.monster("クラゲ")
des.monster("クラゲ")
for i=1,14 do
   des.monster("S")
end
for i=1,4 do
   des.monster("黒ナーガの雛")
   des.monster("黒ナーガ")
end

--#medusa-4.lua
