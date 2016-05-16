//=============================================================================================================
/**
* @file     main.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     April, 2016
*
* @section  LICENSE
*
* Copyright (C) 2016, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Example of dipole fit
*
*/


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <iostream>
#include <vector>
#include <math.h>


#include <fiff/fiff.h>
#include <mne/mne.h>
#include <utils/sphere.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtCore/QCoreApplication>
#include <QCommandLineParser>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace FIFFLIB;
using namespace MNELIB;



#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


//============================= mne_allocs.h =============================

/*
 * Basics...
 */
#define MALLOC(x,t) (t *)malloc((x)*sizeof(t))

#define FREE(x) if ((char *)(x) != NULL) free((char *)(x))


//============================= mne_allocs.c =============================

void mne_free_cmatrix (float **m)

{
  if (m) {
    FREE(*m);
    FREE(m);
  }
}

/*
 * float matrices
 */

#define FREE_CMATRIX(m) mne_free_cmatrix((m))


//============================= mne_types.h =============================

typedef void (*mneUserFreeFunc)(void *);  /* General purpose */







typedef struct {		/* Matrix specification with a channel list */
  int   nrow;			/* Number of rows */
  int   ncol;			/* Number of columns */
  char  **rowlist;		/* Name list for the rows (may be NULL) */
  char  **collist;		/* Name list for the columns (may be NULL) */
  float **data;                  /* The data itself (dense) */
} *mneNamedMatrix,mneNamedMatrixRec;

//typedef struct {		/* Matrix specification with a channel list */
//  int   nrow;			/* Number of rows (same as in data) */
//  int   ncol;			/* Number of columns (same as in data) */
//  char  **rowlist;		/* Name list for the rows (may be NULL) */
//  char  **collist;		/* Name list for the columns (may be NULL) */
//  mneSparseMatrix data;		/* The data itself (sparse) */
//} *mneSparseNamedMatrix,mneSparseNamedMatrixRec;

//typedef struct {		/* Vector specification with a channel list */
//  int    nvec;			/* Number of elements */
//  char   **names;		/* Name list for the elements */
//  float  *data;			/* The data itself */
//} *mneNamedVector,mneNamedVectorRec;

typedef struct {		/* One linear projection item */
  mneNamedMatrix vecs;          /* The original projection vectors */
  int            nvec;          /* Number of vectors = vecs->nrow */
  char           *desc;	        /* Projection item description */
  int            kind;          /* Projection item kind */
  int            active;	/* Is this item active now? */
  int            active_file;	/* Was this item active when loaded from file? */
  int            has_meg;	/* Does it have MEG channels? */
  int            has_eeg;	/* Does it have EEG channels? */
} *mneProjItem,mneProjItemRec;

typedef struct {		/* Collection of projection items and the projector itself */
  mneProjItem    *items;        /* The projection items */
  int            nitems;        /* Number of items */
  char           **names;	/* Names of the channels in the final projector */
  int            nch;	        /* Number of channels in the final projector */
  int            nvec;          /* Number of vectors in the final projector */
  float          **proj_data;	/* The orthogonalized projection vectors picked and orthogonalized from the original data */
} *mneProjOp,mneProjOpRec;






//============================= fwd_types.h =============================

#define FWD_COIL_UNKNOWN      0

#define FWD_COILC_UNKNOWN     0
#define FWD_COILC_EEG         1000
#define FWD_COILC_MAG         1
#define FWD_COILC_AXIAL_GRAD  2
#define FWD_COILC_PLANAR_GRAD 3
#define FWD_COILC_AXIAL_GRAD2 4

#define FWD_COIL_ACCURACY_POINT    0
#define FWD_COIL_ACCURACY_NORMAL   1
#define FWD_COIL_ACCURACY_ACCURATE 2

#define FWD_IS_MEG_COIL(x) ((x) != FWD_COILC_EEG && (x) != FWD_COILC_UNKNOWN)

typedef void (*fwdUserFreeFunc)(void *);  /* General purpose */

typedef struct {
  char         *chname;		/* Name of this channel */
  int          coord_frame;	/* Which coordinate frame are we in? */
  char         *desc;	        /* Description for this type of a coil */
  int          coil_class;	/* Coil class */
  int          type;		/* Coil type */
  int          accuracy;	/* Accuracy */
  float        size;		/* Coil size */
  float        base;		/* Baseline */
  float        r0[3];		/* Coil coordinate system origin */
  float        ex[3];		/* Coil coordinate system unit vectors */
  float        ey[3];		/* This stupid construction needs to be replaced with */
  float        ez[3];		/* a coordinate transformation */
  int          np;		/* Number of integration points */
  float        **rmag;		/* The field point locations */
  float        **cosmag;	/* The corresponding direction cosines */
  float        *w;		/* The weighting coefficients */
} *fwdCoil,fwdCoilRec;


typedef struct {
  fwdCoil *coils;		/* The coil or electrode positions */
  int     ncoil;
  int     coord_frame;		/* Common coordinate frame */
  void    *user_data;		/* We can put whatever in here */
  fwdUserFreeFunc user_data_free;
} *fwdCoilSet,fwdCoilSetRec;	/* A collection of the above */

/*
 * This is a convenient generic field / potential computation function
 */
typedef int (*fwdFieldFunc)(float *rd,float *Q,fwdCoilSet coils,float *res,void *client);
typedef int (*fwdVecFieldFunc)(float *rd,fwdCoilSet coils,float **res,void *client);
typedef int (*fwdFieldGradFunc)(float *rd,float *Q,fwdCoilSet coils, float *res,
                float *xgrad, float *ygrad, float *zgrad, void *client);


/*
 * This is used in the function which evaluates integrals over triangles
 */

typedef struct {
  double **xyz; 	         /* The coordinates */
  double *w;			 /* Weights */
  int    np;			 /* How many */
} *fwdIntPoints,fwdIntPointsRec; /* Integration points in a triangle */

typedef double (*fwdIntApproxEvalFunc)(fwdIntPoints int_points, double *r0, void *user);

/*
 * Definitions for the EEG sphere model
 */

typedef struct {
  float rad;			/* The actual rads */
  float rel_rad;		/* Relative rads */
  float sigma;			/* Conductivity */
} *fwdEegSphereLayer,fwdEegSphereLayerRec;


typedef struct {
  char  *name;			/* Textual identifier */
  int   nlayer;			/* Number of layers */
  fwdEegSphereLayer layers;	/* An array of layers */
  float  r0[3];			/* The origin */

  double *fn;		        /* Coefficients saved to speed up the computations */
  int    nterms;		/* How many? */

  float  *mu;			/* The Berg-Scherg equivalence parameters */
  float  *lambda;
  int    nfit;			/* How many? */
  int    scale_pos;		/* Scale the positions to the surface of the sphere? */
} *fwdEegSphereModel,fwdEegSphereModelRec;

typedef struct {
  fwdEegSphereModel *models;	/* Set of EEG sphere model definitions */
  int               nmodel;
} *fwdEegSphereModelSet,fwdEegSphereModelSetRec;

#define FWD_BEM_UNKNOWN           -1
#define FWD_BEM_CONSTANT_COLL     1
#define FWD_BEM_LINEAR_COLL       2

#define FWD_BEM_IP_APPROACH_LIMIT 0.1

#define FWD_BEM_LIN_FIELD_SIMPLE    1
#define FWD_BEM_LIN_FIELD_FERGUSON  2
#define FWD_BEM_LIN_FIELD_URANKAR   3

typedef struct {		      /* Space to store a solution matrix */
  float **solution;		      /* The solution matrix */
  int   ncoil;			      /* Number of sensors */
  int   np;		              /* Number of potential solution points */
} *fwdBemSolution,fwdBemSolutionRec;  /* Mapping from infinite medium potentials to a particular set of coils or electrodes */









typedef struct {
  char       *surf_name;	/* Name of the file where surfaces were loaded from */
  MNESurface *surfs;		/* The interface surfaces from outside towards inside */
  int        *ntri;		/* Number of triangles on each surface */
  int        *np;		/* Number of vertices on each surface */
  int        nsurf;		/* How many */
  float      *sigma;		/* The conductivities */
  float      **gamma;		/* The gamma factors */
  float      *source_mult;	/* These multiply the infinite medium potentials */
  float      *field_mult;	/* Multipliers for the magnetic field */
  int        bem_method;	/* Which approximation method is used */
  char       *sol_name;		/* Name of the file where the solution was loaded from */

  float      **solution;	/* The potential solution matrix */
  float      *v0;		/* Space for the infinite-medium potentials */
  int        nsol;		/* Size of the solution matrix */

  FiffCoordTrans head_mri_t;	/* Coordinate transformation from head to MRI coordinates */

  float      ip_approach_limit;	/* Controls whether we need to use the isolated problem approach */
  int        use_ip_approach;	/* Do we need it */
} *fwdBemModel,fwdBemModelRec;	/* Holds the BEM model definition */




//============================= fit_types.h =============================

/*
 * These are the type definitions for dipole fitting
 */
typedef void (*fitUserFreeFunc)(void *);

typedef struct {
  float **rd;       /* Dipole locations */
  int   ndip;       /* How many dipoles */
  float **fwd;      /* The forward solution (projected and whitened) */
  float *scales;    /* Scales applied to the columns of fwd */
  float **uu;       /* The left singular vectors of the forward matrix */
  float **vv;       /* The right singular vectors of the forward matrix */
  float *sing;      /* The singular values */
  int   nch;        /* Number of channels */
} *dipoleForward,dipoleForwardRec;

typedef struct {
    float          **rr;        /* These are the guess dipole locations */
    dipoleForward  *guess_fwd;  /* Forward solutions for the guesses */
    int            nguess;      /* How many sources */
} *guessData,guessDataRec;


typedef struct {
    fwdFieldFunc    meg_field;          /* MEG forward calculation functions */
    fwdVecFieldFunc meg_vec_field;
    void            *meg_client;        /* Client data for MEG field computations */
    mneUserFreeFunc meg_client_free;

    fwdFieldFunc    eeg_pot;            /* EEG forward calculation functions */
    fwdVecFieldFunc eeg_vec_pot;
    void            *eeg_client;        /* Client data for EEG field computations */
    mneUserFreeFunc eeg_client_free;
} *dipoleFitFuncs,dipoleFitFuncsRec;

#define COLUMN_NORM_NONE 0	    /* No column normalization requested */
#define COLUMN_NORM_COMP 1	    /* Componentwise normalization */
#define COLUMN_NORM_LOC  2	    /* Dipole locationwise normalization */

typedef struct {                        /* This structure holds all fitting-related data */
    FiffCoordTrans    mri_head_t;       /* MRI <-> head coordinate transformation */
    FiffCoordTrans    meg_head_t;       /* MEG <-> head coordinate transformation */
    int               coord_frame;      /* Common coordinate frame */
    FiffChInfo        chs;              /* Channels */
    int               nmeg;             /* How many MEG */
    int               neeg;             /* How many EEG */
    char              **ch_names;       /* List of all channel names */
    RowVectorXf       pick;             /* Matrix to pick data from the
                                           full data set which may contain channels
                                           we are not interested in */
    fwdCoilSet        meg_coils;        /* MEG coil definitions */
    fwdCoilSet        eeg_els;          /* EEG electrode definitions */
    float             r0[3];            /* Sphere model origin */
    char              *bemname;         /* Using a BEM? */

    fwdEegSphereModel eeg_model;        /* EEG sphere model definition */
    fwdBemModel       bem_model;        /* BEM model definition */

    dipoleFitFuncs    sphere_funcs;     /* These are the sphere model forward functions */
    dipoleFitFuncs    bem_funcs;        /* These are the BEM forward functions */
    dipoleFitFuncs    funcs;            /* Points to one of the two above */
    dipoleFitFuncs    mag_dipole_funcs; /* Functions to fit a magnetic dipole */

    int               fixed_noise;      /* Were fixed noise values used rather than a noise-covariance
                                         * matrix read from a file */
    FiffCov           noise_orig;       /* Noise covariance matrix (original) */
    FiffCov           noise;            /* Noise covariance matrix (weighted to take the selection into account) */
    int               nave;             /* How many averages does this correspond to? */
    mneProjOp         proj;             /* The projection operator to use */
    int               column_norm;      /* What kind of column normalization to apply to the forward solution */
    int               fit_mag_dipoles;  /* Fit magnetic dipoles? */
    void              *user;            /* User data for anything we need */
    fitUserFreeFunc   user_free;        /* Function to free the above */
} *dipoleFitData,dipoleFitDataRec;






//============================= dipole_forward.c =============================


void free_dipole_forward(dipoleForward f)

{
  if (!f)
    return;
  FREE_CMATRIX(f->rd);
  FREE_CMATRIX(f->fwd);
  FREE_CMATRIX(f->uu);
  FREE_CMATRIX(f->vv);
  FREE(f->sing);
  FREE(f->scales);
  FREE(f);
  return;
}








//============================= setup.c =============================


guessData new_guess_data()
{
  guessData res = MALLOC(1,guessDataRec);

  res->rr        = NULL;
  res->guess_fwd = NULL;
  res->nguess    = 0;
  return res;
}

static void free_guess_data(guessData g)

{
  int k;
  if (!g)
    return;

  FREE_CMATRIX(g->rr);
  if (g->guess_fwd) {
    for (k = 0; k < g->nguess; k++)
      free_dipole_forward(g->guess_fwd[k]);
    FREE(g->guess_fwd);
  }
  FREE(g);
  return;
}

static dipoleFitFuncs new_dipole_fit_funcs()
{
  dipoleFitFuncs f = MALLOC(1,dipoleFitFuncsRec);

  f->meg_field     = NULL;
  f->eeg_pot       = NULL;
  f->meg_vec_field = NULL;
  f->eeg_vec_pot   = NULL;
  f->meg_client      = NULL;
  f->meg_client_free = NULL;
  f->eeg_client      = NULL;
  f->eeg_client_free = NULL;

  return f;
}


static void free_dipole_fit_funcs(dipoleFitFuncs f)
{
  if (!f)
    return;

  if (f->meg_client_free && f->meg_client)
    f->meg_client_free(f->meg_client);
  if (f->eeg_client_free && f->eeg_client)
    f->eeg_client_free(f->eeg_client);

  FREE(f);
  return;
}


dipoleFitData new_dipole_fit_data()
{
  dipoleFitData res = MALLOC(1,dipoleFitDataRec);

//  res->mri_head_t    = NULL;
//  res->meg_head_t    = NULL;
//  res->chs           = NULL;
  res->meg_coils     = NULL;
  res->eeg_els       = NULL;
  res->nmeg          = 0;
  res->neeg          = 0;
  res->r0[0]         = 0.0;
  res->r0[1]         = 0.0;
  res->r0[2]         = 0.0;
  res->bemname       = NULL;
  res->bem_model     = NULL;
  res->eeg_model     = NULL;
//  res->noise         = NULL;
  res->nave          = 1;
  res->user          = NULL;
  res->user_free     = NULL;
  res->proj          = NULL;

  res->sphere_funcs     = NULL;
  res->bem_funcs        = NULL;
  res->mag_dipole_funcs = NULL;
  res->funcs            = NULL;
  res->column_norm      = COLUMN_NORM_NONE;
  res->fit_mag_dipoles  = FALSE;

  return res;
}


//void free_dipole_fit_data(dipoleFitData d)
//{
//  if (!d)
//    return;

//  FREE(d->mri_head_t);
//  FREE(d->meg_head_t);
//  FREE(d->chs);
//  fwd_free_coil_set(d->meg_coils);
//  fwd_free_coil_set(d->eeg_els);
//  FREE(d->bemname);
//  mne_free_cov(d->noise);
//  mne_free_name_list(d->ch_names,d->nmeg+d->neeg);
//  fwd_bem_free_model(d->bem_model);
//  fwd_free_eeg_sphere_model(d->eeg_model);
//  if (d->user_free)
//    d->user_free(d->user);

//  mne_free_proj_op(d->proj);

//  free_dipole_fit_funcs(d->sphere_funcs);
//  free_dipole_fit_funcs(d->bem_funcs);
//  free_dipole_fit_funcs(d->mag_dipole_funcs);

//  FREE(d);
//  return;
//}


























//dipoleFitData setup_dipole_fit_data(char  *mriname,		 /* This gives the MRI/head transform */
//                    char  *measname,		 /* This gives the MEG/head transform and
//                                  * sensor locations */
//                    char  *bemname,		 /* BEM model */
//                    float *r0,			 /* Sphere model origin in head coordinates (optional) */
//                    fwdEegSphereModel eeg_model, /* EEG sphere model definition */
//                    int   accurate_coils,	 /* Use accurate coil definitions? */
//                    char  *badname,		 /* Bad channels list */
//                    char  *noisename,		 /* Noise covariance matrix */
//                    float grad_std,              /* Standard deviations for the ad-hoc noise cov (planar gradiometers) */
//                    float mag_std,               /* Ditto for magnetometers */
//                    float eeg_std,               /* Ditto for EEG */
//                    float mag_reg,               /* Noise-covariance regularization factors */
//                    float grad_reg,
//                    float eeg_reg,
//                    int   diagnoise,		 /* Use only the diagonal elements of the noise-covariance matrix */
//                    char  **projnames,           /* SSP file names */
//                    int   nproj,                 /* How many of them */
//                    int   include_meg,           /* Include MEG in the fitting? */
//                    int   include_eeg)           /* Include EEG in the fitting? */
//     /*
//      * Background work for modelling
//      */
//{
//    dipoleFitData  res = new_dipole_fit_data();
//    int            k;
//    char           **badlist = NULL;
//    int            nbad      = 0;
//    char           **file_bads;
//    int            file_nbad;
//    int            coord_frame = FIFFV_COORD_HEAD;
//    mneCovMatrix cov;
//    fwdCoilSet     templates = NULL;
//    mneCTFcompDataSet comp_data  = NULL;
//    fwdCoilSet        comp_coils = NULL;
//    /*
//    * Read the coordinate transformations
//    */
//    if (mriname) {
//    if ((res->mri_head_t = mne_read_mri_transform(mriname)) == NULL)
//    goto bad;
//    }
//    else if (bemname) {
//    err_set_error("Source of MRI / head transform required for the BEM model is missing");
//    goto bad;
//    }
//    else {
//    float move[] = { 0.0, 0.0, 0.0 };
//    float rot[3][3] = { { 1.0, 0.0, 0.0 },
//        { 0.0, 1.0, 0.0 },
//        { 0.0, 0.0, 1.0 } };
//    res->mri_head_t = fiff_make_transform(FIFFV_COORD_MRI,FIFFV_COORD_HEAD,rot,move);
//    }

//    mne_print_coord_transform(stderr,res->mri_head_t);
//    if ((res->meg_head_t = mne_read_meas_transform(measname)) == NULL)
//    goto bad;
//    mne_print_coord_transform(stderr,res->meg_head_t);
//    /*
//    * Read the bad channel lists
//    */
//    if (badname) {
//    if (mne_read_bad_channels(badname,&badlist,&nbad) != OK)
//    goto bad;
//    fprintf(stderr,"%d bad channels read from %s.\n",nbad,badname);
//    }
//    if (mne_read_bad_channel_list(measname,&file_bads,&file_nbad) == OK && file_nbad > 0) {
//    if (!badlist)
//    nbad = 0;
//    badlist = REALLOC(badlist,nbad+file_nbad,char *);
//    for (k = 0; k < file_nbad; k++)
//    badlist[nbad++] = file_bads[k];
//    FREE(file_bads);
//    fprintf(stderr,"%d bad channels read from the data file.\n",file_nbad);
//    }
//    fprintf(stderr,"%d bad channels total.\n",nbad);
//    /*
//    * Read the channel information
//    */
//    if (read_meg_eeg_ch_info(measname,include_meg,include_eeg,badlist,nbad,
//           &res->chs,&res->nmeg,&res->neeg) != OK)
//    goto bad;

//    if (res->nmeg > 0)
//    fprintf(stderr,"Will use %3d MEG channels from %s\n",res->nmeg,measname);
//    if (res->neeg > 0)
//    fprintf(stderr,"Will use %3d EEG channels from %s\n",res->neeg,measname);
//    {
//    char *s = mne_channel_names_to_string(res->chs,res->nmeg+res->neeg);
//    int  n;
//    mne_string_to_name_list(s,&res->ch_names,&n);
//    }
//    /*
//    * Make coil definitions
//    */
//    res->coord_frame = coord_frame;
//    if (coord_frame == FIFFV_COORD_HEAD) {
//    #ifdef USE_SHARE_PATH
//    char *coilfile = mne_compose_mne_name("share/mne","coil_def.dat");
//    #else
//    char *coilfile = mne_compose_mne_name("setup/mne","coil_def.dat");
//    #endif

//    if (!coilfile)
//    goto bad;
//    if ((templates = fwd_read_coil_defs(coilfile)) == NULL) {
//    FREE(coilfile);
//    goto bad;
//    }

//    if ((res->meg_coils = fwd_create_meg_coils(templates,res->chs,res->nmeg,
//                       accurate_coils ? FWD_COIL_ACCURACY_ACCURATE : FWD_COIL_ACCURACY_NORMAL,
//                       res->meg_head_t)) == NULL)
//    goto bad;
//    if ((res->eeg_els = fwd_create_eeg_els(res->chs+res->nmeg,res->neeg,NULL)) == NULL)
//    goto bad;
//    fprintf(stderr,"Head coordinate coil definitions created.\n");
//    }
//    else {
//    err_printf_set_error("Cannot handle computations in %s coordinates",mne_coord_frame_name(coord_frame));
//    goto bad;
//    }
//    /*
//    * Forward model setup
//    */
//    res->bemname   = mne_strdup(bemname);
//    if (r0) {
//    res->r0[0]     = r0[0];
//    res->r0[1]     = r0[1];
//    res->r0[2]     = r0[2];
//    }
//    res->eeg_model = eeg_model;
//    /*
//    * Compensation data
//    */
//    if ((comp_data = mne_read_ctf_comp_data(measname)) == NULL)
//    goto bad;
//    if (comp_data->ncomp > 0) {	/* Compensation channel information may be needed */
//    fiffChInfo comp_chs = NULL;
//    int        ncomp    = 0;

//    fprintf(stderr,"%d compensation data sets in %s\n",comp_data->ncomp,measname);
//    if (mne_read_meg_comp_eeg_ch_info(measname,NULL,0,&comp_chs,&ncomp,NULL,NULL,NULL,NULL) == FAIL)
//    goto bad;
//    if (ncomp > 0) {
//    if ((comp_coils = fwd_create_meg_coils(templates,comp_chs,ncomp,
//                     FWD_COIL_ACCURACY_NORMAL,res->meg_head_t)) == NULL) {
//    FREE(comp_chs);
//    goto bad;
//    }
//    fprintf(stderr,"%d compensation channels in %s\n",comp_coils->ncoil,measname);
//    }
//    FREE(comp_chs);
//    }
//    else {			/* Get rid of the empty data set */
//    mne_free_ctf_comp_data_set(comp_data);
//    comp_data = NULL;
//    }
//    /*
//    * Ready to set up the forward model
//    */
//    if (setup_forward_model(res,comp_data,comp_coils) == FAIL)
//    goto bad;
//    res->column_norm = COLUMN_NORM_LOC;
//    /*
//    * Projection data should go here
//    */
//    if (make_projection(projnames,nproj,res->chs,res->nmeg+res->neeg,&res->proj) == FAIL)
//    goto bad;
//    if (res->proj && res->proj->nitems > 0) {
//    fprintf(stderr,"Final projection operator is:\n");
//    mne_proj_op_report(stderr,"\t",res->proj);

//    if (mne_proj_op_chs(res->proj,res->ch_names,res->nmeg+res->neeg) == FAIL)
//    goto bad;
//    if (mne_proj_op_make_proj(res->proj) == FAIL)
//    goto bad;
//    }
//    /*
//    * Noise covariance
//    */
//    if (noisename) {
//    if ((cov = mne_read_cov(noisename,FIFFV_MNE_SENSOR_COV)) == NULL)
//    goto bad;
//    fprintf(stderr,"Read a %s noise-covariance matrix from %s\n",
//    cov->cov_diag ? "diagonal" : "full", noisename);
//    }
//    else {
//    if ((cov = ad_hoc_noise(res->meg_coils,res->eeg_els,grad_std,mag_std,eeg_std)) == NULL)
//    goto bad;
//    }
//    res->noise = mne_pick_chs_cov_omit(cov,res->ch_names,res->nmeg+res->neeg,TRUE,res->chs);
//    if (res->noise == NULL) {
//    mne_free_cov(cov);
//    goto bad;
//    }
//    fprintf(stderr,"Picked appropriate channels from the noise-covariance matrix.\n");
//    mne_free_cov(cov);
//    /*
//    * Apply the projection operator to the noise-covariance matrix
//    */
//    if (res->proj && res->proj->nitems > 0 && res->proj->nvec > 0) {
//    if (mne_proj_op_apply_cov(res->proj,res->noise) == FAIL)
//    goto bad;
//    fprintf(stderr,"Projection applied to the covariance matrix.\n");
//    }
//    /*
//    * Force diagonal noise covariance?
//    */
//    if (diagnoise) {
//    mne_revert_to_diag_cov(res->noise);
//    fprintf(stderr,"Using only the main diagonal of the noise-covariance matrix.\n");
//    }
//    /*
//    * Regularize the possibly deficient noise-covariance matrix
//    */
//    if (res->noise->cov) {
//    float regs[3];
//    int   do_it;

//    regs[MNE_COV_CH_MEG_MAG]  = mag_reg;
//    regs[MNE_COV_CH_MEG_GRAD] = grad_reg;
//    regs[MNE_COV_CH_EEG]      = eeg_reg;
//    /*
//    * Classify the channels
//    */
//    if (mne_classify_channels_cov(res->noise,res->chs,res->nmeg+res->neeg) == FAIL)
//    goto bad;
//    /*
//    * Do we need to do anything?
//    */
//    for (k = 0, do_it = 0; k < res->noise->ncov; k++) {
//    if (res->noise->ch_class[k] != MNE_COV_CH_UNKNOWN &&
//    regs[res->noise->ch_class[k]] > 0.0)
//    do_it++;
//    }
//    /*
//    * Apply regularization if necessary
//    */
//    if (do_it > 0)
//    mne_regularize_cov(res->noise,regs);
//    else
//    fprintf(stderr,"No regularization applied to the noise-covariance matrix\n");
//    }
//    /*
//    * Do the decomposition and check that the matrix is positive definite
//    */
//    fprintf(stderr,"Decomposing the noise covariance...\n");
//    if (res->noise->cov) {
//    if (mne_decompose_eigen_cov(res->noise) == FAIL)
//    goto bad;
//    fprintf(stderr,"Eigenvalue decomposition done.\n");
//    for (k = 0; k < res->noise->ncov; k++) {
//    if (res->noise->lambda[k] < 0.0)
//    res->noise->lambda[k] = 0.0;
//    }
//    }
//    else {
//    fprintf(stderr,"Decomposition not needed for a diagonal covariance matrix.\n");
//    if (mne_add_inv_cov(res->noise) == FAIL)
//    goto bad;
//    }
//    mne_free_name_list(badlist,nbad);
//    fwd_free_coil_set(templates);
//    fwd_free_coil_set(comp_coils);
//    mne_free_ctf_comp_data_set(comp_data);
//    return res;


//    bad : {
//    mne_free_name_list(badlist,nbad);
//    fwd_free_coil_set(templates);
//    fwd_free_coil_set(comp_coils);
//    mne_free_ctf_comp_data_set(comp_data);
//    free_dipole_fit_data(res);
//    return NULL;
//    }
//}

















//*************************************************************************************************************
//=============================================================================================================
// MAIN
//=============================================================================================================

//=============================================================================================================
/**
* The function main marks the entry point of the program.
* By default, main has the storage class extern.
*
* @param [in] argc (argument count) is an integer that indicates how many arguments were entered on the command line when the program was started.
* @param [in] argv (argument vector) is an array of pointers to arrays of character objects. The array objects are null-terminated strings, representing the arguments that were entered on the command line when the program was started.
* @return the value that was set to exit() (which is 0 if exit() is called via quit()).
*/
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);


    // Command Line Parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Clustered Inverse Example");
    parser.addHelpOption();

    QCommandLineOption sampleEvokedFileOption("e", "Path to evoked <file>.", "file", "./MNE-sample-data/MEG/sample/sample_audvis-ave.fif");
    QCommandLineOption sampleCovFileOption("c", "Path to covariance <file>.", "file", "./MNE-sample-data/MEG/sample/sample_audvis-cov.fif");
    QCommandLineOption sampleBemFileOption("b", "Path to BEM <file>.", "file", "./MNE-sample-data/subjects/sample/bem/sample-5120-bem-sol.fif");
    QCommandLineOption sampleTransFileOption("t", "Path to trans <file>.", "file", "./MNE-sample-data/MEG/sample/sample_audvis_raw-trans.fif");
    parser.addOption(sampleEvokedFileOption);
    parser.addOption(sampleCovFileOption);
    parser.addOption(sampleBemFileOption);
    parser.addOption(sampleTransFileOption);
    parser.process(app);

    //########################################################################################
    // Source Estimate
    QFile fileEvoked(parser.value(sampleEvokedFileOption));
    QFile fileCov(parser.value(sampleCovFileOption));
    QFile fileBem(parser.value(sampleBemFileOption));
    QFile fileTrans(parser.value(sampleTransFileOption));


    bool include_eeg = false;
    bool include_meg = true;


    // === Load data ===
    // Evoked
    std::cout << std::endl << "### Evoked ###" << std::endl;
    fiff_int_t setno = 0;
    QPair<QVariant, QVariant> baseline(QVariant(), 0);
    FiffEvoked evoked(fileEvoked, setno, baseline);
    if(evoked.isEmpty())
        return 1;

    // Cov
    std::cout << std::endl << "### Covariance ###" << std::endl;
    FiffCov noise_cov(fileCov);

    // BEM
    std::cout << std::endl << "### BEM ###" << std::endl;
    MNEBem bem(fileBem);
    if( bem.isEmpty() ) {
        return -1;
    }

    // Trans
    std::cout << std::endl << "### Transformation ###" << std::endl;
    FiffCoordTrans mri_head_t(fileTrans);
    if( mri_head_t.isEmpty() )
    {
        mri_head_t.from = FIFFV_COORD_HEAD;
        mri_head_t.to = FIFFV_COORD_MRI;
    }

    // === Dipole Fit ===

    //FIFFV_BEM_SURF_ID_BRAIN      1 -> Inner Skull
    //FIFFV_BEM_SURF_ID_SKULL      3 -> Outer Skull
    //FIFFV_BEM_SURF_ID_HEAD       4 -> Head
    qDebug() << "bem" << bem[0].id;

    Sphere sp = Sphere::fit_sphere( bem[0].rr );

    std::cout << "sp center" << std::endl << sp.center() << std::endl;
    std::cout << "sp radius" << std::endl << sp.radius() << std::endl;

    Sphere sp_simplex = Sphere::fit_sphere_simplex( bem[0].rr );

    std::cout << "sp simplex center" << std::endl << sp_simplex.center() << std::endl;
    std::cout << "sp simplex radius" << std::endl << sp_simplex.radius() << std::endl;

    //<< TODO: Apply transform function move to class
    std::cout << "rr.rows\n" << bem[0].rr.block(0,0,5,3) << std::endl;
    std::cout << "trans.trans\n" << mri_head_t.trans << std::endl;
    MatrixX3f rr_trans = mri_head_t.apply_trans(bem[0].rr);
    std::cout << "rr_trans.rows\n" << rr_trans.block(0,0,5,3) << std::endl;

    float guess_grid = 0.01;
    float guess_mindist = 0.005;//max(0.005, min_dist_to_inner_skull)
    float guess_exclude = 0.02;


    mri_head_t.print();
    evoked.info.dev_head_t.print();

    if(include_eeg) {

    }




    return app.exec();
}

//*************************************************************************************************************
//=============================================================================================================
// STATIC DEFINITIONS
//=============================================================================================================
