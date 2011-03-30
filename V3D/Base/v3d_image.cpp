#include "Base/v3d_image.h"
#include "Base/v3d_exception.h"

#include <cstdio>
#include <string>
#include <iostream>
#include <limits.h> // for INT_MAX

#if defined(V3DLIB_ENABLE_LIBJPEG)
extern "C"
{
# include <jpeglib.h>
}
#endif

#if defined(V3DLIB_ENABLE_LIBPNG)
# include <png.h>
#endif

using namespace std;

namespace
{

   // Adapted from netpbm

   inline bool
   pnm_getc(FILE * const file, char& dst)
   {
      int ich;

      ich = getc(file);
      if (ich == EOF) return false; // premature EOF
      dst = (char) ich;

      // Skip comments
      if (dst == '#')
      {
         do
         {
	    ich = getc(file);
	    if (ich == EOF) return false; // premature EOF
	    dst = (char) ich;
         } while (dst != '\n' && dst != '\r');
      }

      return true;
   } // end pnm_getc()

   inline bool
   pnm_getuint(FILE * const file, unsigned int& res)
   {
      char ch;
      unsigned int i;

      do
      {
         if (!pnm_getc(file, ch)) return false;
      }
      while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

      if (ch < '0' || ch > '9') return false;

      res = 0;
      do
      {
         unsigned int const digitVal = ch - '0';

         if (res > INT_MAX/10 - digitVal) return false;

         res = res * 10 + digitVal;

         if (!pnm_getc(file, ch)) return false;
      }
      while (ch >= '0' && ch <= '9');

      return true;
   } // end pnm_getuint()

//----------------------------------------------------------------------

#if defined(V3DLIB_ENABLE_LIBPNG)
   /* our method that reads from a FILE* and fills up the buffer that
      libpng wants when parsing a PNG file */
   void
   user_read_callback(png_structp png_ptr, png_bytep data, png_uint_32 length)
   {
      unsigned readlen = fread(data, 1, length, (FILE *)png_get_io_ptr(png_ptr));
      if (readlen != length)
      {
         /* FIXME: then what? png_error()? 20020821 mortene */
      }
   }

   /* our method that write compressed png image data to a FILE* */
   void
   user_write_callback(png_structp png_ptr, png_bytep data, png_uint_32 length)
   {
      unsigned writelen = fwrite(data, 1, length, (FILE *)png_get_io_ptr(png_ptr));
      if (writelen != length)
      {
         /* FIXME: then what? png_error()? 20020821 mortene */
      }
   }

   /* our method that flushes written compressed png image data */
   void
   user_flush_callback(png_structp png_ptr)
   {
      int err = fflush((FILE *)png_get_io_ptr(png_ptr));
      if (err != 0)
      {
         /* FIXME: then what? png_error()? 20020821 mortene */
      }
   }
#endif // defined(V3DLIB_ENABLE_LIBPNG)

//----------------------------------------------------------------------

   enum ImageFileType
   {
      V3D_IMAGE_FILE_TYPE_UNKNOWN = -1,
      V3D_IMAGE_FILE_TYPE_PNM = 0,
      V3D_IMAGE_FILE_TYPE_JPEG = 1,
      V3D_IMAGE_FILE_TYPE_PNG = 2,
   };

   inline ImageFileType
   determineImageFileType(string const& fileName)
   {
      size_t extStart = fileName.find_last_of("./\\");
      if (extStart == fileName.npos || fileName[extStart] != '.')
         return V3D_IMAGE_FILE_TYPE_UNKNOWN;

      string const extension = fileName.substr(extStart+1);
      if (extension == "jpg" || extension == "JPG" ||
          extension == "jpeg" || extension == "JPEG") return V3D_IMAGE_FILE_TYPE_JPEG;

      if (extension == "png" || extension == "PNG") return V3D_IMAGE_FILE_TYPE_PNG;

      if (extension == "ppm" || extension == "PPM" ||
          extension == "pgm" || extension == "PGM") return V3D_IMAGE_FILE_TYPE_PNM;

      return V3D_IMAGE_FILE_TYPE_UNKNOWN;
   } // end determineImageFileType()

} // end namespace <>

namespace V3D
{

   void
   statImageFile(char const * fileName, ImageFileStat& stat)
   {
      ImageFileType fileType = determineImageFileType(std::string(fileName));
      switch (fileType)
      {
         case V3D_IMAGE_FILE_TYPE_PNM:
            statPNMImageFile(fileName, stat);
            return;
#if defined(V3DLIB_ENABLE_LIBJPEG)
         case V3D_IMAGE_FILE_TYPE_JPEG:
            statJPGImageFile(fileName, stat);
            return;
#endif
#if defined(V3DLIB_ENABLE_LIBPNG)
         case V3D_IMAGE_FILE_TYPE_PNG:
            statPNGImageFile(fileName, stat);
            return;
#endif
         default:
            throw Exception(__FILE__, __LINE__, "Unkown or unsupported image file extension.");
      } // end switch()
   } // end statImageFile()

   void
   loadImageFile(char const * fileName, Image<unsigned char>& image)
   {
      ImageFileType fileType = determineImageFileType(std::string(fileName));
      switch (fileType)
      {
         case V3D_IMAGE_FILE_TYPE_PNM:
            loadPNMImageFile(fileName, image);
            return;
#if defined(V3DLIB_ENABLE_LIBJPEG)
         case V3D_IMAGE_FILE_TYPE_JPEG:
            loadJPGImageFile(fileName, image);
            return;
#endif
#if defined(V3DLIB_ENABLE_LIBPNG)
         case V3D_IMAGE_FILE_TYPE_PNG:
            loadPNGImageFile(fileName, image);
            return;
#endif
         default:
            throw Exception(__FILE__, __LINE__, "Unkown or unsupported image file extension.");
      } // end switch()
   } // end loadImageFile()

   void
   saveImageFile(Image<unsigned char> const& image, char const * fileName)
   {
      ImageFileType fileType = determineImageFileType(std::string(fileName));
      switch (fileType)
      {
         case V3D_IMAGE_FILE_TYPE_PNM:
            savePNMImageFile(image, fileName);
            return;
#if defined(V3DLIB_ENABLE_LIBJPEG)
         case V3D_IMAGE_FILE_TYPE_JPEG:
            saveJPGImageFile(image, fileName);
            return;
#endif
#if defined(V3DLIB_ENABLE_LIBPNG)
         case V3D_IMAGE_FILE_TYPE_PNG:
            savePNGImageFile(image, fileName);
            return;
#endif
         default:
            throw Exception(__FILE__, __LINE__, "Unkown or unsupported image file extension.");
      } // end switch()
   } // end saveImageFile()

   void
   statPNMImageFile(char const * fileName, ImageFileStat& stat)
   {
      stat.width = stat.height = stat.numChannels = stat.bitDepth = -1;

      FILE * file = fopen(fileName, "rb");
      if (!file) throw Exception(__FILE__, __LINE__, "Cannot open PNM image file.");;

      int magic[2];
      magic[0] = getc(file);
      magic[1] = getc(file);

      if (magic[0] != 'P' && (magic[1] != '5' || magic[1] != '6'))
         throw Exception(__FILE__, __LINE__, "Wrong or unsupported PNM magic number.");

      unsigned int width, height, maxVal;
      if (!pnm_getuint(file, width)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
      if (!pnm_getuint(file, height)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
      if (!pnm_getuint(file, maxVal)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");

      stat.numChannels = (magic[1] == '5') ? 1 : 3;
      stat.width = width;
      stat.height = height;
      stat.bitDepth = (maxVal > 255) ? 16 : 8;

      fclose(file);
   } // end statPNMImageFile()

   void
   loadPNMImageFile(char const * fileName, Image<unsigned char>& image)
   {
      FILE * file = fopen(fileName, "rb");
      if (!file) throw Exception(__FILE__, __LINE__, "Cannot open PNM image file.");

      int magic[2];
      magic[0] = getc(file);
      magic[1] = getc(file);

      if (magic[0] == 'P' && magic[1] == '6')
      {
         unsigned int width, height, maxVal;
         if (!pnm_getuint(file, width)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
         if (!pnm_getuint(file, height)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
         if (!pnm_getuint(file, maxVal)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");

         if (maxVal > 255) // Does not fit in unsigned char
            throw Exception(__FILE__, __LINE__, "PNM image file has unsupported bit depth.");

         image.resize(width, height, 3);
         unsigned char * pixels = new unsigned char[width*height*3];

         fread(pixels, width*height*3, 1, file);

         for (int y = 0; y < height; ++y)
         {
            int const rowOfs = 3*width*y;
            for (int x = 0; x < width; ++x)
            {
               image(x, y, 0) = pixels[rowOfs + 3*x + 0];
               image(x, y, 1) = pixels[rowOfs + 3*x + 1];
               image(x, y, 2) = pixels[rowOfs + 3*x + 2];
            }
         }

         delete [] pixels;
      }
      else if (magic[0] == 'P' && magic[1] == '5')
      {
         unsigned int width, height, maxVal;
         if (!pnm_getuint(file, width)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
         if (!pnm_getuint(file, height)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");
         if (!pnm_getuint(file, maxVal)) throw Exception(__FILE__, __LINE__, "Cannot read PNM header.");

         if (maxVal > 255) // Does not fit in unsigned char
            throw Exception(__FILE__, __LINE__, "PNM image file has unsupported bit depth.");

         image.resize(width, height, 1);

         fread(image.begin(), width*height, 1, file);
      }

      fclose(file);
   } // end loadPNMImageFile()

   void
   savePNMImageFile(Image<unsigned char> const& image, char const * fileName)
   {
      int const width = image.width();
      int const height = image.height();
      int const nPlanes = image.numChannels();

      if (nPlanes == 3)
      {
         unsigned char * pixels = new unsigned char[width*height*3];

         for (int y = 0; y < height; ++y)
         {
            int const rowOfs = 3*width*y;
            for (int x = 0; x < width; ++x)
            {
               pixels[rowOfs + 3*x + 0] = image(x, y, 0);
               pixels[rowOfs + 3*x + 1] = image(x, y, 1);
               pixels[rowOfs + 3*x + 2] = image(x, y, 2);
            }
         }

         FILE * file = fopen(fileName, "wb");
         if (!file) throw Exception(__FILE__, __LINE__, "Cannot open PNM image file for writing.");

         fprintf(file, "P6\n%i %i\n%i\n", width, height, 255);
         fwrite(pixels, 3*width*height, 1, file);
         fclose(file);

         delete [] pixels;
      }
      else if (nPlanes == 1)
      {
         FILE * file = fopen(fileName, "wb");
         fprintf(file, "P5\n%i %i\n%i\n", width, height, 255);
         fwrite(image.begin(), width*height, 1, file);
         fclose(file);
      }
   }  // end savePNMImageFile()

#if defined(V3DLIB_ENABLE_LIBJPEG)
   void
   statJPGImageFile(char const * fileName, ImageFileStat& stat)
   {
      stat.width = stat.height = stat.numChannels = stat.bitDepth = -1;

      struct jpeg_decompress_struct cinfo;
      struct jpeg_error_mgr jerr;

      FILE * infile;                /* source file */

      if ((infile = fopen(fileName, "rb")) == NULL)
         throw Exception(__FILE__, __LINE__, "Cannot open JPG image file.");

      cinfo.err = jpeg_std_error(&jerr);

      /* Now we can initialize the JPEG decompression object. */
      jpeg_create_decompress(&cinfo);

      jpeg_stdio_src(&cinfo, infile);
      jpeg_read_header(&cinfo, TRUE);

      stat.width       = cinfo.image_width;
      stat.height      = cinfo.image_height;
      stat.numChannels = cinfo.num_components;
      stat.bitDepth    = 8;

      jpeg_destroy_decompress(&cinfo);
      fclose(infile);
   } // end statJPGImageFile()

   void
   loadJPGImageFile(char const * fileName, Image<unsigned char>& image)
   {
      struct jpeg_decompress_struct cinfo;
      struct jpeg_error_mgr jerr;

      FILE * infile;                /* source file */
      JSAMPARRAY buffer;            /* Output row buffer */
      int row_stride;               /* physical row width in output buffer */

      if ((infile = fopen(fileName, "rb")) == NULL)
         throw Exception(__FILE__, __LINE__, "Cannot open JPG image file.");

      cinfo.err = jpeg_std_error(&jerr);

      /* Now we can initialize the JPEG decompression object. */
      jpeg_create_decompress(&cinfo);

      jpeg_stdio_src(&cinfo, infile);
      jpeg_read_header(&cinfo, TRUE);
      jpeg_start_decompress(&cinfo);

      int const nPlanes = cinfo.num_components;
      int const w = cinfo.image_width;
      int const h = cinfo.image_height;
      image.resize(w, h, nPlanes);

      row_stride = w * nPlanes;
      /* Make a one-row-high sample array that will go away when done with image */
      buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

      while ((int)cinfo.output_scanline < h)
      {
         int const y = cinfo.output_scanline;
         /* jpeg_read_scanlines expects an array of pointers to scanlines.
          * Here the array is only one element long, but you could ask for
          * more than one scanline at a time if that's more convenient.
          */
         jpeg_read_scanlines(&cinfo, buffer, 1);

         for (int x = 0; x < w; ++x)
            for (int plane = 0; plane < nPlanes; ++plane)
               image(x, y, plane) = buffer[0][nPlanes*x + plane];
      } // end while

      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      fclose(infile);
   } // end loadJPGImageFile()

   void
   saveJPGImageFile(Image<unsigned char> const& image, char const * fileName, int quality)
   {
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;
      FILE * outfile;               /* target file */
      JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
      int row_stride;               /* physical row width in image buffer */

      int const w = image.width();
      int const h = image.height();
      int const nPlanes = image.numChannels();

      cinfo.err = jpeg_std_error(&jerr);
      /* Now we can initialize the JPEG compression object. */
      jpeg_create_compress(&cinfo);

      if ((outfile = fopen(fileName, "wb")) == NULL)
         throw Exception(__FILE__, __LINE__, "Cannot open JPG image file for writing.");

      jpeg_stdio_dest(&cinfo, outfile);

      cinfo.image_width = w;      /* image width and height, in pixels */
      cinfo.image_height = h;
      cinfo.input_components = nPlanes; /* # of color components per pixel */
      if (nPlanes == 3)
         cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
      else
         cinfo.in_color_space = JCS_GRAYSCALE; /* grayscale of input image */

      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
      jpeg_start_compress(&cinfo, TRUE);
      row_stride = w * nPlanes; /* JSAMPLEs per row in image_buffer */

      row_pointer[0] = new unsigned char[row_stride];

      while (cinfo.next_scanline < cinfo.image_height)
      {
         /* jpeg_write_scanlines expects an array of pointers to scanlines.
          * Here the array is only one element long, but you could pass
          * more than one scanline at a time if that's more convenient.
          */
         int const y = cinfo.next_scanline;
         for (int x = 0; x < w; ++x)
            for (int plane = 0; plane < nPlanes; ++plane)
               row_pointer[0][nPlanes*x + plane] = image(x, y, plane);

         jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }

      delete [] row_pointer[0];

      jpeg_finish_compress(&cinfo);
      fclose(outfile);

      jpeg_destroy_compress(&cinfo);
   } // end saveJPGImageFile()

#endif // #defined(V3DLIB_ENABLE_LIBJPEG)

#if defined(V3DLIB_ENABLE_LIBPNG)
   void
   statPNGImageFile(char const * fileName, ImageFileStat& stat)
   {
      stat.width = stat.height = stat.numChannels = stat.bitDepth = -1;

      FILE *fp = fopen(fileName, "rb");
      if (!fp) throw Exception(__FILE__, __LINE__, "Cannot open PNG image file.");

      unsigned char header[8];
      png_uint_32 width, height;
      int bit_depth, color_type, interlace_type;

      fread(header, 1, 8, fp);
      bool is_png = !png_sig_cmp(header, 0, 8);
      if (!is_png) throw Exception(__FILE__, __LINE__, "Cannot read PNG image header.");

      png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

      png_infop info_ptr = png_create_info_struct(png_ptr);

      /*  we're not using png_init_io(), as we don't want to pass a FILE*
          into libpng, in case it's an MSWindows DLL with a different CRT
          (C run-time library) */

      fseek(fp, 0, SEEK_SET);
      png_set_read_fn(png_ptr, (void *)fp, (png_rw_ptr)user_read_callback);
      /* The call to png_read_info() gives us all of the information from the
       * PNG file before the first IDAT (image data chunk).  REQUIRED
       */
      png_read_info(png_ptr, info_ptr);

      png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                   &interlace_type, NULL, NULL);

      stat.width    = width;
      stat.height   = height;
      stat.bitDepth = bit_depth;

      switch (color_type)
      {
         case PNG_COLOR_TYPE_GRAY:
            stat.numChannels = 1;
            break;
         case PNG_COLOR_TYPE_GRAY_ALPHA:
            stat.numChannels = 2;
            break;
         case PNG_COLOR_TYPE_RGB:
            stat.numChannels = 3;
            break;
         case PNG_COLOR_TYPE_RGB_ALPHA:
            stat.numChannels = 4;
            break;
         default:
            throw Exception(__FILE__, __LINE__, "Unsupported number of channels in PNG image file.");
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

      fclose(fp);
   } // end statPNGImageFile()

   void
   loadPNGImageFile(char const * fileName, Image<unsigned char>& image)
   {
      FILE *fp = fopen(fileName, "rb");
      if (!fp) throw Exception(__FILE__, __LINE__, "Cannot open PNG image file.");

      unsigned char header[8];
      png_uint_32 width, height;
      int bit_depth, color_type, interlace_type;

      fread(header, 1, 8, fp);
      bool is_png = !png_sig_cmp(header, 0, 8);
      if (!is_png) throw Exception(__FILE__, __LINE__, "Cannot read PNG image header.");

      {
         png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

         png_infop info_ptr = png_create_info_struct(png_ptr);

         /*  we're not using png_init_io(), as we don't want to pass a FILE*
             into libpng, in case it's an MSWindows DLL with a different CRT
             (C run-time library) */

         fseek(fp, 0, SEEK_SET);
         png_set_read_fn(png_ptr, (void *)fp, (png_rw_ptr)user_read_callback);
         /* The call to png_read_info() gives us all of the information from the
          * PNG file before the first IDAT (image data chunk).  REQUIRED
          */
         png_read_info(png_ptr, info_ptr);

         png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                      &interlace_type, NULL, NULL);

         /* tell libpng to strip 16 bit/color files down to 8 bits/color */
         png_set_strip_16(png_ptr);

         /* expand paletted colors into true RGB triplets */
         if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);

         /* expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
         if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand(png_ptr);

         /* expand paletted or RGB images with transparency to full alpha channels
          * so the data will be available as RGBA quartets */
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);

         png_read_update_info(png_ptr, info_ptr);

         int nChannels = png_get_channels(png_ptr, info_ptr);

         image.resize(width, height, nChannels);

         int bytes_per_row = png_get_rowbytes(png_ptr, info_ptr);

         unsigned char * buffer = new unsigned char[bytes_per_row * height];

         png_bytepp row_pointers = new png_bytep[height];
         for (unsigned y = 0; y < height; y++)
            row_pointers[y] = buffer + y*bytes_per_row;

         png_read_image(png_ptr, row_pointers);
         png_read_end(png_ptr, info_ptr);

         delete [] row_pointers;

         png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
         fclose(fp);

         for (int chan = 0; chan < nChannels; ++chan)
         {
            unsigned char * p = image.begin(chan);
            for (unsigned y = 0; y < height; ++y)
               for (unsigned  x = 0; x < width; ++x, ++p)
                  *p = buffer[y*bytes_per_row + nChannels*x + chan];
         }

         delete [] buffer;
      } // end scope
   } // end loadImageFilePNG()

   void
   savePNGImageFile(Image<unsigned char> const& image, char const * fileName)
   {
      /* open the file */
      FILE * fp = fopen(fileName, "wb");
      if (!fp) throw Exception(__FILE__, __LINE__, "Cannot open PNG image file for writing.");
  
      /* Create and initialize the png_struct with the desired error handler
       * functions.  If you want to use the default stderr and longjump method,
       * you can supply NULL for the last three parameters.  We also check that
       * the library version is compatible with the one used at compile time,
       * in case we are using dynamically linked libraries.  REQUIRED.
       */
      png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                    NULL, NULL, NULL);

      if (png_ptr == NULL) throw Exception(__FILE__, __LINE__, "Cannot create PNG structures for writing.");

      /* Allocate/initialize the image information data.  REQUIRED */
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (info_ptr == NULL) throw Exception(__FILE__, __LINE__, "Cannot create PNG structures for writing.");

      /* Set error handling.  REQUIRED if you aren't supplying your own
       * error hadnling functions in the png_create_write_struct() call.
       */
//       if (setjmp(png_ptr->jmpbuf)) {
//          /* If we get here, we had a problem reading the file */
//          fclose(fp);
//          png_destroy_write_struct(&png_ptr,  (png_infopp)info_ptr);
//          pngerror = ERR_PNGLIB_WRITE;
//          return 0;
//       }

      /*  we're not using png_init_io(), as we don't want to pass a FILE*
          into libpng, in case it's an MSWindows DLL with a different CRT
          (C run-time library) */
      png_set_write_fn(png_ptr, (void *)fp, (png_rw_ptr)user_write_callback,
                       (png_flush_ptr)user_flush_callback);
  
      /* Set the image information here.  Width and height are up to 2^31,
       * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
       * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
       * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
       * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
       * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
       * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
       */

      int colortype = PNG_COLOR_TYPE_RGB;

      unsigned const width = image.width();
      unsigned const height = image.height();
      unsigned const nChannels = image.numChannels();

      switch (nChannels)
      {
         case 1:
            colortype = PNG_COLOR_TYPE_GRAY;
            break;
         case 3:
            colortype = PNG_COLOR_TYPE_RGB;
            break;
         case 4:
            colortype = PNG_COLOR_TYPE_RGB_ALPHA;
            break;
         default:
            png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
            throw Exception(__FILE__, __LINE__, "Unsupported number of channels for writing a PNG image file.");
      }

      png_set_IHDR(png_ptr, info_ptr, width, height, 8, colortype,
                   PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      /* Optional gamma chunk is strongly suggested if you have any guess
       * as to the correct gamma of the image. */
      /* png_set_gAMA(png_ptr, info_ptr, gamma); */

      /* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */

      /* Write the file header information.  REQUIRED */
      png_write_info(png_ptr, info_ptr);

      /* Once we write out the header, the compression type on the text
       * chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
       * PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
       * at the end.
       */

      /* set up the transformations you want.  Note that these are
       * all optional.  Only call them if you want them. */

      /* invert monocrome pixels */
      /* png_set_invert(png_ptr); */

      /* Shift the pixels up to a legal bit depth and fill in
       * as appropriate to correctly scale the image */
      /* png_set_shift(png_ptr, &sig_bit);*/

      /* pack pixels into bytes */
      /* png_set_packing(png_ptr); */

      /* swap location of alpha bytes from ARGB to RGBA */
      /* png_set_swap_alpha(png_ptr); */

      /* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
       * RGB (4 channels -> 3 channels). The second parameter is not used. */
      /* png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE); */

      /* flip BGR pixels to RGB */
      /* png_set_bgr(png_ptr); */

      /* swap bytes of 16-bit files to most significant byte first */
      /* png_set_swap(png_ptr); */

      /* swap bits of 1, 2, 4 bit packed pixel formats */
      /* png_set_packswap(png_ptr); */


      /* The easiest way to write the image (you may have a different memory
       * layout, however, so choose what fits your needs best).  You need to
       * use the first method if you aren't handling interlacing yourself.
       */

      /* If you are only writing one row at a time, this works */
  
      unsigned const bytesperrow = width * nChannels;

      if (nChannels != 1)
      {
         unsigned char * buffer = new unsigned char[bytesperrow];

         for (unsigned y = 0; y < height; ++y)
         {
            for (unsigned chan = 0; chan < nChannels; ++chan)
            {
               unsigned char const * p = image.begin(chan) + y*width;

               for (unsigned x = 0; x < width; ++x, ++p)
                  buffer[x*nChannels + chan] = *p;
            }

            png_write_row(png_ptr, (png_bytep)buffer);
         } // end for (y)

         delete [] buffer;
      }
      else
      {
         // Faster path for grayscale images
         for (unsigned y = 0; y < height; ++y)
            png_write_row(png_ptr, (png_bytep)(image.begin() + y*width));
      } // end if
  
      /* You can write optional chunks like tEXt, zTXt, and tIME at the end
       * as well.
       */
  
      /* It is REQUIRED to call this to finish writing the rest of the file */
      png_write_end(png_ptr, info_ptr);

      /* if you allocated any text comments, free them here */

      /* clean up after the write, and free any memory allocated */
      png_destroy_write_struct(&png_ptr, &info_ptr);

      /* close the file */
      fclose(fp);

      /* that's it */
   } // end saveImageFilePNG()

#endif // defined(V3DLIB_ENABLE_LIBPNG)

} // end namespace V3D

