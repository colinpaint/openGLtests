// cVg.h - based on Mikko Mononen memon@inside.org nanoVg
#pragma once
//{{{  includes
#include <cstdint>
#include <string>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cPointF.h"
#include "sColourF.h"

#include "../glad/glad.h"
#include "../../glfw/include/GLFW/glfw3.h"

#include "../utils/utils.h"
#include "../utils/cLog.h"
//}}}
//{{{  inline math utils
constexpr float kPi = 3.14159265358979323846264338327f;
constexpr float k2Pi = kPi * 2.f;
constexpr float kPiDiv2 = kPi / 2.f;
constexpr float k180Pi = kPi * 180.f;

inline int clampi (int a, int mn, int mx) { return a < mn ? mn : (a > mx ? mx : a); }
inline float clampf (float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }
inline float absf (float a) { return a >= 0.0f ? a : -a; }

inline float quantize (float a, float d) { return ((int)(a / d + 0.5f)) * d; }
inline float signf (float a) { return a >= 0.0f ? 1.0f : -1.0f; }
inline float cross (float dx0, float dy0, float dx1, float dy1) { return dx1*dy0 - dx0*dy1; }
//{{{
inline float normalize (float& x, float& y) {

  float d = sqrtf(x*x + y*y);
  if (d > 1e-6f) {
    float id = 1.0f / d;
    x *= id;
    y *= id;
    }

  return d;
  }
//}}}

inline float degToRad (float deg) { return deg / k180Pi; }
inline float radToDeg (float rad) { return rad / k180Pi; }
//{{{
inline unsigned int nearestPow2 (unsigned int num) {

  unsigned n = num > 0 ? num - 1 : 0;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;

  return n;
  }
//}}}

//{{{
inline int equal (float x1, float y1, float x2, float y2, float tol) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return dx*dx + dy*dy < tol*tol;
  }
//}}}
//{{{
inline float distPointSeg (float x, float y, float px, float py, float qx, float qy) {

  float pqx = qx - px;
  float pqy = qy - py;
  float dx = x - px;
  float dy = y - py;

  float d = pqx*pqx + pqy*pqy;
  float t = pqx*dx + pqy*dy;
  if (d > 0)
    t /= d;
  if (t < 0)
    t = 0;
  else if (t > 1)
    t = 1;

  dx = px + t*pqx - x;
  dy = py + t*pqy - y;

  return dx*dx + dy*dy;
  }
//}}}
//{{{
inline void intersectRects (float* dst,
                            float ax, float ay, float aw, float ah,
                            float bx, float by, float bw, float bh) {

  float minx = std::max (ax, bx);
  float miny = std::max (ay, by);
  float maxx = std::min (ax+aw, bx+bw);
  float maxy = std::min (ay+ah, by+bh);

  dst[0] = minx;
  dst[1] = miny;
  dst[2] = std::max (0.0f, maxx - minx);
  dst[3] = std::max (0.0f, maxy - miny);
  }
//}}}

//{{{
inline float hue (float h, float m1, float m2) {

  if (h < 0)
    h += 1;
  if (h > 1)
    h -= 1;

  if (h < 1.0f/6.0f)
    return m1 + (m2 - m1) * h * 6.0f;
  else if (h < 3.0f/6.0f)
    return m2;
  else if (h < 4.0f/6.0f)
    return m1 + (m2 - m1) * (2.0f/3.0f - h) * 6.0f;

  return m1;
  }
//}}}
//}}}

class cAtlasText;
class cVg {
public:
  enum eCreateFlags { eNoSolid = 0x01, eNoEdges = 0x02,  eNoDelete = 0x40 };
  //{{{
  enum eBlendFactor {
    NVG_ZERO                = 1<<0,
    NVG_ONE                 = 1<<1,
    NVG_SRC_COLOR           = 1<<2,
    NVG_ONE_MINUS_SRC_COLOR = 1<<3,
    NVG_DST_COLOR           = 1<<4,
    NVG_ONE_MINUS_DST_COLOR = 1<<5,
    NVG_SRC_ALPHA           = 1<<6,
    NVG_ONE_MINUS_SRC_ALPHA = 1<<7,
    NVG_DST_ALPHA           = 1<<8,
    NVG_ONE_MINUS_DST_ALPHA = 1<<9,
    NVG_SRC_ALPHA_SATURATE  = 1<<10
    };
  //}}}
  //{{{
  enum eCompositeOp {
    NVG_SOURCE_OVER,
    NVG_SOURCE_IN,
    NVG_SOURCE_OUT,
    NVG_ATOP,
    NVG_DESTINATION_OVER,
    NVG_DESTINATION_IN,
    NVG_DESTINATION_OUT,
    NVG_DESTINATION_ATOP,
    NVG_LIGHTER,
    NVG_COPY,
    NVG_XOR };
  //}}}
  //{{{
  class cTransform {
  // sx,sy define scaling, kx,ky skewing, tx,ty translation.
  // last row is assumed to be 0,0,1 and not stored.
  //   [sx kx tx]
  //   [ky sy ty]
  //   [ 0  0  1]
  public:
    cTransform();
    cTransform (float sx, float ky, float kx, float sy, float tx, float ty);

    float getAverageScaleX();
    float getAverageScaleY();
    float getAverageScale();
    float getTranslateX() { return mTx; }
    float getTranslateY() { return mTy; }
    cTransform getInverse();
    void getMatrix3x4 (float* matrix3x4);

    void setIdentity();
    void setTranslate (cPointF t);
    void setTranslate (float tx, float ty);
    void setScale (float sx, float sy);
    void setRotate (float angle);
    void setRotateTranslate (float angle, float tx, float ty);
    void setRotateTranslate (float angle, cPointF t);
    void set (float sx, float ky, float kx, float sy, float tx, float ty);

    void multiply (cTransform& t);
    void premultiply (cTransform& t);

    void point (float& x, float& y);
    void point (float srcx, float srcy, float& dstx, float& dsty);
    void pointScissor (float srcx, float srcy, float& dstx, float& dsty);

    // vars
    float mSx;
    float mKy;
    float mKx;
    float mSy;

    float mTx;
    float mTy;

    bool mIdentity;

  private:
    //{{{
    bool isIdentity() {
      return mSx == 1.0f && mKy == 0.0f && mKx == 0.0f && mSy == 1.0f && mTx == 0.0f && mTy == 0.0f;
      }
    //}}}
    };
  //}}}
  //{{{
  struct sPaint {
    //{{{
    void set (const sColourF& colour) {

      mTransform.setIdentity();

      extent[0] = 0.0f;
      extent[1] = 0.0f;

      radius = 0.0f;
      feather = 1.0f;

      innerColour = colour;
      outerColour = colour;

      mImageId = 0;
      }
    //}}}

    cTransform mTransform;
    float extent[2];

    float radius;
    float feather;

    sColourF innerColour;
    sColourF outerColour;

    int mImageId;
    };
  //}}}

  cVg() : cVg(0) {}
  cVg (int flags);
  virtual ~cVg();
  void initialise();

  //{{{  state
  void saveState();
  void restoreState();
  void resetState();

  void setCompositeState (eCompositeOp compositeOp);
  void setCompositeBlendFunc (eBlendFactor sfactor, eBlendFactor dfactor);
  void setCompositeBlendFuncSeparate (eBlendFactor srcRGB, eBlendFactor dstRGB,
                                      eBlendFactor srcAlpha, eBlendFactor dstAlpha);
  //}}}
  //{{{  transform
  void resetTransform();

  void setTranslate (cPointF p);
  void setTranslate (float x, float y);
  void setScale (float x, float y);
  void setRotate (float angle);

  void setTransform (float a, float b, float c, float d, float e, float f);
  //}}}
  //{{{  scissor
  void scissor (cPointF p, cPointF size);
  void intersectScissor (cPointF p, cPointF size);

  void resetScissor();
  //}}}
  //{{{  text
  //{{{
  enum eAlign {
    // Horizontal align
    eAlignLeft   = 1<<0, // Default, align text horizontally to left.
    eAlignCentre = 1<<1, // Align text horizontally to center.
    eAlignRight  = 1<<2, // Align text horizontally to right.

    // Vertical align
    eAlignTop      = 1<<3, // Align text vertically to top.
    eAlignMiddle   = 1<<4, // Align text vertically to middle.
    eAlignBottom   = 1<<5, // Align text vertically to bottom.
    eAlignBaseline = 1<<6, // Default, align text vertically to baseline.
    };
  //}}}
  //{{{
  struct sGlyphPosition {
    const char* str;  // Position of the glyph in the input string.
    float x;          // The x-coordinate of the logical glyph position.
    float minx, maxx; // The bounds of the glyph shape.
    };
  //}}}

  int createFont (const std::string& fontName, uint8_t* data, int dataSize);

  float getTextBounds (cPointF p, const std::string& str, float* bounds);
  float getTextMetrics (float& ascender, float& descender);
  int getTextGlyphPositions (cPointF p, const std::string& str, sGlyphPosition* positions, int maxPositions);

  void setFontById (int font);
  void setFontByName (const std::string& fontName);
  void setFontSize (float size);
  void setTextAlign (int align);
  void setTextLetterSpacing (float spacing);
  void setTextLineHeight (float lineHeight);

  float text (cPointF p, const std::string& str);
  //}}}
  //{{{  image
  enum eImageFlags {              // set of image flags
    eImageGenerateMipmaps = 0x01, // generate mipmaps during creation of the image
    eImageRepeatX         = 0x02, // repeat image in X direction
    eImageRepeatY         = 0x04, // repeat image in Y direction
    eImageFlipY           = 0x08, // flip image in Y direction when rendered
    eImagePreMultiplied   = 0x10, // image data has premultiplied alpha
    eImageNearestPixel    = 0x20, // image interpolation is nearestPixel rather than linear interpolatopn
    };

  enum eTexture { eTextureAlpha, eTextureRgba };

  int createImageRGBA (int width, int height, int imageFlags, const uint8_t* data);
  int createImage (int imageFlags, uint8_t* data, int dataSize);

  void updateImage (int id, const uint8_t* data);

  bool deleteImage (int id);
  //}}}
  //{{{  shape
  enum eWinding { eCounterClockWise, eClockWise };
  enum eLineCap { eButt, eRound, eSquare, eBevel, eMiter };
  enum eShapeCommands { eMoveTo, eLineTo, eBezierTo, eWindingDirection, eClose };

  float getFringeWidth() { return mFringeWidth; }

  void setAlpha (float alpha);
  void setFillColour (const sColourF& colour);
  void setStrokeColour (const sColourF& colour);
  void setStrokeWidth (float size);
  void setFringeWidth (float width);

  void setLineCap (eLineCap cap);
  void setLineJoin (eLineCap join);
  void setMiterLimit (float limit);

  void setFillPaint (const sPaint& paint);
  void setStrokePaint (const sPaint& paint);

  sPaint setBoxGradient (cPointF p, cPointF size, float radius, float feather,
                         const sColourF& innerColour, const sColourF& outerColour);
  sPaint setRadialGradient (cPointF centre, float innerRadius, float outerRadius,
                            const sColourF& innerColour, const sColourF& outerColour);
  sPaint setLinearGradient (cPointF start, cPointF end,
                            const sColourF& innerColour, const sColourF& outerColour);
  sPaint setImagePattern (cPointF centre, cPointF size, float angle, int imageId, float alpha);

  void beginPath();
  void pathWinding (eWinding dir);

  void moveTo (cPointF p);
  void lineTo (cPointF p);
  void bezierTo (cPointF c1, cPointF c2, cPointF p);
  void quadTo (cPointF centre, cPointF p);
  void arcTo (cPointF p1, cPointF p2, float radius);
  void arc (cPointF centre, float r, float a0, float a1, int dir);

  void rect (cPointF p, cPointF size);
  void roundedRectVarying (cPointF p, cPointF size, float tlRadius, float trRadius, float brRadius, float blRadius);
  void roundedRect (cPointF p, cPointF size, float radius);
  void ellipse (cPointF centre, cPointF radius);
  void circle (cPointF centre, float radius);

  void closePath();

  void fill();
  void stroke();
  void triangleFill();
  //}}}
  //{{{  frame
  std::string getFrameStats();

  void toggleEdges() { mDrawEdges = !mDrawEdges; }
  void toggleSolid() { mDrawSolid = !mDrawSolid; }
  void toggleTriangles() { mDrawTriangles = !mDrawTriangles; }

  void beginFrame (int width, int height, float devicePixelRatio);
  void endFrame();
  //}}}

private:
  //{{{  static constexpr
  static constexpr int kMaxStates = 32;
  static constexpr int kMaxFontTextures = 4;
  //}}}
  enum eUniformBindings { FRAG_BINDING };
  enum eUniformLocation { LOCATION_VIEWSIZE, LOCATION_TEX, LOCATION_FRAG, MAX_LOCATIONS };
  enum eShaderType { SHADER_FILL_GRADIENT, SHADER_FILL_IMAGE, SHADER_SIMPLE, SHADER_IMAGE };
  //{{{
  struct sVertex {
    //{{{
    void set (float x, float y, float u, float v) {
      mX = x;
      mY = y;
      mU = u;
      mV = v;
      }
    //}}}
    //{{{
    void set (cPointF p, cPointF uv) {
      mX = p.x;
      mY = p.y;
      mU = uv.x;
      mV = uv.y;
      }
    //}}}

    float mX = 0.0f;
    float mY = 0.0f;

    float mU = 0.0f;
    float mV = 0.0f;
    };
  //}}}
  //{{{
  struct sPathVertices {
    int mNumFillVertices = 0;
    int mFirstFillVertexIndex = 0;

    int mNumStrokeVertices = 0;
    int mFirstStrokeVertexIndex = 0;
    };
  //}}}
  //{{{
  struct sPath {
    int mNumPoints = 0;
    int mFirstPointIndex = 0;

    eWinding mWinding = eCounterClockWise;
    bool mConvex = false;
    bool mClosed = false;
    int mNumBevel = 0;

    sPathVertices mPathVertices;
    };
  //}}}
  //{{{
  struct sDraw {
    enum eType { eStroke, eConvexFill, eStencilFill, eTriangle, eText };
    //{{{
    void set (eType type, int id, int firstPathVerticesIndex, int numPaths, int firstFragIndex,
              int firstVertexIndex, int numVertices) {

      mType = type;
      mId = id;

      mFirstPathVerticesIndex = firstPathVerticesIndex;
      mNumPaths = numPaths;

      mFirstFragIndex = firstFragIndex;

      mTriangleFirstVertexIndex = firstVertexIndex;
      mNumTriangleVertices = numVertices;
      }
    //}}}

    eType mType;
    int mId = 0;

    int mFirstPathVerticesIndex = 0;
    int mNumPaths = 0;

    int mFirstFragIndex = 0;

    int mTriangleFirstVertexIndex = 0;
    int mNumTriangleVertices = 0;
    };
  //}}}
  //{{{
  struct sTexture {
    //{{{
    void reset() {
      id = 0;

      type = 0;
      flags = 0;
      tex = 0;

      width = 0;
      height = 0;
      }
    //}}}

    int id = 0;

    int type = 0;
    int flags = 0;
    GLuint tex = 0;

    int width = 0;
    int height = 0;
    };
  //}}}
  //{{{
  struct cScissor {
    cTransform mTransform;
    float extent[2];
    };
  //}}}
  //{{{
  struct sFrag {
    //{{{
    void setSimple() {
      sUniform.type = SHADER_SIMPLE;
      sUniform.strokeThreshold = -1.0f;
      }
    //}}}
    //{{{
    void setImage (sPaint& paint, cScissor& scissor, sTexture* texture) {

      sUniform.innerColour = paint.innerColour.getPremultiplied();
      sUniform.outerColour = paint.outerColour.getPremultiplied();

      if ((scissor.extent[0] < -0.5f) || (scissor.extent[1] < -0.5f)) {
        // unity transform, inverse
        memset (sUniform.scissorMatrix, 0, sizeof(sUniform.scissorMatrix));
        sUniform.scissorExt[0] = 1.f;
        sUniform.scissorExt[1] = 1.f;
        sUniform.scissorScale[0] = 1.f;
        sUniform.scissorScale[1] = 1.f;
        }
      else {
        // get scissorMatrix from inverse transform
        scissor.mTransform.getInverse().getMatrix3x4 (sUniform.scissorMatrix);
        sUniform.scissorExt[0] = scissor.extent[0];
        sUniform.scissorExt[1] = scissor.extent[1];
        sUniform.scissorScale[0] = scissor.mTransform.getAverageScaleX();
        sUniform.scissorScale[1] = scissor.mTransform.getAverageScaleY();
        }

      memcpy (sUniform.extent, paint.extent, sizeof(sUniform.extent));
      sUniform.strokeMult = 1.f;
      sUniform.strokeThreshold = -1.f;

      sUniform.type = SHADER_IMAGE;
      if (texture->type == eTextureRgba)
        sUniform.texType = (texture->flags & cVg::eImagePreMultiplied) ? 0.f : 1.f;
      else
        sUniform.texType = 2.f;

      // get paintMatrix from inverse transform
      paint.mTransform.getInverse().getMatrix3x4 (sUniform.paintMatrix);
      }
    //}}}
    //{{{
    void setFill (sPaint& paint, cScissor& scissor, float width, float fringe, float strokeThreshold, sTexture* tex) {

      sUniform.innerColour = paint.innerColour.getPremultiplied();
      sUniform.outerColour = paint.outerColour.getPremultiplied();

      if ((scissor.extent[0] < -0.5f) || (scissor.extent[1] < -0.5f)) {
        // unity transform, inverse
        memset (sUniform.scissorMatrix, 0, sizeof(sUniform.scissorMatrix));
        sUniform.scissorExt[0] = 1.f;
        sUniform.scissorExt[1] = 1.f;
        sUniform.scissorScale[0] = 1.f;
        sUniform.scissorScale[1] = 1.f;
        }
      else {
        // get scissorMatrix from inverse transform
        scissor.mTransform.getInverse().getMatrix3x4 (sUniform.scissorMatrix);
        sUniform.scissorExt[0] = scissor.extent[0];
        sUniform.scissorExt[1] = scissor.extent[1];
        sUniform.scissorScale[0] = scissor.mTransform.getAverageScaleX() / fringe;
        sUniform.scissorScale[1] = scissor.mTransform.getAverageScaleY() / fringe;
        }

      memcpy (sUniform.extent, paint.extent, sizeof(sUniform.extent));
      sUniform.strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
      this->sUniform.strokeThreshold = strokeThreshold;

      cTransform inverse;
      if (paint.mImageId) {
        sUniform.type = SHADER_FILL_IMAGE;
        if ((tex->flags & cVg::eImageFlipY) != 0) {
          //{{{  flipY
          cTransform m1;
          m1.setTranslate (0.0f, sUniform.extent[1] * 0.5f);
          m1.multiply (paint.mTransform);

          cTransform m2;
          m2.setScale (1.0f, -1.0f);
          m2.multiply (m1);
          m1.setTranslate (0.0f, -sUniform.extent[1] * 0.5f);
          m1.multiply (m2);
          inverse = m1.getInverse();
          }
          //}}}
        else
          inverse = paint.mTransform.getInverse();

        if (tex->type == eTextureRgba)
          sUniform.texType = (tex->flags & cVg::eImagePreMultiplied) ? 0.f : 1.f;
        else
          sUniform.texType = 2.f;
        }
      else {
        sUniform.type = SHADER_FILL_GRADIENT;
        sUniform.radius = paint.radius;
        sUniform.feather = paint.feather;
        inverse = paint.mTransform.getInverse();
        }
      inverse.getMatrix3x4 (sUniform.paintMatrix);
      }
    //}}}

    union {
      struct {
        float scissorMatrix[12]; // 3vec4's
        float paintMatrix[12];   // 3vec4's
        sColourF innerColour;
        sColourF outerColour;
        float scissorExt[2];
        float scissorScale[2];
        float extent[2];
        float radius;
        float feather;
        float strokeMult;
        float strokeThreshold;
        float texType;
        float type;
        } sUniform;

      #define NANOVG_GL_UNIFORMARRAY_SIZE 11
      float uniformArray[NANOVG_GL_UNIFORMARRAY_SIZE][4];
      };
    };
  //}}}
  //{{{
  class cVertices {
  public:
    cVertices();
    ~cVertices();

    void reset();

    int getNumVertices() { return mNumVertices; }
    sVertex* getVertexPtr (int vertexIndex) { return mVertices + vertexIndex; }

    int alloc (int numVertices);
    void trim (int numVertices);

  private:
    static constexpr int kInitNumVertices = 4000;

    int mNumVertices = 0;
    int mNumAllocatedVertices = 0;
    sVertex* mVertices = nullptr;
    };
  //}}}
  //{{{
  class cShape {
  public:
    //{{{
    class sShapePoint {
    public:
      float x,y;
      float dx, dy;
      float dmx, dmy;
      float len;

      // set of eFlags
      enum eFlags { ePtNONE = 0, ePtCORNER = 0x01, ePtLEFT = 0x02, ePtBEVEL = 0x04, ePtINNERBEVEL = 0x08 };
      uint8_t flags;
      };
    //}}}

    cShape();
    ~cShape();

    float getLastX() { return mLastX; }
    float getLastY() { return mLastY; }
    int getNumCommands() { return mNumCommands; }
    int getNumVertices();

    void addCommand (float* values, int numValues, cTransform& transform);

    void beginPath();
    void flattenPaths();
    void calculateJoins (float w, int lineJoin, float miterLimit);
    void expandFill (cVertices& vertices, float w, eLineCap lineJoin, float miterLimit, float fringeWidth);
    void triangleFill (cVertices& vertices, int& vertexIndex, int& numVertices);
    void expandStroke (cVertices& vertices, float w, eLineCap lineCap, eLineCap lineJoin, float miterLimit, float fringeWidth);

    // vars
    int mNumPaths = 0;
    sPath* mPaths = nullptr;

    int mNumPoints = 0;
    sShapePoint* mPoints = nullptr;

    float mBounds[4]; // xmin,ymin,xmax,ymax
    int mBoundsVertexIndex = 0;

  private:
    //{{{  static constexpr
    static constexpr int kInitCommandsSize = 256;
    static constexpr float kDistanceTolerance = 0.01f;
    static constexpr float kTesselateTolerance = 0.25f;
    //}}}

    float normalize (float& x, float& y);
    int curveDivs (float r, float arc, float tol);
    float polyArea (sShapePoint* points, int mNumPoints);
    void polyReverse (sShapePoint* points, int numPoints);

    sPath* lastPath();
    void lastPathWinding (eWinding winding);
    void closeLastPath();
    void addPath();

    void tesselateBezier (float x1, float y1, float x2, float y2,
                          float x3, float y3, float x4, float y4,
                          int level, sShapePoint::eFlags type);

    sShapePoint* lastPoint();
    void addPoint (float x, float y, sShapePoint::eFlags flags);

    void commandsToPaths();

    void chooseBevel (int bevel, sShapePoint* p0, sShapePoint* p1, float w, float* x0, float* y0, float* x1, float* y1);
    sVertex* roundJoin (sVertex* vertexPtr, sShapePoint* point0, sShapePoint* point1,
                          float lw, float rw, float lu, float ru, int ncap, float fringe);
    sVertex* bevelJoin (sVertex* vertexPtr, sShapePoint* point0, sShapePoint* point1,
                          float lw, float rw, float lu, float ru, float fringe);

    sVertex* buttCapStart (sVertex* vertexPtr, sShapePoint* point, float dx, float dy, float w, float d, float aa);
    sVertex* buttCapEnd (sVertex* vertexPtr, sShapePoint* point, float dx, float dy, float w, float d, float aa);
    sVertex* roundCapStart (sVertex* vertexPtr, sShapePoint* point, float dx, float dy, float w, int ncap, float aa);
    sVertex* roundCapEnd (sVertex* vertexPtr, sShapePoint* point, float dx, float dy, float w, int ncap, float aa);

    // private vars
    int mNumCommands = 0;
    float* mCommands = nullptr;

    float mLastX = 0;
    float mLastY = 0;

    int mNumAllocatedPaths = 0;
    int mNumAllocatedPoints = 0;
    int mNumAllocatedCommands = 0;
    };
  //}}}
  //{{{
  class cShader {
  public:
    ~cShader();

    bool create (const char* opts);
    void getUniforms();

    void setTex (int tex) { glUniform1i (location[LOCATION_TEX], tex); }
    void setViewport (float* viewport) { glUniform2fv (location[LOCATION_VIEWSIZE], 1, viewport); }
    void setFrags (float* frags) { glUniform4fv (location[LOCATION_FRAG], NANOVG_GL_UNIFORMARRAY_SIZE, frags); }

  private:
    void dumpShaderError (GLuint shader, const char* name, const char* type);
    void dumpProgramError (GLuint prog, const char* name);

    // vars
    GLuint prog = 0;
    GLuint frag = 0;
    GLuint vert = 0;

    GLint location[MAX_LOCATIONS];
    };
  //}}}
  //{{{
  struct sCompositeState {
    eBlendFactor srcRGB;
    eBlendFactor dstRGB;
    eBlendFactor srcAlpha;
    eBlendFactor dstAlpha;
    };
  //}}}
  //{{{
  struct sState {
    sCompositeState composite;
    sPaint fillPaint;
    sPaint strokePaint;

    float strokeWidth;
    float miterLimit;
    eLineCap lineJoin;
    eLineCap lineCap;
    float alpha;

    cTransform mTransform;

    cScissor scissor;
    float fontSize;
    float letterSpacing;
    float lineHeight;
    int textAlign;
    int fontId;
    };
  //}}}

  // converts
  GLenum convertBlendFuncFactor (eBlendFactor factor);
  sCompositeState compositeState (eCompositeOp compositeOp);

  // sets
  void setStencilMask (GLuint mask);
  void setStencilFunc (GLenum func, GLint ref, GLuint mask);
  void setBindTexture (GLuint texture);
  void setUniforms (int firstFragIndex, int id);
  void setDevicePixelRatio (float ratio) { devicePixelRatio = ratio; }

  // allocates
  sDraw* allocDraw();
  int allocFrags (int numFrags);
  int allocPathVertices (int numPaths);

  // texture
  int createTexture (int type, int width, int height, int imageFlags, const uint8_t* data, const std::string& debug);
  sTexture* findTextureById (int id);
  bool updateTexture (int id, int x, int y, int width, int height, const uint8_t* data);
  bool getTextureSize (int id, int& width, int& height);

  // render
  void renderStroke (cShape& shape, sPaint& paint, cScissor& scissor, float fringe, float strokeWidth);
  void renderFill (cShape& shape, sPaint& paint, cScissor& scissor, float fringe);
  void renderText (int firstVertexIndex, int numVertices, sPaint& paint, cScissor& scissor);
  void renderTriangles (int firstVertexIndex, int numVertices, sPaint& paint, cScissor& scissor);
  void renderFrame (cVertices& vertices, sCompositeState composite);

  // font
  float getFontScale (sState* state);
  bool allocAtlas();
  void flushAtlasTexture();

  //{{{  vars
  bool mDrawEdges = false;
  bool mDrawSolid = false;
  bool mDrawTriangles = false;
  int mDrawArrays = 0;

  float mViewport[2];
  cShader mShader;

  GLuint mStencilMask = 0;
  GLenum mStencilFunc = 0;
  GLint mStencilFuncRef = 0;
  GLuint mStencilFuncMask = 0;
  GLuint mBindTexture = 0;

  int mTextureId = 0;
  int mNumTextures = 0;
  int mNumAllocatedTextures = 0;
  sTexture* mTextures = nullptr;

  GLuint mVertexBuffer = 0;
  GLuint mVertexArray = 0;
  GLuint mFragBuffer = 0;

  // per frame buffers
  int mNumDraws = 0;
  int mNumAllocatedDraws = 0;
  sDraw* mDraws = nullptr;

  int mNumFrags = 0;
  int mNumAllocatedFrags = 0;
  sFrag* mFrags = nullptr;

  int mNumPathVertices = 0;
  int mNumAllocatedPathVertices = 0;
  sPathVertices* mPathVertices = nullptr;

  int mNumStates = 0;
  sState mStates[kMaxStates];

  cShape mShape;
  cVertices mVertices;

  float mFringeWidth = 1.0f;
  float devicePixelRatio = 1.0f;

  cAtlasText* mAtlasText = nullptr;

  // !!! should vector this !!!
  int mFontTextureIndex = 0;
  int mFontTextureIds[kMaxFontTextures] = { 0 };
  //}}}
  };
