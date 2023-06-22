set(CLIENT_SRC
  info_display.c
  initialization.c
  interactive.c
  interactive_func.c
  interactive_table.c
  parse.c
  parse_func.c
  parse_table.c
  results_display.c
  results_manager.c
  signals.c
  utils.c
  )

add_library(client OBJECT ${CLIENT_SRC})

add_library(xdd-client OBJECT xdd.c)
