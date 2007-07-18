#ifndef _GIFTI_H_
#define _GIFTI_H_

/* ---------------------------------------------------------------------- */
/* These must be 0-based and sequential.
        - 0 matches _UNDEF
        - highest maches _MAX
        - list matches corresponding gifti_*_list
*/

#define GIFTI_IND_ORD_UNDEF     0
#define GIFTI_IND_ORD_HIGH2LOW  1
#define GIFTI_IND_ORD_LOW2HIGH  2
#define GIFTI_IND_ORD_MAX       2

#define GIFTI_CAT_UNDEF         0
#define GIFTI_CAT_COORDINATES   1
#define GIFTI_CAT_FUNCTIONAL    2
#define GIFTI_CAT_NORMALS       3
#define GIFTI_CAT_LABELS        4
#define GIFTI_CAT_RGBA          5
#define GIFTI_CAT_SHAPE         6
#define GIFTI_CAT_TENSORS       7
#define GIFTI_CAT_TOPO_TRI      8
#define GIFTI_CAT_MAX           8

#define GIFTI_DATALOC_UNDEF     0
#define GIFTI_DATALOC_INT       1
#define GIFTI_DATALOC_EXT       2
#define GIFTI_DATALOC_MAX       2

#define GIFTI_ENCODING_UNDEF     0
#define GIFTI_ENCODING_ASCII     1
#define GIFTI_ENCODING_BINARY    2
#define GIFTI_ENCODING_GZIP      3
#define GIFTI_ENCODING_MAX       3

#define GIFTI_ENDIAN_UNDEF       0
#define GIFTI_ENDIAN_BIG         1
#define GIFTI_ENDIAN_LITTLE      2
#define GIFTI_ENDIAN_MAX         2

/* global declarations of matching lists */
extern char * gifti_index_order_list[];
extern char * gifti_category_list[];
extern char * gifti_dataloc_list[];
extern char * nifti_datatype_name_list[];
extern int    nifti_datatype_value_list[];
extern char * gifti_encoding_list[] ;
extern char * gifti_endian_list[];

/* ---------------------------------------------------------------------- */

/* notes:

        - all data should be owned by the struct, i.e. strings should be
          allocated copies, never assigned as a pointer used elsewhere
*/

typedef struct {
    int     length;     /* do we want a nalloc field? */
    char ** name;
    char ** value;
} nvpairs;
typedef nvpairs MetaData;       /* use MetaData? */

typedef struct {
    int     length;
    int   * index;              /* should 0 be invalid ? */
    char ** label;
} LabelTable;

typedef struct {
    char   * dataspace;
    char   * xformspace;
    double   xform[4][4];       /* ******** no type specified ********* */
} CoordSystem;

typedef struct {
    /* attributes */
    int           category;     /* category of data                     */
    int           datatype;     /* numerical type of Data values        */
    int           location;     /* Internal or External                 */
    int           ind_ord;      /* lowest Dim to highest, or reverse    */
    int           num_dim;      /* level of DimX applied                */
    int           dim0, dim1;   /* dimension lengths                    */
    int           dim2, dim3;
    int           dim4, dim5;
    int           encoding;     /* format of Data on disk               */
    int           endian;       /* endian, if binary Encoding           */

    /* elements */
    MetaData      meta;
    CoordSystem * coordsys;
    void        * data;         /* unencoded, uncompressed, swapped?    */

    /* extras */
    int           nvals;        /* number of values (product of Dims)   */
    int           nbyper;       /* number of bytes per value            */
    nvpairs       ex_atrs;      /* extra attributes                     */
} DataArray;

typedef struct {
    /* attributes */
    char        * version;      /* GIFTI version (string??)             */
    int           numDA;        /********* should this be added? ********/

    /* elements */
    MetaData      meta;
    LabelTable    labeltable;
    DataArray  ** darray;

    /* extras */
    int           swapped;      /* were the bytes swapped               */
    int           compressed;   /* was the data compressed              */
    nvpairs       ex_atrs;      /* extra attributes                     */
} gifti_image;

typedef struct {
    int verb;
} gifti_globals;

typedef struct {
    int    type;          /* should match NIFTI_TYPE_* */
    int    nbyper;        /* bytes per value           */
    int    swapsize;      /* bytes per swap piece      */
    char * name;          /* text string match type    */
} gifti_type_ele;


/* prototypes */

/* main interface protos */
gifti_image * gifti_read_image (const char * fname, int read_data );
int gifti_write_image(gifti_image * gim, const char * fname, int write_data);


int    gifti_free_image         (gifti_image * gim );
/* end main interface protos */

int    gifti_darray_nvals       (DataArray * da);
void   gifti_datatype_sizes     (int datatype, int *nbyper, int *swapsize);
char * gifti_datatype2str       (int type);
int    gifti_gim_DA_size        (gifti_image * p, int in_mb);
int    gifti_get_verb           (void);
int    gifti_set_verb           (int level);
int    gifti_set_xml_buf_size   (int buf_size);
int    gifti_str2attr_gifti     (gifti_image * gim, const char * attr,
                                                        const char * val);
int    gifti_str2attr_darray    (DataArray * DA, const char * attr,
                                                 const char * value);
int    gifti_str2ind_ord        (const char * str);
int    gifti_str2category       (const char * str);
int    gifti_str2dataloc        (const char * str);
int    gifti_str2encoding       (const char * str);
int    gifti_str2endian         (const char * str);
int    gifti_str2datatype       (const char * str);

int    gifti_add_empty_darray   (gifti_image * gim);
int    gifti_add_to_nvpairs     (nvpairs * p, const char * name,
                                              const char * value);

int    gifti_free_CoordSystem   (CoordSystem * cs);
int    gifti_free_DataArray_list(DataArray ** darray, int numDA);
int    gifti_free_DataArray     (DataArray * darray);
int    gifti_free_LabelTable    (LabelTable * t);
int    gifti_free_nvpairs       (nvpairs * p);

int    gifti_disp_nvpairs        (const char *mesg, nvpairs *p);
int    gifti_disp_LabelTable     (const char *mesg, LabelTable *p);
int    gifti_disp_CoordSystem    (const char *mesg, CoordSystem *p);
int    gifti_disp_DataArray      (const char *mesg, DataArray *p, int subs);
int    gifti_disp_gifti_image    (const char *mesg, gifti_image *p, int subs);

int    clear_nvpairs            (nvpairs * p);
int    clear_LabelTable         (LabelTable * p);
int    clear_CoordSystem        (CoordSystem * p);

DataArray * gifti_find_DA       (gifti_image * gim, int category, int index);
int         gifti_find_DA_list  (gifti_image * gim, int category,
                                 DataArray *** list,int *len);
int         gifti_DA_rows_cols  (DataArray * da, int *rows, int *cols);


#endif /* _GIFTI_H_ */
