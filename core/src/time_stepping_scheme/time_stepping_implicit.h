#pragma once

#include <Python.h>  // has to be the first included header
#include "time_stepping_scheme/time_stepping_scheme_ode.h"
#include "control/runnable.h"
#include "data_management/time_stepping_implicit.h"
#include "control/dihu_context.h"

namespace TimeSteppingScheme
{

/** The implicit time integration scheme
  */
template<typename DiscretizableInTimeType>
class TimeSteppingImplicit :
  public TimeSteppingSchemeOde<DiscretizableInTimeType>
{
public:
  
  typedef typename DiscretizableInTimeType::FunctionSpace FunctionSpace;

  //! constructor
  TimeSteppingImplicit(DihuContext context, const std::string name);

  //! advance simulation by the given time span [startTime_, endTime_] with given numberTimeSteps, data in solution is used, afterwards new data is in solution
  virtual void advanceTimeSpan() = 0;
  
  //! set the system matrix
  virtual void initialize();
   
protected:
  
  //! precomputes the integration matrix for example A = (I-dtM^(-1)K) for the implicit euler scheme
  virtual void setSystemMatrix(double timeStepWidth) = 0;
   
  //! initialize the linear solve that is needed for the solution of the implicit timestepping system
  void initializeLinearSolver();
  
  //! solves the linear system of equations resulting from the Implicit Euler method time discretization
  void solveLinearSystem(Vec &input, Vec &output);
  
  std::shared_ptr<Data::TimeSteppingImplicit<typename DiscretizableInTimeType::FunctionSpace, DiscretizableInTimeType::nComponents()>> dataImplicit_;  ///< a pointer to the data_ object but of type Data::TimeSteppingImplicit
  std::shared_ptr<Solver::Linear> linearSolver_;   ///< the linear solver used for solving the system
  std::shared_ptr<KSP> ksp_;     ///< the ksp object of the linear solver

};

}  // namespace

#include "time_stepping_scheme/time_stepping_implicit.tpp"
