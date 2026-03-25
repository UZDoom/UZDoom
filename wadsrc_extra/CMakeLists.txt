add_pk3(game_support.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static $<$<BOOL:${BUILD_NONFREE}>:${CMAKE_CURRENT_SOURCE_DIR}/nonfree>)

generate_language_files(game_support_pk3 static/language.csv GAMES)
generate_language_files(game_support_pk3 static/filter/chex.chex3/language.csv GAMES_CHEX3)
generate_language_files(game_support_pk3 static/filter/harmony/language.csv GAMES_HARMONY)
generate_language_files(game_support_pk3 static/filter/hacx.hacx1/after_iwad/language.csv GAMES_HACX)
