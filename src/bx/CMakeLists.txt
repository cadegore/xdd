set(BX_SRC
  bx_buffer_queue.c
  bx_data_structures.h
  bx_target_thread.c
  bx_test.c
  bx_user_interface.c
  bx_worker_queue.c
  bx_worker_thread.c
  )

add_library(bx OBJECT ${BX_SRC})

set(BX_OTHER
  test_bxt.sh
  )

foreach(FILE ${BX_OTHER})
  configure_file("${FILE}" "${FILE}" COPYONLY)
endforeach()
