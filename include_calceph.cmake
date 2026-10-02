include(FetchContent)
  FetchContent_Declare(
  cal_ceph
  GIT_REPOSITORY https://gitlab.com/mipt_ballistics/third-party/calceph
  GIT_TAG 6b71954bac379d1235ac4dc16e827a77c04872c8)
  FetchContent_MakeAvailable(cal_ceph)

