#pragma once

#include <Python.h>  // has to be the first included header

#include "field_variable/structured/02_field_variable_set_get_structured.h"
#include "field_variable/field_variable_data.h"
#include "basis_on_mesh/05_basis_on_mesh.h"

namespace FieldVariable
{

/** FieldVariable class for RegularFixed mesh
 */
template<int D, typename BasisFunctionType>
class FieldVariableData<BasisOnMesh::BasisOnMesh<Mesh::StructuredRegularFixedOfDimension<D>,BasisFunctionType>> :
  public FieldVariableSetGetStructured<BasisOnMesh::BasisOnMesh<Mesh::StructuredRegularFixedOfDimension<D>,BasisFunctionType>>
{
public:
  typedef BasisOnMesh::BasisOnMesh<Mesh::StructuredRegularFixedOfDimension<D>,BasisFunctionType> BasisOnMeshType;
 
  //! inherited constructor 
  using FieldVariableSetGetStructured<BasisOnMesh::BasisOnMesh<Mesh::StructuredRegularFixedOfDimension<D>,BasisFunctionType>>::FieldVariableSetGetStructured;
 
  //! set the meshWidth
  void setMeshWidth(double meshWidth);
  
  //! get the mesh width
  double meshWidth() const;

  //! write a exelem file header to a stream, for a particular element
  void outputHeaderExelem(std::ostream &file, element_no_t currentElementGlobalNo, int fieldVariableNo=-1);

  //! write a exelem file header to a stream, for a particular element
  void outputHeaderExnode(std::ostream &file, node_no_t currentNodeGlobalNo, int &valueIndex, int fieldVariableNo=-1);

  //! tell if 2 elements have the same exfile representation, i.e. same number of versions
  bool haveSameExfileRepresentation(element_no_t element1, element_no_t element2);

  //! get the internal PETSc vector values. The meaning of the values is instance-dependent (different for different BasisOnMeshTypes)
  Vec &values();
  
  //! get the number of components
  int nComponents() const;
  
  //! get the number of elements in the coordinate directions
  //std::array<element_no_t, BasisOnMeshType::Mesh::dim()> nElementsPerCoordinateDirection() const;
  
  //! get the total number of elements
  element_no_t nElements() const;
  
  //! get the names of the components that are part of this field variable
  std::vector<std::string> componentNames() const;
  
protected:
  double meshWidth_;   ///< the uniform mesh width
};

};  // namespace

#include "field_variable/structured/03_field_variable_data_structured_regular_fixed.tpp"