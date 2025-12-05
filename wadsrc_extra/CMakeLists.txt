add_pk3(game_support.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static)

generate_language_files(game_support_pk3 static/language.csv
	games/doom
	games/doom2
	games/chex
	games/heretic
	games/hexen
	games/hexen-deathkings
	games/plutonia
	games/tnt
	games/strife)
generate_language_files(game_support_pk3 static/filter/chex.chex3/language.csv
	games/filter/chex_quest_3)
generate_language_files(game_support_pk3 static/filter/harmony/language.csv
	games/filter/harmony)
generate_language_files(game_support_pk3 static/filter/hacx.hacx1/after_iwad/language.csv
	games/filter/hacx)
