/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Inria - IMB (Université de Bordeaux) - LJLL (UPMC), 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \file mmg3d/intmet.c
 * \brief Metric interpolations.
 * \author Charles Dapogny (LJLL, UPMC)
 * \author Cécile Dobrzynski (Inria / IMB, Université de Bordeaux)
 * \author Pascal Frey (LJLL, UPMC)
 * \author Algiane Froehly (Inria / IMB, Université de Bordeaux)
 * \version 5
 * \copyright GNU Lesser General Public License.
 * \todo doxygen documentation.
 */

#include "mmg3d.h"

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k element index.
 * \param i local index of edge in \a k.
 * \param ip global index of the new point in which we want to compute the metric.
 * \param s interpolation parameter (between 0 and 1).
 * \return 0 if fail, 1 otherwise.
 *
 * Interpolation of anisotropic sizemap at parameter \a s along edge \a i of elt
 * \a k for a special storage of ridges metric (after defsiz call).
 *
 */
int _MMG5_intmet_ani(MMG5_pMesh mesh,MMG5_pSol met,int k,char i,int ip,
                      double s) {
  MMG5_pTetra   pt;
  MMG5_pxTetra  pxt;
  MMG5_pPoint   ppt;
  MMG5_pxPoint  pxp;
  double        *m;
  int           ip1,ip2;

  pt = &mesh->tetra[k];
  m  = &met->m[6*ip];
  ip1 = pt->v[_MMG5_iare[i][0]];
  ip2 = pt->v[_MMG5_iare[i][1]];

  if ( pt->xt ) {
    pxt = &mesh->xtetra[pt->xt];
    if ( pxt->tag[i] & MG_GEO ) {
      ppt = &mesh->point[ip];
      assert(ppt->xp);
      pxp = &mesh->xpoint[ppt->xp];
      return(_MMG5_intridmet(mesh,met,ip1,ip2,s,pxp->n1,m));
    }
    else if ( pxt->tag[i] & MG_BDY ) {
     return(_MMG5_intregmet(mesh,met,k,i,s,m));
    }
    else {
      /* The edge is an internal edge. */
      return(_MMG5_intvolmet(mesh,met,k,i,s,m));
    }
  }
  else {
    /* The edge is an internal edge. */
    return(_MMG5_intvolmet(mesh,met,k,i,s,m));
  }
  return(0);
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k element index.
 * \param i local index of edge in \a k.
 * \param ip global index of the new point in which we want to compute the metric.
 * \param s interpolation parameter (between 0 and 1).
 * \return 0 if fail, 1 otherwise.
 *
 * Interpolation of anisotropic sizemap at parameter \a s along edge \a i of elt
 * \a k for a classic storage of ridges metrics (before defsiz call).
 *
 */
int _MMG5_intmet33_ani(MMG5_pMesh mesh,MMG5_pSol met,int k,char i,int ip,
                      double s) {
  MMG5_pTetra   pt;
  MMG5_pxTetra  pxt;
  MMG5_pPoint   ppt;
  MMG5_pxPoint  pxp;
  double        *m,*n,*mr;
  int           ip1,ip2;

  pt = &mesh->tetra[k];
  ip1 = pt->v[_MMG5_iare[i][0]];
  ip2 = pt->v[_MMG5_iare[i][1]];

  m   = &met->m[6*ip1];
  n   = &met->m[6*ip2];
  mr  = &met->m[6*ip];

  return(_MMG5_mmgIntmet33_ani(m,n,mr,s));
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k element index.
 * \param i local index of edge in \a k.
 * \param ip global index of the new point in which we want to compute the metric.
 * \param s interpolation parameter (between 0 and 1).
 * \return 0 if fail, 1 otherwise.
 *
 * Interpolation of anisotropic sizemap at parameter \a s along edge \a i of elt
 * \a k.
 *
 */
int _MMG5_intmet_iso(MMG5_pMesh mesh,MMG5_pSol met,int k,char i,int ip,
                      double s) {
  MMG5_pTetra   pt;
  int           ip1, ip2;
  double        *m1,*m2,*mm;

  pt = &mesh->tetra[k];
  ip1 = pt->v[_MMG5_iare[i][0]];
  ip2 = pt->v[_MMG5_iare[i][1]];

  m1 = &met->m[met->size*ip1];
  m2 = &met->m[met->size*ip2];
  mm = &met->m[met->size*ip];

  return(_MMG5_interp_iso(m1,m2,mm,s));
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k element index.
 * \param i local index of edge in \a k.
 * \param s interpolation parameter.
 * \param mr computed metric.
 * \return  0 if fail, 1 otherwise.
 *
 * Metric interpolation on edge \a i in elt \a it at
 * parameter \f$ 0 <= s0 <= 1 \f$ from \a p1 result is stored in \a mr. edge
 * \f$ p_1-p_2 \f$ must not be a ridge.
 *
 * */
int _MMG5_intregmet(MMG5_pMesh mesh,MMG5_pSol met,int k,char i,double s,
                    double mr[6]) {
  MMG5_pTetra     pt;
  MMG5_pxTetra    pxt;
  MMG5_Tria       ptt;
  int             ifa0, ifa1, iloc;

  pt   = &mesh->tetra[k];
  pxt  = &mesh->xtetra[pt->xt];
  ifa0 = _MMG5_ifar[i][0];
  ifa1 = _MMG5_ifar[i][1];

  if ( pxt->ftag[ifa0] & MG_BDY ) {
    _MMG5_tet2tri( mesh,k,ifa0,&ptt);
    iloc = _MMG5_iarfinv[ifa0][i];
    assert(iloc >= 0);
    return(_MMG5_interpreg_ani(mesh,met,&ptt,iloc,s,mr));
  }
  else if ( pxt->ftag[ifa1] & MG_BDY ) {
    _MMG5_tet2tri( mesh,k,ifa1,&ptt);
    iloc = _MMG5_iarfinv[ifa1][i];
    assert(iloc >= 0);
    return(_MMG5_interpreg_ani(mesh,met,&ptt,iloc,s,mr));
  }

  /* if we pass here, then i is a boundary edge but the tet has no bdy
   * face. Don't do anything, the edge will be split via a boundary tetra. */
  return(-1);
}



 
/**
 * \param ma pointer on a metric
 * \param mb pointer on a metric
 * \param mp pointer on the computed interpolated metric
 * \param t interpolation parameter (comprise between 0 and 1)
 * \return 1 if success, 0 if fail.
 *
 * Linear interpolation of anisotropic sizemap along an internal edge.
 *
 */
static inline int
_MMG5_intregvolmet(double *ma,double *mb,double *mp,double t) {
  double        dma[6],dmb[6],mai[6],mbi[6],mi[6];
  int           i;

  for (i=0; i<6; i++) {
    dma[i] = ma[i];
    dmb[i] = mb[i];
  }
  if ( !_MMG5_invmat(dma,mai) || !_MMG5_invmat(dmb,mbi) ) {
    fprintf(stderr,"  ## INTERP INVALID METRIC.\n");
    return(0);
  }
  for (i=0; i<6; i++)
    mi[i] = (1.0-t)*mai[i] + t*mbi[i];

  if ( !_MMG5_invmat(mi,mai) ) {
    fprintf(stderr,"  ## INTERP INVALID METRIC.\n");
    return(0);
  }

  for (i=0; i<6; i++)  mp[i] = mai[i];
  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k element index.
 * \param i local index of edge in \a k.
 * \param s interpolation parameter.
 * \param mr computed metric.
 * \return  0 if fail, 1 otherwise.
 *
 * Metric interpolation on edge \a i in elt \a it at
 * parameter \f$ 0 <= s0 <= 1 \f$ from \a p1 result is stored in \a mr. edge
 * \f$ p_1-p_2 \f$ is an internal edge.
 *
 * */
int _MMG5_intvolmet(MMG5_pMesh mesh,MMG5_pSol met,int k,char i,double s,
                    double mr[6]) {
  MMG5_pTetra     pt;
  MMG5_pPoint     pp1,pp2;
  double          *m1,*m2;
  int             ip1,ip2;

  pt  = &mesh->tetra[k];
  
  ip1 = pt->v[_MMG5_iare[i][0]];
  ip2 = pt->v[_MMG5_iare[i][1]];
  
  pp1 = &mesh->point[ip1];
  pp2 = &mesh->point[ip2];
  
  // build metric at ma and mb points (Warn for ridge points) mp points and
  if(MG_SIN(pp1->tag) || (MG_NOM & pp1->tag))
    m1 = &met->m[6*ip1];
  else if(pp1->tag & MG_GEO) {
    m1 = (double*)malloc(6*sizeof(double));
    if (!_MMG5_moymet(mesh,met,pt,m1)) return(0);
  } else {
    m1 = &met->m[6*ip1]; 
    //printf("\n\nm1 %e %e %e %e %e %e\n",m1[0],m1[1],m1[2],m1[3],m1[4],m1[5]);
  }
  if(MG_SIN(pp2->tag)|| (MG_NOM & pp2->tag))
    m2 = &met->m[6*ip2];
  else if(pp2->tag & MG_GEO) {
    m2 = (double*)malloc(6*sizeof(double));
    if (!_MMG5_moymet(mesh,met,pt,m2)) return(0);
  } else {
    m2 = &met->m[6*ip2];
    // printf("m2 %e %e %e %e %e %e\n",m2[0],m2[1],m2[2],m2[3],m2[4],m2[5]);
  }

  _MMG5_intregvolmet(m1,m2,mr,s);
  if(fabs(mr[5]) < 1e-6) {
    printf("%s:%d : Error\n",__FILE__,__LINE__);
    printf("pp1 : %d %d \n",MG_SIN(pp1->tag) || (MG_NOM & pp1->tag),pp1->tag & MG_GEO);
printf("m1 %e %e %e %e %e %e\n",m1[0],m1[1],m1[2],m1[3],m1[4],m1[5]);
    printf("pp2 : %d %d \n",MG_SIN(pp2->tag) || (MG_NOM & pp2->tag),pp2->tag & MG_GEO);
 printf("m2 %e %e %e %e %e %e\n",m2[0],m2[1],m2[2],m2[3],m2[4],m2[5]);
    printf("mr %e %e %e %e %e %e\n",mr[0],mr[1],mr[2],mr[3],mr[4],mr[5]);
    exit(0);
  }
 

  return(1);
}
/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k index of the tetra.
 * \param ip index of the point on which we compute the metric.
 * \param cb barycentric coordinates of \a ip in \a k.
 * \return 1.
 *
 * Linear interpolation of isotropic sizemap in a tetra given the barycentric
 * coordinates of the new point in \a k.
 *
 */
int _MMG5_interp4bar_iso(MMG5_pMesh mesh, MMG5_pSol met, int k, int ip,
                         double cb[4]) {
  MMG5_pTetra pt;

  pt = &mesh->tetra[k];

  met->m[ip] = cb[0]*met->m[pt->v[0]]+cb[1]*met->m[pt->v[1]] +
    cb[2]*met->m[pt->v[2]]+cb[3]*met->m[pt->v[3]];

  return(1);

}

/**
 * \param met pointer toward the metric structure.
 * \param ip index of the point on which we compute the metric.
 * \param cb barycentric coordinates of \a ip in the tetra.
 * \param dm0 metric of the first vertex of the tet.
 * \param dm1 metric of the second vertex of the tet.
 * \param dm2 metric of the third vertex of the tet.
 * \param dm3 metric of the fourth vertex of the tet.
 * \return 1 if success, 0 if fail.
 *
 * Linear interpolation of anisotropic sizemap in a tetra given the barycentric
 * coordinates of the new point in a tetra.
 *
 */
static inline
int _MMG5_interp4barintern(MMG5_pSol met,int ip,double cb[4],double dm0[6],
                           double dm1[6],double dm2[6],double dm3[6]) {
  double        m0i[6],m1i[6],m2i[6],m3i[6],mi[6];
  int           i;

 if ( !_MMG5_invmat(dm0,m0i) || !_MMG5_invmat(dm1,m1i) ||
       !_MMG5_invmat(dm2,m2i) || !_MMG5_invmat(dm3,m3i) ) {
    fprintf(stderr,"  ## INTERP INVALID METRIC.\n");
    return(0);
  }
  for (i=0; i<6; i++)
    mi[i] = cb[0]*m0i[i] + cb[1]*m1i[i] + cb[2]*m2i[i] + cb[3]*m3i[i];

  if ( !_MMG5_invmat(mi,m0i) ) {
    fprintf(stderr,"  ## INTERP INVALID METRIC.\n");
    return(0);
  }

  for (i=0; i<6; i++)  met->m[met->size*ip+i] = m0i[i];

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k index of the tetra.
 * \param ip index of the point on which we compute the metric.
 * \param cb barycentric coordinates of \a ip in \a k.
 * \return 1 if success, 0 if fail.
 *
 * Linear interpolation of anisotropic sizemap in a tetra given the barycentric
 * coordinates of the new point in \a k.
 *
 */
int _MMG5_interp4bar_ani(MMG5_pMesh mesh, MMG5_pSol met, int k, int ip,
                         double cb[4]) {
  MMG5_pTetra   pt;
  MMG5_pPoint   pp1,pp2,pp3,pp4;
  double        dm0[6],dm1[6],dm2[6],dm3[6];
  int           i;

  pt  = &mesh->tetra[k];
  pp1 = &mesh->point[pt->v[0]];
  if(MG_SIN(pp1->tag) || (MG_NOM & pp1->tag)) {
    for (i=0; i<6; i++) {
      dm0[i] = met->m[met->size*pt->v[0]+i];
    }
  } else if(pp1->tag & MG_GEO) {
    if (!_MMG5_moymet(mesh,met,pt,&dm0[0])) return(0);
  } else{
    for (i=0; i<6; i++) {
      dm0[i] = met->m[met->size*pt->v[0]+i];
    }
  }
  pp2 = &mesh->point[pt->v[1]];
  if(MG_SIN(pp2->tag) || (MG_NOM & pp2->tag)) {
    for (i=0; i<6; i++) {
      dm1[i] = met->m[met->size*pt->v[1]+i];
    }
  } else if(pp2->tag & MG_GEO) {
    if (!_MMG5_moymet(mesh,met,pt,&dm1[0])) return(0);
  } else{
    for (i=0; i<6; i++) {
      dm1[i] = met->m[met->size*pt->v[1]+i];
    }
  }
  pp3 = &mesh->point[pt->v[2]];
  if(MG_SIN(pp3->tag) || (MG_NOM & pp3->tag)) {
    for (i=0; i<6; i++) {
      dm2[i] = met->m[met->size*pt->v[2]+i];
    }
  } else if(pp3->tag & MG_GEO) {
    if (!_MMG5_moymet(mesh,met,pt,&dm2[0])) return(0);
  } else{
    for (i=0; i<6; i++) {
      dm2[i] = met->m[met->size*pt->v[2]+i];
    }
  }
  pp4 = &mesh->point[pt->v[3]];
  if(MG_SIN(pp4->tag) || (MG_NOM & pp4->tag)) {
    for (i=0; i<6; i++) {
      dm3[i] = met->m[met->size*pt->v[3]+i];
    }
  } else if(pp4->tag & MG_GEO) {
    if (!_MMG5_moymet(mesh,met,pt,&dm3[0])) return(0);
  } else{
    for (i=0; i<6; i++) {
      dm3[i] = met->m[met->size*pt->v[3]+i];
    }
  }

  return(_MMG5_interp4barintern(met,ip,cb,dm0,dm1,dm2,dm3));
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the metric structure.
 * \param k index of the tetra.
 * \param ip index of the point on which we compute the metric.
 * \param cb barycentric coordinates of \a ip in \a k.
 * \return 1 if success, 0 if fail.
 *
 * Linear interpolation of anisotropic sizemap in a tetra given the barycentric
 * coordinates of the new point in \a k.
 *
 */
int _MMG5_interp4bar33_ani(MMG5_pMesh mesh, MMG5_pSol met, int k, int ip,
                           double cb[4]) {
  MMG5_pTetra   pt;
  MMG5_pPoint   pp1,pp2,pp3,pp4;
  double        dm0[6],dm1[6],dm2[6],dm3[6];
  int           i;

  pt  = &mesh->tetra[k];
  pp1 = &mesh->point[pt->v[0]];
  for (i=0; i<6; i++) {
    dm0[i] = met->m[met->size*pt->v[0]+i];
  }

  pp2 = &mesh->point[pt->v[1]];
  for (i=0; i<6; i++) {
    dm1[i] = met->m[met->size*pt->v[1]+i];
  }

  pp3 = &mesh->point[pt->v[2]];
  for (i=0; i<6; i++) {
    dm2[i] = met->m[met->size*pt->v[2]+i];
  }

  pp4 = &mesh->point[pt->v[3]];
  for (i=0; i<6; i++) {
    dm3[i] = met->m[met->size*pt->v[3]+i];
  }
  return(_MMG5_interp4barintern(met,ip,cb,dm0,dm1,dm2,dm3));
}
