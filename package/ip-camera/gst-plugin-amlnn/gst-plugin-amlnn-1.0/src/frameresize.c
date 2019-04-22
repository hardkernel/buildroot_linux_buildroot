#include <stdlib.h>
#include <directfb.h>
#include <directfb_strings.h>
#include <directfb_util.h>

#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBError( #x, err );                              \
      }                                                        \
  }

static IDirectFB *dfb = NULL;
static IDirectFBSurface *input_surface = NULL;
static IDirectFBSurface *output_surface = NULL;

static void Init () {
  if (dfb) return;

  DFBCHECK (DirectFBInit (NULL, NULL));
  DFBCHECK (DirectFBCreate (&dfb));
}

static void Deinit () {
  if (dfb) {
    DFBCHECK (dfb->Release (dfb));
    dfb = NULL;
  }
}

static void
CreateInputSurface (char* buf, int width, int height) {
  if (input_surface) return;

  DFBSurfaceDescription dsc;
  dsc.width = width;
  dsc.height = height;
  dsc.flags = DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT;
  dsc.caps = DSCAPS_NONE;
  dsc.pixelformat = DSPF_RGB24;
  dsc.preallocated[0].data = buf;
  dsc.preallocated[0].pitch = width * 3;
  dsc.preallocated[1].data = NULL;
  dsc.preallocated[1].pitch = 0;
  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &input_surface));
}

static void
CreateOutputSurface (int width, int height) {
  if (output_surface) return;

  DFBSurfaceDescription dsc;
  dsc.width = width;
  dsc.height = height;
  dsc.flags = DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PIXELFORMAT;
  dsc.caps = DSCAPS_NONE;
  dsc.pixelformat = DSPF_RGB24;
  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &output_surface));

}

static char*
StretchBlit (int w, int h) {
  if (dfb == NULL
      || input_surface == NULL
      || output_surface == NULL) {
    return NULL;
  }

  DFBRectangle dst;
  const char* outbuf = NULL;
  int outpitch = 0;

  dst.x = 0; dst.y = 0;
  dst.w = w;
  dst.h = h;

  DFBCHECK(output_surface->StretchBlit(output_surface,
        input_surface, NULL, &dst));

  DFBCHECK(output_surface->Lock(output_surface, DSLF_READ, (void **)&outbuf, &outpitch));

  size_t bufsize = h * outpitch;
  char* retbuf = (char *) malloc (bufsize);
  if (retbuf) {
    memcpy (retbuf, outbuf, bufsize);
  }

  DFBCHECK(output_surface->Unlock(output_surface));

  return retbuf;
}

static void
DestroyInputSurface() {
  if (input_surface) {
    DFBCHECK (input_surface->Release(input_surface));
    input_surface = NULL;
  }
}

static void
DestroyOutputSurface() {
  if (output_surface) {
    DFBCHECK (output_surface->Release(output_surface));
    output_surface = NULL;
  }
}

void frameresize_init () {
  Init ();
}

void frameresize_deinit() {
  DestroyInputSurface ();
  DestroyOutputSurface ();
  Deinit ();
}

char* frameresize_begin (char* input,
    int iw, int ih, int ow, int oh) {
  CreateInputSurface (input, iw, ih);
  CreateOutputSurface (ow, oh);
  return StretchBlit (ow, oh);
}

void frameresize_end (char *buf) {
  DestroyInputSurface ();
  DestroyOutputSurface ();
  if (buf) {
    free (buf);
  }
}


