add_pk3(game_support.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static)

generate_language_files(game_support_pk3 zdoom_game_strings static/language.csv)
generate_language_files(game_support_pk3 chex_quest_3       static/filter/chex.chex3/language.csv)
generate_language_files(game_support_pk3 harmony            static/filter/harmony/language.csv)
generate_language_files(game_support_pk3 hacx               static/filter/hacx.hacx1/after_iwad/language.csv)
