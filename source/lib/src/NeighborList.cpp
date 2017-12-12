#include "NeighborList.h"
#include <iostream>
// #include <iomanip> 

using namespace std;

bool 
is_loc (const vector<int> & idx, 
	const vector<int> & nat_stt,
	const vector<int> & nat_end)
{
  bool ret = true;
  for (int dd = 0; dd < 3; ++dd) ret = ret && idx[dd] >= nat_stt[dd];
  for (int dd = 0; dd < 3; ++dd) ret = ret && idx[dd] <  nat_end[dd];
  return ret;
}

int 
collapse_index (const vector<int> &	idx,
		const vector<int> &	size)
{
  return (idx[0] * size[1] + idx[1]) * size[2] + idx[2];
}

void
expand_index (vector<int > &		o_idx,
	      const int &		i_idx,
	      const vector<int> &	size)
{
  int tmp1 = i_idx / size[2];
  o_idx[2] = i_idx - tmp1 * size[2];
  o_idx[0] = tmp1 / size[1];
  o_idx[1] = tmp1 - o_idx[0] * size[1];
}

void 
build_clist (vector<vector<int > > &	clist,
	     const vector<double > &	coord,
	     const int &		nloc,
	     const vector<int > &	nat_stt,
	     const vector<int > &	nat_end,
	     const vector<int > &	ext_stt,
	     const vector<int > &	ext_end,
	     const SimulationRegion<double> & region,
	     const vector<int > &	global_grid)
{
  // compute region info, in terms of internal coord
  int nall = coord.size() / 3;
  vector<int> ext_ncell(3);
  for (int dd = 0; dd < 3; ++dd) ext_ncell[dd] = ext_end[dd] - ext_stt[dd];
  int ncell = ext_ncell[0] * ext_ncell[1] * ext_ncell[2];
  vector<double > cell_size (3);
  for (int dd = 0; dd < 3; ++dd) cell_size[dd] = 1./global_grid[dd];
  vector<double > nat_orig(3);
  for (int dd = 0; dd < 3; ++dd) nat_orig[dd] = nat_stt[dd] * cell_size[dd];
  vector<int> idx_orig_shift(3);
  for (int dd = 0; dd < 3; ++dd) idx_orig_shift[dd] = nat_stt[dd] - ext_stt[dd];
  
  // allocate the reserve the cell list
  clist.resize (ncell);
  int esti_natom_per_cell = nall / ncell + 10;
  for (unsigned ii = 0; ii < clist.size(); ++ii){
    clist[ii].clear();
    clist[ii].reserve (esti_natom_per_cell);
  }

  // build the cell list
  for (int ii = 0; ii < nloc; ++ii){
    double inter[3];
    region.phys2Inter (inter, &(coord[ii*3]));
    vector<int > idx(3);
    for (int dd = 0; dd < 3; ++dd){
      idx[dd] = (inter[dd] - nat_orig[dd]) / cell_size[dd];
      if (inter[dd] - nat_orig[dd] < 0.) idx[dd] --;
      if (idx[dd] < nat_stt[dd]) {
	cerr << "# warning: loc idx out of lower bound " << endl;
	idx[dd] = nat_stt[dd];
      }
      else if (idx[dd] >= nat_end[dd]) {
	cerr << "# warning: loc idx out of upper bound " << endl;
	idx[dd] = nat_end[dd] - 1;
      }
      idx[dd] += idx_orig_shift[dd];
    }
    clist[collapse_index(idx, ext_ncell)].push_back (ii);
  }
  for (int ii = nloc; ii < nall; ++ii){
    double inter[3];
    region.phys2Inter (inter, &(coord[ii*3]));
    vector<int > idx(3);
    for (int dd = 0; dd < 3; ++dd){
      idx[dd] = (inter[dd] - nat_orig[dd]) / cell_size[dd];
      if (inter[dd] - nat_orig[dd] < 0.) idx[dd] --;
      if (idx[dd] < ext_stt[dd]) {
	cerr << "# warning: ghost idx out of lower bound " << endl;
	idx[dd] = ext_stt[dd];
      }
      else if (idx[dd] >= ext_end[dd]) {
	cerr << "# warning: ghost idx out of upper bound " << endl;
	idx[dd] = ext_end[dd] - 1;
      }
      idx[dd] += idx_orig_shift[dd];
    }
    clist[collapse_index(idx, ext_ncell)].push_back (ii);
  }
}

void 
build_clist (vector<vector<int > > &	clist,
	     const vector<double > &	coord,
	     const vector<int>  &	sel,
	     const vector<int > &	nat_stt,
	     const vector<int > &	nat_end,
	     const SimulationRegion<double> & region)
{
  // compute region info, in terms of internal coord
  int nall = coord.size() / 3;
  vector<int> nat_ncell(3);
  for (int dd = 0; dd < 3; ++dd) nat_ncell[dd] = nat_end[dd] - nat_stt[dd];
  int ncell = nat_ncell[0] * nat_ncell[1] * nat_ncell[2];
  vector<double > cell_size (3);
  for (int dd = 0; dd < 3; ++dd) cell_size[dd] = 1./nat_end[dd];
  vector<double > nat_orig(3);
  for (int dd = 0; dd < 3; ++dd) nat_orig[dd] = nat_stt[dd] * cell_size[dd];
  
  // allocate the reserve the cell list
  clist.resize (ncell);
  int esti_natom_per_cell = nall / ncell + 10;
  for (unsigned ii = 0; ii < clist.size(); ++ii){
    clist[ii].clear();
    clist[ii].reserve (esti_natom_per_cell);
  }

  // build the cell list
  for (unsigned _ = 0; _ < sel.size(); ++_){
    int ii = sel[_];
    double inter[3];
    region.phys2Inter (inter, &(coord[ii*3]));
    vector<int > idx(3);
    for (int dd = 0; dd < 3; ++dd){
      idx[dd] = (inter[dd] - nat_orig[dd]) / cell_size[dd];
      if (inter[dd] - nat_orig[dd] < 0.) idx[dd] --;
      if (idx[dd] < nat_stt[dd]) {
	cerr << "# warning: loc idx out of lower bound " << endl;
	idx[dd] = nat_stt[dd];
      }
      else if (idx[dd] >= nat_end[dd]) {
	cerr << "# warning: loc idx out of upper bound " << endl;
	idx[dd] = nat_end[dd] - 1;
      }
    }
    clist[collapse_index(idx, nat_ncell)].push_back (ii);
  }
}


void
build_nlist_cell (vector<vector<int> > &	nlist0,
		  vector<vector<int> > &	nlist1,
		  const int &			cidx,
		  const int &			tidx, 
		  const vector<vector<int > > &	clist,
		  const vector<double > &	coord,
		  const double &		rc02,
		  const double &		rc12,
		  const vector<int> &		shift = {0, 0, 0},
		  const vector<double > &	boxt = {0., 0., 0., 0., 0., 0., 0., 0., 0.})
{
  int nloc = nlist0.size();
  // loop over c (current) cell
  for (unsigned ii = 0; ii < clist[cidx].size(); ++ii){
    int i_idx = clist[cidx][ii];
    assert (i_idx < nloc);
    // loop over t (target) cell
    for (unsigned jj = 0; jj < clist[tidx].size(); ++jj){
      int j_idx = clist[tidx][jj];
      if (cidx == tidx && j_idx <= i_idx) continue;
      double diff[3];
      for (int dd0 = 0; dd0 < 3; ++dd0) {
	diff[dd0] = coord[i_idx*3 + dd0] - coord[j_idx*3 + dd0];
	for (int dd1 = 0; dd1 < 3; ++dd1) {
	  diff[dd0] += shift[dd1] * boxt[3*dd1+dd0];
	}
      }
      double r2 = MathUtilities::dot<double> (diff, diff);
      if (r2 < rc02) {
	nlist0[i_idx].push_back (j_idx);
	if (j_idx < nloc) nlist0[j_idx].push_back (i_idx);
      }
      else if (r2 < rc12) {
	nlist1[i_idx].push_back (j_idx);
	if (j_idx < nloc) nlist1[j_idx].push_back (i_idx);
      }      
    }
  }
}

void
build_nlist_cell (vector<vector<int> > &	nlist0,
		  vector<vector<int> > &	nlist1,
		  const int &			cidx,
		  const int &			tidx, 
		  const vector<vector<int > > &	clist0,
		  const vector<vector<int > > &	clist1,
		  const vector<double > &	coord,
		  const double &		rc02,
		  const double &		rc12,
		  const vector<int> &		shift = {0, 0, 0},
		  const vector<double > &	boxt = {0., 0., 0., 0., 0., 0., 0., 0., 0.})
{
  // loop over c (current) cell
  for (unsigned ii = 0; ii < clist0[cidx].size(); ++ii){
    int i_idx = clist0[cidx][ii];
    // loop over t (target) cell
    for (unsigned jj = 0; jj < clist1[tidx].size(); ++jj){
      int j_idx = clist1[tidx][jj];
      if (cidx == tidx && j_idx == i_idx) continue;
      double diff[3];
      for (int dd0 = 0; dd0 < 3; ++dd0) {
	diff[dd0] = coord[i_idx*3 + dd0] - coord[j_idx*3 + dd0];
	for (int dd1 = 0; dd1 < 3; ++dd1) {
	  diff[dd0] += shift[dd1] * boxt[3*dd1+dd0];
	}
      }
      double r2 = MathUtilities::dot<double> (diff, diff);
      if (r2 < rc02) {
	nlist0[i_idx].push_back (j_idx);
      }
      else if (r2 < rc12) {
	nlist1[i_idx].push_back (j_idx);
      }      
    }
  }
}

void
build_nlist (vector<vector<int > > &	nlist0,
	     vector<vector<int > > &	nlist1,
	     const vector<double > &	coord,
	     const int &		nloc,
	     const double &		rc0,
	     const double &		rc1,
	     const vector<int > &	nat_stt_,
	     const vector<int > &	nat_end_,
	     const vector<int > &	ext_stt_,
	     const vector<int > &	ext_end_,
	     const SimulationRegion<double> & region,
	     const vector<int > &	global_grid)
{
  // normalize the index
  // i require that the ext_stt = {0, 0, 0}
  vector<int > nat_stt (nat_stt_);
  vector<int > nat_end (nat_end_);
  vector<int > ext_stt (ext_stt_);
  vector<int > ext_end (ext_end_);
  
  // compute the clist
  vector<vector<int > > clist;
  build_clist (clist, coord, nloc, nat_stt, nat_end, ext_stt, ext_end, region, global_grid);

  // compute the region info
  int nall = coord.size() / 3;
  vector<int> ext_ncell(3);
  for (int dd = 0; dd < 3; ++dd) ext_ncell[dd] = ext_end[dd] - ext_stt[dd];

  // compute number of iter according to the cut-off
  assert (rc0 <= rc1);
  vector<int> niter (3);
  double to_face [3];
  region.toFaceDistance (to_face);
  for (int dd = 0; dd < 3; ++dd){
    double cell_size = to_face[dd] / nat_end[dd];
    niter[dd] = rc1 / cell_size;
    if (niter[dd] * cell_size < rc1) niter[dd] += 1;
    assert (niter[dd] * cell_size >= rc1);
  }
  // check the validity of the iters
  for (int dd = 0; dd < 3; ++dd){
    assert (nat_stt[dd] - niter[dd] >= ext_stt[dd]);
    assert (nat_end[dd] + niter[dd] <= ext_end[dd]);
  }

  // allocate the nlists
  double density = nall / region.getVolume();
  nlist0.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist0[ii].clear();
    nlist0[ii].reserve ( 4./3. * 3.14 * (rc0*rc0*rc0) * density * 1.5 + 20);
  }  
  nlist1.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist1[ii].clear();
    nlist1[ii].reserve ( 4./3. * 3.14 * (rc1*rc1*rc1 - rc0*rc0*rc0) * density * 1.5 + 20);
  }

  // shift of the idx origin
  vector<int> idx_orig_shift(3);
  for (int dd = 0; dd < 3; ++dd) idx_orig_shift[dd] = nat_stt[dd] - ext_stt[dd];

  // compute the nlists
  double rc02 = 0;
  if (rc0 > 0) rc02 = rc0 * rc0;
  double rc12 = rc1 * rc1;
  vector<int> cidx(3);
  for (cidx[0] = nat_stt[0]; cidx[0] < nat_end[0]; ++cidx[0]){
    for (cidx[1] = nat_stt[1]; cidx[1] < nat_end[1]; ++cidx[1]){
      for (cidx[2] = nat_stt[2]; cidx[2] < nat_end[2]; ++cidx[2]){
	vector<int> mcidx(3);
	for (int dd = 0; dd < 3; ++dd) mcidx[dd] = cidx[dd] + idx_orig_shift[dd];
	int clp_cidx = collapse_index (mcidx, ext_ncell);
	vector<int> tidx(3);
	for (tidx[0] = cidx[0] - niter[0]; tidx[0] < cidx[0] + niter[0] + 1; ++tidx[0]) {
	  for (tidx[1] = cidx[1] - niter[1]; tidx[1] < cidx[1] + niter[1] + 1; ++tidx[1]) {
	    for (tidx[2] = cidx[2] - niter[2]; tidx[2] < cidx[2] + niter[2] + 1; ++tidx[2]) {
	      vector<int> mtidx(3);
	      for (int dd = 0; dd < 3; ++dd) mtidx[dd] = tidx[dd] + idx_orig_shift[dd];
	      int clp_tidx = collapse_index (mtidx, ext_ncell);
	      if (is_loc(tidx, nat_stt, nat_end) && clp_tidx < clp_cidx) continue;
	      build_nlist_cell (nlist0, nlist1, clp_cidx, clp_tidx, clist, coord, rc02, rc12);
	    }
	  }
	}
      }
    }
  }
}


// assume nat grid is the global grid. only used for serial simulations
void
build_nlist (vector<vector<int > > &	nlist0,
	     vector<vector<int > > &	nlist1,
	     const vector<double > &	coord,
	     const double &		rc0,
	     const double &		rc1,
	     const vector<int > &	grid,
	     const SimulationRegion<double> & region)
{
  int nloc = coord.size() / 3;
  // compute the clist
  vector<int> nat_stt(3, 0);
  vector<int> nat_end(grid);
  vector<vector<int > > clist;
  build_clist (clist, coord, nloc, nat_stt, nat_end, nat_stt, nat_end, region, nat_end);
  
  // compute the region info
  int nall = coord.size() / 3;
  vector<int> nat_ncell(3);
  for (int dd = 0; dd < 3; ++dd) nat_ncell[dd] = nat_end[dd] - nat_stt[dd];

  // compute number of iter according to the cut-off
  assert (rc0 <= rc1);
  vector<int> niter (3);
  double to_face [3];
  region.toFaceDistance (to_face);
  for (int dd = 0; dd < 3; ++dd){
    double cell_size = to_face[dd] / nat_end[dd];
    niter[dd] = rc1 / cell_size;
    if (niter[dd] * cell_size < rc1) niter[dd] += 1;
    assert (niter[dd] * cell_size >= rc1);
  }
  // check the validity of the iters
  for (int dd = 0; dd < 3; ++dd){
    assert (niter[dd] <= (nat_end[dd] - nat_stt[dd]) / 2);
  }

  // allocate the nlists
  double density = nall / region.getVolume();
  nlist0.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist0[ii].clear();
    nlist0[ii].reserve ( 4./3. * 3.14 * (rc0*rc0*rc0) * density * 1.5 + 20);
  }  
  nlist1.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist1[ii].clear();
    nlist1[ii].reserve ( 4./3. * 3.14 * (rc1*rc1*rc1 - rc0*rc0*rc0) * density * 1.5 + 20);
  }
  
  // physical cell size
  vector<double> phys_cs(9);
  for (int dd = 0; dd < 9; ++dd) phys_cs[dd] = region.getBoxTensor()[dd];

  // compute the nlists
  double rc02 = 0;
  if (rc0 > 0) rc02 = rc0 * rc0;
  double rc12 = rc1 * rc1;
  vector<int> cidx(3);
  for (cidx[0] = nat_stt[0]; cidx[0] < nat_end[0]; ++cidx[0]){
    for (cidx[1] = nat_stt[1]; cidx[1] < nat_end[1]; ++cidx[1]){
      for (cidx[2] = nat_stt[2]; cidx[2] < nat_end[2]; ++cidx[2]){
	int clp_cidx = collapse_index (cidx, nat_ncell);
	vector<int> tidx(3);
	vector<int> stidx(3);
	vector<int> shift(3);
	for (tidx[0] = cidx[0] - niter[0]; tidx[0] < cidx[0] + niter[0] + 1; ++tidx[0]) {
	  shift[0] = 0;
	  if      (tidx[0] < 0)			shift[0] += 1;
	  else if (tidx[0] >= nat_ncell[0])	shift[0] -= 1;
	  stidx[0] = tidx[0] + shift[0] * nat_ncell[0];
	  for (tidx[1] = cidx[1] - niter[1]; tidx[1] < cidx[1] + niter[1] + 1; ++tidx[1]) {
	    shift[1] = 0;
	    if      (tidx[1] < 0)		shift[1] += 1;
	    else if (tidx[1] >= nat_ncell[1])	shift[1] -= 1;
	    stidx[1] = tidx[1] + shift[1] * nat_ncell[1];
	    for (tidx[2] = cidx[2] - niter[2]; tidx[2] < cidx[2] + niter[2] + 1; ++tidx[2]) {
	      shift[2] = 0;
	      if      (tidx[2] < 0)		shift[2] += 1;
	      else if (tidx[2] >= nat_ncell[2])	shift[2] -= 1;
	      stidx[2] = tidx[2] + shift[2] * nat_ncell[2];
	      int clp_tidx = collapse_index (stidx, nat_ncell);
	      if (clp_tidx < clp_cidx) continue;
	      build_nlist_cell (nlist0, nlist1, clp_cidx, clp_tidx, clist, coord, rc02, rc12, shift, phys_cs);
	    }
	  }
	}
      }
    }
  }
}


void
build_nlist (vector<vector<int > > &	nlist0,
	     vector<vector<int > > &	nlist1,
	     const vector<double > &	coord,
	     const vector<int> &	sel0,
	     const vector<int> &	sel1,
	     const double &		rc0,
	     const double &		rc1,
	     const vector<int > &	grid,
	     const SimulationRegion<double> & region)
{
  int nloc = coord.size() / 3;
  // compute the clist
  vector<int> nat_stt(3, 0);
  vector<int> nat_end(grid);
  vector<vector<int > > clist0, clist1;
  build_clist (clist0, coord, sel0, nat_stt, nat_end, region);
  build_clist (clist1, coord, sel1, nat_stt, nat_end, region);
  
  // compute the region info
  int nall = coord.size() / 3;
  vector<int> nat_ncell(3);
  for (int dd = 0; dd < 3; ++dd) nat_ncell[dd] = nat_end[dd] - nat_stt[dd];

  // compute number of iter according to the cut-off
  assert (rc0 <= rc1);
  vector<int> niter (3);
  double to_face [3];
  region.toFaceDistance (to_face);
  for (int dd = 0; dd < 3; ++dd){
    double cell_size = to_face[dd] / nat_end[dd];
    niter[dd] = rc1 / cell_size;
    if (niter[dd] * cell_size < rc1) niter[dd] += 1;
    assert (niter[dd] * cell_size >= rc1);
  }
  // check the validity of the iters
  for (int dd = 0; dd < 3; ++dd){
    assert (niter[dd] <= (nat_end[dd] - nat_stt[dd]) / 2);
  }

  // allocate the nlists
  double density = nall / region.getVolume();
  nlist0.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist0[ii].clear();
    nlist0[ii].reserve ( 4./3. * 3.14 * (rc0*rc0*rc0) * density * 1.5 + 20);
  }  
  nlist1.resize (nloc);
  for (int ii = 0; ii < nloc; ++ii){
    nlist1[ii].clear();
    nlist1[ii].reserve ( 4./3. * 3.14 * (rc1*rc1*rc1 - rc0*rc0*rc0) * density * 1.5 + 20);
  }
  
  // physical cell size
  vector<double> phys_cs(9);
  for (int dd = 0; dd < 9; ++dd) phys_cs[dd] = region.getBoxTensor()[dd];

  // compute the nlists
  double rc02 = 0;
  if (rc0 > 0) rc02 = rc0 * rc0;
  double rc12 = rc1 * rc1;
  vector<int> cidx(3);
  for (cidx[0] = nat_stt[0]; cidx[0] < nat_end[0]; ++cidx[0]){
    for (cidx[1] = nat_stt[1]; cidx[1] < nat_end[1]; ++cidx[1]){
      for (cidx[2] = nat_stt[2]; cidx[2] < nat_end[2]; ++cidx[2]){
	int clp_cidx = collapse_index (cidx, nat_ncell);
	vector<int> tidx(3);
	vector<int> stidx(3);
	vector<int> shift(3);
	for (tidx[0] = cidx[0] - niter[0]; tidx[0] < cidx[0] + niter[0] + 1; ++tidx[0]) {
	  shift[0] = 0;
	  if      (tidx[0] < 0)			shift[0] += 1;
	  else if (tidx[0] >= nat_ncell[0])	shift[0] -= 1;
	  stidx[0] = tidx[0] + shift[0] * nat_ncell[0];
	  for (tidx[1] = cidx[1] - niter[1]; tidx[1] < cidx[1] + niter[1] + 1; ++tidx[1]) {
	    shift[1] = 0;
	    if      (tidx[1] < 0)		shift[1] += 1;
	    else if (tidx[1] >= nat_ncell[1])	shift[1] -= 1;
	    stidx[1] = tidx[1] + shift[1] * nat_ncell[1];
	    for (tidx[2] = cidx[2] - niter[2]; tidx[2] < cidx[2] + niter[2] + 1; ++tidx[2]) {
	      shift[2] = 0;
	      if      (tidx[2] < 0)		shift[2] += 1;
	      else if (tidx[2] >= nat_ncell[2])	shift[2] -= 1;
	      stidx[2] = tidx[2] + shift[2] * nat_ncell[2];
	      int clp_tidx = collapse_index (stidx, nat_ncell);
	      build_nlist_cell (nlist0, nlist1, clp_cidx, clp_tidx, clist0, clist1, coord, rc02, rc12, shift, phys_cs);
	    }
	  }
	}
      }
    }
  }
}

   

