set(XNI_SRC
  xni.c
  xni.h
  xni_ib.c
  xni_internal.h
  xni_tcp.c
  xni_udt.c
  )

add_library(xni OBJECT ${XNI_SRC})
