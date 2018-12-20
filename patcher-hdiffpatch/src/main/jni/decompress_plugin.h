//decompress_plugin_demo.h
//  decompress plugin demo for HDiffz\HPatchz
/*
 The MIT License (MIT)
 Copyright (c) 2012-2017 HouSisong
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef HPatch_decompress_plugin_demo_h
#define HPatch_decompress_plugin_demo_h
//decompress plugin demo:
//  zlibDecompressPlugin;
//  bz2DecompressPlugin;
//  lzmaDecompressPlugin;
//  lz4DecompressPlugin;
//  zstdDecompressPlugin;

#include <stdlib.h> //malloc free
#include <zstd/zstd.h>22
#include "HPatch/patch_types.h"

#define kDecompressBufSize (1024*32)
#ifndef _IsNeedIncludeDefaultCompressHead
#   define _IsNeedIncludeDefaultCompressHead 1
#endif

#ifdef  _CompressPlugin_zlib
#if (_IsNeedIncludeDefaultCompressHead)
#   include "zlib.h" // http://zlib.net/  https://github.com/madler/zlib
#endif
    typedef struct _zlib_TDecompress{
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        const struct hpatch_TStreamInput* codeStream;
        
        unsigned char*  dec_buf;
        size_t          dec_buf_size;
        z_stream        d_stream;
    } _zlib_TDecompress;
    static int  _zlib_is_can_open(struct hpatch_TDecompress* decompressPlugin,
                                  const hpatch_compressedDiffInfo* compressedDiffInfo){
        return (compressedDiffInfo->compressedCount==0)
            ||(0==strcmp(compressedDiffInfo->compressType,"zlib"));
    }

    static _zlib_TDecompress*  _zlib_decompress_open_by(hpatch_TDecompress* decompressPlugin,
                                                        const hpatch_TStreamInput* codeStream,
                                                        hpatch_StreamPos_t code_begin,
                                                        hpatch_StreamPos_t code_end,
                                                        int  isSavedWindowBits,
                                                        unsigned char* _mem_buf,size_t _mem_buf_size){
        _zlib_TDecompress* self=0;
        int ret;
        signed char kWindowBits=-MAX_WBITS;
        if (isSavedWindowBits){//load kWindowBits
            if (code_end-code_begin<1) return 0;
            if (1!=codeStream->read(codeStream->streamHandle,code_begin,
                                    (unsigned char*)&kWindowBits,(unsigned char*)&kWindowBits+1)) return 0;
            ++code_begin;
        }
        
        self=(_zlib_TDecompress*)_hpatch_align_upper(_mem_buf,sizeof(hpatch_StreamPos_t));
        assert((_mem_buf+_mem_buf_size)>((unsigned char*)self+sizeof(_zlib_TDecompress)));
        _mem_buf_size=(_mem_buf+_mem_buf_size)-((unsigned char*)self+sizeof(_zlib_TDecompress));
        _mem_buf=(unsigned char*)self+sizeof(_zlib_TDecompress);
        
        memset(self,0,sizeof(_zlib_TDecompress));
        self->dec_buf=_mem_buf;
        self->dec_buf_size=_mem_buf_size;
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        ret = inflateInit2(&self->d_stream,kWindowBits);
        if (ret!=Z_OK) return 0;
        return self;
    }
    static hpatch_decompressHandle  _zlib_decompress_open(hpatch_TDecompress* decompressPlugin,
                                                          hpatch_StreamPos_t dataSize,
                                                          const hpatch_TStreamInput* codeStream,
                                                          hpatch_StreamPos_t code_begin,
                                                          hpatch_StreamPos_t code_end){
        _zlib_TDecompress* self=0;
        unsigned char* _mem_buf=(unsigned char*)malloc(sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!_mem_buf) return 0;
        self=_zlib_decompress_open_by(decompressPlugin,codeStream,code_begin,code_end,1,
                                      _mem_buf,sizeof(_zlib_TDecompress)+kDecompressBufSize);
        if (!self)
            free(_mem_buf);
        return self;
    }
    static void _zlib_decompress_close_by(struct hpatch_TDecompress* decompressPlugin,_zlib_TDecompress* self){
        if (!self) return;
        if (self->dec_buf!=0){
            if (Z_OK!=inflateEnd(&self->d_stream)) assert(0);
        }
        memset(self,0,sizeof(_zlib_TDecompress));
    }
    static void _zlib_decompress_close(struct hpatch_TDecompress* decompressPlugin,
                            hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        _zlib_decompress_close_by(decompressPlugin,self);
        if (self) free(self);
    }
    static long _zlib_decompress_part(const hpatch_TDecompress* decompressPlugin,
                                      hpatch_decompressHandle decompressHandle,
                                      unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        assert(out_part_data!=out_part_data_end);
        
        self->d_stream.next_out = out_part_data;
        self->d_stream.avail_out =(uInt)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            uInt avail_out_back,avail_in_back;
            int ret;
            long codeLen=(long)(self->code_end - self->code_begin);
            if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
                long readLen=codeLen;
                if ((size_t)readLen>self->dec_buf_size) readLen=(long)self->dec_buf_size;
                if (readLen<=0) return 0;//error;
                self->d_stream.next_in=self->dec_buf;
                if (readLen!=self->codeStream->read(self->codeStream->streamHandle,self->code_begin,
                                                    self->dec_buf,self->dec_buf+readLen)) return 0;//error;
                self->d_stream.avail_in=(uInt)readLen;
                self->code_begin+=readLen;
            }
            
            avail_out_back=self->d_stream.avail_out;
            avail_in_back=self->d_stream.avail_in;
            ret=inflate(&self->d_stream,Z_PARTIAL_FLUSH);
            if (ret==Z_OK){
                if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                    return 0;//error;
            }else if (ret==Z_STREAM_END){
                if (self->d_stream.avail_out!=0)
                    return 0;//error;
            }else{
                return 0;//error;
            }
        }
        return (long)(out_part_data_end-out_part_data);
    }
    static hpatch_inline int _zlib_is_decompress_finish(const hpatch_TDecompress* decompressPlugin,
                                                        hpatch_decompressHandle decompressHandle){
        _zlib_TDecompress* self=(_zlib_TDecompress*)decompressHandle;
        return   (self->code_begin==self->code_end)
                &(self->d_stream.avail_in==0)
                &(self->d_stream.avail_out==0);
    }
    static hpatch_TDecompress zlibDecompressPlugin={_zlib_is_can_open,_zlib_decompress_open,
                                                    _zlib_decompress_close,_zlib_decompress_part};
#endif//_CompressPlugin_zlib
    
#ifdef  _CompressPlugin_bz2
#if (_IsNeedIncludeDefaultCompressHead)
#   include "lib/bzlib.h" // http://www.bzip.org/
#endif
    typedef struct _bz2_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        bz_stream       d_stream;
        unsigned char   dec_buf[kDecompressBufSize];
    } _bz2_TDecompress;
    static int  _bz2_is_can_open(struct hpatch_TDecompress* decompressPlugin,
                                 const hpatch_compressedDiffInfo* compressedDiffInfo){
        return (compressedDiffInfo->compressedCount==0)
            ||(0==strcmp(compressedDiffInfo->compressType,"bz2"));
    }
    static hpatch_decompressHandle  _bz2_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const struct hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        int ret;
        _bz2_TDecompress* self=(_bz2_TDecompress*)malloc(sizeof(_bz2_TDecompress));
        if (!self) return 0;
        memset(self,0,sizeof(_bz2_TDecompress)-kDecompressBufSize);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        ret=BZ2_bzDecompressInit(&self->d_stream,0,0);
        if (ret!=BZ_OK){ free(self); return 0; }
        return self;
    }
    static void _bz2_close(struct hpatch_TDecompress* decompressPlugin,
                            hpatch_decompressHandle decompressHandle){
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        if (!self) return;
        if (BZ_OK!=BZ2_bzDecompressEnd(&self->d_stream)) assert(0);
        free(self);
    }
    static long _bz2_decompress_part(const struct hpatch_TDecompress* decompressPlugin,
                                      hpatch_decompressHandle decompressHandle,
                                      unsigned char* out_part_data,unsigned char* out_part_data_end){
        _bz2_TDecompress* self=(_bz2_TDecompress*)decompressHandle;
        assert(out_part_data!=out_part_data_end);
        
        self->d_stream.next_out =(char*)out_part_data;
        self->d_stream.avail_out =(unsigned int)(out_part_data_end-out_part_data);
        while (self->d_stream.avail_out>0) {
            unsigned int avail_out_back,avail_in_back;
            int ret;
            long codeLen=(long)(self->code_end - self->code_begin);
            if ((self->d_stream.avail_in==0)&&(codeLen>0)) {
                long readLen=kDecompressBufSize;
                long readed;
                self->d_stream.next_in=(char*)self->dec_buf;
                if (readLen>codeLen) readLen=codeLen;
                if (readLen<=0) return 0;//error;
                readed=self->codeStream->read(self->codeStream->streamHandle,self->code_begin,
                                              self->dec_buf,self->dec_buf+readLen);
                if (readed!=readLen) return 0;//error;
                self->d_stream.avail_in=(unsigned int)readLen;
                self->code_begin+=readLen;
            }
            
            avail_out_back=self->d_stream.avail_out;
            avail_in_back=self->d_stream.avail_in;
            ret=BZ2_bzDecompress(&self->d_stream);
            if (ret==BZ_OK){
                if ((self->d_stream.avail_in==avail_in_back)&&(self->d_stream.avail_out==avail_out_back))
                    return 0;//error;
            }else if (ret==BZ_STREAM_END){
                if (self->d_stream.avail_out!=0)
                    return 0;//error;
            }else{
                return 0;//error;
            }
        }
        return (long)(out_part_data_end-out_part_data);
    }
    static hpatch_TDecompress bz2DecompressPlugin={_bz2_is_can_open,_bz2_open,
                                                   _bz2_close,_bz2_decompress_part};
#endif//_CompressPlugin_bz2
    
#ifdef  _CompressPlugin_lzma
#if (_IsNeedIncludeDefaultCompressHead)
#   include "lib/LzmaDec.h" // "lzma/C/LzmaDec.h" http://www.7-zip.org/sdk.html
#endif
    typedef struct _lzma_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        CLzmaDec        decEnv;
        SizeT           decCopyPos;
        SizeT           decReadPos;
        unsigned char   dec_buf[kDecompressBufSize];
    } _lzma_TDecompress;
    static void * __lzma_dec_Alloc(ISzAllocPtr p, size_t size){
        return malloc(size);
    }
    static void __lzma_dec_Free(ISzAllocPtr p, void *address){
        if (address) free(address);
    }
    static int  _lzma_is_can_open(struct hpatch_TDecompress* decompressPlugin,
                                  const hpatch_compressedDiffInfo* compressedDiffInfo){
        return (compressedDiffInfo->compressedCount==0)
            ||(0==strcmp(compressedDiffInfo->compressType,"lzma"));
    }
    static hpatch_decompressHandle  _lzma_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const struct hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _lzma_TDecompress* self=0;
        SRes ret;
        unsigned char propsSize=0;
        unsigned char props[256];
        ISzAlloc alloc={__lzma_dec_Alloc,__lzma_dec_Free};
        //load propsSize
        if (code_end-code_begin<1) return 0;
        if (1!=codeStream->read(codeStream->streamHandle,code_begin,
                                &propsSize,&propsSize+1)) return 0;
        ++code_begin;
        if (propsSize>(code_end-code_begin)) return 0;
        //load props
        if (propsSize!=codeStream->read(codeStream->streamHandle,code_begin,
                                        props,props+propsSize)) return 0;
        code_begin+=propsSize;

        self=(_lzma_TDecompress*)malloc(sizeof(_lzma_TDecompress));
        if (!self) return 0;
        memset(self,0,sizeof(_lzma_TDecompress)-kDecompressBufSize);
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        
        self->decCopyPos=0;
        self->decReadPos=kDecompressBufSize;
        
        LzmaDec_Construct(&self->decEnv);
        ret=LzmaDec_Allocate(&self->decEnv,props,propsSize,&alloc);
        if (ret!=SZ_OK){ free(self); return 0; }
        LzmaDec_Init(&self->decEnv);
        return self;
    }
    static void _lzma_close(struct hpatch_TDecompress* decompressPlugin,
                            hpatch_decompressHandle decompressHandle){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        ISzAlloc alloc={__lzma_dec_Alloc,__lzma_dec_Free};
        if (!self) return;
        LzmaDec_Free(&self->decEnv,&alloc);
        free(self);
    }
    static long _lzma_decompress_part(const struct hpatch_TDecompress* decompressPlugin,
                                      hpatch_decompressHandle decompressHandle,
                                      unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lzma_TDecompress* self=(_lzma_TDecompress*)decompressHandle;
        unsigned char* out_cur=out_part_data;
        assert(out_part_data!=out_part_data_end);
        while (out_cur<out_part_data_end){
            long copyLen=(long)(self->decEnv.dicPos-self->decCopyPos);
            if (copyLen>0){
                if (copyLen>(out_part_data_end-out_cur))
                    copyLen=(long)(out_part_data_end-out_cur);
                memcpy(out_cur,self->decEnv.dic+self->decCopyPos,copyLen);
                out_cur+=copyLen;
                self->decCopyPos+=copyLen;
                if ((self->decEnv.dicPos==self->decEnv.dicBufSize)
                    &&(self->decEnv.dicPos==self->decCopyPos)){
                    self->decEnv.dicPos=0;
                    self->decCopyPos=0;
                }
            }else{
                ELzmaStatus status;
                SizeT inSize,dicPos_back;
                SRes res;
                long codeLen=(long)(self->code_end - self->code_begin);
                if ((self->decReadPos==kDecompressBufSize)&&(codeLen>0)) {
                    long readLen=kDecompressBufSize;
                    long readed;
                    if (readLen>codeLen) readLen=codeLen;
                    if (readLen<=0) return 0;//error;
                    self->decReadPos=kDecompressBufSize-readLen;
                    readed=self->codeStream->read(self->codeStream->streamHandle,self->code_begin,
                                                  self->dec_buf+self->decReadPos,
                                                  self->dec_buf+self->decReadPos+readLen);
                    if (readed!=readLen) return 0;//error;
                    self->code_begin+=readLen;
                }

                inSize=kDecompressBufSize-self->decReadPos;
                dicPos_back=self->decEnv.dicPos;
                res=LzmaDec_DecodeToDic(&self->decEnv,self->decEnv.dicBufSize,
                                        self->dec_buf+self->decReadPos,&inSize,LZMA_FINISH_ANY,&status);
                if(res==SZ_OK){
                    if ((inSize==0)&&(self->decEnv.dicPos==dicPos_back))
                        return 0;//error;
                }else{
                    return 0;//error;
                }
                self->decReadPos+=inSize;
            }
        }
        return (long)(out_part_data_end-out_part_data);
    }
    static hpatch_TDecompress lzmaDecompressPlugin={_lzma_is_can_open,_lzma_open,
                                                    _lzma_close,_lzma_decompress_part};
#endif//_CompressPlugin_lzma

#if (defined(_CompressPlugin_lz4) || defined(_CompressPlugin_lz4hc))
#if (_IsNeedIncludeDefaultCompressHead)
#   include "lib/lz4.h" // "lz4/lib/lz4.h" https://github.com/lz4/lz4
#endif
    typedef struct _lz4_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;
        
        LZ4_streamDecode_t *s;
        int                kLz4CompressBufSize;
        int                code_buf_size;
        int                data_begin;
        int                data_end;
        unsigned char      buf[1];
    } _lz4_TDecompress;
    static int  _lz4_is_can_open(struct hpatch_TDecompress* decompressPlugin,
                                  const hpatch_compressedDiffInfo* compressedDiffInfo){
        return (compressedDiffInfo->compressedCount==0)
             ||(0==strcmp(compressedDiffInfo->compressType,"lz4"));
    }
    #define _lz4_read_len4(len,in_code,code_begin,code_end) { \
        unsigned char _temp_buf4[4];  \
        if (4>code_end-code_begin) return 0; \
        if (4!=in_code->read(in_code->streamHandle,code_begin,_temp_buf4,_temp_buf4+4)) \
            return 0; \
        len=_temp_buf4[0]|(_temp_buf4[1]<<8)|(_temp_buf4[2]<<16)|(_temp_buf4[3]<<24); \
        code_begin+=4; \
    }
    static hpatch_decompressHandle  _lz4_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const struct hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        const int kMaxLz4CompressBufSize=(1<<20)*64; //defence attack
        _lz4_TDecompress* self=0;
        int kLz4CompressBufSize=0;
        int code_buf_size=0;
        assert(code_begin<code_end);
        {//read kLz4CompressBufSize
            _lz4_read_len4(kLz4CompressBufSize,codeStream,code_begin,code_end);
            if ((kLz4CompressBufSize<=0)||(kLz4CompressBufSize>=kMaxLz4CompressBufSize)) return 0;
            code_buf_size=LZ4_compressBound(kLz4CompressBufSize);
        }
        self=(_lz4_TDecompress*)malloc(sizeof(_lz4_TDecompress)+kLz4CompressBufSize+code_buf_size);
        if (!self) return 0;
        memset(self,0,sizeof(_lz4_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->kLz4CompressBufSize=kLz4CompressBufSize;
        self->code_buf_size=code_buf_size;
        self->data_begin=0;
        self->data_end=0;
        
        self->s = LZ4_createStreamDecode();
        if (!self->s){ free(self); return 0; }
        return self;
    }
    static void _lz4_close(struct hpatch_TDecompress* decompressPlugin,
                            hpatch_decompressHandle decompressHandle){
        _lz4_TDecompress* self=(_lz4_TDecompress*)decompressHandle;
        if (!self) return;
        if (0!=LZ4_freeStreamDecode(self->s)) assert(0);
        free(self);
    }
    static long _lz4_decompress_part(const struct hpatch_TDecompress* decompressPlugin,
                                      hpatch_decompressHandle decompressHandle,
                                      unsigned char* out_part_data,unsigned char* out_part_data_end){
        _lz4_TDecompress* self=(_lz4_TDecompress*)decompressHandle;
        unsigned char* data_buf=self->buf;
        unsigned char* code_buf=self->buf+self->kLz4CompressBufSize;
        long result=(long)(out_part_data_end-out_part_data);

        while (out_part_data<out_part_data_end) {
            long dataLen=self->data_end-self->data_begin;
            if (dataLen>0){
                if (dataLen>(out_part_data_end-out_part_data))
                    dataLen=(long)(out_part_data_end-out_part_data);
                memcpy(out_part_data,data_buf+self->data_begin,dataLen);
                out_part_data+=(size_t)dataLen;
                self->data_begin+=dataLen;
            }else{
                int codeLen;
                _lz4_read_len4(codeLen,self->codeStream,self->code_begin,self->code_end);
                if ((codeLen<=0)||(codeLen>self->code_buf_size)
                    ||((size_t)codeLen>(self->code_end-self->code_begin))) return 0;
                if (codeLen!=self->codeStream->read(self->codeStream->streamHandle,self->code_begin,
                                                    code_buf,code_buf+codeLen)) return 0;
                self->code_begin+=codeLen;
                self->data_begin=0;
                self->data_end=LZ4_decompress_safe_continue(self->s,(const char*)code_buf,(char*)data_buf,
                                                            codeLen,self->kLz4CompressBufSize);
                if (self->data_end<=0) return 0;
            }
        }
        
        return result;
    }
    static hpatch_TDecompress lz4DecompressPlugin={_lz4_is_can_open,_lz4_open,
                                                   _lz4_close,_lz4_decompress_part};
#endif//_CompressPlugin_lz4 or _CompressPlugin_lz4hc

#ifdef  _CompressPlugin_zstd
#if (_IsNeedIncludeDefaultCompressHead)
#   include "/zstd/zstd.h" // "zstd.h" https://github.com/facebook/zstd
#endif
    typedef struct _zstd_TDecompress{
        const struct hpatch_TStreamInput* codeStream;
        hpatch_StreamPos_t code_begin;
        hpatch_StreamPos_t code_end;

        ZSTD_inBuffer      s_input;
        ZSTD_outBuffer     s_output;
        size_t             data_begin;
        ZSTD_DStream*      s;
        unsigned char      buf[1];
    } _zstd_TDecompress;
    static int  _zstd_is_can_open(struct hpatch_TDecompress* decompressPlugin,
                                  const hpatch_compressedDiffInfo* compressedDiffInfo){
        return (compressedDiffInfo->compressedCount==0)
             ||(0==strcmp(compressedDiffInfo->compressType,"zstd"));
    }
    static hpatch_decompressHandle  _zstd_open(struct hpatch_TDecompress* decompressPlugin,
                                               hpatch_StreamPos_t dataSize,
                                               const struct hpatch_TStreamInput* codeStream,
                                               hpatch_StreamPos_t code_begin,
                                               hpatch_StreamPos_t code_end){
        _zstd_TDecompress* self=0;
        size_t  ret;
        size_t _input_size=ZSTD_DStreamInSize();
        size_t _output_size=ZSTD_DStreamOutSize();
        assert(code_begin<code_end);
        self=(_zstd_TDecompress*)malloc(sizeof(_zstd_TDecompress)+_input_size+_output_size);
        if (!self) return 0;
        memset(self,0,sizeof(_zstd_TDecompress));
        self->codeStream=codeStream;
        self->code_begin=code_begin;
        self->code_end=code_end;
        self->s_input.src=self->buf;
        self->s_input.size=_input_size;
        self->s_input.pos=_input_size;
        self->s_output.dst=self->buf+_input_size;
        self->s_output.size=_output_size;
        self->s_output.pos=0;
        self->data_begin=0;
        
        self->s = ZSTD_createDStream();
        if (!self->s){ free(self); return 0; }
        ret=ZSTD_initDStream(self->s);
        if (ZSTD_isError(ret)) { ZSTD_freeDStream(self->s); free(self); return 0; }
        return self;
    }
    static void _zstd_close(struct hpatch_TDecompress* decompressPlugin,
                            hpatch_decompressHandle decompressHandle){
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        if (!self) return;
        if (0!=ZSTD_freeDStream(self->s)) assert(0);
        free(self);
    }
    static long _zstd_decompress_part(const struct hpatch_TDecompress* decompressPlugin,
                                      hpatch_decompressHandle decompressHandle,
                                      unsigned char* out_part_data,unsigned char* out_part_data_end){
        _zstd_TDecompress* self=(_zstd_TDecompress*)decompressHandle;
        long result=(long)(out_part_data_end-out_part_data);
        while (out_part_data<out_part_data_end) {
            long dataLen=(long)(self->s_output.pos-self->data_begin);
            if (dataLen>0){
                if (dataLen>(out_part_data_end-out_part_data))
                    dataLen=(long)(out_part_data_end-out_part_data);
                memcpy(out_part_data,(const unsigned char*)self->s_output.dst+self->data_begin,dataLen);
                out_part_data+=dataLen;
                self->data_begin+=dataLen;
            }else{
                size_t ret;
                if (self->s_input.pos==self->s_input.size) {
                    self->s_input.pos=0;
                    if (self->s_input.size>self->code_end-self->code_begin){
                        self->s_input.size=(size_t)(self->code_end-self->code_begin);
                        if (self->s_input.size==0) return 0;
                    }
                    if ((long)self->s_input.size!=self->codeStream->read(self->codeStream->streamHandle,
                            self->code_begin, (unsigned char*)self->s_input.src,
                            (unsigned char*)self->s_input.src+self->s_input.size)) return 0;
                    self->code_begin+=self->s_input.size;
                }
                self->s_output.pos=0;
                self->data_begin=0;
                ret=ZSTD_decompressStream(self->s,&self->s_output,&self->s_input);
                if (ZSTD_isError(ret)) return 0;
            }
        }
        return result;
    }
    static hpatch_TDecompress zstdDecompressPlugin={_zstd_is_can_open,_zstd_open,
                                                    _zstd_close,_zstd_decompress_part};
#endif//_CompressPlugin_zstd

#endif
