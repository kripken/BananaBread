
enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_GEOM
};

#define MAPVERSION 32           // bump if map format changes, see worldio.cpp

struct octaheader
{
    char magic[4];              // "OCTA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int lightmaps;
    int blendmap;
    int numvars;
    int numvslots;
};
    
struct compatheader             // map file format header
{
    char magic[4];              // "OCTA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int lightmaps;
    int lightprecision, lighterror, lightlod;
    uchar ambient;
    uchar watercolour[3];
    uchar blendmap;
    uchar lerpangle, lerpsubdiv, lerpsubdivsize;
    uchar bumperror;
    uchar skylight[3];
    uchar lavacolour[3];
    uchar waterfallcolour[3];
    uchar reserved[10];
    char maptitle[128];
};

#define WATER_AMPLITUDE 0.4f
#define WATER_OFFSET 1.1f

enum 
{ 
    MATSURF_NOT_VISIBLE = 0,
    MATSURF_VISIBLE,
    MATSURF_EDIT_ONLY
};

#define TEX_SCALE 8.0f

struct vertexff { vec pos; bvec norm; uchar reserved; float u, v; float lmu, lmv; };
struct vertex { vec pos; bvec norm; uchar reserved; float u, v; short lmu, lmv; bvec tangent; uchar bitangent; };
 
#define VTXSIZE (renderpath==R_FIXEDFUNCTION ? sizeof(vertexff) : sizeof(vertex))

