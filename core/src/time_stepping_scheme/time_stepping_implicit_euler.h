#pragma once

#include <Python.h>  // has to be the first included header
//#include "time_stepping_scheme/time_stepping_scheme_ode.h"
#include "time_stepping_scheme/time_stepping_implicit.h"
//#include "control/runnable.h"
//#include "data_management/time_stepping.h"
#include "control/dihu_context.h"

namespace TimeSteppingScheme
{

/** The implicit Euler integration scheme (backward Euler), u_{t+1} = u_{t} + dt*f(t+1)
  */
template<typename DiscretizableInTimeType>
class ImplicitEuler :
  public TimeSteppingImplicit<DiscretizableInTimeType>
{
public:

  //! constructor
  ImplicitEuler(DihuContext context);

  //! advance simulation by the given time span [startTime_, endTime_] with given numberTimeSteps, data in solution is used, afterwards new data is in solution
  //void advanceTimeSpan();
  
  //! run the simulation
  //void run();
  
  //! get the system matrix, corresponding to the specific time integration. (I - d*tM^(-1)*K) for the implicit Euler scheme.
  //std::shared_ptr<PartitionedPetscMat<FunctionSpace>> systemMatrix();
   
protected:
  
  //! precomputes the integration matrix for example A=I-dtM^(-1)K for the implicit euler scheme
  void setSystemMatrix(double timeStepWidth);
  
  //! initialize the linear solve that is needed for the solution of the implicit timestepping system
  //void initializeLinearSolver();
  
  //! solves the linear system of equations resulting from the Implicit Euler method time discretization
  //void solveLinearSystem(Vec &input, Vec &output);
  
  //std::shared_ptr<PartitionedPetscMat<FunctionSpace>> systemMatrix_;     ///< the system matrix for implicit time stepping, (I - dt*M^-1*K)
  //std::shared_ptr<Solver::Linear> linearSolver_;   ///< the linear solver used for solving the system
  //std::shared_ptr<KSP> ksp_; 
  
};

}  // namespace

#include "time_stepping_scheme/time_stepping_implicit_euler.tpp"