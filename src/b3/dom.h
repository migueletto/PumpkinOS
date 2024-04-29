typedef enum {
  DOM_DOCUMENT,
  DOM_TAG,
  DOM_TEXT
} node_type_t;

typedef enum {
  TAG_UNKNOWN,
  TAG_A,
  TAG_ABBR,
  TAG_ACRONYM,
  TAG_ADDRESS,
  TAG_ALTGLYPH,
  TAG_ALTGLYPHDEF,
  TAG_ALTGLYPHITEM,
  TAG_ANIMATECOLOR,
  TAG_ANIMATEMOTION,
  TAG_ANIMATETRANSFORM,
  TAG_ANNOTATION_XML,
  TAG_APPLET,
  TAG_AREA,
  TAG_ARTICLE,
  TAG_ASIDE,
  TAG_AUDIO,
  TAG_B,
  TAG_BASE,
  TAG_BASEFONT,
  TAG_BDI,
  TAG_BDO,
  TAG_BGSOUND,
  TAG_BIG,
  TAG_BLINK,
  TAG_BLOCKQUOTE,
  TAG_BODY,
  TAG_BR,
  TAG_BUTTON,
  TAG_CANVAS,
  TAG_CAPTION,
  TAG_CENTER,
  TAG_CITE,
  TAG_CLIPPATH,
  TAG_CODE,
  TAG_COL,
  TAG_COLGROUP,
  TAG_DATA,
  TAG_DATALIST,
  TAG_DD,
  TAG_DEL,
  TAG_DESC,
  TAG_DETAILS,
  TAG_DFN,
  TAG_DIALOG,
  TAG_DIR,
  TAG_DIV,
  TAG_DL,
  TAG_DT,
  TAG_EM,
  TAG_EMBED,
  TAG_FEBLEND,
  TAG_FECOLORMATRIX,
  TAG_FECOMPONENTTRANSFER,
  TAG_FECOMPOSITE,
  TAG_FECONVOLVEMATRIX,
  TAG_FEDIFFUSELIGHTING,
  TAG_FEDISPLACEMENTMAP,
  TAG_FEDISTANTLIGHT,
  TAG_FEDROPSHADOW,
  TAG_FEFLOOD,
  TAG_FEFUNCA,
  TAG_FEFUNCB,
  TAG_FEFUNCG,
  TAG_FEFUNCR,
  TAG_FEGAUSSIANBLUR,
  TAG_FEIMAGE,
  TAG_FEMERGE,
  TAG_FEMERGENODE,
  TAG_FEMORPHOLOGY,
  TAG_FEOFFSET,
  TAG_FEPOINTLIGHT,
  TAG_FESPECULARLIGHTING,
  TAG_FESPOTLIGHT,
  TAG_FETILE,
  TAG_FETURBULENCE,
  TAG_FIELDSET,
  TAG_FIGCAPTION,
  TAG_FIGURE,
  TAG_FONT,
  TAG_FOOTER,
  TAG_FOREIGNOBJECT,
  TAG_FORM,
  TAG_FRAME,
  TAG_FRAMESET,
  TAG_GLYPHREF,
  TAG_H1,
  TAG_H2,
  TAG_H3,
  TAG_H4,
  TAG_H5,
  TAG_H6,
  TAG_HEAD,
  TAG_HEADER,
  TAG_HGROUP,
  TAG_HR,
  TAG_HTML,
  TAG_I,
  TAG_IFRAME,
  TAG_IMAGE,
  TAG_IMG,
  TAG_INPUT,
  TAG_INS,
  TAG_ISINDEX,
  TAG_KBD,
  TAG_KEYGEN,
  TAG_LABEL,
  TAG_LEGEND,
  TAG_LI,
  TAG_LINEARGRADIENT,
  TAG_LINK,
  TAG_LISTING,
  TAG_MAIN,
  TAG_MALIGNMARK,
  TAG_MAP,
  TAG_MARK,
  TAG_MARQUEE,
  TAG_MATH,
  TAG_MENU,
  TAG_META,
  TAG_METER,
  TAG_MFENCED,
  TAG_MGLYPH,
  TAG_MI,
  TAG_MN,
  TAG_MO,
  TAG_MS,
  TAG_MTEXT,
  TAG_MULTICOL,
  TAG_NAV,
  TAG_NEXTID,
  TAG_NOBR,
  TAG_NOEMBED,
  TAG_NOFRAMES,
  TAG_NOSCRIPT,
  TAG_OBJECT,
  TAG_OL,
  TAG_OPTGROUP,
  TAG_OPTION,
  TAG_OUTPUT,
  TAG_P,
  TAG_PARAM,
  TAG_PATH,
  TAG_PICTURE,
  TAG_PLAINTEXT,
  TAG_PRE,
  TAG_PROGRESS,
  TAG_Q,
  TAG_RADIALGRADIENT,
  TAG_RB,
  TAG_RP,
  TAG_RT,
  TAG_RTC,
  TAG_RUBY,
  TAG_S,
  TAG_SAMP,
  TAG_SCRIPT,
  TAG_SECTION,
  TAG_SELECT,
  TAG_SLOT,
  TAG_SMALL,
  TAG_SOURCE,
  TAG_SPACER,
  TAG_SPAN,
  TAG_STRIKE,
  TAG_STRONG,
  TAG_STYLE,
  TAG_SUB,
  TAG_SUMMARY,
  TAG_SUP,
  TAG_SVG,
  TAG_TABLE,
  TAG_TBODY,
  TAG_TD,
  TAG_TEMPLATE,
  TAG_TEXTAREA,
  TAG_TEXTPATH,
  TAG_TFOOT,
  TAG_TH,
  TAG_THEAD,
  TAG_TIME,
  TAG_TITLE,
  TAG_TR,
  TAG_TRACK,
  TAG_TT,
  TAG_U,
  TAG_UL,
  TAG_VAR,
  TAG_VIDEO,
  TAG_WBR,
  TAG_XMP
} tag_type_t;

typedef struct {
  char *name;
  tag_type_t id;
} tag_name_t;

typedef struct node_class_t {
  char *name;
  struct node_class_t *next;
} node_class_t;

typedef struct {
  uint32_t seq;
  uint32_t nthreads;
  ht *tagnames;
  ht *colors;
  ht *classes;
  ht *ids;
  ht *tags;
  script_engine_t *engine;
  script_ref_t document_ref;
  script_ref_t window_ref;
  uint32_t nextid;
  int pe;
  int (*fetch)(char *path, uint32_t id);
} html_env_t;

typedef struct tree_t {
  node_type_t type;
  tag_type_t tag;
  struct tree_t *root;
  struct tree_t *parent;
  struct tree_t *children;
  struct tree_t *next;
  struct tree_t *last;
  char *name;
  ht *attributes;
  ht *style;
  node_class_t *classes;
  node_class_t *last_class;
  int nchildren;
  int64_t obj;
  uint32_t id;
  void *data;
} tree_t;
