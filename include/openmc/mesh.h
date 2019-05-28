//! \file mesh.h
//! \brief Mesh types used for tallies, Shannon entropy, CMFD, etc.

#ifndef OPENMC_MESH_H
#define OPENMC_MESH_H

#include <memory> // for unique_ptr
#include <vector>
#include <unordered_map>

#include "hdf5.h"
#include "pugixml.hpp"
#include "xtensor/xarray.hpp"

#include "openmc/particle.h"
#include "openmc/position.h"

namespace openmc {

//==============================================================================
// Global variables
//==============================================================================

class Mesh;

namespace model {

extern std::vector<std::unique_ptr<Mesh>> meshes;
extern std::unordered_map<int32_t, int32_t> mesh_map;

} // namespace model

class Mesh
{
public:
  // Constructors
  Mesh() = default;
  Mesh(pugi::xml_node node);

  // Methods

  //! Determine which bins were crossed by a particle
  //
  //! \param[in] p Particle to check
  //! \param[out] bins Bins that were crossed
  //! \param[out] lengths Fraction of tracklength in each bin
  virtual void bins_crossed(const Particle* p, std::vector<int>& bins,
                            std::vector<double>& lengths) const = 0;

  //! Determine which surface bins were crossed by a particle
  //
  //! \param[in] p Particle to check
  //! \param[out] bins Surface bins that were crossed
  virtual void
  surface_bins_crossed(const Particle* p, std::vector<int>& bins) const = 0;

  //! Get bin at a given position in space
  //
  //! \param[in] r Position to get bin for
  //! \return Mesh bin
  virtual int get_bin(Position r) const = 0;

  //! Get bin given mesh indices
  //
  //! \param[in] Array of mesh indices
  //! \return Mesh bin
  virtual int get_bin_from_indices(const int* ijk) const = 0;

  //! Get mesh indices given a position
  //
  //! \param[in] r Position to get indices for
  //! \param[out] ijk Array of mesh indices
  //! \param[out] in_mesh Whether position is in mesh
  virtual void get_indices(Position r, int* ijk, bool* in_mesh) const = 0;

  //! Get mesh indices corresponding to a mesh bin
  //
  //! \param[in] bin Mesh bin
  //! \param[out] ijk Mesh indices
  virtual void get_indices_from_bin(int bin, int* ijk) const = 0;

  //! Write mesh data to an HDF5 group
  //
  //! \param[in] group HDF5 group
  virtual void to_hdf5(hid_t group) const = 0;

  // Data members

  int id_ {-1};  //!< User-specified ID
};

//==============================================================================
//! Tessellation of n-dimensional Euclidean space by congruent squares or cubes
//==============================================================================

class RegularMesh : public Mesh
{
public:
  // Constructors
  RegularMesh() = default;
  RegularMesh(pugi::xml_node node);

  // Overriden methods

  void bins_crossed(const Particle* p, std::vector<int>& bins,
                    std::vector<double>& lengths) const override;

  void surface_bins_crossed(const Particle* p, std::vector<int>& bins)
  const override;

  int get_bin(Position r) const override;

  int get_bin_from_indices(const int* ijk) const override;

  void get_indices(Position r, int* ijk, bool* in_mesh) const override;

  void get_indices_from_bin(int bin, int* ijk) const override;

  void to_hdf5(hid_t group) const override;

  // New methods

  //! Check where a line segment intersects the mesh and if it intersects at all
  //
  //! \param[in,out] r0 In: starting position, out: intersection point
  //! \param[in] r1 Ending position
  //! \param[out] ijk Indices of the mesh bin containing the intersection point
  //! \return Whether the line segment connecting r0 and r1 intersects mesh
  bool intersects(Position& r0, Position r1, int* ijk) const;

  //! Count number of bank sites in each mesh bin / energy bin
  //
  //! \param[in] bank Array of bank sites
  //! \param[out] Whether any bank sites are outside the mesh
  //! \return Array indicating number of sites in each mesh/energy bin
  virtual xt::xarray<double>
  count_sites(const std::vector<Particle::Bank>& bank, bool* outside) const;

  // Data members

  double volume_frac_; //!< Volume fraction of each mesh element
  int n_dimension_; //!< Number of dimensions
  xt::xarray<int> shape_; //!< Number of mesh elements in each dimension
  xt::xarray<double> lower_left_; //!< Lower-left coordinates of mesh
  xt::xarray<double> upper_right_; //!< Upper-right coordinates of mesh
  xt::xarray<double> width_; //!< Width of each mesh element

private:
  bool intersects_1d(Position& r0, Position r1, int* ijk) const;
  bool intersects_2d(Position& r0, Position r1, int* ijk) const;
  bool intersects_3d(Position& r0, Position r1, int* ijk) const;
};

class RectilinearMesh : public Mesh
{
public:
  // Constructors
  RectilinearMesh(pugi::xml_node node);

  // Overriden methods

  void bins_crossed(const Particle* p, std::vector<int>& bins,
                    std::vector<double>& lengths) const override;

  void surface_bins_crossed(const Particle* p, std::vector<int>& bins)
  const override;

  int get_bin(Position r) const override;

  int get_bin_from_indices(const int* ijk) const override;

  void get_indices(Position r, int* ijk, bool* in_mesh) const override;

  void get_indices_from_bin(int bin, int* ijk) const override;

  void to_hdf5(hid_t group) const override;

  // New methods

  //! Check where a line segment intersects the mesh and if it intersects at all
  //
  //! \param[in,out] r0 In: starting position, out: intersection point
  //! \param[in] r1 Ending position
  //! \param[out] ijk Indices of the mesh bin containing the intersection point
  //! \return Whether the line segment connecting r0 and r1 intersects mesh
  bool intersects(Position& r0, Position r1, int* ijk) const;

  // Data members

  xt::xarray<int> shape_; //!< Number of mesh elements in each dimension

private:
  std::vector<std::vector<double>> grid_;
};

//==============================================================================
// Non-member functions
//==============================================================================

//! Read meshes from either settings/tallies
//
//! \param[in] root XML node
void read_meshes(pugi::xml_node root);

//! Write mesh data to an HDF5 group
//
//! \param[in] group HDF5 group
void meshes_to_hdf5(hid_t group);

void free_memory_mesh();

} // namespace openmc

#endif // OPENMC_MESH_H
