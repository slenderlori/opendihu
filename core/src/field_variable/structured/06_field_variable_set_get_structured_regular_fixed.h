#pragma once

#include <Python.h>  // has to be the first included header

#include "field_variable/structured/05_field_variable_data_structured_regular_fixed.h"
#include "field_variable/field_variable_set_get.h"
#include "function_space/function_space.h"

namespace FieldVariable
{

/** FieldVariable class for RegularFixed mesh
 */
template<typename FunctionSpaceType, int nComponents>
class FieldVariableSetGetRegularFixed :
  public FieldVariableData<FunctionSpaceType,nComponents>
{
public:

  //! inherited constructor
  using FieldVariableData<FunctionSpaceType,nComponents>::FieldVariableData;

  using FieldVariableData<FunctionSpaceType,nComponents>::getValues;
  using FieldVariableData<FunctionSpaceType,nComponents>::getElementValues;
  using FieldVariableData<FunctionSpaceType,nComponents>::getValue;
  using FieldVariableData<FunctionSpaceType,nComponents>::getValuesWithGhosts;
  using FieldVariableData<FunctionSpaceType,nComponents>::getValuesWithoutGhosts;
  using FieldVariableData<FunctionSpaceType,nComponents>::setValue;
  using FieldVariableData<FunctionSpaceType,nComponents>::setValues;

  //! for a specific component, get all values
  void getValuesWithGhosts(int componentNo, std::vector<double> &values, bool onlyNodalValues=false) const;

  //! for a specific component, get all values
  void getValuesWithoutGhosts(int componentNo, std::vector<double> &values, bool onlyNodalValues=false) const;

  //! for a specific component, get values from their local dof no.s
  template<int N>
  void getValues(int componentNo, std::array<dof_no_t,N> dofLocalNo, std::array<double,N> &values) const;

  //! for a specific component, get values from their local dof no.s, as vector
  void getValues(int componentNo, std::vector<dof_no_t> dofLocalNo, std::vector<double> &values) const;

  //! get values from their local dof no.s for all components
  template<int N>
  void getValues(std::array<dof_no_t,N> dofLocalNo, std::array<std::array<double,nComponents>,N> &values) const;

  //! for a specific component, get the values corresponding to all element-local dofs
  void getElementValues(int componentNo, element_no_t elementNo, std::array<double,FunctionSpaceType::nDofsPerElement()> &values) const;

  //! get the values corresponding to all element-local dofs for all components
  void getElementValues(element_no_t elementNo, std::array<std::array<double,nComponents>,FunctionSpaceType::nDofsPerElement()> &values) const;

  //! for a specific component, get a single value from local dof no.
  double getValue(int componentNo, node_no_t dofLocalNo) const;

  //! copy the values from another field variable of the same type
  void setValues(FieldVariable<FunctionSpaceType,nComponents> &rhs);
};

};  // namespace

#include "field_variable/structured/06_field_variable_set_get_structured_regular_fixed.tpp"
