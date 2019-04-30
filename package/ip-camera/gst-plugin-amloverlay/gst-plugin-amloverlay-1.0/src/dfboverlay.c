#include <stdlib.h>
#include <directfb.h>
#include <directfb_strings.h>
#include <directfb_util.h>

#define MAX_MULTILINES 32

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

static IDirectFBFont *CreateFont (const char *fontfile,
    int size, int outline_width, int outline_opacity) {
  IDirectFBFont *font = NULL;
  DFBFontAttributes attributes = DFFA_NONE;
  DFBFontDescription  fdesc;

  if (outline_width == 0) {
    attributes |= DFFA_OUTLINED;
  } else {
    attributes = DFFA_NONE;
  }

  /* Create the font. */
  fdesc.flags           = DFDESC_HEIGHT | DFDESC_ATTRIBUTES | DFDESC_OUTLINE_WIDTH | DFDESC_OUTLINE_OPACITY;
  fdesc.height          = size;
  fdesc.attributes      = attributes;
  fdesc.outline_width   = outline_width;
  fdesc.outline_opacity = outline_opacity;

  DFBCHECK (dfb->CreateFont (dfb, fontfile, &fdesc, &font));

  return font;

}

static void DestroyFont (IDirectFBFont *font) {
  if (font) {
    DFBCHECK (font->Release (font));
    font = NULL;
  }
}

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

static void CreateInputSurface (unsigned char* buf, int width, int height) {
  if (input_surface) return;

  DFBSurfaceDescription sdsc;
  sdsc.width = width;
  sdsc.height = height;
  sdsc.flags = DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT;
  sdsc.caps = DSCAPS_NONE;
  sdsc.pixelformat = DSPF_RGB24;
  sdsc.preallocated[0].data = buf;
  sdsc.preallocated[0].pitch = width * 3;
  sdsc.preallocated[1].data = NULL;
  sdsc.preallocated[1].pitch = 0;
  DFBCHECK (dfb->CreateSurface (dfb, &sdsc, &input_surface));
}

static void DestroyInputSurface() {
  if (input_surface) {
    DFBCHECK (input_surface->Release(input_surface));
    input_surface = NULL;
  }
}

static int
string_line_break (char *str, char *lines[]) {
  if (str == NULL) return 0;
  char *p = str;
  int n = 0;
  if (lines) {
    lines[n] = p;
  }
  n ++;
  while (*p != '\0') {
    if (*p == '\n') {
      if (lines) {
        *p = '\0';
        if (*(p+1) != '\0') {
          lines[n] = p+1;
        }
      }
      if (*(p+1) != '\0') {
        n++;
      }
    }
    p++;
  }
  return n;
}

void overlay_init() {
  Init();
}

void overlay_deinit() {
  DestroyInputSurface ();
  Deinit ();
}

void* overlay_create_font (const char* fontfile, int fontsize, int outline_width) {
  return (void *)CreateFont (fontfile, fontsize, outline_width, 128);
}

void overlay_destroy_font (void *font) {
  DestroyFont ((IDirectFBFont *)font);
}

void overlay_create_inputbuffer(unsigned char* buf, int width, int height) {
  CreateInputSurface (buf, width, height);
}

void overlay_destroy_inputbuffer() {
  DestroyInputSurface ();
}

void overlay_get_string_wh(const char* txt, void *font, int *w, int *h) {
  int sw = 0, sh = 0;
  if (font) {
    IDirectFBFont *f = (IDirectFBFont *)font;
    DFBCHECK (f->GetStringWidth(f, txt, -1, &sw));
    DFBCHECK (f->GetHeight(f, &sh));
  }

  int lines = string_line_break (txt, NULL);
  lines = lines > 0 ? lines : 1;

  if (w) *w = sw;
  if (h) *h = sh * lines;
}

static void swapARGB2ABGR(char *data, int pitch, int height) {
  if (data == NULL) return;
  int pixel_w = pitch / 4;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < pixel_w; w++) {
      int byte_index = h * pitch + w * 4;
      char r = data[byte_index + 0]; //R
      data[byte_index + 0] = data[byte_index + 2]; // R->B
      data[byte_index + 2] = r; // B->R;
    }
  }
}

static void FixAlpha(char *data, int pitch, int height, char alpha) {
  if (data == NULL) return;
  int pixel_w = pitch / 4;
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < pixel_w; w++) {
      int alpha_index = h * pitch + w * 4 + 3;
      data[alpha_index] = (data[alpha_index] * alpha) >> 8;
    }
  }
}

void *overlay_create_text_surface(const char* txt, void *font, int color, int bgcolor) {
  if (txt == NULL || strlen(txt) == 0 || font == NULL) return NULL;

  DFBSurfaceDescription desc;
  IDirectFBSurface *text_surface = NULL;
  IDirectFBFont *f = (IDirectFBFont *)font;

  char *mtxt = strdup (txt);
  char *txt_lines[MAX_MULTILINES];
  int lines = string_line_break (mtxt, txt_lines);
  int i;
  int line_height;
  DFBCHECK (f->GetHeight(f, &line_height));

  desc.flags = DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT;
  desc.pixelformat = DSPF_ARGB;
  DFBCHECK (f->GetStringWidth(f, txt, -1, &desc.width));
  desc.height = line_height * lines;

  u8 b = (bgcolor >> 24) & 0xff;
  u8 g = (bgcolor >> 16) & 0xff;
  u8 r = (bgcolor >>  8) & 0xff;
  u8 a = (bgcolor >>  0) & 0xff;

  DFBCHECK (dfb->CreateSurface (dfb, &desc, &text_surface));
  DFBCHECK (text_surface->Clear (text_surface, r, g, b, a));

  b = (color >> 24) & 0xff;
  g = (color >> 16) & 0xff;
  r = (color >>  8) & 0xff;
  a = (color >>  0) & 0xff;
  DFBColorID color_ids[2] = {DCID_PRIMARY, DCID_OUTLINE};
  DFBColor colors[2] = {
    { a, r, g, b},
    { a, r, g, b }
  };

  DFBCHECK (text_surface->SetFont (text_surface, font));
  DFBCHECK (text_surface->SetColors (text_surface, color_ids, colors, 2));
  DFBCHECK (text_surface->SetSrcBlendFunction(text_surface, DSBF_SRCALPHA));
  DFBCHECK (text_surface->SetDstBlendFunction(text_surface, DSBF_INVSRCALPHA));
  for (i = 0; i < lines; i++) {
    int y = line_height * i;
    DFBCHECK (text_surface->DrawString (text_surface, txt_lines[i], -1, 0, y,
          DSTF_TOPLEFT | DSTF_OUTLINE | DSTF_BLEND_FUNCS));
  }
  free (mtxt);

  char *data = NULL;
  int pitch = 0;
  DFBCHECK (text_surface->Lock (text_surface, DSLF_WRITE, (void **) &data, &pitch));
  FixAlpha (data, pitch, desc.height, a);
  DFBCHECK (text_surface->Unlock (text_surface));

  return (void *)text_surface;

}

void *overlay_create_image_surface(const char* img, int width, int height) {
  IDirectFBImageProvider *provider = NULL;
  DFBSurfaceDescription desc;
  IDirectFBSurface *img_surface = NULL;
  DFBCHECK (dfb->CreateImageProvider (dfb, img, &provider));

  if (provider == NULL) return NULL;

  if (width >0 && height > 0) {
    desc.flags = DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT;
    desc.pixelformat = DSPF_ARGB;
    desc.width = width;
    desc.height = height;
  } else {
    DFBCHECK (provider->GetSurfaceDescription (provider, &desc));
  }
  DFBCHECK (dfb->CreateSurface (dfb, &desc, &img_surface));
  DFBCHECK (provider->RenderTo (provider, img_surface, NULL));
  provider->Release (provider);

  char *data = NULL;
  int pitch = 0;
  DFBCHECK (img_surface->Lock (img_surface, DSLF_WRITE, (void **) &data, &pitch));
  swapARGB2ABGR (data, pitch, desc.height);
  DFBCHECK (img_surface->Unlock (img_surface));

  return img_surface;
}

void overlay_destroy_surface(void *surface) {
  if (surface) {
    IDirectFBSurface *s = (IDirectFBSurface *)surface;
    DFBCHECK (s->Release (s));
  }
}

void overlay_draw_surface(void *surface, int x, int y) {
  if (surface == NULL || input_surface == NULL) return;
  IDirectFBSurface *s = (IDirectFBSurface *)surface;

  DFBCHECK (input_surface->SetSrcBlendFunction(input_surface, DSBF_SRCALPHA));
  DFBCHECK (input_surface->SetDstBlendFunction(input_surface, DSBF_INVSRCALPHA));
  DFBCHECK (input_surface->SetBlittingFlags(input_surface,
        //DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_DST_PREMULTIPLY));
           DSBLIT_BLEND_ALPHACHANNEL));
  DFBCHECK (input_surface->Blit (input_surface, s, NULL, x, y));
}

void overlay_draw_rect(int x, int y, int w, int h, int thickness, int color) {
  if (input_surface) {
    u8 b = (color >> 24) & 0xff;
    u8 g = (color >> 16) & 0xff;
    u8 r = (color >>  8) & 0xff;
    u8 a = (color >>  0) & 0xff;
    DFBCHECK (input_surface->SetColor (input_surface, r, g, b, a));
    for (int t = 0; t < thickness; t++) {
      DFBCHECK (input_surface->DrawRectangle (input_surface, x + t, y + t, w - t * 2, h - t * 2));
    }
  }
}
