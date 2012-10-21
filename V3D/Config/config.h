#ifndef CONFIG_H
#define CONFIG_H

/**

  This header file is for directly including the shader files in the binary
  which makes it easier to ship an exectuable.

  The #defines are only for QtCreator which does not support the CMake add_definitions yet.

  -- Simon A. Eugster
  */

#ifndef DISABLE_REDEFINITIONS
#define V3DLIB_ENABLE_LIBJPEG
#define V3DLIB_ENABLE_LIBPNG
#define V3DLIB_ENABLE_GPGPU
#endif

#endif // CONFIG_H
