set(BASE_SRC
  heartbeat.c
  io_buffers.c
  lockstep.c
  restart.c
  schedule.c
  target_cleanup.c
  target_init.c
  target_offset_table.c
  target_open.c
  target_pass.c
  target_pass_e2e_specific.c
  target_pass_wt_locator.c
  target_thread.c
  target_ttd_after_pass.c
  target_ttd_before_io_op.c
  target_ttd_before_pass.c
  verify.c
  worker_thread.c
  worker_thread_cleanup.c
  worker_thread_init.c
  worker_thread_io.c
  worker_thread_io_for_os.c
  worker_thread_ttd_after_io_op.c
  worker_thread_ttd_before_io_op.c
  # xint_nclk.c
  xint_plan.c
  )

add_library(base OBJECT ${BASE_SRC})
