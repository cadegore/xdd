set(FS_SRC
  sg.c
  xint_preallocate.c
  xint_pretruncate.c
  )

add_library(fs OBJECT ${FS_SRC})
