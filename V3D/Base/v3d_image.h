// -*- C++ -*-
#ifndef V3D_IMAGE_H
#define V3D_IMAGE_H

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <Base/v3d_exception.h>
#include <fstream>
#include <vector>
#include <stack>
#include <Math/v3d_linear.h>
#ifdef V3DLIB_ENABLE_IMDEBUG
#  include <imdebug.h>
#endif

namespace V3D
{

   template <typename Elem>
   struct Image
   {
         typedef Elem         value_type;
         typedef Elem const * const_iterator;
         typedef Elem       * iterator;

         Image()
            : _width(0), _height(0), _nChannels(0), _planeSize(0), _data(0)
         { }

         // (Deep) Copy constructor.  I wanted this for use with STL
         // in non-performance-critical code.
         // Otherwise, please degenerate the copy constructor by
         // declaring it private and not defining it.
         Image( const Image<Elem> &im )
            : _width(0), _height(0), _nChannels(0), _planeSize(0), _data(0)
         {
            copyFrom(im);
         }

         Image(int w, int h, int nChannels = 1)
            : _width(w), _height(h), _nChannels(nChannels), _data(0)
         {
            _planeSize = w*h;
            _data = new Elem[w*h*nChannels];
         }

         Image(int w, int h, int nChannels, Elem const& value)
            : _width(w), _height(h), _nChannels(nChannels), _data(0)
         {
            _planeSize = w*h;
            _data = new Elem[w*h*nChannels];
            std::fill(_data, _data+w*h*nChannels, value);
         }

         ~Image()
         {
            if (_data) delete [] _data;
         }

         Image<Elem>& operator=(Image<Elem> const& rhs)
         {
            this->copyFrom(rhs);
            return *this;
         }

         void resize(int w, int h, int nChannels = 1)
         {
            if(w==_width && h==_height && nChannels==_nChannels)
                return;
            if (_data) delete [] _data;
            _width = w;
            _height = h;
            _nChannels = nChannels;
            _planeSize = w*h;
            _data = new Elem[w*h*nChannels];
         }

         void resize(int w, int h, int nChannels, Elem const& value)
         {
            this->resize(w, h, nChannels);
            std::fill(_data, _data+w*h*nChannels, value);
         }

         template <typename Elem2>
         void copyFrom( const Image<Elem2> &im )
         {
            resize(im.width(),im.height(),im.numChannels());
            for(int i=0; i<im.numChannels(); i++)
               std::copy(im.begin(i),im.end(i),begin(i));
         }

         void fill( Elem val )
         {
            std::fill(_data, _data+_width*_height*_nChannels, val);
         }

         unsigned int width()       const { return _width; }
         unsigned int height()      const { return _height; }
         unsigned int numChannels() const { return _nChannels; }

         bool contains( int x, int y ) const
         {
            return x>=0 && x<_width && y>=0 && y<_height;
         }

         Elem const& operator()(int x, int y, int channel = 0) const
         {
            return _data[channel*_planeSize + y*_width + x];
         }

         Elem& operator()(int x, int y, int channel = 0)
         {
            return _data[channel*_planeSize + y*_width + x];
         }

         const_iterator begin(int channel = 0) const { return _data + channel*_planeSize; }
         iterator       begin(int channel = 0)       { return _data + channel*_planeSize; }

         const_iterator end(int channel = 0) const { return _data + (channel+1)*_planeSize; }
         iterator       end(int channel = 0)       { return _data + (channel+1)*_planeSize; }

         Elem const& minElement(int chan = 0) const { return *min_element(this->begin(chan), this->end(chan)); }
         Elem&       minElement(int chan = 0)       { return *min_element(this->begin(chan), this->end(chan)); }

         Elem const& maxElement(int chan = 0) const { return *max_element(this->begin(chan), this->end(chan)); }
         Elem&       maxElement(int chan = 0)       { return *max_element(this->begin(chan), this->end(chan)); }

      protected:
         int    _width, _height, _nChannels, _planeSize;
         Elem * _data;
   }; // end struct Image

//----------------------------------------------------------------------

   struct ImageFileStat
   {
         unsigned width, height;
         unsigned numChannels;
         unsigned bitDepth;
   }; // end struct ImageFileStat

   void statImageFile(char const * fileName, ImageFileStat& stat);
   void loadImageFile(char const * fileName, Image<unsigned char>& image);
   void saveImageFile(Image<unsigned char> const& image, char const * fileName);

   //! Reads PPM (P6) and PGM (P5) image files.
   void statPNMImageFile(char const * fileName, ImageFileStat& stat);
   void loadPNMImageFile(char const * fileName, Image<unsigned char>& image);
   void savePNMImageFile(Image<unsigned char> const& image, char const * fileName);

#if defined(V3DLIB_ENABLE_LIBJPEG)
   void statJPGImageFile(char const * fileName, ImageFileStat& stat);
   void loadJPGImageFile(char const * fileName, Image<unsigned char>& image);
   void saveJPGImageFile(Image<unsigned char> const& image, char const * fileName, int quality = 85);
#endif

#if defined(V3DLIB_ENABLE_LIBPNG)
   void statPNGImageFile(char const * fileName, ImageFileStat& stat);
   void loadPNGImageFile(char const * fileName, Image<unsigned char>& image);
   void savePNGImageFile(Image<unsigned char> const& image, char const * fileName);
#endif

   inline void statDataImageFile(const char *filename, ImageFileStat &stat)
   {
      std::ifstream in(filename,std::ios_base::binary|std::ios_base::in);
      verify(in.is_open(),"Failed to open image data file.");
      in.read((char*)&stat.width,sizeof(unsigned int));
      in.read((char*)&stat.height,sizeof(unsigned int));
      in.read((char*)&stat.numChannels,sizeof(unsigned int));
      in.read((char*)&stat.bitDepth,sizeof(unsigned int));
   }

   template<typename Elem>
   void loadDataImageFile(const char *filename, Image<Elem> &image)
   {
      // Stat file.
      ImageFileStat stat;
      unsigned int type;
      std::ifstream in(filename,std::ios_base::binary|std::ios_base::in);
      verify(in.is_open(),"Failed to open image data file.");
      in.read((char*)&stat.width,sizeof(unsigned int));
      in.read((char*)&stat.height,sizeof(unsigned int));
      in.read((char*)&stat.numChannels,sizeof(unsigned int));
      in.read((char*)&stat.bitDepth,sizeof(unsigned int));
      in.read((char*)&type,sizeof(unsigned int));
      // TODO: Create a mechanism for getting a type constant.
      //       Then type can be verified exactly.
      // Verify bit depth of type.
      verify(sizeof(Elem)*8==stat.bitDepth,"Image data incompatible with type.");
      // Read image.
      image.resize(stat.width,stat.height,stat.numChannels);
      in.read((char*)&image(0,0,0),sizeof(Elem)*stat.width*stat.height*stat.numChannels);
   }

   template<typename Elem>
   void saveDataImageFile(const Image<Elem> &image, const char *filename)
   {
      // Open file.
      std::ofstream out(filename,std::ios_base::out|std::ios_base::binary);
      verify(out.is_open(),"Failed to open iamge data file.");
      // Write image stat.
      ImageFileStat stat;
      stat.width = image.width();
      stat.height = image.height();
      stat.numChannels = image.numChannels();
      stat.bitDepth = sizeof(Elem)*8;
      // TODO: Create a mechanism for getting a type constant.
      //       Then type can be stored exactly.
      unsigned int type = 0xffffffff;
      out.write((char*)&stat.width,sizeof(unsigned int));
      out.write((char*)&stat.height,sizeof(unsigned int));
      out.write((char*)&stat.numChannels,sizeof(unsigned int));
      out.write((char*)&stat.bitDepth,sizeof(unsigned int));
      out.write((char*)&type,sizeof(unsigned int));
      // Write image data.
      out.write((char*)&image(0,0,0),sizeof(Elem)*image.width()*image.height()*image.numChannels());
   }

   template <typename Elem>
   inline void
   saveImageChannel(Image<Elem> const& im, int channel, Elem minVal, Elem maxVal, char const * name)
   {
      int const w = im.width();
      int const h = im.height();

      Elem const len = maxVal - minVal;
      Image<unsigned char> byteIm(w, h, 1);

      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            Elem c = std::max(Elem(0), std::min(Elem(255), Elem(255) * (im(x, y, channel) - minVal) / len));
            byteIm(x, y) = int(c);
         }

      saveImageFile(byteIm, name);
   } // end saveImageChannel()

   template <typename Elem>
   inline void
   saveImageChannel(Image<Elem> const& im, int channel, char const * name)
   {
      int const w = im.width();
      int const h = im.height();

      if (w == 0 || h == 0) return;

      Elem minVal = im(0, 0, channel);
      Elem maxVal = minVal;

      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            minVal = std::min(minVal, im(x, y, channel));
            maxVal = std::max(maxVal, im(x, y, channel));
         }

      saveImageChannel(im, channel, minVal, maxVal, name);
   } // end saveImageChannel()

   template <typename Elem>
   inline void
   copyImageChannel(Image<Elem> const& im, int channel, Image<Elem> &out)
   {
       out.resize(im.width(),im.height(),1);
       for(int y=0; y<im.height(); y++)
           for(int x=0; x<im.width(); x++)
               out(x,y) = im(x,y,channel);
   }

} // end namespace V3D

#endif
