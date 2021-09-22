#ifndef cimg_plugin_fits
#define cimg_plugin_fits

//! Load image from a FITS file.
/**
  \param filename Filename, as a C-string.
**/
CImg<T>& load_fits(const char *const filename) {
  return _load_fits(0,filename);
}

//! Load image from a FITS file \newinstance.
static CImg<T> get_load_fits(const char *const filename) {
  return CImg<T>().load_fits(filename);
}

//! Load image from a FITS file \overloading.
CImg<T>& load_fits(std::FILE *const file) {
  return _load_fits(file,0);
}

//! Load image from a FITS file \newinstance.
static CImg<T> get_load_fits(std::FILE *const file) {
  return CImg<T>().load_fits(file);
}

CImg<T>& _load_fits(std::FILE *const file, const char *const filename) {
  if (!file && !filename)
    throw CImgArgumentException(_cimg_instance
                                "load_fits(): Specified filename is (null).",
                                cimg_instance);

  std::FILE *const nfile = file?file:cimg::fopen(filename,"rb");
  unsigned int W, H, D = 1;
  CImg<charT> item(16384,1,1,1,0);
  int err;
  const longT cimg_iobuffer = (longT)24*1024*1024;
  err=std::fscanf(nfile,"%80c",item.data()) != 1;
  if (err || (std::strncmp(item.data(),"SIMPLE  =                    T",30)!=0)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): FITS header not found in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }

  err=std::fscanf(nfile,"%80c",item.data()) != 1;
  if (err || (std::strncmp(item.data(),"BITPIX  =                   16",30)!=0)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): bad BITPIX field value in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  err=std::fscanf(nfile,"%80c",item.data()) != 1;
  if (err || (std::strncmp(item.data(),"NAXIS   =                    2",30)!=0)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): bad NAXIS field in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  err=std::fscanf(nfile,"%80c",item.data()) != 1;
  if (err || (cimg_sscanf(item,"NAXIS1  =                 %u",&H)!=1)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): bad NAXIS1 field in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  err=std::fscanf(nfile,"%80c",item.data()) != 1;
  if (err || (cimg_sscanf(item,"NAXIS2  =                 %u",&W)!=1)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): bad NAXIS2 field in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  while ((err=std::fscanf(nfile,"%80c",item.data()))!=EOF && (std::strncmp(item.data(),"END",3)!=0));
  if (err != 1 || (std::strncmp(item.data(),"END",3)!=0)) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): no END field in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  long int offset = ((std::ftell(nfile)-1)/2880+1)*2880;
  if (std::fseek(nfile,offset,SEEK_SET)!=0) {
    if (!file) cimg::fclose(nfile);
    throw CImgIOException(_cimg_instance
                          "load_fits(): seek error in file '%s'.",
                          cimg_instance,
                          filename?filename:"(FILE*)");
  }
  if (filename) { // Check that dimensions specified in file does not exceed the buffer dimension
    const cimg_int64 siz = cimg::fsize(filename)-offset;
    if (W*H*D>siz)
      throw CImgIOException(_cimg_instance
                            "load_fits(): Specified image dimensions in file '%s' exceed file size.",
                            cimg_instance,
                            filename);
  }

  CImg<ushortT> raw;
  assign(W,H,D,1);
  T *ptrd = data(0,0,0,0);
  for (longT to_read = (longT)size(); to_read>0; ) {
    raw.assign(std::min(to_read,cimg_iobuffer/2));
    cimg::fread(raw._data,raw._width,nfile);
    to_read-=raw._width;
    const unsigned short *ptrs = raw._data;
    for (ulongT off = (ulongT)raw._width; off; --off) *(ptrd++) = (T)*(ptrs++);
  }
  if (!file) cimg::fclose(nfile);
  return *this;
}

#ifndef cimg_load_plugin
#define cimg_load_plugin(filename) \
  if (!cimg::strncasecmp(cimg::split_filename(filename),"fits",4)) return load_fits(filename);
#endif
#endif
