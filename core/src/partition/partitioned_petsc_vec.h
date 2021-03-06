#pragma once

#include <memory>

#include "control/types.h"
#include "partition/rank_subset.h"
#include "function_space/function_space.h"
#include "mesh/type_traits.h"
#include "partition/partitioned_petsc_vec_base.h"

// forward declaration
namespace FunctionSpace
{
template<typename MeshType, typename BasisFunctionType>
class FunctionSpace;
};

/** This encapsulates a Petsc Vec, combined with the partition of the mesh.
 *  For each component a local Vec is stored that holds all values of that component.
 *  Global Petsc numbering: such that each rank has its own contiguous subset in this numbering of the total range.
 *  Global Natural numbering: normal indexing proceeding fastest in x, then in y, then in z direction, over the whole domain.
 *  Local numbering: starting with 0, first all non-ghost values, then the ghost indices.
 * *
 *  This particular standard specialization is for non-structured meshes or no meshes and currently completely serial, 
 *  it is the placeholder as long as the partial specialization for unstructured meshes is not implemented. (It will never be)
 *  This means some of the methods here have no effect.
 */
template<typename FunctionSpaceType, int nComponents, typename = typename FunctionSpaceType::Mesh>
class PartitionedPetscVec : 
  public PartitionedPetscVecBase<FunctionSpaceType>
{
public:
  //! constructor
  PartitionedPetscVec(std::shared_ptr<Partition::MeshPartition<FunctionSpaceType,typename FunctionSpaceType::Mesh>> meshPartition, std::string name);
 
  //! constructor, copy from existing vector
  PartitionedPetscVec(PartitionedPetscVec<FunctionSpaceType,nComponents> &rhs, std::string name);
  
  //! constructor, copy from existing vector
  template<int nComponents2>
  PartitionedPetscVec(PartitionedPetscVec<FunctionSpaceType,nComponents2> &rhs, std::string name);
  
  //! this has to be called before the vector is manipulated (i.e. VecSetValues or vecZeroEntries is called)
  void startGhostManipulation();
  
  //! this has to be called after the vector is manipulated (i.e. VecSetValues or vecZeroEntries is called)
  void finishGhostManipulation();
  
  //! zero all values in the local ghost buffer. Needed if between startGhostManipulation() and finishGhostManipulation() only some ghost will be reassigned. To prevent that the "old" ghost values that were present in the local ghost values buffer get again added to the real values which actually did not change.
  void zeroGhostBuffer();

  //! wrapper to the PETSc VecSetValues, acting only on the local data, the indices ix are the local dof nos
  void setValues(int componentNo, PetscInt ni, const PetscInt ix[], const PetscScalar y[], InsertMode iora);
  
  //! wrapper to the PETSc VecSetValue, acting only on the local data
  void setValue(int componentNo, PetscInt row, PetscScalar value, InsertMode mode);
  
  //! set values from another vector
  void setValues(PartitionedPetscVec<FunctionSpaceType,nComponents> &rhs);

  //! wrapper to the PETSc VecGetValues, acting only on the local data, the indices ix are the local dof nos
  void getValues(int componentNo, PetscInt ni, const PetscInt ix[], PetscScalar y[]);
  
  //! wrapper to the PETSc VecGetValues, on the global vector with global/Petsc indexing. This is not the global natural numbering!
  void getValuesGlobalPetscIndexing(int componentNo, PetscInt ni, const PetscInt ix[], PetscScalar y[]);
  
  //! set all entries to zero, wraps VecZeroEntries
  void zeroEntries();
  
  //! get the internal PETSc vector values, the local vector for the specified component
  Vec &valuesLocal(int componentNo = 0);

  //! get the internal PETSc vector values, the global vector for the specified component
  Vec &valuesGlobal(int componentNo = 0);

  //! fill a contiguous vector with all components after each other, "struct of array"-type data layout.
  //! after manipulation of the vector has finished one has to call restoreContiguousValuesGlobal
  Vec &getContiguousValuesGlobal();

  //! copy the values back from a contiguous representation where all components are in one vector to the standard internal format of PartitionedPetscVec where there is one local vector with ghosts for each component.
  //! this has to be called
  void restoreContiguousValuesGlobal();

  //! output the vector to stream, for debugging
  void output(std::ostream &stream);

protected:
  
  //! create the values vectors
  void createVector();
  
  std::array<Vec,nComponents> values_;  ///< the (serial) Petsc vectors that contains all the data, one for each component
  Vec valuesContiguous_ = PETSC_NULL;   ///< global vector that has all values of the components concatenated, i.e. in a "struct of arrays" memory layout
  bool valuesContiguousInUse_ = false;  ///< if the current representation of the vector is in valuesContiguous, not in the values vector
};

/** This is the partial specialization for structured meshes.
 *  An own DMDA object is generated, separately from the one in MeshPartition. This object now refers to nodes (as opposite the one of MeshPartition which refers to elements). 
 *  This object is created such that it matches the partition given by the meshPartition.
 */
template<typename MeshType, typename BasisFunctionType, int nComponents>
class PartitionedPetscVec<
  FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,
  nComponents,
  Mesh::isStructured<MeshType>> : 
  public PartitionedPetscVecBase<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>>
{
public:
 
  //! constructor, construct a petsc Vec with meshPartition that can hold the values for a field variable with nComponents components
  PartitionedPetscVec(std::shared_ptr<Partition::MeshPartition<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,MeshType>> meshPartition, std::string name);
 
  //! constructor, copy from existing vector
  PartitionedPetscVec(PartitionedPetscVec<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,nComponents> &rhs, std::string name);
 
  //! constructor, copy from existing vector
  template<int nComponents2>
  PartitionedPetscVec(PartitionedPetscVec<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,nComponents2> &rhs, std::string name);
 
  //! this has to be called before the vector is manipulated (i.e. VecSetValues or vecZeroEntries is called)
  void startGhostManipulation();
  
  //! this has to be called after the vector is manipulated (i.e. VecSetValues or vecZeroEntries is called)
  void finishGhostManipulation();
  
  //! zero all values in the local ghost buffer. Needed if between startGhostManipulation() and finishGhostManipulation() only some ghost will be reassigned. To prevent that the "old" ghost values that were present in the local ghost values buffer get again added to the real values which actually did not change.
  void zeroGhostBuffer();

  //! wrapper to the PETSc VecSetValues, acting only on the local data, the indices ix are the local dof nos
  void setValues(int componentNo, PetscInt ni, const PetscInt ix[], const PetscScalar y[], InsertMode iora);
  
  //! wrapper to the PETSc VecSetValue, acting only on the local data
  void setValue(int componentNo, PetscInt row, PetscScalar value, InsertMode mode);

  //! for a single component vector set all values. They have to be enough for all local dof including ghosts.
  void setValuesWithGhosts(int componentNo, std::vector<double> &values, InsertMode petscInsertMode);
  
  //! for a single component vector set all values. values does not contain ghost dofs.
  void setValuesWithoutGhosts(int componentNo, std::vector<double> &values, InsertMode petscInsertMode);

  //! set values from another vector
  void setValues(PartitionedPetscVec<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,nComponents> &rhs);

  //! set values of a specific component from another vector, this is the opposite operation to extractComponent
  void setValues(int componentNo, std::shared_ptr<PartitionedPetscVec<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,1>> fieldVariable);

  //! extract a single component
  void extractComponent(int componentNo, std::shared_ptr<PartitionedPetscVec<FunctionSpace::FunctionSpace<MeshType,BasisFunctionType>,1>> extractedFieldVariable);

  //! wrapper to the PETSc VecGetValues, acting only on the local data, the indices ix are the local dof nos
  void getValues(int componentNo, PetscInt ni, const PetscInt ix[], PetscScalar y[]);
  
  //! wrapper to the PETSc VecGetValues, on the global vector with global/Petsc indexing. This is not the global natural numbering!
  void getValuesGlobalPetscIndexing(int componentNo, PetscInt ni, const PetscInt ix[], PetscScalar y[]);
  
  //! get all locally stored values, i.e. with ghosts
  void getLocalValues(int componentNo, std::vector<double> &values);
  
  //! set all entries to zero, wraps VecZeroEntries
  void zeroEntries();
  
  //! get the local Vector of a specified component
  Vec &valuesLocal(int componentNo = 0);
  
  //! get the global Vector of a specified component
  Vec &valuesGlobal(int componentNo = 0);
  
  //! fill a contiguous vector with all components after each other, "struct of array"-type data layout.
  //! after manipulation of the vector has finished one has to call restoreContiguousValuesGlobal
  Vec &getContiguousValuesGlobal();

  //! copy the values back from a contiguous representation where all components are in one vector to the standard internal format of PartitionedPetscVec where there is one local vector with ghosts for each component.
  //! this has to be called
  void restoreContiguousValuesGlobal();

  //! get a vector of local dof nos (from meshPartition), without ghost dofs
  std::vector<PetscInt> &localDofNosWithoutGhosts();
  
  //! output the vector to stream, for debugging
  void output(std::ostream &stream);
  
protected:
 
  //! create a distributed Petsc vector, according to partition
  void createVector();
  
  std::shared_ptr<DM> dm_;    ///< PETSc DMDA object that stores topology information and everything needed for communication of ghost values
  bool ghostManipulationStarted_;   ///< if startGhostManipulation() was called but not yet finishGhostManipulation(). This indicates that finishGhostManipulation() can be called next without giving an error.
  
  std::array<Vec,nComponents> vectorLocal_;   ///< local vector that holds the local Vecs, is filled by startGhostManipulation and can the be manipulated, afterwards the results need to get copied back by finishGhostManipulation
  std::array<Vec,nComponents> vectorGlobal_;  ///< the global distributed vector that holds the actual data
  Vec valuesContiguous_ = PETSC_NULL;   ///< global vector that has all values of the components concatenated, i.e. in a "struct of arrays" memory layout
  bool valuesContiguousInUse_ = false;  ///< if the current representation of the vector is in valuesContiguous, not in the values vector
};


template<typename FunctionSpaceType, int nComponents>
std::ostream &operator<<(std::ostream &stream, PartitionedPetscVec<FunctionSpaceType,nComponents> &vector);

#include "partition/partitioned_petsc_vec_default.tpp"
#include "partition/partitioned_petsc_vec_structured.tpp"
