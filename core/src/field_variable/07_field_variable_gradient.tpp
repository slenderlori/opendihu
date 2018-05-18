#include "field_variable/field_variable.h"

namespace FieldVariable
{
  
//! get values from their global dof no.s for all components, this eventually does not get all values if there are multiple versions
template<typename BasisOnMeshType>
void FieldVariable<BasisOnMeshType,1>::
computeGradientField(FieldVariable<BasisOnMeshType, BasisOnMeshType::dim()> &gradientField)
{
  // initialize gradient field variable to 0
  gradientField.setValues(0.0);
  gradientField.flushSetValues();
   
  // define constants
  const int nDofsPerElement = BasisOnMeshType::nDofsPerElement();
  const int D = BasisOnMeshType::dim();
  
  const dof_no_t nDofs = this->mesh_->nDofs();
  std::vector<int> nSummands(nDofs,0.0);   ///< the number of elements that are adjacent to the node
  
  // loop over elements
  for (element_no_t elementNo = 0; elementNo < this->mesh_->nElements(); elementNo++)
  {
    // get global dof nos of this element
    std::array<dof_no_t,nDofsPerElement> elementDofs = this->mesh_->getElementDofNos(elementNo);
  
    // compute gradient at every dof, as continuous to current element (gradients have discontinuities between elements at dofs)
    std::array<double,nDofsPerElement> solutionValues;
    this->getElementValues(elementNo, solutionValues);
  
    // get geometry field (which are the node positions for Lagrange basis and node positions and derivatives for Hermite)
    std::array<Vec3,nDofsPerElement> geometryValues;
    this->mesh_->getElementGeometry(elementNo, geometryValues);
    
    std::array<double,D> xi;
    
    // loop over dofs in element, where to compute the gradient
    for (int dofIndex = 0; dofIndex < nDofsPerElement; dofIndex++)
    {
      // set xi to dofIndex
      for (int i = 0; i < D; i++)
      {
        if (i == 0)
          xi[i] = double(dofIndex % 2);
        else if (i == 1)
          xi[i] = double((dofIndex % 4) / 2);
        else if (i == 2)
          xi[i] = double(dofIndex / 4);
      }
      
      VLOG(2) << "element " << elementNo << " dofIndex " << dofIndex << ", xi " << xi << " g:" << geometryValues;
      
      // compute the 3xD jacobian of the parameter space to world space mapping
      Tensor2<D> jacobianParameterSpace = MathUtility::transformToDxD<D,D>(BasisOnMeshType::computeJacobian(geometryValues, xi));
      double jacobianDeterminant;
      Tensor2<D> inverseJacobianParameterSpace = MathUtility::computeInverse<D>(jacobianParameterSpace, jacobianDeterminant);
      
      std::array<double,D> gradPhiWorldSpace{0.0};
        
      // loop over dofs in element that contribute to the gradient at dofIndex
      for (int dofIndex2 = 0; dofIndex2 < nDofsPerElement; dofIndex2++)
      {
        // get gradient at dof
        std::array<double,D> gradPhiParameterSpace = this->mesh_->gradPhi(dofIndex2, xi);

        VLOG(2) << "  dofIndex2=" << dofIndex2 << ", xi=" << xi << ", gradPhiParameterSpace: " << gradPhiParameterSpace;
        
        
        std::array<double,D> gradPhiWorldSpaceDofIndex2{0.0};
        
        // transform grad from parameter space to world space
        for (int direction = 0; direction < D; direction++)
        {
          VLOG(2) << "   component " << direction;
          for (int k = 0; k < D; k++)
          {
            // jacobianParameterSpace[columnIdx][rowIdx] = dX_rowIdx/dxi_columnIdx
            // inverseJacobianParameterSpace[columnIdx][rowIdx] = dxi_rowIdx/dX_columnIdx because of inverse function theorem
           
            const double dphiDofIndex2_dxik = gradPhiParameterSpace[k];   // dphi_dofIndex/dxi_k
            const double dxik_dXdirection = inverseJacobianParameterSpace[direction][k];  // dxi_k/dX_direction
            
            
            VLOG(2) << "     += " << dphiDofIndex2_dxik << " * " << dxik_dXdirection;
            
            gradPhiWorldSpaceDofIndex2[direction] += dphiDofIndex2_dxik * dxik_dXdirection;  
          }
        }
        
        VLOG(2) << "  gradPhiWorldSpaceDofIndex2: " << gradPhiWorldSpaceDofIndex2 << " multiply with solution value at dof " << dofIndex2 << ", " << solutionValues[dofIndex2];
        
        VLOG(2) << " sum contributions from the other ansatz functions at this dof: " << gradPhiWorldSpace; 
        
        gradPhiWorldSpace += gradPhiWorldSpaceDofIndex2 * solutionValues[dofIndex2];
       
        VLOG(2) << "                                                             -> " << gradPhiWorldSpace;
      }  // dofIndex2
    
      dof_no_t dofNo = elementDofs[dofIndex];
      
      VLOG(2) << "   global dof " << dofNo << ", add value " << gradPhiWorldSpace;
      
      // add value to gradient field variable
      gradientField.setValue(dofNo, gradPhiWorldSpace, ADD_VALUES);
      
      // increase counter of number of summands for that dof
      nSummands[dofNo]++;
      
    }  // dofIndex
  }  // elementNo
  
  gradientField.flushSetValues();
  
  // divide by number of summands
  for (dof_no_t localDofNo = 0; localDofNo < nDofs; localDofNo++)
  {
    VecD<D> value = gradientField.getValue(localDofNo);
    
    VLOG(2) << "dof " << localDofNo << " value: " << value << ", nSummands: " << nSummands[localDofNo];
   
    
    value /= nSummands[localDofNo];
    
    gradientField.setValue(localDofNo, value, INSERT_VALUES);
  }
  
  gradientField.flushSetValues();
}

};  // namespace