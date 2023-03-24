typedef struct {
  double x0, y0, x1, y1;
} MapBoundingType;

typedef struct {
  Int32 x0, pad1, y0, pad2, x1, pad3, y1, pad4;
} MapIBoundingType;

typedef struct {
  UInt8 r, g, b;
  UInt8 pad;
} MapColorType;

typedef struct {
  UInt16 id;
  double a, rf;
  Int16 dx, dy, dz;
} MapDatumType;

#define MAP_DEFAULTNAME	"Map"
#define MAP_VERSION	1

#define MAP_TYPE_PICTURE	1
#define MAP_TYPE_PDB		2

#define MAP_SCALE_NONE	0
#define MAP_SCALE_KM	1
#define MAP_SCALE_MI	2

#define MAX_LAYERS	128
#define MAX_LAYERS_PALM	16

#define MAP_NAME	32
#define MAP_OBJNAME	64
#define MAP_FILENAME	256
#define MAX_PROJ_ARGS	16
#define MAX_REGIONS	16
#define MAX_DATUMS	1024
#define MAX_COLORS	256
#define MAX_SYMBOLS	65536
#define MAX_FONTS	32
#define MAX_LABELS	256
#define MAX_RECSIZE	65440

#define MAX_FILES	32
#define MAX_COUNTRIES   256
#define MAX_STATES      256
#define MAX_VCITIES     256
#define MAX_HIGHWAYS    256

			// regionrec
#define MAX_OBJ_POINTS	((MAX_RECSIZE - (MAX_REGIONS * MAX_REGIONS * 4)) / 2)
#define MAX_OBJ_PLINES	((MAX_RECSIZE - (MAX_REGIONS * MAX_REGIONS * 4)) / 2)

				// 8 registros pointrec
#define MAX_PLINE_POINTS	(8 * (MAX_RECSIZE / 8))

				// 32 registros pointrec
#define MAX_LARGE_PLINE_POINTS	(32 * (MAX_RECSIZE / 8))

#define MAP_CUSTOM	0
#define MAP_POINT	1
#define MAP_PLINE	2
#define MAP_REGION	3

#define MAP_LAYER_ACTIVE	0x01
#define MAP_LAYER_NAMED		0x02
#define MAP_LAYER_SYMBOL	0x04
#define MAP_LAYER_COLOR		0x08
#define MAP_LAYER_LARGE		0x10

#define NUM_ZOOM		16
#define DEFAULT_ZOOM		7

typedef struct {
  int argc;
  char *argv[MAX_PROJ_ARGS];
} MapProjType;

typedef struct {
  char name[MAP_NAME];
  UInt16 type;
  UInt16 flags;
  UInt16 datarec;
  UInt16 symbolrec;
  UInt16 pointrec;
  UInt16 textrec;
  UInt16 indexrec;
  UInt16 colorrec;
  UInt16 poirec;
  UInt16 nobjs;
  UInt16 ndatarecs;
  UInt16 nsymbolrecs;
  UInt16 npointrecs;
  UInt16 ntextrecs;
  UInt16 nindexrecs;
  UInt16 npoirecs;
  UInt16 pad[2];
  UInt16 regionrec[NUM_ZOOM];
  UInt16 xregions, yregions;
  MapBoundingType bounds;
  MapColorType color, bgcolor, textcolor, padc;
} MapLayerType;

typedef struct {
  UInt16 version;
  UInt16 datum;
  UInt16 nlayers;
  UInt16 paleterec;
  char name[MAP_NAME];
  MapBoundingType bounds;
  MapColorType color, bgcolor, textcolor, padc;
  MapLayerType layer[1];
} MapType;

// Layer POINT
// pointrec contem nobjs * MapCoordType

// Layer LINE
// pointrec contem nobjs * 2 * MapCoordType

// Layer PLINE
// datarec contem nobjs * MapPlineType
// pointrec contem nobjs * n * MapCoordType

// Layer REGION
// datarec contem nobjs * MapPlineType
// pointrec contem nobjs * n * MapCoordType

// Se Layer flags & MAP_LAYER_NAMED:
// textrec contem nobjs * (strlen(name)+1)
// indexrec contem nobjs * UInt32 : offset dos nomes no textrec

typedef struct {
  Int32 x, y;
} MapICoordType;

typedef struct {
  double x, y;
} MapCoordType;

typedef struct {
  UInt16 firstpoint;
  UInt16 npoints;
} MapPlineType;

typedef struct {
  UInt16 type;
  UInt16 poiindex;
} MapPOIType;

typedef struct {
  UInt32 number;
  UInt32 street;
  UInt32 city;
  UInt32 zip;
  UInt32 phone;
} MapPOIIndexType;

typedef struct {
  char *name;
  char *code;
  char *fullname; // name + separador + code
  Int32 label;
} MapCountryType;

typedef struct {
  char *name;
  char *code;
  char *fullname; // name + separador + code
  Int32 label;
  char *country;
  Int32 countryindex;
} MapStateType;

// Cidades que aparecem em POIs mas nao tem um ponto correspondente
typedef struct {
  char *name;
  char *state;
  Int32 label;
  Int32 stateindex;
} MapVirtualCityType;

typedef struct {
  char *name;
  char *state;
  Int32 label;
  Int32 stateindex;
} MapHighwayType;

typedef struct {
  char *state;
  Int32 stateindex;
} MapCityType;

typedef struct {
  void *im;
  int font, color, bgcolor, textcolor, x, y;
} MapImageType;

// ainda em desenvolvimento
typedef struct {
  char file[MAP_FILENAME];
} MapFontType;

typedef struct {
  Int16 layer, type, x, y, color;
  FontID font;
  double angle;
  char *label;
} MapLabelType;

typedef struct {
  MapType *map;
  char htmlprefix[MAP_FILENAME], htmldir[MAP_FILENAME],
       cmdprefix[MAP_FILENAME], port[MAP_NAME],
       symbolfile[MAP_FILENAME], callbackprefix[MAP_FILENAME];
  double reduction[MAX_LAYERS];
  int width, height, scale;
  UInt8 *datarec[MAX_LAYERS], *pointrec[MAX_LAYERS], *textrec[MAX_LAYERS];
  UInt32 datalen[MAX_LAYERS], pointlen[MAX_LAYERS], textlen[MAX_LAYERS];
  UInt32 *indexrec[MAX_LAYERS], indexlen[MAX_LAYERS];
  UInt32 *poirec[MAX_LAYERS], poilen[MAX_LAYERS];
  UInt8 *symbolrec[MAX_LAYERS];
  UInt32 symbollen[MAX_LAYERS];
  UInt8 *colorrec[MAX_LAYERS];
  UInt32 colorlen[MAX_LAYERS];
  UInt16 *regionrec[MAX_LAYERS][NUM_ZOOM];
  UInt32 regionlen[MAX_LAYERS][NUM_ZOOM];
  UInt8 *objzoom[MAX_LAYERS];
  UInt16 *mstype[MAX_LAYERS];
  UInt8 *mszone[MAX_LAYERS];
  char *mapfile[MAX_LAYERS], *textfile[MAX_LAYERS];
  char *fontname[MAX_FONTS], *fontfile[MAX_FONTS];
  int mapfont, layerfont[MAX_LAYERS];
  int textfield[MAX_LAYERS], hasindex[MAX_LAYERS];
  MapCityType *city[MAX_LAYERS];
  MapCountryType *country;
  MapStateType *state;
  MapVirtualCityType *vcity;
  MapHighwayType *highway;
  int ncountries, nstates, nvcities, nhighways;
  void **symbols;
  MapProjType projdef, layerprojdef[MAX_LAYERS];
  MapProjType scaleprojdef, pdbprojdef;
  void *map_proj, *scale_proj, *pdb_proj;
  void *mutex, *env, *scratch;
  MapImageType *im;
  int seq, sock, ncolors;
  MapDatumType datum[MAX_DATUMS];
  MapColorType palete[MAX_COLORS];
  int nlabels;
  MapLabelType *label;
  UInt32 id, product;
  UInt16 version;
  UInt8 priority, streetbeforenumber, zipbeforecity, overview, background;
  char *copyright, *series, *family, *repository;
  int nfiles, fileid[MAX_FILES], layerid[MAX_LAYERS];
  char *file[MAX_FILES];
} MapWrapper;
