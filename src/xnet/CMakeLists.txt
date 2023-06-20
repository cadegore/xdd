set(XNET_SRC
  xnet_end_to_end.c
  xnet_end_to_end_init.c
  xnet_utils.c
  )

add_library(xnet OBJECT ${XNET_SRC})
