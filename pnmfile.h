#ifndef   PNM_FILE_H
#define   PNM_FILE_H

#include   <cstdlib>
#include   <climits>
#include   <cstring>
#include   <fstream>
#include   "image.h"
#include   "misc.h"
#include   <stdio.h>
#include   <iostream>//for   debug,qiansen

#define   BUF_SIZE   256

class   pnm_error   {   };

static   void   read_packed(unsigned   char   *data,   int   size,   std::ifstream   &f)   {
      unsigned   char   c   =   0;

      int   bitshift   =   -1;
      for   (int   pos   =   0;   pos   <   size;   pos++)   {
          if   (bitshift   ==   -1)   {
              c   =   f.get();
              bitshift   =   7;
          }
          data[pos]   =   (c   >>   bitshift)   &   1;
          bitshift--;
          }
}

static   void   write_packed(unsigned   char   *data,   int   size,   std::ofstream   &f)   {
      unsigned   char   c   =   0;

      int   bitshift   =   7;
      for   (int   pos   =   0;   pos   <   size;   pos++)   {
              c   =   c   |   (data[pos]   <<   bitshift);
              bitshift--;
              if   ((bitshift   ==   -1)   ||   (pos   ==   size-1))   {
				  f.put(c);
				  bitshift   =   7;
				  c   =   0;
              }
      }
}

/*   read   PNM   field,   skipping   comments   */
static   void   pnm_read(std::ifstream   &file,   char   *buf)   {
      char   doc[BUF_SIZE];
      char   c;

      file   >>   c;
      while   (c   ==   '#')   {
          file.getline(doc,   BUF_SIZE);
          file   >>   c;
      }
      file.putback(c);

      file.width(BUF_SIZE);
      file   >>   buf;
      file.ignore();
}

static   image<uchar>   *loadPBM(const   char   *name)   {
      char   buf[BUF_SIZE];

      /*   read   header   */
      std::ifstream   file(name,   std::ios::in   |   std::ios::binary);
      pnm_read(file,   buf);
      if   (strncmp(buf,   "P4",   2))
          throw   pnm_error();

      pnm_read(file,   buf);
      int   width   =   atoi(buf);
      pnm_read(file,   buf);
      int   height   =   atoi(buf);

      /*   read   data   */
      image<uchar>   *im   =   new   image<uchar>(width,   height);
      for   (int   i   =   0;   i   <   height;   i++)
          read_packed(imPtr(im,   0,   i),   width,   file);

      return   im;
}

static   void   savePBM(image<uchar>   *im,   const   char   *name)   {
      int   width   =   im->width();
      int   height   =   im->height();
      std::ofstream   file(name,   std::ios::out   |   std::ios::binary);

      file   <<   "P4\n"   <<   width   <<   "   "   <<   height   <<   "\n";
      for   (int   i   =   0;   i   <   height;   i++)
          write_packed(imPtr(im,   0,   i),   width,   file);
}

static   image<uchar>   *loadPGM(const   char   *name)   {
      char   buf[BUF_SIZE];

      /*   read   header   */
      std::ifstream   file(name,   std::ios::in   |   std::ios::binary);
      pnm_read(file,   buf);
      if   (strncmp(buf,   "P5",   2))
          throw   pnm_error();

      pnm_read(file,   buf);
      int   width   =   atoi(buf);
      pnm_read(file,   buf);
      int   height   =   atoi(buf);

      pnm_read(file,   buf);
      if   (atoi(buf)   >   UCHAR_MAX)
          throw   pnm_error();

      /*   read   data   */
      image<uchar>   *im   =   new   image<uchar>(width,   height);
      file.read((char   *)imPtr(im,   0,   0),   width   *   height   *   sizeof(uchar));

      return   im;
}

static   void   savePGM(image<uchar>   *im,   const   char   *name)   {
      int   width   =   im->width();
      int   height   =   im->height();
      std::ofstream   file(name,   std::ios::out   |   std::ios::binary);

      file   <<   "P5\n"   <<   width   <<   "   "   <<   height   <<   "\n"   <<   UCHAR_MAX   <<   "\n";
      file.write((char   *)imPtr(im,   0,   0),   width   *   height   *   sizeof(uchar));
}

static   image<rgb>   *loadPPM(const   char   *name)   {
      char   buf[BUF_SIZE],   doc[BUF_SIZE];

      /*   read   header   */
      std::ifstream   file(name,   std::ios::in   |   std::ios::binary);
      pnm_read(file,   buf);
      if   (strncmp(buf,   "P5",   2)){
          //throw   pnm_error();
          std::cout<<"pnm   version   is   P6,may   be   not   supported."<<std::endl;
      }
      pnm_read(file,   buf);
      int   width   =   atoi(buf);
      pnm_read(file,   buf);
      int   height   =   atoi(buf);

      pnm_read(file,   buf);
      if   (atoi(buf)   >   UCHAR_MAX)
          throw   pnm_error();

      /*   read   data   */
      image<rgb>   *im   =   new   image<rgb>(width,   height);
      file.read((char   *)imPtr(im,   0,   0),   width   *   height   *   sizeof(rgb));

      return   im;
}

static   void   savePPM(image<rgb>   *im,   const   char   *name)   {
      int   width   =   im->width();
      int   height   =   im->height();
      std::ofstream   file(name,   std::ios::out   |   std::ios::binary);

      file   <<   "P6\n"   <<   width   <<   "   "   <<   height   <<   "\n"   <<   UCHAR_MAX   <<   "\n";
      file.write((char   *)imPtr(im,   0,   0),   width   *   height   *   sizeof(rgb));
}

template   <class   T>
void   load_image(image<T>   **im,   const   char   *name)   {
      char   buf[BUF_SIZE];

      /*   read   header   */
      std::ifstream   file(name,   std::ios::in   |   std::ios::binary);
      pnm_read(file,   buf);
      if   (strncmp(buf,   "VLIB",   9))
          throw   pnm_error();

      pnm_read(file,   buf);
      int   width   =   atoi(buf);
      pnm_read(file,   buf);
      int   height   =   atoi(buf);

      /*   read   data   */
      *im   =   new   image<T>(width,   height);
      file.read((char   *)imPtr((*im),   0,   0),   width   *   height   *   sizeof(T));
}

template   <class   T>
void   save_image(image<T>   *im,   const   char   *name)   {
      int   width   =   im->width();
      int   height   =   im->height();
      std::ofstream   file(name,   std::ios::out   |   std::ios::binary);

      file   <<   "VLIB\n"   <<   width   <<   "   "   <<   height   <<   "\n";
      file.write((char   *)imPtr(im,   0,   0),   width   *   height   *   sizeof(T));
}

#endif

/*   a   simple   image   class
filename:   image.h   */

#ifndef   IMAGE_H
#define   IMAGE_H

#include   <cstring>

template   <class   T>
class   image   {
    public:
      /*   create   an   image   */
      image(const   int   width,   const   int   height,   const   bool   init   =   true);

      /*   delete   an   image   */
      ~image();

      /*   init   an   image   */
      void   init(const   T   &val);

      /*   copy   an   image   */
      image<T>   *copy()   const;

      /*   get   the   width   of   an   image.   */
      int   width()   const   {   return   w;   }

      /*   get   the   height   of   an   image.   */
      int   height()   const   {   return   h;   }

      /*   image   data.   */
      T   *data;

      /*   row   pointers.   */
      T   **access;

    private:
      int   w,   h;
};

/*   use   imRef   to   access   image   data.   */
#define   imRef(im,   x,   y)   (im->access[y][x])

/*   use   imPtr   to   get   pointer   to   image   data.   */
#define   imPtr(im,   x,   y)   &(im->access[y][x])

template   <class   T>
image<T>::image(const   int   width,   const   int   height,   const   bool   init)   {
      w   =   width;
      h   =   height;
      data   =   new   T[w   *   h];     //   allocate   space   for   image   data
      access   =   new   T*[h];       //   allocate   space   for   row   pointers

      //   initialize   row   pointers
      for   (int   i   =   0;   i   <   h;   i++)
          access[i]   =   data   +   (i   *   w);

      if   (init)
          memset(data,   0,   w   *   h   *   sizeof(T));
}

template   <class   T>
image<T>::~image()   {
      delete   []   data;
      delete   []   access;
}

template   <class   T>
void   image<T>::init(const   T   &val)   {
      T   *ptr   =   imPtr(this,   0,   0);
      T   *end   =   imPtr(this,   w-1,   h-1);
      while   (ptr   <=   end)
          *ptr++   =   val;
}


template   <class   T>
image<T>   *image<T>::copy()   const   {
      image<T>   *im   =   new   image<T>(w,   h,   false);
      memcpy(im->data,   data,   w   *   h   *   sizeof(T));
      return   im;
}

#endif