#include <stdio.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/file.h>
#include <libavformat/avio.h>
#include "libavcodec/avcodec.h"
#include "libavcodec/decode.c"
#include "libavformat/avformat.h"
#include "fuzz/memory_c.h"
#include "libavformat/internal.h"

// clang -g -O3 -fsanitize=fuzzer,address,undefined,signed-integer-overflow -I ./ fuzz/fuzz8.c fuzz/memory_c.c fuzz/cvector.c libavformat/libavformat.a  libavcodec/libavcodec.a  libavutil/libavutil.a libswresample/libswresample.a fuzz/libFuzzer.a -lz

typedef struct buffer_data {
    uint8_t *ptr;
    uint8_t *ptr_start;
    size_t size;    ///< buffer size
}buffer_data;

static int64_t io_seek(void* opaque,int64_t offset,int whence)
{
    struct buffer_data *bd = (struct buffer_data*)opaque;
    if (whence == AVSEEK_SIZE) {
        return bd->size;
    }
    
    if (whence == SEEK_CUR) {
        bd->ptr += offset;
    } else if (whence == SEEK_SET) {
        bd->ptr = bd->ptr_start+offset;
    } else if (whence == SEEK_END) {
        bd->ptr = bd->ptr_start + bd->size + offset;
    }
    
    return (int64_t)(bd->ptr - bd->ptr_start);
}

static int io_read(void *opaque, uint8_t *buf, int buf_size)
{
    static int total = 0;
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, (int)(bd->ptr_start+bd->size-bd->ptr));
    total += buf_size;
    if (buf_size <= 0)
        return AVERROR_EOF;
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;

    return buf_size;
}

#define CONFIG_LZMA 0

static int anum = 0;
static int vnum = 0;

// extern const AVInputFormat  ff_image_exr_pipe_demuxer;

// int main(int argc, char** argv)
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) 
{

    
    AVInputFormat *fmt = NULL;
    fmt  = av_find_input_format("gif");
    if(!fmt) {
        printf("av_find_input_format falied!\n\n");
        exit(0);
    }
    
    // AVInputFormat *fmt = &ff_image_exr_pipe_demuxer;
    
    mem_init();
    AVFormatContext *inFmtCtx = NULL;
    int ret = 0;

    AVIOContext *ioCtx;
    uint8_t *io_ctx_buffer = NULL;
    size_t io_ctx_buffer_size = 4096;
    buffer_data bd = {0};
    bd.ptr = data;
    bd.ptr_start = data;
    bd.size = size;
    
    inFmtCtx = avformat_alloc_context();
    if (inFmtCtx == NULL)   return 0;
    io_ctx_buffer = (uint8_t*)av_mallocz(io_ctx_buffer_size);
    ioCtx = avio_alloc_context(io_ctx_buffer,(int)io_ctx_buffer_size,0,&bd,&io_read,NULL,&io_seek);
    if (ioCtx == NULL)  return 0;
    inFmtCtx->pb = ioCtx;

    
    ret = avformat_open_input(&inFmtCtx, NULL, fmt, NULL);
    // char *filename = "/tmp/exr.exr";
    // ret = avformat_open_input(&inFmtCtx, filename, NULL, NULL);
    if (ret < 0)    goto end;

    ret = avformat_find_stream_info(inFmtCtx,NULL);
    if (ret < 0)    goto end;

    av_dump_format(inFmtCtx,0,NULL,0);

    int videoStream_Index = -1;
    int audioStream_Index = -1;
    for (int i = 0;i<inFmtCtx->nb_streams;i++) {
        AVStream *stream = inFmtCtx->streams[i];
        enum AVCodecID cId = stream->codecpar->codec_id;
        int format = stream->codecpar->format;
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream_Index = i;
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream_Index = i;
        } 
    }
   
    AVPacket *packet = av_packet_alloc();
    while ((ret = av_read_frame(inFmtCtx, packet)) >= 0) {
        if (videoStream_Index == packet->stream_index) {
            vnum++;
        } else if (audioStream_Index == packet->stream_index){
            anum++;
        }
        av_packet_unref(packet);
    }

 end:   
    avformat_close_input(&inFmtCtx);
    
    if (ioCtx) {
        av_freep(&ioCtx->buffer);
    }
    avio_context_free(&ioCtx);
    mem_clear();
    return 0;

}
