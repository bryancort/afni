
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gifti_io.h"
#include "gifti_test.h"

int show_help()
{
    fprintf(stderr,
    "------------------------------------------------------------\n"
    "gifti_test  - test reading/writing a GIFTI dataset\n"
    "\n"
    "    examples:\n"
    "        1. read in a GIFTI dataset (verbose, show output?)\n"
    "\n"
    "            gifti_test -infile dset.gii\n"
    "            gifti_test -infile dset.gii -verb 3\n"
    "            gifti_test -infile dset.gii -show\n"
    "\n"
    "        2. copy a GIFTI dataset (check differences?)\n"
    "\n"
    "            gifti_test -infile dset.gii -gfile copy.gii\n"
    "            diff dset.gii copy.gii\n"
    "\n"
    "        3. copy a GIFTI data, but write out only 3 surf indices: 0,4,5\n"
    "\n"
    "            gifti_test -infile time_series.gii -gfile ts3.gii  \\\n"
    "                       -slist 3 0 4 5\n"
    "\n"
    "        4. create .asc surfaces dataset (surf.asc)\n"
    "\n"
    "            gifti_test -infile pial.gii -prefix surf\n"
    "\n"
    "        5. create .1D time series surface dataset (surf.1D)\n"
    "\n"
    "            gifti_test -infile time_series.gii -prefix surf\n"
    "\n"
    "    options:\n"
    "       -help           : show this help\n"
    "\n"
    "       -b64_check TYPE : set method for checking base64 errors\n"
    "                  TYPE = NONE       : no checks - assume all is well\n"
    "                  TYPE = CHECK      : report whether errors were found\n"
    "                  TYPE = COUNT      : count the number of bad chars\n"
    "                  TYPE = SKIP       : ignore any bad characters\n"
    "                  TYPE = SKIPnCOUNT : ignore but count bad characters\n"
    "\n"
    "       -buf_size       : set buffer size\n"
    "                         e.g. -buf_size 1024\n"
    "       -encoding  TYPE : set the data encoding for any output file\n"
    "                  TYPE = ASCII      : ASCII encoding\n"
    "                  TYPE = BASE64     : base64 binary\n"
    "                  TYPE = BASE64GZIP : base64 compressed binary\n"
    "       -gfile   OUTPUT : write out dataset as gifti image\n"
    "       -gifti_hist     : show giftilib history\n"
    "       -gifti_ver      : show giftilib version\n"
    "       -infile  INPUT  : write out dataset as gifti image\n"
    "       -no_data        : do not write out data\n"
    "       -prefix  OUTPUT : write out dataset(s) as surf images\n"
    "                         (these are either .asc or .1D files)\n"
    "       -show           : show final gifti image\n"
    "       -slist LEN s0...: restrict output to list of length LEN\n"
    "       -verb    VERB   : set verbose level\n"
    "       -zlevel  LEVEL  : set compression level (-1 or 0..9)\n"
    "------------------------------------------------------------\n"
    );
    return 0;
}

int main( int argc, char * argv[] )
{
    gifti_image * gim;
    char        * infile = NULL, * prefix = NULL, * gfile = NULL;
    int         * slist = NULL, slen = 0;
    int           c, ac, show = 0, data = 1, zlevel;
    int           encoding = 0; /* no change, else 1,2,3  */

    if( argc <= 1 ) {
        show_help();
        return 1;
    }

    for( ac = 1; ac < argc; ac++ )
    {
        if( !strcmp(argv[ac], "-buf_size") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-buf_size");
            if( gifti_set_xml_buf_size(atoi(argv[ac])) ) return 1;
        } else if( !strcmp(argv[ac], "-b64_check") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-b64_check");
            if     ( !strcmp(argv[ac], "NONE" ) ) gifti_set_b64_check(0);
            else if( !strcmp(argv[ac], "CHECK") ) gifti_set_b64_check(1);
            else if( !strcmp(argv[ac], "COUNT") ) gifti_set_b64_check(2);
            else if( !strcmp(argv[ac], "SKIP" ) ) gifti_set_b64_check(3);
            else if( !strcmp(argv[ac], "SKIPnCOUNT" ) ) gifti_set_b64_check(4);
            else {
                fprintf(stderr,"** invalid parm to -b64_check: %s\n",argv[ac]);
                return 1;
            }
        } else if( !strcmp(argv[ac], "-encoding") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-encoding");
            if     ( !strcmp(argv[ac], "ASCII" ) )
                encoding = GIFTI_ENCODING_ASCII;
            else if( !strcmp(argv[ac], "BASE64") )
                encoding = GIFTI_ENCODING_B64BIN;
            else if( !strcmp(argv[ac], "BASE64GZIP") )
                encoding = GIFTI_ENCODING_B64GZ;
            else {
                fprintf(stderr,"** invalid parm to -encoding: %s\n",argv[ac]);
                return 1;
            }
        } else if( !strcmp(argv[ac], "-gifti_hist") ) {
            gifti_disp_lib_hist();
            return 0;
        } else if( !strcmp(argv[ac], "-gifti_ver") ) {
            gifti_disp_lib_version();
            return 0;
        } else if( !strcmp(argv[ac], "-gfile") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-gfile");
            gfile = argv[ac];
        } else if( !strcmp(argv[ac], "-help") ) {
            show_help();
            return 1;
        } else if( !strcmp(argv[ac], "-infile") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-infile");
            infile = argv[ac];
        } else if( !strcmp(argv[ac], "-no_data") ) {
            data = 0;
        } else if( !strcmp(argv[ac], "-prefix") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-prefix");
            prefix = argv[ac];
        } else if( !strcmp(argv[ac], "-show") ) {
            show = 1;
        } else if( !strcmp(argv[ac], "-slist") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-slist");
            slen = atol(argv[ac]);
            if( slen <= 0 ){ 
                fprintf(stderr,"** bad -slist length, '%s'\n", argv[ac]);
                return 1;
            }

            slist = (int *)malloc(slen*sizeof(int));
            if( !slist ) {
                fprintf(stderr,"** failed alloc of len %d slist\n",slen);
                return 1;
            }
            for( c = 0; c < slen; c++ ) {
                ac++;
                if( ac >= argc ) {
                    fprintf(stderr,"** have only %d of %d -slist files\n",
                            c, slen);
                    return 1;
                }
                slist[c] = atol(argv[ac]);
                if( slist[c] < 0 ){ 
                    fprintf(stderr,"** bad slist index[%d], '%s'\n",c,argv[ac]);
                    return 1;
                }
            }
            
        } else if( !strcmp(argv[ac], "-verb") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-verb");
            gifti_set_verb( atoi(argv[ac]) );
        } else if( !strcmp(argv[ac], "-zlevel") ) {
            ac++;
            CHECK_NEXT_OPT(ac, argc, "-zlevel");
            zlevel = atoi(argv[ac]);
            if( gifti_set_zlevel(zlevel) ) return 1;
        } else {
            fprintf(stderr,"** unknown option: '%s'\n",argv[ac]);
            return 1;
        }
    }

    /* be sure we have something to read */
    if( !infile ) {
        fprintf(stderr,"** missing option: -infile\n");
        return 1;
    }

    /* actually read the dataset */
    gim = gifti_read_da_list(infile, 0, slist, slen);
    if( !gim ) {
        fprintf(stderr,"** failed gifti_read_da_list()\n");
        return 1;
    }

    if( show ) gifti_disp_gifti_image("FINAL IMAGE", gim, 1 );

    /* possibly adjust encoding */
    if( encoding > GIFTI_ENCODING_UNDEF && encoding <= GIFTI_ENCODING_MAX )
        for( c = 0; c < gim->numDA; c++ )
            if( gim->darray[c]->encoding ) gim->darray[c]->encoding = encoding;

    if( gfile ) gifti_write_image(gim, gfile, data);
    if( prefix ) write_as_ascii(gim, prefix);

    /* clean up */
    gifti_free_image(gim);  gim = NULL;

    if( slist ){ free(slist); slist = NULL; slen = 0; }

    return 0;
}

int write_as_ascii(gifti_image * gim, char * prefix)
{
    giiDataArray  * dac; /* coords */
    giiDataArray  * dat; /* triangles */
    giiDataArray ** da_list; /* time series? */
    int             len;

    fprintf(stderr,"-- trying to write data with prefix '%s'\n", prefix);

    /* write surface file, *.1D */
    if( (dac = gifti_find_DA(gim, GIFTI_CAT_COORDINATES, 0)) &&
        (dat = gifti_find_DA(gim, GIFTI_CAT_TOPO_TRI, 0))    )
        (void) write_surf_file(dac, dat, prefix, 1);
    else
        fprintf(stderr,"** failed to find coordinate struct\n");

    if( gifti_find_DA_list(gim, GIFTI_CAT_FUNCTIONAL, &da_list, &len) ) {
        fprintf(stderr,"** failed to find functional list\n");
        return 1;
    }

    /* write time series file, *.1D */
    if( len > 0 ) {
        (void) write_1D_file(da_list, len, prefix, 1); 
        free(da_list);
    }
    else
        fprintf(stderr,"** no functionals in gifti_image\n");

    return 0;
}


/* if dlist contains 1 element, write out as 2-D list,
   else each DA must have only 1 dimension */
int write_1D_file(giiDataArray ** dlist, int len, char * prefix, int add_suf)
{
    giiDataArray * da;
    FILE         * fp;
    char         * name = prefix;
    char         * nbuf = NULL;
    int            rows, cols, c;

    if( add_suf ) {     /* create a new name */
        nbuf = (char *)malloc(strlen(prefix) + strlen(".1D") + 1);
        strcpy(nbuf, prefix);
        strcat(nbuf, ".1D");
        name = nbuf;
    }

    if( len == 1 ){     /* write out as 2D list */
        /* note the number of rows and columns */
        fprintf(stderr,"++ writing 1D '%s' from single DA\n", name);
        da = dlist[0];
        if( gifti_DA_rows_cols(da, &rows, &cols) ) {
            if( nbuf ) free(nbuf);
            return 1;
        }

        if( !(fp = fopen(name, "w")) ) {
            fprintf(stderr,"** failed to open '%s' for 'w'\n",name);
            if( nbuf ) free(nbuf);
            return 1;
        }

        fprintf(stderr,"++ 1D write, RxC = %d x %d\n", rows, cols);
        if( da->ind_ord == GIFTI_IND_ORD_COL_MAJOR ) {
            fprintf(stderr,"-- writing data rows in reverse order\n");
            for(c = rows-1; c >= 0; c-- )
                ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
        } else {
            fprintf(stderr,"-- writing data rows in normal order\n");
            for(c = 0; c < rows; c++ )
                ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
        }
    } else {            /* write da->nvals lines of 'num values */
        void ** vlist = (void **)malloc(len * sizeof(void *));

        fprintf(stderr,"++ writing 1D '%s' from DA list (%d)\n", name, len);

        /* set data pointers */
        for( c = 0; c < len; c++ ) {
            vlist[c] = dlist[c]->data;
            if( dlist[c]->nvals != dlist[0]->nvals ) {
                fprintf(stderr,"** d[%d] has %u vals, but d[0] has %u\n",
                        c,(unsigned)dlist[c]->nvals,(unsigned)dlist[0]->nvals);
                free(vlist);
                return 1;
            } else if (dlist[c]->datatype != dlist[0]->datatype) {
                fprintf(stderr,"** d[%d] has type %d, but d[0] has %d\n",
                        c, dlist[c]->datatype, dlist[0]->datatype);
                free(vlist);
                return 1;
            }
        }

        /* good to go */
        if( !(fp = fopen(name, "w")) ) {
            fprintf(stderr,"** failed to open '%s' for 'w'\n",name);
            if( nbuf ) free(nbuf);
            return 1;
        }

        fprintf(stderr,"++ 1D write, RxC = %u x %d\n",
                (unsigned)dlist[0]->nvals, len);
        ewrite_many_lines(vlist, dlist[0]->datatype,len, dlist[0]->nvals, 0,fp);
                          
        free(vlist);
    }

    fprintf(stderr,"++ 1D write, apparent success\n");

    fclose(fp);

    return 0;
}


int write_surf_file(giiDataArray * dc, giiDataArray * dt, char * prefix,
                    int add_suf)
{
    giiDataArray * da;
    FILE         * fp;
    char         * name = prefix;
    char         * nbuf = NULL;
    int            crows, ccols, trows, tcols, rows, cols, c;

    if( add_suf ) {     /* create a new name */
        nbuf = (char *)malloc(strlen(prefix) + strlen(".asc") + 1);
        strcpy(nbuf, prefix);
        strcat(nbuf, ".asc");
        name = nbuf;
    }

    if( !(fp = fopen(name, "w")) ) {
        fprintf(stderr,"** failed to open '%s' for 'w'\n",name);
        if( nbuf ) free(nbuf);
        return 1;
    }

    /* note the number of rows and columns */
    if( gifti_DA_rows_cols(dc, &crows, &ccols) ) {
        fclose(fp);
        if( nbuf ) free(nbuf);
        return 1;
    } else if( gifti_DA_rows_cols(dt, &trows, &tcols) ) {
        fclose(fp);
        if( nbuf ) free(nbuf);
        return 1;
    }

    fprintf(fp, "#!ascii version of surface\n"
                "%d %d\n", crows, trows);

    /* write out the coordinates */

    da = dc;
    rows = crows;
    cols = ccols;

    if( da->ind_ord == GIFTI_IND_ORD_COL_MAJOR ) {
        fprintf(stderr,"-- writing coord rows in reverse order\n");
        for(c = rows-1; c >= 0; c-- )
            ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
    } else {
        fprintf(stderr,"-- writing coord rows in normal order\n");
        for(c = 0; c < rows; c++ )
            ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
    }

    /* write out the triangles */

    da = dt;
    rows = trows;
    cols = tcols;

    if( da->ind_ord == GIFTI_IND_ORD_COL_MAJOR ) {
        fprintf(stderr,"-- writing triangle rows in reverse order\n");
        for(c = rows-1; c >= 0; c-- )
            ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
    } else {
        fprintf(stderr,"-- writing triangle rows in normal order\n");
        for(c = 0; c < rows; c++ )
            ewrite_data_line(da->data, da->datatype, c, cols, 0, 1, fp);
    }


    fclose(fp);

    return 0;
}


int ewrite_data_line(void * data, int type, int row, int cols, int spaces,
                     int trail0, FILE * fp)
{
    int c;
    if( !data || row < 0 || cols <= 0 || !fp ) return 1;

    fprintf(fp, "%*s", spaces, " ");
    switch( type ) {
        default :
            fprintf(stderr,"** write_data_line, unknown type %d\n",type);
            return -1;
        case 2: {       /* NIFTI_TYPE_UINT8 */
            unsigned char * ptr = (unsigned char *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c]);
            break;
        }
        case 4: {       /* NIFTI_TYPE_INT16 */
            short * ptr = (short *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c]);
            break;
        }
        case 8: {       /* NIFTI_TYPE_INT32 */
            int * ptr = (int *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c]);
            break;
        }
        case 16: {      /* NIFTI_TYPE_FLOAT32 */
            float * ptr = (float *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%f ", ptr[c]);
            break;
        }
        case 32: {      /* NIFTI_TYPE_COMPLEX64 */
            float * ptr = (float *)data + row * cols;
            for(c = 0; c < 2*cols; c+=2)fprintf(fp, "%f %f   ",ptr[c],ptr[c+1]);            break;
        }
        case 64: {      /* NIFTI_TYPE_FLOAT64 */
            double * ptr = (double *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%f ", ptr[c]);
            break;
        }
        case 128: {     /* NIFTI_TYPE_RGB24 */
            unsigned char * ptr = (unsigned char *)data + row * cols;
            for( c = 0; c < 3*cols; c+=3 )
                fprintf(fp, "%u %u %u   ", ptr[c], ptr[c+1], ptr[c+2]);
            break;
        }
        case 256: {     /* NIFTI_TYPE_INT8 */
            char * ptr = (char *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c]);
            break;
        }
        case 512: {     /* NIFTI_TYPE_UINT16 */
            unsigned short * ptr = (unsigned short *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c]);
            break;
        }
        case 768: {     /* NIFTI_TYPE_UINT32 */
            unsigned int * ptr = (unsigned int *)data + row * cols;
            for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c]);
            break;
        }
        case 1024: {    /* NIFTI_TYPE_INT64 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1280: {    /* NIFTI_TYPE_UINT64 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1536: {    /* NIFTI_TYPE_FLOAT128 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1792: {    /* NIFTI_TYPE_COMPLEX128 */
            double * ptr = (double *)data + row * cols;
            for(c = 0; c < 2*cols; c+=2)fprintf(fp, "%f %f   ",ptr[c],ptr[c+1]);            break;
        }
        case 2048: {    /* NIFTI_TYPE_COMPLEX256 */
            /* rcr - do we need to check #defines? */
            break;
        }
    }

    if( trail0 ) fputs(" 0", fp);  /* maybe write trailing zero */

    fputc('\n', fp);

    return 0;
}


/* write out as cols by rows (else we'd use ewrite_data_line) */
int ewrite_many_lines(void ** data, int type, long long cols, long long rows,
                      int spaces, FILE * fp)
{
    long long r, c;

    if( !data || rows == 0 || cols == 0 || !fp ) return 1;

    fprintf(fp, "%*s", spaces, " ");
    switch( type ) {
        default :
            fprintf(stderr,"** write_data_line, unknown type %d\n",type);
            return -1;
        case 2: {       /* NIFTI_TYPE_UINT8 */
            unsigned char ** ptr = (unsigned char **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 4: {       /* NIFTI_TYPE_INT16 */
            short ** ptr = (short **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 8: {       /* NIFTI_TYPE_INT32 */
            int ** ptr = (int **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 16: {      /* NIFTI_TYPE_FLOAT32 */
            float ** ptr = (float **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%f ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 32: {      /* NIFTI_TYPE_COMPLEX64 */
            break;
        }
        case 64: {      /* NIFTI_TYPE_FLOAT64 */
            double ** ptr = (double **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%f ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 128: {     /* NIFTI_TYPE_RGB24 */
            break;
        }
        case 256: {     /* NIFTI_TYPE_INT8 */
            char ** ptr = (char **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%d ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 512: {     /* NIFTI_TYPE_UINT16 */
            unsigned short ** ptr = (unsigned short **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 768: {     /* NIFTI_TYPE_UINT32 */
            unsigned int ** ptr = (unsigned int **)data;
            for( r = 0; r < rows; r++ ) {
                for( c = 0; c < cols; c++ ) fprintf(fp, "%u ", ptr[c][r]);
                fputc('\n', fp);
            }
            break;
        }
        case 1024: {    /* NIFTI_TYPE_INT64 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1280: {    /* NIFTI_TYPE_UINT64 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1536: {    /* NIFTI_TYPE_FLOAT128 */
            /* rcr - do we need to check #defines? */
            break;
        }
        case 1792: {    /* NIFTI_TYPE_COMPLEX128 */
            break;
        }
        case 2048: {    /* NIFTI_TYPE_COMPLEX256 */
            /* rcr - do we need to check #defines? */
            break;
        }
    }

    return 0;
}

