// -*- C++ -*-

#include "config.h"

#ifndef V3D_IMAGEPROCESSING_H
#define V3D_IMAGEPROCESSING_H

#include "v3d_image.h"
#include "Math/v3d_linear.h"

namespace V3D {

   template <typename Elem, typename Elem2>
   Elem2 bilinearSample( const Image<Elem> &im, Elem2 x, Elem2 y, int c=0 )
   {
      int x0 = floor(x);
      int y0 = floor(y);
      Elem2 xf = x-x0;
      Elem2 yf = y-y0;
      return (1-yf)*(1-xf)*im(x0  ,y0  ,c) +
         (1-yf)*(  xf)*im(x0+1,y0  ,c) +
         (  yf)*(1-xf)*im(x0  ,y0+1,c) +
         (  yf)*(  xf)*im(x0+1,y0+1,c);
   }

   template <typename Elem, typename Elem2>
   Elem2 bilinearSampleBorder( const Image<Elem> &im, Elem2 x, Elem2 y, int c=0, 
                               Elem2 border = 0 )
   {
      int x0 = floor(x);
      int y0 = floor(y);
      Elem2 xf = x-x0;
      Elem2 yf = y-y0;
      if(x0>=0 && x0+1<im.width() && y0>=0 && y0+1<im.height()) {
         return (1-yf)*(1-xf)*im(x0  ,y0  ,c) +
            (1-yf)*(  xf)*im(x0+1,y0  ,c) +
            (  yf)*(1-xf)*im(x0  ,y0+1,c) +
            (  yf)*(  xf)*im(x0+1,y0+1,c);
      } else {
         return border;
      }
   }

   template<typename Elem, typename Elem2, typename Elem3>
   inline void
   convolveImage( const Image<Elem> &im, const Image<Elem2> &kernel, Image<Elem3> &out )
   {
      // NOTE:  This code could be much more optimized.

      verify(im.width() >= kernel.width(),"image must be larger than kernel");
      verify(im.height() >= kernel.height(),"image must be larger than kernel");
      verify(im.numChannels() == kernel.numChannels(),"number of channels must be the same");

      if (out.width() != im.width() ||
          out.height() != im.height() ||
          out.numChannels() != im.numChannels())
         out.resize(im.width(), im.height(), im.numChannels());

      int xx0, xx1, yy0, yy1, kx, ky, kx0, ky0;
      Elem2 sum, denom;

      for (int ch = 0; ch < im.numChannels(); ++ch)
      {
         for(int y = 0; y < im.height(); ++y)
         {
            yy0 = y - kernel.height()/2;
            yy1 = yy0 + kernel.height();
            yy0 = std::max(yy0, 0);
            yy1 = std::min(yy1, int(im.height()));

            ky0 = yy0 - y + kernel.height()/2;

            for(int x = 0; x < im.width(); ++x)
            {
               xx0 = x - kernel.width()/2;
               xx1 = xx0 + kernel.width();
               xx0 = std::max(xx0, 0);
               xx1 = std::min(xx1, int(im.width()));

               kx0 = xx0 - x + kernel.width()/2;

               sum = 0; denom = 0;
               for(int yy = yy0, ky = ky0; yy < yy1; ++yy, ++ky) {
                  for(int xx = xx0, kx = kx0; xx < xx1; ++xx, ++kx)
                  {
                     sum += im(xx, yy, ch) * kernel(kx, ky, ch);
                     denom += kernel(kx, ky, ch);
                  }
               }
               out(x, y, ch) = (Elem3)(sum / denom);
            } // end for (x)
         } // end for (y)
      } // end for (ch)
   } // end convolveImage()

   inline int choose(int n, int k)
   {
      if (k > n)
         return 0;

      if (k > n/2)
         k = n-k; // faster

      double accum = 1;
      for (int i = 1; i <= k; i++)
         accum = accum * (n-k+i) / i;

      return (int)(accum + 0.5); // avoid rounding error
   }

   template<typename Elem>
   inline void
   boxFilterImage( const Image<Elem> &im, int boxWidth, int boxHeight,
                   Image<Elem> &out, Image<Elem>& temp)
   {
      int x,y;
      int halfw = boxWidth/2;
      int halfh = boxHeight/2;
      double sum,weight;
      temp.resize(im.width(),im.height());
      out.resize(im.width(),im.height(),im.numChannels());
      for(int i=0; i<im.numChannels(); i++) {
         // Horizontal.
         for(y=0; y<im.height(); y++) {
            sum = 0;
            weight = 0;
            for(x=0; x<halfw; x++) {
               sum += im(x,y,i);
               weight++;
            }
            for(x=halfw; x<2*halfw+1; x++) {
               sum += im(x,y,i);
               weight++;
               temp(x-halfw,y) = (Elem)(sum/weight);
            }
            for(x=halfw+1; x<im.width()-halfw; x++) {
               sum -= im(x-halfw-1,y,i);
               sum += im(x+halfw,y,i);
               temp(x,y) = sum/weight;
            }
            for(; x<im.width(); x++) {
               sum -= im(x-halfw-1,y,i);
               weight--;
               temp(x,y) = sum/weight;
            }
         }
         // Vertical.
         for(x=0; x<im.width(); x++) {
            sum = 0;
            weight = 0;
            for(y=0; y<halfh; y++) {
               sum += temp(x,y);
               weight++;
            }
            for(y=halfh; y<2*halfh+1; y++) {
               sum += temp(x,y);
               weight++;
               out(x,y-halfh,i) = (Elem)(sum/weight);
            }
            for(y=halfh+1; y<im.height()-halfh; y++) {
               sum -= temp(x,y-halfh-1);
               sum += temp(x,y+halfh);
               out(x,y,i) = sum/weight;
            }
            for(; y<im.height(); y++) {
               sum -= temp(x,y-halfh-1);
               weight--;
               out(x,y,i) = sum/weight;
            }
         }
      }
   }

   template<typename Elem>
   inline void
   boxFilterImage( const Image<Elem> &im, int boxWidth, int boxHeight, Image<Elem> &out)
   {
      Image<Elem> temp;
      boxFilterImage(im, boxWidth, boxHeight, out, temp);
   }

   // Note: boxWidth and boxHeight must be odd
   template<typename Elem>
   inline void
   boxFilterImage_fast(Image<Elem> const& im, int boxWidth, int boxHeight,
                       Image<Elem>& out, Image<Elem>& temp)
   {
      int const w = im.width();
      int const h = im.height();
      int const nChannels = im.numChannels();

      int const W2 = boxWidth/2;
      int const H2 = boxHeight/2;

      Elem const WH = boxWidth*boxHeight;

      temp.resize(w, h, 1);
      out.resize(w, h, nChannels);

      std::vector<Elem> row(w+boxWidth);

      for (int ch = 0; ch < nChannels; ++ch)
      {
         // Horizontal pass
         for (int y = 0; y < h; ++y)
         {
            for (int x = 0; x < W2; ++x) row[x]    = im(0, y, ch);
            for (int x = 0; x < w; ++x)  row[x+W2] = im(x, y, ch);
            for (int x = 0; x < W2; ++x) row[w+x]  = im(w-1, y, ch);

            Elem sum = 0;
            for (int x = 0; x < boxWidth; ++x) sum += row[x];
            for (int x = 0; x < w-1; ++x)
            {
               temp(x, y) = sum;
               sum -= row[x];
               sum += row[x+boxWidth];
            }
            temp(w-1, y) = sum;
         } // end for (y)

         // Vertical pass
         for (int x = 0; x < w; ++x) row[x] = (H2+1)*temp(x, 0);
         for (int dy = 1; dy <= H2; ++dy)
            for (int x = 0; x < w; ++x) row[x] += temp(x, dy);
         for (int y = 0; y < h; ++y)
         {
            int const Y0 = std::max(0, y-H2);
            int const Y1 = std::min(h-1, y+H2+1);

            for (int x = 0; x < w; ++x)
            {
               out(x, y, ch) = row[x] / WH;
               row[x] -= temp(x, Y0);
               row[x] += temp(x, Y1);
            } // end for (x)
         } // end for (y)
      } // end for (ch)
   } // end boxFilterImage_fast()

   template<typename Elem>
   inline void
   boxFilterImage_fast(Image<Elem> const& im, int boxWidth, int boxHeight, Image<Elem> &out)
   {
      Image<Elem> temp;
      boxFilterImage_fast(im, boxWidth, boxHeight, out, temp);
   }

   template <typename Elem, typename BinaryFunc>
   inline void
   boxFilterImage_fast(Image<Elem> const &im1, Image<Elem> const& im2, int boxWidth, int boxHeight,
                       BinaryFunc op, Image<Elem>& out, Image<Elem>& temp, Image<Elem>& temp2)
   {
      int const w = im1.width();
      int const h = im2.height();

      temp2.resize(w, h, im1.numChannels());

      for (int ch = 0; ch < im1.numChannels(); ++ch)
      {
         for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
               temp2(x, y, ch) = op(im1(x, y, ch), im2(x, y, ch));
      }

      boxFilterImage_fast(temp2, boxWidth, boxHeight, out, temp);
   } // end boxFilterImage_fast()

   template <typename Elem, typename BinaryFunc>
   inline void
   boxFilterImage_fast(Image<Elem> const& im1, Image<Elem> const& im2,
                       int boxWidth, int boxHeight, BinaryFunc op, Image<Elem>& out)
   {
      Image<Elem> temp, temp2;
      boxFilterImage_fast(im1, im2, boxWidth, boxHeight, op, out, temp, temp2);
   }


   template<typename Elem>
   inline void binomialFilterImage( const Image<Elem> &im, int Nx, int Ny, Image<Elem> &out,
                                    Image<Elem>& temp )
   {
      // TODO:  Use factored binomial kernels for better speed.
      Image<float> kernelX(Nx+1,1,im.numChannels());
      Image<float> kernelY(1,Ny+1,im.numChannels());
      int x,y,i;
      for(i=0; i<im.numChannels(); i++)
      {
         for(x=0; x<=Nx; x++)
            kernelX(x,0,i) = (float)choose(Nx,x)/(float)(1<<Nx);
         for(y=0; y<=Ny; y++)
            kernelY(0,y,i) = (float)choose(Ny,y)/(float)(1<<Ny);
      }
      convolveImage(im,kernelX,temp);
      convolveImage(temp,kernelY,out);
   }

   template<typename Elem>
   inline void
   binomialFilterImage( const Image<Elem> &im, int Nx, int Ny, Image<Elem> &out)
   {
      Image<Elem> temp;
      binomialFilterImage(im, Nx, Ny, out, temp);
   }

   template<typename Elem>
   void meanFilterImage( const Image<Elem> &in, int w, int h, Image<Elem> &out,
                         Elem bias, Image<Elem>& temp)
   {
      boxFilterImage(in,w,h,out,temp);
      for(int i=0; i<in.numChannels(); i++)
         for(int y=0; y<in.height(); y++)
            for(int x=0; x<in.width(); x++)
               out(x,y,i) = in(x,y,i) - out(x,y,i) + bias;
   }

   template<typename Elem>
   void meanFilterImage( const Image<Elem> &in, int w, int h, Image<Elem> &out,
                         Elem bias = 0)
   {
      Image<Elem> temp;
      meanFilterImage(in, w, h, out, bias, temp);
   }

   template<typename Elem,typename Elem2>
   void rankFilterImage( const Image<Elem> &in, int w, int h, Image<Elem2> &out )
   {
      out.resize(in.width(),in.height(),in.numChannels());
      w = (w/2)*2+1;
      h = (h/2)*2+1;
      for(int i=0; i<in.numChannels(); i++) {
         for(int y=0; y<in.height(); y++) {
            for(int x=0; x<in.width(); x++) {
               int xx0 = max(0,x-w/2);
               int xx1 = min<int>(in.width()-1,x+w/2);
               int yy0 = max(0,y-h/2);
               int yy1 = min<int>(in.height()-1,y+h/2);
               out(x,y,i) = 0;
               for(int xx=xx0; xx<=xx1; xx++) {
                  for(int yy=yy0; yy<=yy1; yy++) {
                     if(in(x,y,i) > in(xx,yy,i))
                        out(x,y,i)++;
                  }
               }
               out(x,y,i) = double(out(x,y,i)) * w*h / ((xx1-xx0+1)*(yy1-yy0+1));
            }
         }
      }
   }

   template<typename Elem, typename Elem2>
   void resampleImage( const Image<Elem> &im, Image<Elem2> &out )
   {
      // TODO: Apply low-pass filter if downsampling.
      Image<Elem> im2(im.width(),im.height(),im.numChannels());
      binomialFilterImage(im,0,0,im2);

      verify(im.numChannels()==out.numChannels(),"number of channels must be the same");

      for(int y=0; y<out.height(); y++) {
         for(int x=0; x<out.width(); x++) {
            for(int i=0; i<out.numChannels(); i++) {
               float xp = (float)(x+0.5f)*im.width()/out.width()-0.5f;
               int xx = floor(xp);
               float xf = xp-xx;
               float yp = (float)(y+0.5f)*im.height()/out.height()-0.5f;
               int yy = floor(yp);
               float yf = yp-yy;
               if(xx<0) {
                  xx = 0;
                  xf = 0;
               }
               if(xx==im.width()-1) {
                  xf = 0;
               }
               if(yy<0) {
                  yy = 0;
                  yf = 0;
               }
               if(yy==im.height()-1) {
                  yf = 0;
               }
               out(x,y,i) = (1-xf)*(1-yf)*im2(xx  ,yy  ,i) +
                  (  xf)*(1-yf)*im2(xx+1,yy  ,i) +
                  (1-xf)*(  yf)*im2(xx  ,yy+1,i) +
                  (  xf)*(  yf)*im2(xx+1,yy+1,i);
            }
         }
      }
   }

   template<typename Elem>
   void floodFill( Image<Elem> &im, int x0, int y0, Elem val )
   {
      Elem initval = im(x0,y0);
      if(initval==val)
         return;
      int imw = im.width();
      int imh = im.height();
      std::stack<std::pair<int,int> > flood;
      flood.push(std::pair<int,int>(x0,y0));
      while(!flood.empty()) {
         int x = flood.top().first;
         int y = flood.top().second;
         flood.pop();
         //if(x>=0 && x<imw && y>=0 && y<imh && im(x,y)==initval) {
            im(x,y) = val;
            if(x<imw-1 && im(x+1,y)==initval)
               flood.push(std::pair<int,int>(x+1,y));
            if(x>0 && im(x-1,y)==initval)
               flood.push(std::pair<int,int>(x-1,y));
            if(y<imh-1 && im(x,y+1)==initval)
               flood.push(std::pair<int,int>(x,y+1));
            if(y>0 && im(x,y-1)==initval)
               flood.push(std::pair<int,int>(x,y-1));
         //}
      }
   }

//=========================================================================

    template<class Elem>
    class ImagePyramid
    {
    public:
        ImagePyramid() {}

        void resize( int w, int h, int channels, int levels )
        {
            _levels.resize(levels);
            for(int i=0; i<levels; i++)
                _levels[i].resize(w>>i,h>>i,channels);
        }

        void generate( const Image<Elem> &im, int levels )
        {
            // Copy base level.
            _levels.resize(levels);
            _levels[0].copyFrom(im);
            // Build pyramid.
            for(int i=1; i<_levels.size(); i++) {
                _levels[i].resize(im.width()>>i,im.height()>>i,im.numChannels());
                for(int y=0; y<_levels[i].height(); y++) {
                    for(int x=0; x<_levels[i].width(); x++) {
                        for(int c=0; c<_levels[i].numChannels(); c++) {
                            _levels[i](x,y,c) = (Elem)
                                (((double)_levels[i-1](2*x+0,2*y+0,c) +
                                  (double)_levels[i-1](2*x+1,2*y+0,c) +
                                  (double)_levels[i-1](2*x+0,2*y+1,c) +
                                  (double)_levels[i-1](2*x+1,2*y+1,c)) / 4);
                        } // channels
                    } // x
                } // y
            } // levels
        }

        const Image<Elem> &level( int l ) const { return _levels[l]; }
        Image<Elem> &level( int l ) { return _levels[l]; }
        int numLevels() const { return _levels.size(); }

    private:
        vector<Image<Elem> > _levels;
    };
   
    template<class Elem>
    Elem warpMipmapTrilinear( const ImagePyramid<Elem> &in,
                              const Matrix3x3d &H,
                              int x,
                              int y,
                              int c = 1 )
        // TODO:  Make this function able to return values for all channels in one call.
    {
        float area,level;
        float mx0,my0,mx1,my1;
        int xx0,yy0,xx1,yy1,ll;
        float u0,v0,u1,v1,w;

        // TODO: Replace this code with derivative-based scale selection.
        // Warp pixel corners and pixel center.
        Vector3d m00 = H*Vector3d(x+0.0,y+0.0,1);
        m00 = 1.0/m00[2] * m00;
        Vector3d m01 = H*Vector3d(x+1.0,y+0.0,1);
        m01 = 1.0/m01[2] * m01;
        Vector3d m10 = H*Vector3d(x+0.0,y+1.0,1);
        m10 = 1.0/m10[2] * m10;
        Vector3d m11 = H*Vector3d(x+1.0,y+1.0,1);
        m11 = 1.0/m11[2] * m11;
        Vector3d m = H*Vector3d(x+0.5,y+0.5,1);
        m = 1.0/m[2] * m;
        // Measure area of projected pixel (bounding box).
        area = (max(max(m00[0],m01[0]),max(m10[0],m11[0])) - min(min(m00[0],m01[0]),min(m10[0],m11[0])))*
               (max(max(m00[1],m01[1]),max(m10[1],m11[1])) - min(min(m00[1],m01[1]),min(m10[1],m11[1])));
        // Compute pyramid level.   2^(2*level)=area -> level=log2(area)/2
        level = 0.5f*log(area)/log(2.0f);
        level = max(min(level,(float)in.numLevels()-2.0f),0.0f);

        // Find pixels in pyramid.
        ll = (int)level;
        mx0 = m[0]/(1<<ll)-0.5f;
        my0 = m[1]/(1<<ll)-0.5f;
        mx1 = m[0]/(2<<ll)-0.5f;
        my1 = m[1]/(2<<ll)-0.5f;
        xx0 = (int)(floor(mx0));
        yy0 = (int)(floor(my0));
        xx1 = (int)(floor(mx1));
        yy1 = (int)(floor(my1));
        // Border handling (hack for now).
        if(xx0<0 || xx1<0 || yy0<0 || yy1<0 || 
            xx0>=in.level(ll).width()-1 || xx1>=in.level(ll+1).width()-1 ||
            yy0>=in.level(ll).height()-1 || yy1>=in.level(ll+1).height()-1 ||
            ll<0 || ll>=in.numLevels()-1)
        {
            return (float)(rand()%256);
        }
        // Compute trilinear coefficients.
        u0 = mx0-xx0;
        v0 = my0-yy0;
        u1 = mx1-xx1;
        v1 = my1-yy1;
        w = level-ll;
        // Compute trilinear interpolation.
        //return mx1;
        //return in.level(ll)(xx0,yy0);
        //return in.level(0)[y][x];
        return (1-u0)*(1-v0)*(1-w)*in.level(ll  )(xx0  ,yy0  ,c) + //[yy0  ][xx0  ] +
               (  u0)*(1-v0)*(1-w)*in.level(ll  )(xx0+1,yy0  ,c) + //[yy0  ][xx0+1] +
               (1-u0)*(  v0)*(1-w)*in.level(ll  )(xx0  ,yy0+1,c) + //[yy0+1][xx0  ] +
               (  u0)*(  v0)*(1-w)*in.level(ll  )(xx0+1,yy0+1,c) + //[yy0+1][xx0+1] +
               (1-u1)*(1-v1)*(  w)*in.level(ll+1)(xx1  ,yy1  ,c) + //[yy1  ][xx1  ] +
               (  u1)*(1-v1)*(  w)*in.level(ll+1)(xx1+1,yy1  ,c) + //[yy1  ][xx1+1] +
               (1-u1)*(  v1)*(  w)*in.level(ll+1)(xx1  ,yy1+1,c) + //[yy1+1][xx1  ] +
               (  u1)*(  v1)*(  w)*in.level(ll+1)(xx1+1,yy1+1,c);  //[yy1+1][xx1+1];
    }

    template<class Elem>
    Elem warpBilinear( const ImagePyramid<Elem> &in,
                       const Matrix3x3d &H,
                       int x,
                       int y,
                       int c = 1 )
    {
        Vector3d m = H*Vector3d(x+0.5,y+0.5,1);
        /*Vector3d m;
        m[0] = H[0][0]*x + H[0][1]*y + H[0][2];
        m[1] = H[1][0]*x + H[1][1]*y + H[1][2];
        m[2] = H[2][0]*x + H[2][1]*y + H[2][2];*/
        m = 1.0/m[2] * m;
        m[0] -= 0.5;
        m[1] -= 0.5;
        int xx0 = (int)(floor(m[0]));
        int yy0 = (int)(floor(m[1]));
        double u0 = m[0]-xx0;
        double v0 = m[1]-yy0;
        if(xx0<0 || yy0<0 ||
            xx0>=in.level(0).width()-1 || yy0>=in.level(0).height()-1)
        {
            return (float)(rand()%256);
        }
        return (1-u0)*(1-v0)*in.level(0)(xx0  ,yy0  ,c) + //[yy0  ][xx0  ] +
               (  u0)*(1-v0)*in.level(0)(xx0+1,yy0  ,c) + //[yy0  ][xx0+1] +
               (1-u0)*(  v0)*in.level(0)(xx0  ,yy0+1,c) + //[yy0+1][xx0  ] +
               (  u0)*(  v0)*in.level(0)(xx0+1,yy0+1,c);  //[yy0+1][xx0+1] +
    }

    template<class Elem>
    void warpImageBilinear( Image<Elem> &out,
                            const ImagePyramid<Elem> &in,
                            const Matrix3x3d &H )
    {
        for(int y=0; y<out.height(); y++)
            for(int x=0; x<out.width(); x++)
                for(int c=0; c<out.numChannels(); c++)
                    out(x,y,c) = warpBilinear(in,H,x,y,c);
    }

    template<class Elem>
    void warpImageMipmapTrilinear( Image<Elem> &out,
                                   const ImagePyramid<Elem> &in,
                                   const Matrix3x3d &H )
    {
        for(int y=0; y<out.height(); y++)
            for(int x=0; x<out.width(); x++)
                for(int c=0; c<out.numChannels(); c++)
                    out(x,y,c) = warpMipmapTrilinear(in,H,x,y,c);
    }

//======================================================================

   template<typename Elem>
   void convertRGBToGrayscale( const Image<Elem> &rgb, Image<Elem> &gray,
                               double rf=0.3, double gf=0.59, double bf=0.11 )
   {
      // Ensure output size.
      if(rgb.width()!=gray.width() || rgb.height()!=gray.height())
         gray.resize(rgb.width(),rgb.height(),1);
      // Convert.
      int x,y;
      for(y=0; y<rgb.height(); y++) {
         for(x=0; x<rgb.width(); x++) {
            Elem r = rgb(x,y,0);
            Elem g = rgb(x,y,1);
            Elem b = rgb(x,y,2);
            gray(x,y) = (Elem)(r*rf+g*gf+b*bf);
         }
      }
   }

   template<typename Elem>
   void convertRGBToRGBInterleaved( const Image<Elem> &rgb, Elem *rgbI,
                                    int sizeBytes )
   {
      verify(sizeBytes>=rgb.width()*rgb.height()*3*sizeof(Elem),"Buffer too small.");
      int x,y;
      for(y=0; y<rgb.height(); y++) {
         for(x=0; x<rgb.width(); x++) {
            int i = y*rgb.width()*3 + x*3;
            rgbI[i+0] = rgb(x,y,0);
            rgbI[i+1] = rgb(x,y,1);
            rgbI[i+2] = rgb(x,y,2);
         }
      }
   }

   template<typename Elem>
   void convertToUchar( const Image<Elem> &in, Image<unsigned char> &out,
                        double scale = 0.0, double bias = 0 )
   {
      if(scale==0.0) {  // auto scale
         Elem maxval = *std::max_element(in.begin(0),in.end(in.numChannels()-1));
         scale = 255.0/(double)maxval;
      }
      if(in.width()!=out.width() || in.height()!=out.height()
         || in.numChannels()!=out.numChannels())
      {
         out.resize(in.width(),in.height(),in.numChannels());
      }
      int x,y,c;
      for(y=0; y<in.height(); y++) {
         for(x=0; x<in.width(); x++) {
            for(c=0; c<in.numChannels(); c++) {
               double val = ((double)in(x,y,c)+bias)*scale;
               out(x,y,c) = (unsigned char)std::min(std::max(val,0.0),255.0);
            }
         }
      }
   }

   template<typename Elem, typename Elem2, int channels>
   void convertIndexedImage( const Image<Elem> &in,
                             const std::vector<InlineVector<Elem2,channels> > &map,
                             Image<Elem2> &out )
   {
      if(in.width()!=out.width() || in.height()!=out.height()
         || channels!=out.numChannels())
      {
         out.resize(in.width(),in.height(),channels);
      }
      int x,y,c;
      for(y=0; y<in.height(); y++) {
         for(x=0; x<in.width(); x++) {
            int i = in(x,y);
            while(i<0)
               i += map.size();
            i = i % map.size();
            for(c=0; c<channels; c++) {
               out(x,y,c) = map[i][c];
            }
         }
      }
   }

//======================================================================

   // RGB values are expected to be in [0, 1]
   inline Vector3f
   convertRGBPixelToYUV(Vector3f const& rgb)
   {
      float const Wr = 0.299f;
      float const Wb = 0.114f;
      float const Wg = 1.0f - Wr - Wb;

      Vector3f yuv;
      yuv[0] = Wr*rgb[0] + Wg*rgb[1] + Wb*rgb[2];
      yuv[1] = 0.436f * (rgb[2] - yuv[0])/(1.0f - Wb);
      yuv[2] = 0.615f * (rgb[0] - yuv[0])/(1.0f - Wr);
      return yuv;
   }

   inline void
   convertRGBImageToYUV(Image<unsigned char> const& src, Image<float>& dst)
   {
      int const w = src.width();
      int const h = src.height();
      dst.resize(w, h, 3);

      Vector3f rgb, yuv;

      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            rgb[0] = src(x, y, 0) / 255.0f;
            rgb[1] = src(x, y, 1) / 255.0f;
            rgb[2] = src(x, y, 2) / 255.0f;
            yuv = convertRGBPixelToYUV(rgb);
            dst(x, y, 0) = yuv[0];
            dst(x, y, 1) = yuv[1];
            dst(x, y, 2) = yuv[2];
         }
   } // end convertRGBImageToYUV()

   // RGB values are expected to be in [0, 1], S and L results are in [0, 1]
   inline Vector3f
   convertRGBPixelToHSL(Vector3f const& rgb)
   {
      Vector3f hsl;
      float const r = rgb[0];
      float const g = rgb[1];
      float const b = rgb[2];
      float h = 0.0f, s = 0.0f;

      float const minimum = std::min(std::min(r, g), b);
      float const maximum = std::max(std::max(r, g), b);
      float const delta = maximum - minimum;
      float const l = (minimum+maximum)/2;

      if (delta > 0)
      {
         s = (l <= 0.5f) ? (delta / (2*l)) : (delta / (2.0f - 2*l));
      }

      if (delta > 0)
      {
         if (r == maximum)
            h = (g - b) / delta;
         else if (g == maximum)
            h = 2 + (b - r) / delta;
         else if (b == maximum)
            h = 4 + (r - g) / delta;
      }
      h *= 60;
      if (h < 0) h += 360;

      h /= 255.0f;

      hsl[0] = h; hsl[1] = s; hsl[2] = l;
      return hsl;
   }

   inline void
   convertRGBImageToHSL(Image<unsigned char> const& src, Image<float>& dst)
   {
      int const w = src.width();
      int const h = src.height();
      dst.resize(w, h, 3);

      Vector3f rgb, hsl;

      for (int y = 0; y < h; ++y)
         for (int x = 0; x < w; ++x)
         {
            rgb[0] = src(x, y, 0) / 255.0f;
            rgb[1] = src(x, y, 1) / 255.0f;
            rgb[2] = src(x, y, 2) / 255.0f;
            hsl = convertRGBPixelToHSL(rgb);
            dst(x, y, 0) = hsl[0];
            dst(x, y, 1) = hsl[1];
            dst(x, y, 2) = hsl[2];
         }
   } // end convertRGBImageToHSL()

   // RGB values are expected to be in [0, 1]
   inline Vector3f
   convertRGBPixelTo_sRGB(Vector3f const& rgb)
   {
      float const th = 0.04045;
      Vector3f srgb;
      srgb[0] = (rgb[0] < th) ? (rgb[0] / 12.92f) : powf((rgb[0] + 0.055f) / 1.055f, 2.4f);
      srgb[1] = (rgb[1] < th) ? (rgb[1] / 12.92f) : powf((rgb[1] + 0.055f) / 1.055f, 2.4f);
      srgb[2] = (rgb[2] < th) ? (rgb[2] / 12.92f) : powf((rgb[2] + 0.055f) / 1.055f, 2.4f);
      return srgb;
   }

   inline Vector3f
   convert_sRGBPixelToXYZ(Vector3f const& srgb)
   {
      float const R = srgb[0], G = srgb[1], B = srgb[2];

      Vector3f xyz;
      xyz[0] = (float) (R * 0.412424 + G * 0.357579 + B * 0.180464);
      xyz[1] = (float) (R * 0.212656 + G * 0.715158 + B * 0.072186);
      xyz[2] = (float) (R * 0.019332 + G * 0.119193 + B * 0.950444);
      return xyz;
   } // end convertRGBPixelToXYZ()

   // RGB values are expected to be in [0, 1]
   inline Vector3f
   convertRGBPixelToXYZ(Vector3f const& rgb)
   {
      Vector3f srgb = convertRGBPixelTo_sRGB(rgb);
      return convert_sRGBPixelToXYZ(srgb);
   } // end convertRGBPixelToXYZ()

   inline Vector3f
   convertXYZPixelToCIELab(Vector3f const& xyz)
   {
      float const one_over_3 = 1.0f/3.0f;
      float const c2 = 16.0f/116.0f;

      float const X = (xyz[0] > 0.0088565) ? pow(xyz[0], one_over_3) : (7.787*xyz[0] + c2);
      float const Y = (xyz[1] > 0.0088565) ? pow(xyz[1], one_over_3) : (7.787*xyz[1] + c2);
      float const Z = (xyz[2] > 0.0088565) ? pow(xyz[2], one_over_3) : (7.787*xyz[2] + c2);

      Vector3f lab;
      lab[0] = 116.0f*Y - 16.0f;
      lab[1] = 500.0f * (X - Y);
      lab[2] = 200.0f * (Y - Z);
      return lab;
   } // end convertXYZPixelToCIELab()

//---------------------------------------------------------------------
   // Image display function.
   template<typename Elem>
   inline void showImage( const Image<Elem> &im, double scale = 0.0 )
   {
#ifdef V3DLIB_ENABLE_IMDEBUG
      Image<unsigned char> out;
      convertToUchar(im,out,scale);
      if(im.numChannels()==3) {
         unsigned char *rgb = new unsigned char[im.width()*im.height()*im.numChannels()];
         convertRGBToRGBInterleaved(out,rgb,im.width()*im.height()*im.numChannels());
         imdebug("rgb w=%d h=%d %p",im.width(),im.height(),rgb);
         delete[] rgb;
      } else if(im.numChannels()==1) {
         imdebug("lum w=%d h=%d %p",im.width(),im.height(),out.begin());
      } else {
         verify(false,"Num channels must be 1 or 3");
      }
#endif
   }

   inline void showFloatImage( const Image<float> &im )
   {
#ifdef V3DLIB_ENABLE_IMDEBUG
      if(im.numChannels()==1) {
         imdebug("lum *auto b=32f w=%d h=%d %p",im.width(),im.height(),im.begin());
      } else {
         verify(false,"Num channels must be 1");
      }
#endif
   }

} // namespace V3D

#endif
