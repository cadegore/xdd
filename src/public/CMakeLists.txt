set(PUBLIC_SRC
  libxdd.c
  libxdd.h
  xdd_types.h
  xdd_types_internal.h
  )

add_library(public OBJECT ${PUBLIC_SRC})
