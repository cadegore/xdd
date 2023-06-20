set(NET_SRC
  end_to_end.c
  end_to_end_init.c
  net_utils.c
  read_after_write.c
  )

add_library(net OBJECT ${NET_SRC})
