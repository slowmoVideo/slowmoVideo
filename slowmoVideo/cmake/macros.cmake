
macro(copy_files FILELIST)
  foreach (FILENAME ${FILELIST})
    set(FILE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(FILE_DST "${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}")
    message(-- " Copying " ${FILENAME} " to " ${FILE_DST})

    configure_file(${FILE_SRC} ${FILE_DST} COPYONLY)
  endforeach(FILENAME)
endmacro(copy_files)

macro(copy_files_basedir FILELIST)
  foreach (FILENAME ${FILELIST})
    set(FILE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(FILE_DST "${CMAKE_BINARY_DIR}/${FILENAME}")
    message(-- " Copying ${FILE_SRC} to ${FILE_DST}")

    configure_file(${FILE_SRC} ${FILE_DST} COPYONLY)
  endforeach(FILENAME)
endmacro(copy_files_basedir)
