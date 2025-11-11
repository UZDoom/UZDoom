add_pk3(uzdoom.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static)

generate_language_files(uzdoom_pk3 common static/language.0)
generate_language_files(uzdoom_pk3 zdoom_engine_strings static/language.csv)
